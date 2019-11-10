#include "tcpflow_loader.hh"

tcpflow_loader::tcpflow_loader(const std::string &path, const std::vector<std::string> &client_ips)
        : path(path), client_ips(client_ips), file_listing(path, client_ips) {}

void tcpflow_loader::load() {
    file_infos = file_listing.list_files();

    for (auto &file_info : file_infos) {
        parsed_streams[file_info] = std::make_unique<tcpflow_stream_parser>(file_info);
        parsed_streams[file_info]->parse_stream();
    }

    postprocess_streams();

    merge_into_all();
}

void tcpflow_loader::merge_into_all() {
    for (auto &parsed_stream : parsed_streams) {
        all_kafka_messages.insert(all_kafka_messages.end(), parsed_stream.second->kafka_messages.begin(),
                                  parsed_stream.second->kafka_messages.end());
        all_invalid_messages.insert(all_invalid_messages.end(), parsed_stream.second->invalid_messages.begin(),
                                    parsed_stream.second->invalid_messages.end());
        all_stream_started_messages.insert(all_stream_started_messages.end(), parsed_stream.second->stream_started_messages.begin(),
                                    parsed_stream.second->stream_started_messages.end());
        all_stream_ended_messages.insert(all_stream_ended_messages.end(), parsed_stream.second->stream_ended_messages.begin(),
                                           parsed_stream.second->stream_ended_messages.end());
    }

    std::stable_sort(all_kafka_messages.begin(), all_kafka_messages.end(),
                     [](const std::shared_ptr<tcpflow_kafka_message> &lhs,
                        const std::shared_ptr<tcpflow_kafka_message> &rhs) {
                         return lhs->timestamp < rhs->timestamp;
                     });

    std::stable_sort(all_invalid_messages.begin(), all_invalid_messages.end(),
                     [](const std::shared_ptr<tcpflow_invalid_message> &lhs,
                        const std::shared_ptr<tcpflow_invalid_message> &rhs) {
                         return lhs->timestamp < rhs->timestamp;
                     });

    std::stable_sort(all_stream_started_messages.begin(), all_stream_started_messages.end(),
                     [](const std::shared_ptr<tcpflow_stream_started_message> &lhs,
                        const std::shared_ptr<tcpflow_stream_started_message> &rhs) {
                         return lhs->timestamp < rhs->timestamp;
                     });

    std::stable_sort(all_stream_ended_messages.begin(), all_stream_ended_messages.end(),
                     [](const std::shared_ptr<tcpflow_stream_ended_message> &lhs,
                        const std::shared_ptr<tcpflow_stream_ended_message> &rhs) {
                         return lhs->timestamp < rhs->timestamp;
                     });
}

void tcpflow_loader::postprocess_streams() {
    // FIXME: O(N^2) complexity
    for (auto &client_stream : parsed_streams) {
        if (client_stream.first->source.type != CLIENT) continue;
        for (auto &kafka_broker_stream : parsed_streams) {
            if (kafka_broker_stream.first->source.type != KAFKA_BROKER) continue;
            if (client_stream.first->source != kafka_broker_stream.first->dest) continue;
            if (client_stream.first->dest != kafka_broker_stream.first->source) continue;
            if (client_stream.first->connection_number != kafka_broker_stream.first->connection_number) continue;

            postprocess_match_correlations(client_stream.second, kafka_broker_stream.second);
        }
    }
}

void tcpflow_loader::postprocess_match_correlations(std::unique_ptr<tcpflow_stream_parser> &client,
                                                    std::unique_ptr<tcpflow_stream_parser> &kafka_broker) {
    std::vector<std::shared_ptr<tcpflow_kafka_message>> merged_messages;
    merged_messages.insert(merged_messages.end(), client->kafka_messages.begin(), client->kafka_messages.end());
    merged_messages.insert(merged_messages.end(), kafka_broker->kafka_messages.begin(),
                           kafka_broker->kafka_messages.end());
    std::stable_sort(merged_messages.begin(), merged_messages.end(),
                     [](const std::shared_ptr<tcpflow_kafka_message> &lhs,
                        const std::shared_ptr<tcpflow_kafka_message> &rhs) {
                         return lhs->timestamp < rhs->timestamp;
                     });

    std::map<int32_t, std::shared_ptr<tcpflow_kafka_message>> correlation_to_client;
    for (auto &message : merged_messages) {
        if (message->stream->source.type == CLIENT) {
            correlation_to_client[message->correlation_id] = message;
        } else {
            auto client_correlation = correlation_to_client.find(message->correlation_id);
            if (client_correlation != correlation_to_client.end()) {
                message->add_correlated(client_correlation->second);
            }
        }
    }
}

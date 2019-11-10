#include "two_column_report_generator.hh"

two_column_report_generator::two_column_report_generator()
        : two_column_report_generator_html("reports/two_column/two_column_report.html"),
          client_message_block_html("reports/two_column/client_message_block.html"),
          client_invalid_message_block_html("reports/two_column/client_invalid_message_block.html"),
          kafka_broker_message_block_html("reports/two_column/kafka_broker_message_block.html"),
          kafka_broker_invalid_message_block_html("reports/two_column/kafka_broker_invalid_message_block.html"),
          stream_started_block_html("reports/two_column/stream_started_block.html"),
          stream_ended_block_html("reports/two_column/stream_ended_block.html"),
          log_info_block_html("reports/two_column/log_info_block.html"),
          api_names_set(10),
          client_addr_set(10),
          kafka_broker_addr_set(10),
          connection_set(10) {}

void two_column_report_generator::save_report() {
    render_log_info_lines();
    render_stream_started_messages();
    render_kafka_messages();
    render_invalid_messages();
    render_stream_ended_messages();
    render_report();
    rendered_output.save(output_html_path);
}

void two_column_report_generator::render_kafka_messages() {
    for (auto &kafka_message : kafka_messages) {
        if (kafka_message->stream->source.type == CLIENT) {
            render_client_message(kafka_message);
        } else {
            render_kafka_broker_message(kafka_message);
        }
    }
}

void two_column_report_generator::render_client_message(std::shared_ptr<tcpflow_kafka_message> &kafka_message) {
    auto block_template = client_message_block_html.get();
    block_template.replace("$SOURCE_IP$", kafka_message->stream->source.ip_addr)
            .replace("$DEST_IP$", kafka_message->stream->dest.ip_addr)
            .replace("$SOURCE_PORT$", kafka_message->stream->source.port)
            .replace("$DEST_PORT$", kafka_message->stream->dest.port)
            .replace("$SOURCE_INDEX$", client_addr_set.get_index(kafka_message->get_client_addr_id()))
            .replace("$DEST_INDEX$", kafka_broker_addr_set.get_index(kafka_message->stream->dest.ip_addr))
            .replace("$SOURCE_COLOR$", client_addr_set.get_color(kafka_message->get_client_addr_id()))
            .replace("$DEST_COLOR$", kafka_broker_addr_set.get_color(kafka_message->stream->dest.ip_addr))
            .replace("$CONNECTION_INDEX$", connection_set.get_index(kafka_message->stream->get_connection_id()))
            .replace("$CONNECTION_COLOR$", connection_set.get_color(kafka_message->stream->get_connection_id()))
            .replace("$API_KEY_NAME$", report_util::name_for_api_key(kafka_message->api_key))
            .replace("$API_KEY_COLOR$", api_names_set.get_color(report_util::name_for_api_key(kafka_message->api_key)))
            .replace("$API_VERSION$", kafka_message->api_version)
            .replace("$PAYLOAD_SIZE$", kafka_message->payload_size)
            .replace("$PAYLOAD_HEXDUMP$", report_util::hexdump(kafka_message->payload))
            .replace("$PAYLOAD_ASCII$", report_util::asciidump(kafka_message->payload))
            .replace("$TIME$", report_util::epoch_to_string(kafka_message->timestamp))
            .replace("$CORRELATION_ID$", kafka_message->correlation_id)
            .replace("$CLIENT_ID$", kafka_message->client_id);
    rendered_blocks.emplace_back(block_template.contents, kafka_message->timestamp);
}

void two_column_report_generator::render_kafka_broker_message(std::shared_ptr<tcpflow_kafka_message> &kafka_message) {
    auto block_template = kafka_broker_message_block_html.get();
    block_template.replace("$SOURCE_IP$", kafka_message->stream->source.ip_addr)
            .replace("$DEST_IP$", kafka_message->stream->dest.ip_addr)
            .replace("$SOURCE_PORT$", kafka_message->stream->source.port)
            .replace("$DEST_PORT$", kafka_message->stream->dest.port)
            .replace("$SOURCE_INDEX$", kafka_broker_addr_set.get_index(kafka_message->stream->source.ip_addr))
            .replace("$DEST_INDEX$", client_addr_set.get_index(kafka_message->get_client_addr_id()))
            .replace("$SOURCE_COLOR$", kafka_broker_addr_set.get_color(kafka_message->stream->source.ip_addr))
            .replace("$DEST_COLOR$", client_addr_set.get_color(kafka_message->get_client_addr_id()))
            .replace("$CONNECTION_INDEX$", connection_set.get_index(kafka_message->stream->get_connection_id()))
            .replace("$CONNECTION_COLOR$", connection_set.get_color(kafka_message->stream->get_connection_id()))
            .replace("$API_KEY_NAME$", report_util::name_for_api_key(kafka_message->api_key))
            .replace("$API_KEY_COLOR$", api_names_set.get_color(report_util::name_for_api_key(kafka_message->api_key)))
            .replace("$API_VERSION$", kafka_message->api_version)
            .replace("$PAYLOAD_SIZE$", kafka_message->payload_size)
            .replace("$PAYLOAD_HEXDUMP$", report_util::hexdump(kafka_message->payload))
            .replace("$PAYLOAD_ASCII$", report_util::asciidump(kafka_message->payload))
            .replace("$TIME$", report_util::epoch_to_string(kafka_message->timestamp))
            .replace("$CORRELATION_ID$", kafka_message->correlation_id)
            .replace("$CLIENT_ID$", kafka_message->client_id);
    rendered_blocks.emplace_back(block_template.contents, kafka_message->timestamp);
}

void two_column_report_generator::render_invalid_messages() {
    for (auto &invalid_message : invalid_messages) {
        if (invalid_message->stream->source.type == CLIENT) {
            render_client_invalid_message(invalid_message);
        } else {
            render_kafka_broker_invalid_message(invalid_message);
        }
    }
}

void
two_column_report_generator::render_client_invalid_message(std::shared_ptr<tcpflow_invalid_message> &invalid_message) {
    auto block_template = client_invalid_message_block_html.get();
    block_template.replace("$SOURCE_IP$", invalid_message->stream->source.ip_addr)
            .replace("$DEST_IP$", invalid_message->stream->dest.ip_addr)
            .replace("$SOURCE_PORT$", invalid_message->stream->source.port)
            .replace("$DEST_PORT$", invalid_message->stream->dest.port)
            .replace("$CONNECTION_INDEX$", connection_set.get_index(invalid_message->stream->get_connection_id()))
            .replace("$CONNECTION_COLOR$", connection_set.get_color(invalid_message->stream->get_connection_id()))
            .replace("$TIME$", report_util::epoch_to_string(invalid_message->timestamp));
    rendered_blocks.emplace_back(block_template.contents, invalid_message->timestamp);
}

void two_column_report_generator::render_kafka_broker_invalid_message(
        std::shared_ptr<tcpflow_invalid_message> &invalid_message) {
    auto block_template = kafka_broker_invalid_message_block_html.get();
    block_template.replace("$SOURCE_IP$", invalid_message->stream->source.ip_addr)
            .replace("$DEST_IP$", invalid_message->stream->dest.ip_addr)
            .replace("$SOURCE_PORT$", invalid_message->stream->source.port)
            .replace("$DEST_PORT$", invalid_message->stream->dest.port)
            .replace("$CONNECTION_INDEX$", connection_set.get_index(invalid_message->stream->get_connection_id()))
            .replace("$CONNECTION_COLOR$", connection_set.get_color(invalid_message->stream->get_connection_id()))
            .replace("$TIME$", report_util::epoch_to_string(invalid_message->timestamp));
    rendered_blocks.emplace_back(block_template.contents, invalid_message->timestamp);
}

void two_column_report_generator::render_stream_started_messages() {
    for (auto &stream_started_message : stream_started_messages) {
        if (stream_started_message->stream->source.type != CLIENT) continue;

        auto block_template = stream_started_block_html.get();
        block_template.replace("$SOURCE_IP$", stream_started_message->stream->source.ip_addr)
                .replace("$DEST_IP$", stream_started_message->stream->dest.ip_addr)
                .replace("$SOURCE_PORT$", stream_started_message->stream->source.port)
                .replace("$DEST_PORT$", stream_started_message->stream->dest.port)
                .replace("$CONNECTION_INDEX$",
                         connection_set.get_index(stream_started_message->stream->get_connection_id()))
                .replace("$CONNECTION_COLOR$",
                         connection_set.get_color(stream_started_message->stream->get_connection_id()))
                .replace("$TIME$", report_util::epoch_to_string(stream_started_message->timestamp));
        rendered_blocks.emplace_back(block_template.contents, stream_started_message->timestamp);
    }
}

void two_column_report_generator::render_stream_ended_messages() {
    for (auto &stream_ended_message : stream_ended_messages) {
        if (stream_ended_message->stream->source.type != KAFKA_BROKER) continue;

        auto block_template = stream_ended_block_html.get();
        block_template.replace("$SOURCE_IP$", stream_ended_message->stream->dest.ip_addr)
                .replace("$DEST_IP$", stream_ended_message->stream->source.ip_addr)
                .replace("$SOURCE_PORT$", stream_ended_message->stream->dest.port)
                .replace("$DEST_PORT$", stream_ended_message->stream->source.port)
                .replace("$CONNECTION_INDEX$",
                         connection_set.get_index(stream_ended_message->stream->get_connection_id()))
                .replace("$CONNECTION_COLOR$",
                         connection_set.get_color(stream_ended_message->stream->get_connection_id()))
                .replace("$TIME$", report_util::epoch_to_string(stream_ended_message->timestamp));
        rendered_blocks.emplace_back(block_template.contents, stream_ended_message->timestamp);
    }
}

void two_column_report_generator::render_log_info_lines() {
    for (auto &log_info_line : log_info_lines) {
        auto block_template = log_info_block_html.get();
        block_template.replace("$LOG_LINE$", log_info_line->log_line)
                .replace("$TIME$", report_util::epoch_to_string(log_info_line->timestamp));
        rendered_blocks.emplace_back(block_template.contents, log_info_line->timestamp);
    }
}

void two_column_report_generator::render_report() {
    std::stable_sort(rendered_blocks.begin(), rendered_blocks.end());

    std::string rendered_blocks_html;
    for (auto &rendered_block : rendered_blocks) {
        rendered_blocks_html += rendered_block.html;
    }

    rendered_output = two_column_report_generator_html.get()
            .replace("$BLOCKS$", rendered_blocks_html);
}
#include <utility>
#include <fstream>
#include <iostream>
#include <arpa/inet.h>

#include "tcpflow_stream_parser.hh"

tcpflow_stream_parser::tcpflow_stream_parser(const std::shared_ptr<tcpflow_file_info>& parsed_stream) : parsed_stream(
        parsed_stream), findx_parser(parsed_stream) {}

void tcpflow_stream_parser::parse_stream() {
    std::ifstream input_stream(parsed_stream->path);

    if (parsed_stream->file_size > 0) {
        auto current_message = std::make_shared<tcpflow_stream_started_message>();
        current_message->stream = parsed_stream;
        current_message->position_in_stream = 0;
        current_message->timestamp = findx_parser.timestamp_for_byte(current_message->position_in_stream);
        stream_started_messages.push_back(current_message);
    }

    while (!input_stream.eof() && input_stream.tellg() != -1 && input_stream.tellg() < parsed_stream->file_size) {
        bool is_valid = true;
        auto current_message = std::make_shared<tcpflow_kafka_message>();

        current_message->stream = parsed_stream;
        current_message->position_in_stream = input_stream.tellg();
        current_message->timestamp = findx_parser.timestamp_for_byte(current_message->position_in_stream);

        input_stream.read(reinterpret_cast<char *>(&current_message->size), sizeof(current_message->size));
        current_message->size = ntohl(current_message->size);
        current_message->payload_size = current_message->size;

        if (current_message->size <= 0 || input_stream.gcount() != sizeof(current_message->size)) {
            is_valid = false;
        } else if (current_message->stream->source.type == CLIENT) {
            is_valid = parse_client_message(input_stream, current_message);
        } else if (current_message->stream->source.type == KAFKA_BROKER) {
            is_valid = parse_kafka_broker_message(input_stream, current_message);
        } else {
            throw std::runtime_error("Unsupported source type.");
        }

        if (is_valid) {
            kafka_messages.push_back(current_message);
        } else {
            auto invalid_message = std::make_shared<tcpflow_invalid_message>();
            invalid_message->position_in_stream = current_message->position_in_stream;
            invalid_message->timestamp = current_message->timestamp;
            invalid_message->stream = current_message->stream;
            invalid_messages.push_back(invalid_message);
            break;
        }
    }

    if (parsed_stream->file_size > 0) {
        auto current_message = std::make_shared<tcpflow_stream_ended_message>();
        current_message->stream = parsed_stream;
        current_message->position_in_stream = parsed_stream->file_size - 1;
        current_message->timestamp = findx_parser.timestamp_for_byte(current_message->position_in_stream);
        stream_ended_messages.push_back(current_message);
    }

    input_stream.close();
}

bool tcpflow_stream_parser::parse_client_message(std::ifstream &input_stream,
                                                 std::shared_ptr<tcpflow_kafka_message> &current_message) const {
    input_stream.read(reinterpret_cast<char *>(&current_message->api_key), sizeof(current_message->api_key));
    current_message->api_key = ntohs(current_message->api_key);
    current_message->payload_size -= sizeof(current_message->api_key);
    if (input_stream.gcount() != sizeof(current_message->api_key)) return false;

    input_stream.read(reinterpret_cast<char *>(&current_message->api_version),
                      sizeof(current_message->api_version));
    current_message->api_version = ntohs(current_message->api_version);
    current_message->payload_size -= sizeof(current_message->api_version);
    if (input_stream.gcount() != sizeof(current_message->api_version)) return false;

    input_stream.read(reinterpret_cast<char *>(&current_message->correlation_id),
                      sizeof(current_message->correlation_id));
    current_message->correlation_id = ntohl(current_message->correlation_id);
    current_message->payload_size -= sizeof(current_message->correlation_id);
    if (input_stream.gcount() != sizeof(current_message->correlation_id)) return false;

    int16_t client_id_len;
    input_stream.read(reinterpret_cast<char *>(&client_id_len),
                      sizeof(client_id_len));
    client_id_len = ntohs(client_id_len);
    current_message->payload_size -= sizeof(client_id_len);
    if (input_stream.gcount() != sizeof(client_id_len)) return false;

    if (client_id_len > 0) {
        current_message->client_id.resize(client_id_len, '?');
        input_stream.read(current_message->client_id.data(), client_id_len);
        current_message->payload_size -= client_id_len;
        if (input_stream.gcount() != client_id_len) return false;
    }

    if (current_message->payload_size < 0) return false;
    else {
        current_message->payload.resize(current_message->payload_size, '?');
        input_stream.read(current_message->payload.data(), current_message->payload_size);
        if (input_stream.gcount() != current_message->payload_size) return false;
    }

    return true;
}

bool tcpflow_stream_parser::parse_kafka_broker_message(std::ifstream &input_stream,
                                                       std::shared_ptr<tcpflow_kafka_message> &current_message) const {
    input_stream.read(reinterpret_cast<char *>(&current_message->correlation_id),
                      sizeof(current_message->correlation_id));
    current_message->correlation_id = ntohl(current_message->correlation_id);
    current_message->payload_size -= sizeof(current_message->correlation_id);
    if (input_stream.gcount() != sizeof(current_message->correlation_id)) return false;

    if (current_message->payload_size < 0) return false;
    else {
        current_message->payload.resize(current_message->payload_size, '?');
        input_stream.read(current_message->payload.data(), current_message->payload_size);
        if (input_stream.gcount() != current_message->payload_size) return false;
    }

    return true;
}

void tcpflow_kafka_message::add_correlated(std::shared_ptr<tcpflow_kafka_message> &client_request) {
    api_key = client_request->api_key;
    api_version = client_request->api_version;
    client_id = client_request->client_id;
}

std::ostream &operator<<(std::ostream &os, const tcpflow_kafka_message &message) {
    os << "stream: " << *message.stream << " size: " << message.size << " api_key: " << message.api_key
       << " api_version: " << message.api_version << " correlation_id: " << message.correlation_id << " client_id: "
       << message.client_id;
    return os;
}

std::string tcpflow_kafka_message::get_client_addr_id() const {
    if (stream->source.type == CLIENT) return stream->source.ip_addr + ";" + client_id;
    return stream->dest.ip_addr + ";" + client_id;
}
#pragma once

#include <ostream>
#include <memory>
#include "tcpflow_file_listing.hh"
#include "tcpflow_findx_parser.hh"

struct tcpflow_kafka_message {
    std::shared_ptr<tcpflow_file_info> stream;
    uint64_t position_in_stream;
    long double timestamp;

    int32_t size;
    int32_t payload_size;

    int16_t api_key;
    int16_t api_version;
    int32_t correlation_id;

    std::string client_id;

    std::vector<char> payload;

    void add_correlated(std::shared_ptr<tcpflow_kafka_message> &client_request);
    [[nodiscard]] std::string get_client_addr_id() const;

private:
    friend std::ostream &operator<<(std::ostream &os, const tcpflow_kafka_message &message);
};

struct tcpflow_invalid_message {
    std::shared_ptr<tcpflow_file_info> stream;
    uint64_t position_in_stream;
    long double timestamp;
};

struct tcpflow_stream_started_message {
    std::shared_ptr<tcpflow_file_info> stream;
    uint64_t position_in_stream;
    long double timestamp;
};
struct tcpflow_stream_ended_message {
    std::shared_ptr<tcpflow_file_info> stream;
    uint64_t position_in_stream;
    long double timestamp;
};

/*!
 * \brief Parses Kafka TCP stream from tcpflow output.
 */
class tcpflow_stream_parser {
    std::shared_ptr<tcpflow_file_info> parsed_stream;
    tcpflow_findx_parser findx_parser;

    bool parse_client_message(std::ifstream &input_stream, std::shared_ptr<tcpflow_kafka_message> &current_message) const;
    bool parse_kafka_broker_message(std::ifstream &input_stream, std::shared_ptr<tcpflow_kafka_message> &current_message) const;
public:
    std::vector<std::shared_ptr<tcpflow_kafka_message>> kafka_messages;
    std::vector<std::shared_ptr<tcpflow_invalid_message>> invalid_messages;
    std::vector<std::shared_ptr<tcpflow_stream_started_message>> stream_started_messages;
    std::vector<std::shared_ptr<tcpflow_stream_ended_message>> stream_ended_messages;

    tcpflow_stream_parser();
    explicit tcpflow_stream_parser(const std::shared_ptr<tcpflow_file_info>& parsed_stream);

    void parse_stream();
};

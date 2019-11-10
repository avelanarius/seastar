#pragma once

#include <vector>
#include <memory>
#include "../../tcpflow/tcpflow_stream_parser.hh"
#include "../templated_file.hh"
#include "../report_util.hh"
#include "../indexed_set.hh"
#include "../../log_info/log_info_loader.hh"

struct two_column_report_rendered_block {
    std::string html;
    long double timestamp;

    two_column_report_rendered_block(const std::string &html, long double timestamp)
            : html(html), timestamp(timestamp) {}

    bool operator<(const two_column_report_rendered_block &other) const {
        return timestamp < other.timestamp;
    }
};

class two_column_report_generator {
    templated_file two_column_report_generator_html;
    templated_file client_message_block_html;
    templated_file client_invalid_message_block_html;
    templated_file kafka_broker_message_block_html;
    templated_file kafka_broker_invalid_message_block_html;
    templated_file stream_started_block_html;
    templated_file stream_ended_block_html;
    templated_file log_info_block_html;

    templated_file_instance rendered_output;

    std::vector<two_column_report_rendered_block> rendered_blocks;

    indexed_set api_names_set;
    indexed_set client_addr_set;
    indexed_set kafka_broker_addr_set;
    indexed_set connection_set;

    void render_kafka_messages();

    void render_client_message(std::shared_ptr<tcpflow_kafka_message> &kafka_message);
    void render_kafka_broker_message(std::shared_ptr<tcpflow_kafka_message> &kafka_message);

    void render_invalid_messages();

    void render_client_invalid_message(std::shared_ptr<tcpflow_invalid_message> &invalid_message);
    void render_kafka_broker_invalid_message(std::shared_ptr<tcpflow_invalid_message> &invalid_message);

    void render_stream_started_messages();

    void render_report();

public:
    std::string output_html_path;
    std::vector<std::shared_ptr<tcpflow_kafka_message>> kafka_messages;
    std::vector<std::shared_ptr<tcpflow_invalid_message>> invalid_messages;
    std::vector<std::shared_ptr<tcpflow_stream_started_message>> stream_started_messages;
    std::vector<std::shared_ptr<tcpflow_stream_ended_message>> stream_ended_messages;
    std::vector<std::shared_ptr<log_info_line>> log_info_lines;

    two_column_report_generator();

    void save_report();

    void render_stream_ended_messages();

    void render_log_info_lines();
};

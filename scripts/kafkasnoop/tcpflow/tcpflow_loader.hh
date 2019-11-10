#pragma once

#include <string>
#include <map>
#include "tcpflow_file_listing.hh"
#include "tcpflow_stream_parser.hh"

/*!
 * \brief Loads all files from tcpflow directory and parses all.
 */
class tcpflow_loader {
    std::string path;
    std::vector<std::string> client_ips;

    tcpflow_file_listing file_listing;
    std::vector<std::shared_ptr<tcpflow_file_info>> file_infos;

    std::map<std::shared_ptr<tcpflow_file_info>, std::unique_ptr<tcpflow_stream_parser>> parsed_streams;

    void postprocess_streams();

    void postprocess_match_correlations(std::unique_ptr<tcpflow_stream_parser> &client, std::unique_ptr<tcpflow_stream_parser> &kafka_broker);

    void merge_into_all();
public:
    std::vector<std::shared_ptr<tcpflow_kafka_message>> all_kafka_messages;
    std::vector<std::shared_ptr<tcpflow_invalid_message>> all_invalid_messages;
    std::vector<std::shared_ptr<tcpflow_stream_started_message>> all_stream_started_messages;
    std::vector<std::shared_ptr<tcpflow_stream_ended_message>> all_stream_ended_messages;

    tcpflow_loader(const std::string &path, const std::vector<std::string> &client_ips);

    void load();
};

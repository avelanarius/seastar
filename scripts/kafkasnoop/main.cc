#include <iostream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include "tcpflow/tcpflow_loader.hh"
#include "reports/templated_file.hh"
#include "reports/two_column/two_column_report_generator.hh"
#include "log_info/log_info_loader.hh"

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " INPUT_PATH"
            << " CLIENT_IPS" << std::endl;
        return 1;
    }
    std::string input_path = argv[1];

    std::vector<std::string> client_ips;
    boost::split(client_ips, argv[2], boost::is_any_of(","));

    tcpflow_loader flow_loader(input_path + "/tcpflow", client_ips);
    flow_loader.load();

    log_info_loader info_loader(input_path + "/log_info.txt");
    info_loader.load();

    two_column_report_generator two_column_generator;
    two_column_generator.output_html_path = input_path + "/two_column.html";
    two_column_generator.invalid_messages = flow_loader.all_invalid_messages;
    two_column_generator.kafka_messages = flow_loader.all_kafka_messages;
    two_column_generator.stream_started_messages = flow_loader.all_stream_started_messages;
    two_column_generator.stream_ended_messages = flow_loader.all_stream_ended_messages;
    two_column_generator.log_info_lines = info_loader.log_info_lines;
    two_column_generator.save_report();
    return 0;
}
cmake_minimum_required(VERSION 3.15)
project(kafkasnoop)

set(CMAKE_CXX_STANDARD 17)

add_executable(kafkasnoop main.cc tcpflow/tcpflow_file_listing.cc tcpflow/tcpflow_file_listing.hh tcpflow/tcpflow_stream_parser.cc tcpflow/tcpflow_stream_parser.hh tcpflow/tcpflow_findx_parser.cc tcpflow/tcpflow_findx_parser.hh tcpflow/tcpflow_loader.cc tcpflow/tcpflow_loader.hh reports/templated_file.cc reports/templated_file.hh reports/two_column/two_column_report_generator.cc reports/two_column/two_column_report_generator.hh reports/report_util.cc reports/report_util.hh reports/indexed_set.cc reports/indexed_set.hh log_info/log_info_loader.cc log_info/log_info_loader.hh)

target_compile_options(kafkasnoop PRIVATE -Wall -Wextra -pedantic)
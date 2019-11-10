#include <utility>
#include <fstream>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/split.hpp>
#include <iostream>

#include "tcpflow_findx_parser.hh"

tcpflow_findx_parser::tcpflow_findx_parser(std::shared_ptr<tcpflow_file_info> parsedStream) : parsed_stream(std::move(
        parsedStream)) {
    load_entries();
}

void tcpflow_findx_parser::load_entries() {
    auto findx_path = parsed_stream->path;
    findx_path += ".findx";
    std::ifstream input_findx(findx_path);

    std::string line;
    while (std::getline(input_findx, line)) {
        std::vector<std::string> line_parts;
        boost::split(line_parts, line, boost::is_any_of("|"));

        tcpflow_findx_entry entry{};
        entry.byte_index = std::stoll(line_parts[0]);
        entry.timestamp = std::stold(line_parts[1]);
        entry.length = std::stoll(line_parts[2]);

        entries.push_back(entry);
    }

    std::sort(entries.begin(), entries.end());

    input_findx.close();
}

long double tcpflow_findx_parser::timestamp_for_byte(uint64_t byte_index) const {
    tcpflow_findx_entry upper_bound_entry{};
    upper_bound_entry.byte_index = byte_index;

    auto it = std::upper_bound(entries.begin(), entries.end(), upper_bound_entry);
    --it;
    return it->timestamp;
}

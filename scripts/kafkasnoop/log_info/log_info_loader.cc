#include "log_info_loader.hh"
#include "../tcpflow/tcpflow_findx_parser.hh"

#include <utility>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <fstream>

log_info_loader::log_info_loader(std::string path) : path(std::move(path)) {}

void log_info_loader::load() {
    std::ifstream input(path);

    std::string line;
    while (std::getline(input, line)) {
        std::vector<std::string> line_parts;
        boost::split(line_parts, line, boost::is_any_of("|"));

        auto entry = std::make_shared<log_info_line>();
        entry->timestamp = std::stold(line_parts[0]);
        entry->log_line = line_parts[1];

        log_info_lines.push_back(entry);
    }

    std::stable_sort(log_info_lines.begin(), log_info_lines.end(),
                     [](const std::shared_ptr<log_info_line> &lhs,
                        const std::shared_ptr<log_info_line> &rhs) {
                         return lhs->timestamp < rhs->timestamp;
                     });

    input.close();
}

#include <utility>
#include <iostream>
#include <boost/algorithm/string/split.hpp>
#include <boost/algorithm/string/classification.hpp>
#include <boost/algorithm/string/replace.hpp>

#include "tcpflow_file_listing.hh"

tcpflow_file_listing::tcpflow_file_listing(std::string path, std::vector<std::string> client_ips) : path(std::move(
        path)), client_ips(std::move(client_ips)) {}

std::vector<std::shared_ptr<tcpflow_file_info>> tcpflow_file_listing::list_files() const {
    std::vector<std::shared_ptr<tcpflow_file_info>> listed_files;
    for (const auto &entry : std::filesystem::directory_iterator(path)) {
        if (!is_valid_entry(entry)) continue;

        auto file_info = parse_entry(entry);
        if (is_file_info_filtered_out(file_info)) continue;

        listed_files.push_back(std::make_shared<tcpflow_file_info>(file_info));
    }

    return listed_files;
}

tcpflow_file_info tcpflow_file_listing::parse_entry(const std::filesystem::directory_entry &entry) const {
    tcpflow_file_info file_info{};
    file_info.path = entry.path();
    file_info.filename = file_info.path.filename().generic_string();
    file_info.file_size = entry.file_size();

    std::vector<std::string> filename_parts;
    boost::split(filename_parts, file_info.filename, boost::is_any_of("-"));

    file_info.source.ip_addr = normalize_ip(filename_parts[0]);
    file_info.source.port = std::stoi(filename_parts[1]);
    file_info.source.type = std::find(client_ips.begin(), client_ips.end(), file_info.source.ip_addr) == client_ips.end()
                            ? KAFKA_BROKER : CLIENT;

    file_info.dest.ip_addr = normalize_ip(filename_parts[2]);
    file_info.dest.port = std::stoi(filename_parts[3]);
    file_info.dest.type = std::find(client_ips.begin(), client_ips.end(), file_info.dest.ip_addr) == client_ips.end()
                          ? KAFKA_BROKER : CLIENT;

    file_info.connection_number = std::stoll(filename_parts[4]);

    return file_info;
}

bool tcpflow_file_listing::is_valid_entry(const std::filesystem::directory_entry &entry) const {
    return std::regex_match(entry.path().filename().generic_string(), filename_regex);
}

std::string tcpflow_file_listing::normalize_ip(const std::string &ip) const {
    std::string normalized_ip = "." + ip;
    boost::replace_all(normalized_ip, ".0", ".");
    boost::replace_all(normalized_ip, ".0", ".");
    boost::replace_first(normalized_ip, ".", "");
    return normalized_ip;
}

bool tcpflow_file_listing::is_file_info_filtered_out(const tcpflow_file_info &info) const {
    return info.source.type == KAFKA_BROKER && info.dest.type == KAFKA_BROKER;
}

std::ostream &operator<<(std::ostream &os, const tcpflow_file_info &info) {
    os << "path: " << info.path << " filename: " << info.filename << " source_ip: " << info.source.ip_addr
       << " source_port: " << info.source.port << " source_type: " << info.source.type << " dest_ip: " << info.dest.ip_addr
       << " dest_port: " << info.dest.port << " dest_type: " << info.dest.type << " connection_number: "
       << info.connection_number;
    return os;
}

std::string tcpflow_file_info::get_connection_id() const {
    if (source.type == CLIENT) {
        return source.to_string() + ";" + dest.to_string() + ";"
               + std::to_string(connection_number);
    }
    return dest.to_string() + ";" + source.to_string() + ";"
           + std::to_string(connection_number);
}

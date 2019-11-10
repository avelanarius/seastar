#pragma once

#include <string>
#include <vector>
#include <regex>
#include <filesystem>
#include <ostream>

enum endpoint_type {
    CLIENT, KAFKA_BROKER
};

struct tcpflow_endpoint {
    std::string ip_addr;
    uint32_t port;
    endpoint_type type;

    std::string to_string() const {
        return ip_addr + ":" + std::to_string(port);
    }

    bool operator==(const tcpflow_endpoint &other) const {
        return ip_addr == other.ip_addr && port == other.port && type == other.type;
    }

    bool operator!=(const tcpflow_endpoint &other) const {
        return !(*this == other);
    }
};

struct tcpflow_file_info {
        std::filesystem::path path;
    std::string filename;
    uintmax_t file_size;

    tcpflow_endpoint source;
    tcpflow_endpoint dest;

    uint64_t connection_number;

    [[nodiscard]] std::string get_connection_id() const;

    friend std::ostream &operator<<(std::ostream &os, const tcpflow_file_info &info);
};

/*!
 * \brief Parses filenames created by tcpflow with command:
 * > sudo tcpflow -T %A-%a-%B-%b-%#
 * Example filename: 172.013.000.002-09092-172.013.000.004-56882-0
 */
class tcpflow_file_listing {
    inline static const std::regex filename_regex{R"(\d{3}\.\d{3}\.\d{3}\.\d{3}-\d{5}-\d{3}\.\d{3}\.\d{3}\.\d{3}-\d{5}-\d+)"};

    std::string path;
    std::vector<std::string> client_ips;

    [[nodiscard]] bool is_valid_entry(const std::filesystem::directory_entry &entry) const;
    [[nodiscard]] tcpflow_file_info parse_entry(const std::filesystem::directory_entry &entry) const;

    [[nodiscard]] bool is_file_info_filtered_out(const tcpflow_file_info &info) const;

    [[nodiscard]] std::string normalize_ip(const std::string &ip) const;

public:
    tcpflow_file_listing(std::string path, std::vector<std::string> client_ips);

    [[nodiscard]] std::vector<std::shared_ptr<tcpflow_file_info>> list_files() const;
};
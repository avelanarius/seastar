#pragma once

#include <string>
#include <vector>
#include <memory>

struct log_info_line {
    std::string log_line;
    long double timestamp;
};

class log_info_loader {
    std::string path;

public:
    std::vector<std::shared_ptr<log_info_line>> log_info_lines;

    explicit log_info_loader(std::string path);

    void load();
};

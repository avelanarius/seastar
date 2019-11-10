#pragma once

#include <string>
#include <vector>

class report_util {
public:
    [[nodiscard]] static std::string name_for_api_key(int16_t key);
    [[nodiscard]] static std::string epoch_to_string(long double secs_since_epoch);
    [[nodiscard]] static std::string hexdump(const std::vector<char> &data);
    [[nodiscard]] static std::string asciidump(const std::vector<char> &data);
};
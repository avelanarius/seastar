#pragma once

#include <map>
#include <vector>
#include <string>

class indexed_set {
    std::map<std::string, uint64_t> indexes;
    uint64_t current_index;
    uint64_t color_count;
    std::vector<uint64_t> random_color_permutation;

public:
    indexed_set(uint64_t color_count);

    uint64_t get_index(const std::string &elem);
    uint64_t get_color(const std::string &elem);
};
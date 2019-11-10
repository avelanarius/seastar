#include "indexed_set.hh"
#include <random>
#include <algorithm>

indexed_set::indexed_set(uint64_t color_count)
    : current_index(0), color_count(color_count) {
    std::mt19937 gen(std::random_device{}());
    for (uint64_t i = 0; i < color_count; i++) {
        random_color_permutation.push_back(i);
    }
    std::shuffle(random_color_permutation.begin(), random_color_permutation.end(), gen);
}

uint64_t indexed_set::get_index(const std::string &elem) {
    auto it = indexes.find(elem);
    if (it == indexes.end()) {
        current_index++;
        indexes[elem] = current_index;
        return current_index;
    }
    return it->second;
}

uint64_t indexed_set::get_color(const std::string &elem) {
    auto index = get_index(elem);
    return random_color_permutation[index % color_count];
}

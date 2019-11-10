#pragma once

#include <string>

class templated_file_instance {
public:
    std::string contents;

    templated_file_instance& replace(const std::string &pattern, const std::string &value);
    template<typename T> templated_file_instance& replace(const std::string &pattern, const T &value) {
        return replace(pattern, std::to_string(value));
    }

    void save(const std::string &destination) const;
};

/*!
 * \brief Helper class for HTML templates.
 */
class templated_file {
    std::string path;
    std::string contents;

    void read_file();
public:
    explicit templated_file(std::string path);

    [[nodiscard]] templated_file_instance get() const;
};

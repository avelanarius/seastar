#include <utility>
#include <fstream>
#include <boost/algorithm/string/replace.hpp>

#include "templated_file.hh"

templated_file::templated_file(std::string path) : path(std::move(path)) {
    read_file();
}

void templated_file::read_file() {
    std::ifstream stream(path);
    contents = std::string((std::istreambuf_iterator<char>(stream)),
                    std::istreambuf_iterator<char>());
    stream.close();
}

templated_file_instance templated_file::get() const {
    templated_file_instance instance;
    instance.contents = contents;
    return instance;
}

templated_file_instance &templated_file_instance::replace(const std::string &pattern, const std::string &value) {
    boost::replace_all(contents, pattern, value);
    return *this;
}

void templated_file_instance::save(const std::string &destination) const {
    std::ofstream stream(destination);
    stream << contents;
    stream.close();
}

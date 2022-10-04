#pragma once

#include <string>
#include <sstream>

namespace PathHelper {
    bool is_relative(const std::string& filepath);
    std::string combine(const std::string& root, const std::string& relative);
    std::string catalog_from_filepath(const std::string& filepath);
    std::string file_name(const std::string& filepath);
    std::string file_extension(const std::string& filepath);
}
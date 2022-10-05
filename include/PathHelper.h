#pragma once

#include <string>
#include <sstream>
#include "HTTPTextFetcher.h"

namespace PathHelper {
    bool is_relative(const std::string& filepath);
    std::string combine(const std::string& root, const std::string& relative);
    std::string catalog_from_filepath(const std::string& filepath);
    std::string file_name(const std::string& filepath);
    std::string file_extension(const std::string& filepath);
    bool get_text(const std::string& path, std::stringstream& acceptor);
}
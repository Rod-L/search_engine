#include <vector>
#include "PathHelper.h"

bool PathHelper::is_relative(const std::string &filepath) {
    return filepath.find(':') == std::string::npos;
}

std::string PathHelper::combine(const std::string& root, const std::string& relative) {
    std::string part;
    std::vector<std::string> parts;
    std::stringstream parser(root);

    while(!parser.eof()) {
        std::getline(parser, part, '/');
        if (!part.empty()) parts.push_back(part);
    }

    parser.str(relative);
    parser.seekg(0);
    parser.clear();
    while(!parser.eof()) {
        std::getline(parser, part, '/');
        if (part == ".." && !parts.empty()) parts.pop_back();
        else parts.push_back(part);
    }

    parser.str("");
    parser.seekg(0);
    parser.clear();
    for (auto& part : parts) {
        parser << part << '/';
    }
    std::string result = parser.str();
    result.resize(result.size() - 1);
    return result;
}

std::string PathHelper::catalog_from_filepath(const std::string& filepath) {
    auto slash_pos = filepath.rfind('/');
    if (slash_pos == std::string::npos) return "";
    return filepath.substr(0, slash_pos + 1);
}

std::string PathHelper::file_name(const std::string& filepath) {
    auto slash_pos = filepath.rfind('/');
    if (slash_pos == std::string::npos) slash_pos = 0;

    auto dot_pos = filepath.rfind('.');
    if (dot_pos == std::string::npos) dot_pos = filepath.length();

    return filepath.substr(slash_pos, dot_pos - slash_pos);
}

std::string PathHelper::file_extension(const std::string& filepath) {
    auto dot_pos = filepath.rfind('.');
    if (dot_pos == std::string::npos) dot_pos = filepath.length();

    return filepath.substr(dot_pos, filepath.length() - dot_pos);
}
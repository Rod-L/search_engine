#include "PathHelper.h"

std::string PathHelper::current_dir() {
    auto path = std::filesystem::current_path();
    return path.string();
}

bool PathHelper::is_relative(const std::string &filepath) {
    return filepath.find(':') == std::string::npos;
}

bool PathHelper::file_exist(const std::string &filepath) {
    std::filesystem::path path(filepath);
    std::error_code err_code;

    bool exist = std::filesystem::exists(path, err_code);
    return exist;
}

std::string PathHelper::combine_path(const std::string& root, const std::string& relative) {
    char div;
    std::string part;
    std::vector<std::string> parts;
    std::stringstream parser(root);

    div = root.find("/") == std::string::npos ? '\\' : '/';
    while(std::getline(parser, part, div)) {
        if (!part.empty()) parts.push_back(part);
    }

    parser.str(relative);
    parser.seekg(0);
    parser.clear();
    div = relative.find("/") == std::string::npos ? '\\' : '/';
    while(std::getline(parser, part, div)) {
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
    if (slash_pos == std::string::npos) {
        slash_pos = 0;
    } else {
        ++slash_pos;
    }

    auto dot_pos = filepath.rfind('.');
    if (dot_pos == std::string::npos) dot_pos = filepath.length();

    return filepath.substr(slash_pos, dot_pos - slash_pos);
}

std::string PathHelper::file_extension(const std::string& filepath) {
    auto dot_pos = filepath.rfind('.');
    if (dot_pos == std::string::npos) dot_pos = filepath.length();

    return filepath.substr(dot_pos, filepath.length() - dot_pos);
}

bool PathHelper::get_text(const std::string& path, std::stringstream& acceptor) {
    std::ifstream file(path);
    if (!file.is_open()) return false;
    acceptor << file.rdbuf();
    acceptor.seekg(0);
    return true;
}
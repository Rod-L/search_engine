#pragma once

#include <cpr/cpr.h>
#include <sstream>

namespace HTTPFetcher {
    std::string get_text(const std::string& str_url);
    bool is_url(const std::string& str_url);
}

bool HTTPFetcher::is_url(const std::string& str_url) {
    return str_url.substr(0,4) == "http" && str_url.find("://") != std::string::npos;
}

std::string HTTPFetcher::get_text(const std::string& str_url) {
    cpr::Url url{str_url};

    std::stringstream input;
    std::stringstream output;

    auto response = cpr::Get(url, cpr::Header({{"accept", "text/*"}}));
    if (response.status_code != 200) return "";

    input.str(response.text);
    //std::ofstream dump_file("dump.txt");
    //dump_file << response.text;
    //dump_file.close();

    std::string buf;
    while(!input.eof()) {
        std::getline(input, buf, '>');
        if (buf.substr(0, 6) == "script" || buf.substr(0, 5) == "style") continue;
        input >> std::ws;
        std::getline(input, buf, '<');
        output << buf << std::endl;
    }

    return output.str();
}
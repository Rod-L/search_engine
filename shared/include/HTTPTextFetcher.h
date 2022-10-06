#pragma once

#include <cpr/cpr.h>
#include <sstream>

namespace HTTPFetcher {
    bool is_url(const std::string& str_url);
    bool is_html(const std::string& filepath);
    bool get_html(const std::string& url, std::string& acceptor);
    void get_text(const std::string& html, std::stringstream& acceptor);
}
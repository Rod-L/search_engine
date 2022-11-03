#include "HTTPTextFetcher.h"

bool HTTPFetcher::is_http_link(const std::string& str_url) {
    return str_url.size() > 8 && str_url.substr(0,4) == "http" && str_url.find("://") != std::string::npos;
}

bool HTTPFetcher::is_html(const std::string& str_url) {
    return str_url.substr(str_url.size() - 5, 5) == ".html";
}

bool HTTPFetcher::get_html(const std::string& http_link, std::stringstream& acceptor) {
    auto response = cpr::Get(cpr::Url{http_link}, cpr::Header({{"accept", "text/*"}}));
    if (response.status_code != 200) return false;
    acceptor.str(response.text);
    return true;
}

void HTTPFetcher::get_text(std::stringstream& html, std::stringstream& acceptor) {
    acceptor.str("");
    std::string tag;
    std::string buf;
    while(true) {
        std::getline(html, tag, '>');
        html >> std::ws;
        std::getline(html, buf, '<');
        if (html.eof()) break;
        tag.resize(6);
        if (buf.empty() || tag == "script" || tag == "style ") continue;
        acceptor << buf << std::endl;
    }
}
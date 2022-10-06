#include "HTTPTextFetcher.h"

bool HTTPFetcher::is_url(const std::string& str_url) {
    return str_url.size() > 8 && str_url.substr(0,4) == "http" && str_url.find("://") != std::string::npos;
}

bool HTTPFetcher::is_html(const std::string& str_url) {
    return str_url.substr(str_url.size() - 5, 5) == ".html";
}

bool HTTPFetcher::get_html(const std::string& url, std::string& acceptor) {
    auto response = cpr::Get(cpr::Url{url}, cpr::Header({{"accept", "text/*"}}));
    if (response.status_code != 200) return false;
    acceptor = response.text;
    return true;
}

void HTTPFetcher::get_text(const std::string& html, std::stringstream& acceptor) {
    std::stringstream input;
    std::stringstream output;

    input.str(html);

    std::string buf;
    while(!input.eof()) {
        std::getline(input, buf, '>');
        if (buf.substr(0, 6) == "script" || buf.substr(0, 5) == "style") continue;
        input >> std::ws;
        std::getline(input, buf, '<');
        acceptor << buf << std::endl;
    }
}
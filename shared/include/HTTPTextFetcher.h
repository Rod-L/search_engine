#pragma once

#include <cpr/cpr.h>
#include <sstream>

namespace HTTPFetcher {
    /**
     * @param str_url URL to analyze
     * @return true, if passed URL is http link
     */
    bool is_http_link(const std::string& str_url);

    /**
     * @param filepath path to file
     * @return true, if extension of the file is 'html'
     */
    bool is_html(const std::string& filepath);

    /**
     * @param http_link link to get content from
     * @param acceptor string stream to put content to
     * @return true, if operation is successful
     */
    bool get_html(const std::string& http_link, std::stringstream& acceptor);

    /** Fetches text from html content, tags and service fields are ignored
     * @param html content of the html file
     * @param acceptor string stream to put text content to
     */
    void get_text(std::stringstream& html, std::stringstream& acceptor);
}
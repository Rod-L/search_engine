#include <catch2/catch_test_macros.hpp>

#include "HTTPTextFetcher.h"

using namespace HTTPFetcher;

TEST_CASE("HTTPFetcher_is_http_link") {
    REQUIRE(!is_http_link("C:/case.test"));
    REQUIRE(is_http_link("http://www.example.com"));
    REQUIRE(is_http_link("https://www.example.com"));
}

TEST_CASE("HTTPFetcher_is_html") {
    REQUIRE(!is_html("C:/case.test"));
    REQUIRE(!is_html("https://www.example.com"));
    REQUIRE(is_html("C:/case.html"));
    REQUIRE(is_html("case.html"));
}
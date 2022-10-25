#include <catch2/catch_test_macros.hpp>

#include "PathHelper.h"

using namespace PathHelper;

TEST_CASE("PathHelper_is_relative") {
    REQUIRE(is_relative("case1.test"));
    REQUIRE(is_relative("./case1.test"));
    REQUIRE(is_relative("../case1.test"));
    REQUIRE(is_relative("../../case1.test"));
    REQUIRE(!is_relative("C:/case1.test"));
    REQUIRE(!is_relative("D:/__/case1.test"));
}

TEST_CASE("PathHelper_catalog_from_filepath") {
    REQUIRE(catalog_from_filepath("C:/case1.test") == "C:/");
    REQUIRE(catalog_from_filepath("C:/") == "C:/");
    REQUIRE(catalog_from_filepath("C:/catalog/case1.test") == "C:/catalog/");
    REQUIRE(catalog_from_filepath("../case1.test") == "../");
    REQUIRE(catalog_from_filepath("case1.test") == "");
}

TEST_CASE("PathHelper_file_name") {
    REQUIRE(file_name("config.json") == "config");
    REQUIRE(file_name("config..json") == "config.");
    REQUIRE(file_name("../catalog/config.json") == "config");
    REQUIRE(file_name("C:/config.json") == "config");
}

TEST_CASE("PathHelper_file_extension") {
    REQUIRE(file_extension("config.json") == ".json");
    REQUIRE(file_extension("config..json") == ".json");
    REQUIRE(file_extension("../catalog/config.json") == ".json");
    REQUIRE(file_extension("C:/config.json") == ".json");
    REQUIRE(file_extension("C:/config") == "");
}

TEST_CASE("PathHelper_combine_path") {
    REQUIRE(combine_path("C:/", "config.json") == "C:/config.json");
    REQUIRE(combine_path("C:/", "../config.json") == "config.json");
    REQUIRE(combine_path("C:/", "../../config.json") == "../config.json");

    REQUIRE(combine_path("../catalog/", "config.json") == "../catalog/config.json");
    REQUIRE(combine_path("../catalog/", "../config.json") == "../config.json");
    REQUIRE(combine_path("../catalog/", "../../config.json") == "config.json");
    REQUIRE(combine_path("../catalog/", "../../../config.json") == "../config.json");
}
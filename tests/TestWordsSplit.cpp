#include <catch2/catch_test_macros.hpp>

#include "InvertedIndex.h"

TEST_CASE("InvertedIndex_SplitWords_non_letters") {
    std::string word = "*****";
    auto results = InvertedIndex::split_by_non_letters(word, 2);

    REQUIRE(results.size() == 0);
}

TEST_CASE("InvertedIndex_SplitWords_0") {
    std::string word = "test";
    auto results = InvertedIndex::split_by_non_letters(word, 2);

    REQUIRE(results.size() == 1);
    REQUIRE(results[0] == "test");
}

TEST_CASE("InvertedIndex_SplitWords_1") {
    std::string word = "(test)";
    auto results = InvertedIndex::split_by_non_letters(word, 2);

    REQUIRE(results.size() == 1);
    REQUIRE(results[0] == "test");
}

TEST_CASE("InvertedIndex_SplitWords_2") {
    std::string word = "(test)__next";
    auto results = InvertedIndex::split_by_non_letters(word, 2);

    REQUIRE(results.size() == 2);
    REQUIRE(results[0] == "test");
    REQUIRE(results[1] == "next");
}

TEST_CASE("InvertedIndex_SplitWords_3") {
    std::string word = ":'''''___________(test)__next*********abc------";
    auto results = InvertedIndex::split_by_non_letters(word, 2);

    REQUIRE(results.size() == 3);
    REQUIRE(results[0] == "test");
    REQUIRE(results[1] == "next");
    REQUIRE(results[2] == "abc");
}

TEST_CASE("InvertedIndex_SplitWords_4") {
    std::string word = "ch1_ch1_ch1";
    auto results = InvertedIndex::split_by_non_letters(word, 2);

    REQUIRE(results.size() == 3);
    REQUIRE(results[0] == "ch1");
    REQUIRE(results[1] == "ch1");
    REQUIRE(results[2] == "ch1");
}
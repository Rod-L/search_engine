#include <catch2/catch_test_macros.hpp>
#include "InvertedIndex.h"

bool have_match(const Entry& value, std::vector<Entry>& entries) {
    for (auto& entry : entries) {
        if (entry == value) return true;
    }
    return false;
}

void TestInvertedIndexFunctionality(
        const std::vector<std::string>& docs,
        const std::vector<std::string>& requests,
        const std::vector<std::vector<Entry>>& expected) {

    std::vector<std::vector<Entry>> result;
    InvertedIndex idx;
    idx.update_text_base(docs);

    for(auto& request : requests) {
        std::vector<Entry> word_count = idx.get_word_count(request);
        result.push_back(word_count);
    }

    REQUIRE(result.size() == expected.size());

    for (int i = 0; i < expected.size(); ++i) {
        auto& exp = expected[i];
        auto& res = result[i];
        REQUIRE(exp.size() == res.size()); // Amount of answers is equal for each requests

        for (auto& exp_entry : exp) {
            REQUIRE(have_match(exp_entry, res));
        }
    }

    REQUIRE(result == expected);
}

TEST_CASE("InvertedIndex_TestBasic") {
    const std::vector<std::string> docs = {
            "london is the capital of great britain",
            "big ben is the nickname for the Great bell of the striking clock"
    };
    const std::vector<std::string> requests = {"london", "the"};
    const std::vector<std::vector<Entry>> expected = {
            {
                    {0, 1}
            },
            {
                    {0, 1}, {1, 3}
            }
    };
    TestInvertedIndexFunctionality(docs, requests, expected);
}

TEST_CASE("InvertedIndex_TestBasic2") {
    const std::vector<std::string> docs = {
            "milk milk milk milk water water water",
            "milk water water",
            "milk milk milk milk milk water water water water water",
            "americano cappuccino"
    };
    const std::vector<std::string> requests = {"milk", "water", "cappuccino"};
    const std::vector<std::vector<Entry>> expected = {
            {
                    {0, 4}, {1, 1}, {2, 5}
            },
            {
                    {0, 3}, {1, 2}, {2, 5}
            },
            {
                    {3, 1}
            }
    };
    TestInvertedIndexFunctionality(docs, requests, expected);
}

TEST_CASE("InvertedIndex_MissingWord") {
    const std::vector<std::string> docs = {
            "a b c d e f g h i j k l",
            "statement"
    };
    const std::vector<std::string> requests = {"m", "statement"};
    const std::vector<std::vector<Entry>> expected = {
            {
            },
            {
                    {1, 1}
            }
    };
    TestInvertedIndexFunctionality(docs, requests, expected);
}

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
    std::string word = "ch_ch_ch";
    auto results = InvertedIndex::split_by_non_letters(word, 2);

    REQUIRE(results.size() == 3);
    REQUIRE(results[0] == "ch");
    REQUIRE(results[1] == "ch");
    REQUIRE(results[2] == "ch");
}
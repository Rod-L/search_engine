#include <catch2/catch_test_macros.hpp>
#include "InvertedIndex.h"

void display_entry_vector(const std::vector<Entry>& entries) {
    for (auto& entry : entries) {
        std::cout << "doc_id: " << entry.doc_id << ", count: " << entry.count << std::endl;
    }
}

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

    for (int i = 0; i < 400; ++i) {
        std::vector<std::vector<Entry>> result;
        InvertedIndex idx;
        idx.update_text_base(docs);

        for (auto &request: requests) {
            std::vector<Entry> word_count = idx.get_word_count(request);
            result.push_back(word_count);
        }

        REQUIRE(result.size() == expected.size());

        for (int i = 0; i < expected.size(); ++i) {
            auto &exp = expected[i];
            auto &res = result[i];
            REQUIRE(exp.size() == res.size()); // Amount of answers is equal for each requests

            for (auto &exp_entry: exp) {
                bool success = have_match(exp_entry, res);
                if (!success) {
                    std::cout << "Expected:" << std::endl;
                    display_entry_vector(exp);
                    std::cout << "Got: " << std::endl;
                    display_entry_vector(res);
                }
                REQUIRE(success);
            }
        }
    }
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
                    {1, 3}, {0, 1}
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

TEST_CASE("InvertedIndex_IndexExtension") {
    const std::vector<std::string> docs1 = {
            "../examples/2/Sample 1.txt",
            "../examples/2/Sample 2.txt",
            "../examples/2/Sample 3.txt"
    };

    const std::vector<std::string> docs2 = {
            "../examples/2/Sample 2.txt",
            "../examples/1/Annotations/12 Angry men.txt",
            "../examples/1/Annotations/Fight club.txt",
            "../examples/1/Annotations/Forrest Gump.txt"
    };

    InvertedIndex idx;
    idx.extend_document_base(docs1, 4);
    idx.extend_document_base(docs2, 4);
}

TEST_CASE("InvertedIndex_IndexationPerfomance") {
    const std::vector<std::string> docs = {
            "../Meyer David. Behemoth 1.txt",
            "../Meyer David. Behemoth 2.txt",
            "../Meyer David. Behemoth 3.txt",
            "../Meyer David. Behemoth 4.txt",
            "../Meyer David. Behemoth 5.txt",
            "../Meyer David. Behemoth 6.txt",
//            "../../examples/1/Annotations/12 Angry men.txt",
//            "../../examples/1/Annotations/Fight club.txt",
//            "../../examples/1/Annotations/Forrest Gump.txt",
//            "../../examples/1/Annotations/Il buono, il brutto, il cattivo.txt",
//            "../../examples/1/Annotations/Inception.txt",
//            "../../examples/1/Annotations/Pulp Fiction.txt",
//            "../../examples/1/Annotations/Schindler's List.txt",
//            "../../examples/1/Annotations/The Dark Knight.txt",
//            "../../examples/1/Annotations/The Godfather part II.txt",
//            "../../examples/1/Annotations/The Godfather.txt",
//            "../../examples/1/Annotations/The Lord of the Rings The Fellowship of the Ring.txt",
//            "../../examples/1/Annotations/The Lord of the Rings The Return of the King.txt",
//            "../../examples/1/Annotations/The Shawshank Redemption.txt"
    };

    InvertedIndex idx;
    for (int i = 0; i < 10; ++i) {
        idx.update_document_base(docs);
    }
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

TEST_CASE("InvertedIndex_URLContent") {
    const std::vector<std::string> docs = {"https://example.com/"};
    const std::string request = "Domain";

    InvertedIndex idx;
    idx.update_document_base(docs);

    std::vector<Entry> word_count = idx.get_word_count(request);
    REQUIRE((word_count.empty() || word_count.size() == 1));

    if (!word_count.empty()) {
        REQUIRE(word_count[0].count == 2);
    }
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
    std::string word = "ch1_ch1_ch1";
    auto results = InvertedIndex::split_by_non_letters(word, 2);

    REQUIRE(results.size() == 3);
    REQUIRE(results[0] == "ch1");
    REQUIRE(results[1] == "ch1");
    REQUIRE(results[2] == "ch1");
}
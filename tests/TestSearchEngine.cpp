#include <catch2/catch_test_macros.hpp>
#include "InvertedIndex.h"
#include "SearchEngine.h"

void FillDB(InvertedIndex& indexer) {
    std::string doc, content;
    bool success;

    doc = "Test doc 1.txt";
    content = "Word1 Word1 Word1 Word1 Word2 Word3";
    success = indexer.add_document(doc, content);
    REQUIRE(success == true);

    doc = "Test doc 2.txt";
    content = "Word1 Word2 Word4";
    success = indexer.add_document(doc, content);
    REQUIRE(success == true);

    doc = "Test doc 3.txt";
    content = "Word1 Word1 Word2 Word4";
    success = indexer.add_document(doc, content);
    REQUIRE(success == true);
}

TEST_CASE("SearchEngine_Search_1") {
    InvertedIndex indexer;
    FillDB(indexer);

    SearchServer server(indexer);
    std::vector<std::string> queries{"Word1"};
    auto result = server.search(queries);

    for (auto& query_results : result) {
        for (auto& entry : query_results) std::cout << entry.doc_id << " : " << entry.rank << std::endl;
    }

    REQUIRE(result.size() == 1);
    REQUIRE(result[0].size() == 3);
    REQUIRE(result[0][0].doc_id == 0);
    REQUIRE(result[0][1].doc_id == 2);
    REQUIRE(result[0][2].doc_id == 1);
}

TEST_CASE("SearchEngine_Search_2") {
    InvertedIndex indexer;
    FillDB(indexer);

    SearchServer server(indexer);
    std::vector<std::string> queries{"Word4 Word1 Word1", "Word3"};
    auto result = server.search(queries);

    REQUIRE(result.size() == 2);
    REQUIRE(result[0].size() == 3);
    REQUIRE(result[1].size() == 1);

    REQUIRE(result[0][0].doc_id == 0);
    REQUIRE(result[0][1].doc_id == 2);
    REQUIRE(result[0][2].doc_id == 1);

    REQUIRE(result[1][0].doc_id == 0);
}

TEST_CASE("SearchEngine_Search_3") {
    const std::vector<std::string> docs = {
            "milk milk milk milk water water water",
            "milk water water",
            "milk milk milk milk milk water water water water water",
            "americano cappuccino"
    };
    const std::vector<std::string> request = {"milk water", "sugar"};
    const std::vector<std::vector<RelativeIndex>> expected = {
            {
                    {2, 1},
                    {0, 0.7},
                    {1, 0.3}
            },
            {
            }
    };
    InvertedIndex idx;
    idx.update_text_base(docs);
    SearchServer srv(idx);
    std::vector<std::vector<RelativeIndex>> result = srv.search(request);
    REQUIRE(result == expected);
}

TEST_CASE("SearchEngine_Search_Top5") {
    const std::vector<std::string> docs = {
            "london is the capital of great britain",
            "paris is the capital of france",
            "berlin is the capital of germany",
            "rome is the capital of italy",
            "madrid is the capital of spain",
            "lisboa is the capital of portugal",
            "bern is the capital of switzerland",
            "moscow is the capital of russia",
            "kiev is the capital of ukraine",
            "minsk is the capital of belarus",
            "astana is the capital of kazakhstan",
            "beijing is the capital of china",
            "tokyo is the capital of japan",
            "bangkok is the capital of thailand",
            "welcome to moscow the capital of russia the third rome",
            "amsterdam is the capital of netherlands",
            "helsinki is the capital of finland",
            "oslo is the capital of norway",
            "stockholm is the capital of sweden",
            "riga is the capital of latvia",
            "tallinn is the capital of estonia",
            "warsaw is the capital of poland",
        };

    const std::vector<std::string> request = {"moscow is the capital of russia"};
    const std::vector<std::vector<RelativeIndex>> expected = {
            {
                    {7, 1},
                    {14, 1},
                    {0, 0.666666687},
                    {1, 0.666666687},
                    {2, 0.666666687}
            }
    };
    InvertedIndex idx;
    idx.update_text_base(docs);
    SearchServer srv(idx);

    std::vector<std::vector<RelativeIndex>> result = srv.search(request, 5);

    REQUIRE(result.size() == 1);
    REQUIRE(result[0].size() == 5);

    for (int i = 0; i < 5; ++i) {
         auto val = std::abs(result[0][i].rank - expected[0][i].rank);
         REQUIRE(val < 0.001);
    }
}
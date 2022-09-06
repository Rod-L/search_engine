#pragma once

#include <vector>
#include <sstream>
#include <unordered_set>
#include "InvertedIndex.h"
#include "RelativeIndex.h"

class SearchServer {
public:
    SearchServer() = default;

    /**
    * @param idx prepared InvertedIndex to use for search queries
    */
    explicit SearchServer(InvertedIndex& idx) : _index(idx) {};

    /**
    * @param queries_input list of queries
    * @return array of lists with responses, sorted by relevance
    */
    std::vector<std::vector<RelativeIndex>> search(const std::vector<std::string>& queries_input, int responses_limit = 0);

    /**
    * Updates underlying InvertedIndex
    * @param input_docs list of filenames
    */
    void update_document_base(std::vector<std::string>& input_docs);

private:
    std::vector<RelativeIndex> process_query(std::vector<std::string>& words, int responses_limit = 0);
    static std::vector<std::string> query_to_words(const std::string& query);

    InvertedIndex _index;
};
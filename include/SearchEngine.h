#pragma once

#include <vector>
#include <sstream>
#include <unordered_set>
#include "InvertedIndex.h"
#include "RelativeIndex.h"

class SearchServer: public InvertedIndex {
public:
    SearchServer() = default;

    /**
    * @param queries_input list of queries
    * @return array of lists with responses, sorted by relevance
    */
    std::vector<std::vector<RelativeIndex>> search(const std::vector<std::string>& queries_input, int responses_limit = 0);

    /**
    * clears underlying inverted index
    */
    void clear_index();

private:
    std::vector<RelativeIndex> process_query(std::vector<std::string>& words, int responses_limit = 0);
    static std::vector<std::string> query_to_words(const std::string& query);
};
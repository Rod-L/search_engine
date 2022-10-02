#include "SearchEngine.h"

std::vector<std::string> SearchServer::query_to_words(const std::string& query) {
    std::stringstream stream(query);
    std::string word;
    std::unordered_set<std::string> unique_words;
    std::vector<std::string> result;

    while (stream >> word) {
        if (unique_words.find(word) == unique_words.end()) {
            unique_words.insert(word);
            result.push_back(word);
        }
    }
    return result;
}

std::vector<RelativeIndex> SearchServer::process_query(std::vector<std::string>& words, int responses_limit) {
    // count absolute ranks for each document
    std::map<size_t, size_t> docsTotalRank;
    for (auto& word : words) {
        auto entries = get_word_count(word);
        for (auto& entry : entries) {
            docsTotalRank[entry.doc_id] += entry.count;
        }
    }

    // find max absolute ranks
    size_t max_rank = 0;
    for (auto& pair : docsTotalRank) max_rank = std::max(max_rank, pair.second);

    // calculate relative ranks
    std::vector<RelativeIndex> result;
    result.reserve(docsTotalRank.size());
    for (auto& pair : docsTotalRank) {
        result.emplace_back(pair.first, static_cast<float>(pair.second) / static_cast<float>(max_rank));
    }

    // sort result by rank
    for (int i = 0; i < result.size(); ++i) {
        auto max = i;
        for (int pos = i; pos < result.size(); ++pos) {
            if (result[pos].rank > result[max].rank) max = pos;
        }
        if (max != i) std::swap(result[i], result[max]);
    }

    // sort result by doc_id inside ranks
    for (int i = 0; i < result.size(); ++i) {
        auto min = i;
        auto rank = result[i].rank;
        for (int pos = i; pos < result.size() && result[pos].rank == rank; ++pos) {
            if (result[pos].doc_id < result[min].doc_id) min = pos;
        }
        if (min != i) std::swap(result[i], result[min]);
    }

    if (responses_limit > 0 && responses_limit < result.size()) result.resize(responses_limit);

    return result;
}

std::vector<std::vector<RelativeIndex>> SearchServer::search(const std::vector<std::string>& queries_input, int responses_limit) {
    std::vector<std::vector<RelativeIndex>> result;
    for (auto& query : queries_input) {
        auto words = query_to_words(query);
        auto proc = process_query(words, responses_limit);
        result.push_back(proc);
    }
    return result;
}

void SearchServer::clear_index() {
    clear();
}
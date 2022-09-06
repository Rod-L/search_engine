#include "InvertedIndex.h"

//// Entry struct

bool Entry::operator==(const Entry& other) const {
    return (doc_id == other.doc_id && count == other.count);
}

//// WordIndex struct

WordIndex::WordIndex(const WordIndex& other) {
    index = other.index;
};

//// InvertedIndex class

InvertedIndex::InvertedIndex(const InvertedIndex& other) {
    freq_dictionary = other.freq_dictionary;
}

void InvertedIndex::update_document_base(const std::vector<std::string>& input_docs) {
    freq_dictionary.clear();

    std::vector<std::thread> threads;

    bool(::InvertedIndex::*func)(std::string);
    func = &InvertedIndex::add_document;
    for (auto& doc : input_docs) {
        threads.emplace_back(func, this, doc);
    }

    for (auto& thread : threads) {
        if (thread.joinable()) thread.join();
    }
}

void InvertedIndex::update_text_base(const std::vector<std::string>& input_texts) {
    freq_dictionary.clear();

    std::vector<std::thread> threads;

    bool(::InvertedIndex::*func)(std::string, std::string);
    func = &InvertedIndex::add_document;
    for (int i = 0; i < input_texts.size(); ++i) {
        threads.emplace_back(func, this, std::to_string(i), input_texts[i]);
    }

    for (auto& thread : threads) {
        if (thread.joinable()) thread.join();
    }
}

bool InvertedIndex::add_document(std::string filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "AddDocument failed: could not open file '" << filename << "'." << std::endl;
        return false;
    }

    docs_access.lock();
    docs.push_back(filename);
    docs_access.unlock();
    index_doc(docs.size() - 1, file);
    return true;
}

bool InvertedIndex::add_document(std::string filename, std::string text) {
    std::stringstream content(text);
    docs_access.lock();
    docs.push_back(filename);
    docs_access.unlock();
    index_doc(docs.size() - 1, content);
    return true;
}

std::vector<Entry> InvertedIndex::get_word_count(const std::string& word) {
    std::vector<Entry> result;
    auto entries = freq_dictionary.find(word);

    if (entries == freq_dictionary.end()) return result;

    for (auto& pair : entries->second.index) result.push_back(pair.second);

    return result;
}

void InvertedIndex::count_word(std::string &word, size_t doc_id) {
    WordIndex* word_index;
    auto it = freq_dictionary.find(word);

    if (it == freq_dictionary.end()) {
        freq_dict_access.lock();
        freq_dictionary.insert({word, WordIndex()});
        freq_dict_access.unlock();
        word_index = &freq_dictionary[word];
    } else {
        word_index = &(it->second);
    }

    word_index->access.lock();
    auto& entries = word_index->index;
    auto pair = entries.find(doc_id);
    if (pair == entries.end()) {
        entries[doc_id] = Entry{.doc_id = (size_t)doc_id, .count = 1};
    } else {
        pair->second.count += 1;
    }
    word_index->access.unlock();
}

template<typename StreamT>
void InvertedIndex::index_doc(size_t doc_id, StreamT& stream) {
    std::string word;
    while (stream >> word) count_word(word, doc_id);
}

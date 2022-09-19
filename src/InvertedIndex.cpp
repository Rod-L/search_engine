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
    docs.clear();
    docs.resize(input_docs.size());

    std::vector<std::thread> threads;
    threads.reserve(input_docs.size());

    bool(::InvertedIndex::*func)(size_t, std::string);
    func = &InvertedIndex::add_document;
    for (size_t doc_id = 0; doc_id < input_docs.size(); ++doc_id) {
        auto& doc = input_docs[doc_id];
        threads.emplace_back(func, this, doc_id, doc);
    }

    for (auto& thread : threads) {
        if (thread.joinable()) thread.join();
    }

    int loaded_count = 0;
    for (auto& doc : docs) {
        if (!doc.empty()) ++loaded_count;
    }
    std::cout << "Database updated, documents loaded: " << loaded_count << std::endl;
}

void InvertedIndex::update_text_base(const std::vector<std::string>& input_texts) {
    freq_dictionary.clear();
    docs.clear();
    docs.reserve(input_texts.size());

    std::vector<std::thread> threads;
    threads.reserve(input_texts.size());

    bool(::InvertedIndex::*func)(size_t, std::string, std::string);
    func = &InvertedIndex::add_document;
    for (int i = 0; i < input_texts.size(); ++i) {
        auto doc = std::to_string(i);
        docs.push_back(doc);
        threads.emplace_back(func, this, static_cast<size_t>(i), doc, input_texts[i]);
    }

    for (auto& thread : threads) {
        if (thread.joinable()) thread.join();
    }
    std::cout << "Database updated, texts loaded: " << docs.size() << std::endl;
}

bool InvertedIndex::add_document(size_t doc_id, std::string filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "add_document failed: could not open file '" << filename << "'." << std::endl;
        return false;
    }

    docs_access.lock();
    docs[doc_id] = filename;
    docs_access.unlock();
    index_doc(doc_id, file);
    file.close();
    return true;
}

bool InvertedIndex::add_document(size_t doc_id, std::string filename, std::string text) {
    std::stringstream content(text);
    index_doc(doc_id, content);
    return true;
}

std::vector<Entry> InvertedIndex::get_word_count(const std::string& word) {
    std::vector<Entry> result;
    auto entries = freq_dictionary.find(word);

    if (entries == freq_dictionary.end()) return result;

    result.reserve(entries->second.index.size());
    for (auto& pair : entries->second.index) result.push_back(pair.second);

    return result;
}

void InvertedIndex::count_word(std::string &word, size_t doc_id) {
    freq_dict_access.lock();
    auto& word_index = freq_dictionary[word];
    freq_dict_access.unlock();

    word_index.access.lock();
    auto& entries = word_index.index;
    auto pair = entries.find(doc_id);
    if (pair == entries.end()) {
        entries[doc_id] = Entry{static_cast<size_t>(doc_id), 1};
    } else {
        pair->second.count += 1;
    }
    word_index.access.unlock();
}

template<typename StreamT>
void InvertedIndex::index_doc(size_t doc_id, StreamT& stream) {
    std::string word;
    while (stream >> word) {
        count_word(word, doc_id);
        auto parts = InvertedIndex::split_by_non_letters(word);
        if (!parts.empty() && parts[0] != word) {
            for (auto& part : parts) count_word(part, doc_id);
        }
    }
}

std::vector<std::string> InvertedIndex::split_by_non_letters(std::string& word, int min_part_size) {
    auto is_letter = [](char v){
        return v >= 'a' && v <= 'z' || v >= 'A' && v <= 'Z';
    };

    std::vector<std::string> result;

    int start = -1;
    int word_size = static_cast<int>(word.size());
    for (int i = 0; i < word_size; ++i) {
        char symbol = word[i];

        if (is_letter(symbol)) {
            if (start < 0) start = i;
            continue;
        }

        int part_size = i - start;
        if (start >= 0 && part_size >= min_part_size) result.push_back(word.substr(start, part_size));
        start = i + 1;
    }

    int part_size = word_size - start;
    if (part_size >= min_part_size) result.push_back(word.substr(start, part_size));

    return result;
};
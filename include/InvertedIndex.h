#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>

#include <mutex>
#include <thread>

struct Entry {
    size_t doc_id, count;

    bool operator==(const Entry& other) const;
};

struct WordIndex {
    std::mutex access;
    std::map<size_t, Entry> index;

    WordIndex() = default;

    WordIndex(const WordIndex& other);
};

class InvertedIndex {
public:
    InvertedIndex() = default;

    InvertedIndex(const InvertedIndex& other);

    /**
    * Fill and index database from files
    * @param input_docs list of filenames
    */
    void update_document_base(const std::vector<std::string>& input_docs);

    /**
    * Fill and index database from files
    * @param input_texts list of texts to index
    */
    void update_text_base(const std::vector<std::string>& input_texts);

    /**
    * Add and index document
    * @param filename document filename
    * @return true if file processed successfully
    */
    bool add_document(std::string filename);

    /**
    * Add and index document
    * @param filename name of text to identify in database
    * @param text content of the document
    * @return true if text processed successfully
    */
    bool add_document(std::string filename, std::string text);

    /**
    * @param word - word to look for in database
    * @return list of entries {doc_id, count}
    */
    std::vector<Entry> get_word_count(const std::string& word);

private:
    std::mutex docs_access;
    std::vector<std::string> docs;

    std::mutex freq_dict_access;
    std::map<std::string, WordIndex> freq_dictionary;

    void count_word(std::string& word, size_t doc_id);

    template<typename StreamT>
    void index_doc(size_t doc_id, StreamT& stream);
};


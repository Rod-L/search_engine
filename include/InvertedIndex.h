#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <ctime>
#include <mutex>

#include "DocInfoStruct.h"

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


    void extend_document_base(const std::vector<std::string>& input_docs);

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

    void report_indexation_finish(const std::string& filename, const DocInfo& doc_info);

    /**
    * Add and index document
    * @param filename document filename
    * @return true if file processed successfully
    */
    bool add_document(size_t doc_id, const std::string& filename);

    /**
    * Add and index document
    * @param filename name of text to identify in database
    * @param text content of the document
    * @return true if text processed successfully
    */
    bool add_text(size_t doc_id, const std::string& text);

    /**
    * @param word - word to look for in database
    * @return list of entries {doc_id, count}
    */
    std::vector<Entry> get_word_count(const std::string& word);

    static std::vector<std::string> split_by_non_letters(std::string& word, int min_part_size = 2);

    /**Dumps index to given ostream for future save or inspect.*/
    void dump_index(std::ofstream& output) const;

    /**
    * Attempts to load index from given istream.
    * @param input - std::ifstream - opened file stream to load index from.
    * @return true if index loaded successfully.
    * */
    bool load_index(std::ifstream& input);

    bool docs_loaded(const std::vector<std::string>& input_docs) const;

    const std::vector<std::string>& get_loaded_docs() const;

    const std::vector<DocInfo>& get_docs_info() const;

protected:
    void clear();

private:
    const int _index_dump_version = 1;

    std::vector<DocInfo> docs_info;

    std::mutex freq_dict_access;
    std::map<std::string, WordIndex> freq_dictionary;

    void flush_doc_index(size_t doc_id);
    void merge_dict(std::map<std::string, WordIndex>& destination, std::map<std::string, WordIndex>& source);

    void count_word(std::string& word, size_t doc_id);
    void count_word(std::map<std::string, WordIndex>& dict, std::string &word, size_t doc_id);

    template<typename StreamT>
    void index_doc(size_t doc_id, StreamT& stream);

    template<typename StreamT>
    void index_doc2(size_t doc_id, StreamT& stream);

    int dump_num_size() const;

    template<typename uint>
    void internal_dump_index(std::ofstream& output) const;

    template<typename uint>
    bool internal_load_index(std::ifstream& input);
};


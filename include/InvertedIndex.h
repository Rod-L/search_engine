#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <ctime>
#include <thread>
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
    enum IndexationMethod {
        SeparatedAccess,
        IndependentDicts
    };

    int max_indexation_threads = 4;
    IndexationMethod indexation_method = SeparatedAccess;

    InvertedIndex() = default;
    InvertedIndex(const InvertedIndex& other);
    ~InvertedIndex();

    /**
    * Extends index database by files
    * @param docs_by_id map, where key is Id of the doc, value is filepath or URL.\n
    * If Id is already claimed by other document, existing index will be flushed and replaced by newly generated one.\n
    * If Id is greater, than border of docs_info array, docs_info will be resized to fit new Id.
    * @param wait_for_completion if true, indexation threads will be joined before exiting function
    */
    void extend_document_base(const std::map<size_t,std::string>& docs_by_id, bool wait_for_completion = true);

    /**
    * Fill docs_info by files and index them. Index base will be completely cleared beforehand.
    * @param input_docs list of filenames
    * @param wait_for_completion if true, indexation threads will be joined before exiting function
    */
    void update_document_base(const std::vector<std::string>& input_docs, bool wait_for_completion = true);

    /**
    * Fill docs_info by texts and index them. Index base will be completely cleared beforehand.
    * @param input_texts list of texts to index
    */
    void update_text_base(const std::vector<std::string>& input_texts);

    /**
    * Add and index document.\n
    * If index base already contains data by passed doc_id, it won't be cleared automatically!
    * Existing index should be cleared manually by "void clear_doc_index(size_t doc_id)", if necessary.
    * @param doc_id selected Id for the document.
    * @param filename document filename
    * @return true if file processed successfully
    */
    bool add_document(size_t doc_id, const std::string& filename);

    /**
    * Add and index document
    * If index base already contains data by passed doc_id, it won't be cleared automatically!
    * Existing index should be cleared manually by "void clear_doc_index(size_t doc_id)", if necessary.
    * @param doc_id selected Id for the document.
    * @param text content of the document
    * @return true if text processed successfully
    */
    bool add_text(size_t doc_id, const std::string& text);

    /**
    * @param word - word to look for in database
    * @return list of entries {doc_id, count}
    */
    std::vector<Entry> get_word_count(const std::string& word);

    /**Dumps index to given ostream for future save or inspect.*/
    void dump_index(std::ofstream& output);

    /**
    * Attempts to load index from given istream.
    * @param input - std::ifstream - opened file stream to load index from.
    * @return true if index loaded successfully.
    * */
    bool load_index(std::ifstream& input);

    /** @return true if all docs from input_docs present in database */
    bool docs_loaded(const std::vector<std::string>& input_docs) const;

    /**
    * @return constant reference to underlying docs_info vector.
    */
    const std::vector<DocInfo>& get_docs_info() const;

    /** removes documents of passed indexes from index base.\n
    * Gaps in index line closed by shifting documents with greater Id to freed spots (void rebase_doc_index(...) used).
    */
    void remove_documents(const std::vector<size_t>& ids);

    /**
    * Clears index of document by passed Id, does not remove entry from docs_info.
    * @param doc_id Id of the document, index of which should be removed.
    */
    void flush_doc_index(size_t doc_id);

    /**
    * Takes line, returns list of its parts, where each part is sequence of latin letters / digits / symbols with code > 127
    * @param word string to split
    * @param min_part_size minimum size of part (sequences of size less than value are not added to returned vector)
    * @return vector of strings, where each string is part of initial one.
    */
    static std::vector<std::string> split_by_non_letters(std::string& word, int min_part_size = 2);

protected:
    /** removes all entries from docs_info and clears index database */
    void clear();

private:
    bool interrupt_indexation = false;
    static const int _index_dump_version = 2;

    std::mutex docs_access;
    std::vector<DocInfo> docs_info;

    std::mutex freq_dict_access;
    std::map<std::string, WordIndex> freq_dictionary;

    bool indexation_handler_active = false;
    std::vector<size_t> indexation_queue;
    void indexation_queue_handler();
    std::thread* indexation_handler_thread = nullptr;

    int indexation_in_progress() const;
    void wait_for_indexation(bool interrupt = false);

    void rebase_doc_index(size_t old_id, size_t new_id);
    void merge_dict(std::map<std::string, WordIndex>& destination, std::map<std::string, WordIndex>& source);

    void count_word(std::string& word, size_t doc_id);
    void count_word(std::map<std::string, WordIndex>& dict, std::string& word, size_t doc_id);

    template<typename StreamT>
    void index_doc(size_t doc_id, StreamT& stream);
    template<typename StreamT>
    void index_doc(std::map<std::string, WordIndex>& dict, size_t doc_id, StreamT& stream);
    template<typename StreamT>
    void independent_index_doc(size_t doc_id, StreamT& stream);
    void report_indexation_finish(const std::string& filename, const DocInfo& doc_info);

    int dump_num_size() const;

    template<typename uint>
    void internal_dump_index(std::ofstream& output) const;

    template<typename uint>
    bool internal_load_index(std::ifstream& input);
};


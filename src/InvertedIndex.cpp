#include "InvertedIndex.h"
#include "PathHelper.h"
#include "GlobalLocks.h"

//// Entry struct

bool Entry::operator==(const Entry& other) const {
    return (doc_id == other.doc_id && count == other.count);
}

//// WordIndex struct

WordIndex::WordIndex(const WordIndex& other) {
    index = other.index;
};

//// DocInfo struct


//// InvertedIndex class

InvertedIndex::InvertedIndex(const InvertedIndex& other) {
    freq_dictionary = other.freq_dictionary;
}

const std::vector<DocInfo> &InvertedIndex::get_docs_info() const {
    return docs_info;
}

bool InvertedIndex::docs_loaded(const std::vector<std::string>& input_docs) const {
    auto size = input_docs.size();
    if (docs_info.size() != size) return false;
    for (int i = 0; i < size; ++i) {
        if (docs_info[i].filepath != input_docs[i]) return false;
    }
    return true;
}

void InvertedIndex::extend_document_base(const std::vector<std::string>& input_docs, int max_threads) {
    std::map<size_t,std::pair<std::string,std::thread*>> docs_by_id;

    GlobalLocks::cout.lock();

    if (docs_info.empty()) {
        auto size = input_docs.size();
        for (int i = 0; i < size; ++i) docs_by_id[i] = {input_docs[i], nullptr};
        docs_info.resize(size);
    } else {
        size_t next_doc_id = docs_info.size();
        for (auto &doc: input_docs) {
            size_t doc_id;
            for (doc_id = 0; doc_id < docs_info.size(); ++doc_id) {
                if (docs_info[doc_id].filepath == doc) break;
            }

            if (doc_id < docs_info.size()) {
                if (docs_info[doc_id].indexing_in_progress) {
                    std::cout << "Indexing of document '" << doc << "' is already in progress." << std::endl;
                    continue;
                }
                flush_doc_index(doc_id);
                docs_by_id[doc_id] = {doc, nullptr};
            } else {
                docs_by_id[next_doc_id] = {doc, nullptr};
                ++next_doc_id;
            }
        }
        docs_info.resize(next_doc_id);
    }

    if (docs_by_id.empty()) {
        GlobalLocks::cout.unlock();
        return;
    }

    std::vector<std::thread> threads;
    threads.reserve(input_docs.size());
    std::cout << "Starting indexation of " << docs_by_id.size() << " files." << std::endl;

    GlobalLocks::cout.unlock();

    auto max_threads_reached = [this, &docs_by_id, max_threads](){
        int count = 0;
        for (auto& pair : docs_by_id) {
            if (docs_info[pair.first].indexing_in_progress) ++count;
            if (count >= max_threads) {
                return true;
            }
        }
        return false;
    };

    for (auto& pair : docs_by_id) {
        auto& doc_info = docs_info[pair.first];
        doc_info.indexed = false;
        doc_info.indexing_in_progress = true;

        threads.emplace_back(&InvertedIndex::add_document, this, pair.first, pair.second.first);
        pair.second.second = &threads.back();
        if (max_threads_reached()) threads.back().join();
    }

    int loaded_count = 0;
    for (auto& pair : docs_by_id) {
        auto thread_ptr = pair.second.second;
        if (thread_ptr->joinable()) thread_ptr->join();
        if (!docs_info[pair.first].indexing_error) ++loaded_count;
    }

    int total_docs = 0;
    for (auto& doc_info : docs_info) {
        if (doc_info.indexed) ++total_docs;
    }

    GlobalLocks::cout.lock();
    std::cout << "Indexation finished, documents loaded: " << loaded_count
              << " (total documents in database: " << total_docs << ")" << std::endl;
    GlobalLocks::cout.unlock();
}

void InvertedIndex::update_document_base(const std::vector<std::string>& input_docs, int max_threads) {
    for (auto& doc_info : docs_info) {
        if (doc_info.indexing_in_progress) {
            GlobalLocks::cout.lock();
            std::cout << "Active indexation tasks detected, full update aborted.\n"
                         "Wait until all current tasks are finished and repeat command." << std::endl;
            GlobalLocks::cout.unlock();
            return;
        }
    }

    clear();
    extend_document_base(input_docs, max_threads);
}

void InvertedIndex::update_text_base(const std::vector<std::string>& input_texts) {
    clear();
    docs_info.resize(input_texts.size());

    std::vector<std::thread> threads;
    threads.reserve(input_texts.size());

    for (int i = 0; i < input_texts.size(); ++i) {
        auto& doc_info = docs_info[i];
        doc_info.filepath = std::to_string(i);
        threads.emplace_back(&InvertedIndex::add_text, this, static_cast<size_t>(i), input_texts[i]);

        doc_info.indexed = false;
        doc_info.indexing_in_progress = true;
    }

    for (auto& thread : threads) {
        if (thread.joinable()) thread.join();
    }
    std::cout << "Database updated, texts loaded: " << docs_info.size() << std::endl;
}

void InvertedIndex::report_indexation_finish(const std::string& filename, const DocInfo& doc_info) {
    GlobalLocks::cout.lock();
    std::cout << "'" << filename << "' processed:";
    if (doc_info.indexing_error) {
        std::cout << std::endl << doc_info.error_text << std::endl;
    } else {
        std::cout << " success." << std::endl;
    }
    GlobalLocks::cout.unlock();
}

bool InvertedIndex::add_document(size_t doc_id, const std::string& filename) {
    auto& doc_info = docs_info[doc_id];
    doc_info.filepath = filename;
    doc_info.error_text.clear();
    std::stringstream error_str;

    auto set_info = [&doc_info](bool indexed, bool in_progress, bool error) {
        doc_info.indexed = indexed;
        doc_info.indexing_error = error;
        doc_info.indexing_in_progress = in_progress;
        doc_info.index_date = std::time(nullptr);
    };

    set_info(false, true, false);

    std::stringstream content;
    bool have_text;

    if (HTTPFetcher::is_url(filename)) {
        std::string html;
        have_text = HTTPFetcher::get_html(filename, html);
        if (have_text) HTTPFetcher::get_text(html, content);
    } else {
        have_text = PathHelper::get_text(filename, content);
    }

    if (have_text) {
        index_doc(doc_id, content);
    } else {
        doc_info.indexing_error = true;
        error_str << "add_document failed: could not fetch text from '" << filename << "'.";
    }

    if (doc_info.indexing_error) {
        doc_info.error_text = error_str.str();
        set_info(false, false, true);
    } else {
        set_info(true, false, false);
    }

    report_indexation_finish(filename, doc_info);
    return !doc_info.indexing_error;
}

bool InvertedIndex::add_text(size_t doc_id, const std::string& text) {
    auto& doc_info = docs_info[doc_id];
    doc_info.indexing_in_progress = true;

    std::stringstream content(text);
    index_doc2(doc_id, content);

    doc_info.error_text.clear();
    doc_info.indexing_error = false;
    doc_info.indexing_in_progress = false;
    doc_info.indexed = true;
    doc_info.index_date = std::time(nullptr);
    return true;
}

std::vector<Entry> InvertedIndex::get_word_count(const std::string& word) {
    std::vector<Entry> result;
    freq_dict_access.lock();
    auto entries = freq_dictionary.find(word);
    freq_dict_access.unlock();

    if (entries == freq_dictionary.end()) return result;

    entries->second.access.lock();
    result.reserve(entries->second.index.size());
    for (auto& pair : entries->second.index) result.push_back(pair.second);
    entries->second.access.unlock();

    return result;
}

//// Indexing

void InvertedIndex::clear() {
    freq_dictionary.clear();
    docs_info.clear();
}

void InvertedIndex::flush_doc_index(size_t doc_id) {
    for (auto& word_index : freq_dictionary) {
        word_index.second.index.erase(doc_id);
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

        if (is_letter(symbol) || isdigit(symbol)) {
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
}

//// Indexing blocking method

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

//// Indexing independent dicts method

void InvertedIndex::merge_dict(std::map<std::string, WordIndex>& destination, std::map<std::string, WordIndex>& source) {
    for (auto& word_index : source) {
        auto& word = word_index.first;
        auto& index = word_index.second.index;
        auto& dest_index = destination[word].index;
        for (auto& entry : index) {
            dest_index[entry.first] = entry.second;
        }
    }
}

void InvertedIndex::count_word(std::map<std::string, WordIndex>& dict, std::string &word, size_t doc_id) {
    auto& entries = dict[word].index;
    auto pair = entries.find(doc_id);
    if (pair == entries.end()) {
        entries[doc_id] = Entry{static_cast<size_t>(doc_id), 1};
    } else {
        pair->second.count += 1;
    }
}

template<typename StreamT>
void InvertedIndex::index_doc2(size_t doc_id, StreamT& stream) {
    std::map<std::string, WordIndex> dict;
    std::string word;
    while (stream >> word) {
        count_word(word, doc_id);
        auto parts = InvertedIndex::split_by_non_letters(word);
        if (!parts.empty() && parts[0] != word) {
            for (auto& part : parts) count_word(dict, part, doc_id);
        }
    }

    freq_dict_access.lock();
    merge_dict(freq_dictionary, dict);
    freq_dict_access.unlock();
}

//// Save/load to/from file

#define PUT_NUM(STREAM, TYPENAME, VAL) {TYPENAME buf = static_cast<TYPENAME>(VAL); STREAM.write((char*)(&buf), sizeof(TYPENAME));};

int InvertedIndex::dump_num_size() const {
    size_t n = docs_info.size();
    for (auto& word_index : freq_dictionary) {
        for (auto& entry : word_index.second.index) n = std::max(n, entry.second.count);
    }

    if (n > 0xFFFFFFFF) return 8;
    if (n > 0xFFFF) return 4;
    if (n > 0xFF) return 2;
    return 1;
}

void InvertedIndex::dump_index(std::ofstream& output) const {
    output << "SearchEngine" << '\0';
    PUT_NUM(output, int, _index_dump_version);

    int n_size = dump_num_size();
    PUT_NUM(output, uint8_t, n_size);
    switch (n_size) {
        case 8: internal_dump_index<uint64_t>(output); break;
        case 4: internal_dump_index<uint32_t>(output); break;
        case 2: internal_dump_index<uint16_t>(output); break;
        case 1: internal_dump_index<uint8_t>(output); break;
        default: throw std::runtime_error("Undefined type of numerical size for index dump.");
    }
}

template<typename uint>
void InvertedIndex::internal_dump_index(std::ofstream& output) const {
    PUT_NUM(output, uint, docs_info.size());

    for (auto& doc_info : docs_info) output << doc_info.filepath << '\0';

    PUT_NUM(output, uint, freq_dictionary.size());
    for (auto& word_index : freq_dictionary) {
        output << word_index.first << '\0';
        PUT_NUM(output, uint, word_index.second.index.size());

        for (auto& entry : word_index.second.index) {
            PUT_NUM(output, uint, entry.second.doc_id);
            PUT_NUM(output, uint, entry.second.count);
        }
    }
}

#define READ_NUM_TO(STREAM, TYPENAME, BUF) STREAM.read((char*)(&BUF), sizeof(TYPENAME));
#define READ_NUM(STREAM, TYPENAME, BUF_NAME) TYPENAME BUF_NAME; READ_NUM_TO(STREAM, TYPENAME, BUF_NAME);

bool InvertedIndex::load_index(std::ifstream& input) {
    std::string head;
    std::getline(input, head, '\0');

    if (head != "SearchEngine") {
        std::cerr << "Could not load previously saved index: index file is corrupted." << std::endl;
        return false;
    }

    READ_NUM(input, int, dump_version);
    if (static_cast<int>(dump_version) != _index_dump_version) {
        std::cerr << "Could not load previously saved index: versions of binary dump do not match." << std::endl;
        return false;
    }

    READ_NUM(input, uint8_t, n_size);
    switch (n_size) {
        case 8: return internal_load_index<uint64_t>(input);
        case 4: return internal_load_index<uint32_t>(input);
        case 2: return internal_load_index<uint16_t>(input);
        case 1: return internal_load_index<uint8_t>(input);
        default: std::cerr << "Could not load previously saved index: Undefined type of numerical size for index load." << std::endl;
    }
    return false;
}

template<typename uint>
bool InvertedIndex::internal_load_index(std::ifstream& input) {

    READ_NUM(input, uint, docs_amount);
    docs_info.resize(docs_amount);

    for (uint i = 0; i < docs_amount; ++i) {
        docs_info[i].indexed = true;
        std::getline(input, docs_info[i].filepath, '\0');
    }

    READ_NUM(input, uint, dict_size);

    for (uint i = 0; i < dict_size; ++i) {
        std::string word;
        std::getline(input, word, '\0');
        auto& word_index = freq_dictionary[word];

        READ_NUM(input, uint, docs_count);
        for (uint j = 0; j < docs_count; ++j) {
            READ_NUM(input, uint, doc_id);
            READ_NUM(input, uint, count);

            if (doc_id > docs_amount) {
                clear();
                std::cerr << "Could not load previously saved index: index file is corrupted: incorrect document id detected." << std::endl;
                return false;
            }

            auto& entry = word_index.index[static_cast<size_t>(doc_id)];
            entry.doc_id = static_cast<size_t>(doc_id);
            entry.count = static_cast<size_t>(count);
        }
    }

    return true;
}





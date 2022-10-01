#include "InvertedIndex.h"
#include "HTTPTextFetcher.h"

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

const std::vector<std::string>& InvertedIndex::get_loaded_docs() const  {
    return docs;
}

bool InvertedIndex::docs_loaded(const std::vector<std::string>& input_docs) const {
    return docs == input_docs;
}

void InvertedIndex::extend_document_base(const std::vector<std::string>& input_docs) {
    std::map<size_t,std::pair<std::string,std::thread*>> docs_by_id;
    size_t next_doc_id = docs.size();

    for (auto& doc : input_docs) {
        size_t doc_id;
        for (doc_id = 0; doc_id < docs.size(); ++doc_id) {
            if (docs[doc_id] == doc) break;
        }

        if (doc_id < docs.size()) {
            flush_doc_index(doc_id);
            docs_by_id[doc_id] = {doc, nullptr};
        } else {
            docs_by_id[next_doc_id] = {doc, nullptr};
            ++next_doc_id;
        }
    }

    docs.resize(next_doc_id);
    std::vector<std::thread> threads;
    threads.reserve(input_docs.size());

    std::vector<std::string> loading_errors;
    loading_errors.resize(docs.size());
    for (auto& pair : docs_by_id) {
        threads.emplace_back(&InvertedIndex::add_document, this, pair.first, pair.second.first, std::ref(loading_errors));
        pair.second.second = &threads.back();
    }

    for (auto& pair : docs_by_id) {
        std::cout << "Processing doc #" << pair.first << ": " << pair.second.first << std::endl;
        auto thread_ptr = pair.second.second;
        if (thread_ptr->joinable()) thread_ptr->join();

        std::string& err_str = loading_errors[pair.first];
        std::cout << (err_str.empty() ? "Success" : err_str) << std::endl;
    }
}

void InvertedIndex::update_document_base(const std::vector<std::string>& input_docs) {
    clear();
    docs.resize(input_docs.size());

    std::vector<std::string> loading_errors;
    loading_errors.resize(input_docs.size());

    std::vector<std::thread> threads;
    threads.reserve(input_docs.size());

    for (size_t doc_id = 0; doc_id < input_docs.size(); ++doc_id) {
        threads.emplace_back(&InvertedIndex::add_document, this, doc_id, input_docs[doc_id], std::ref(loading_errors));
    }

    int loading_id = 0;
    int loaded_count = 0;
    for (auto& thread : threads) {
        std::cout << "Processing doc #" << loading_id << ": " << input_docs[loading_id] << std::endl;
        if (thread.joinable()) thread.join();
        if (!loading_errors[loading_id].empty()) {
            std::cout << loading_errors[loading_id] << std::endl;
        } else {
            ++loaded_count;
        }
        ++loading_id;
    }
    std::cout << "Database updated, documents loaded: " << loaded_count << std::endl;
}

void InvertedIndex::update_text_base(const std::vector<std::string>& input_texts) {
    clear();
    docs.reserve(input_texts.size());

    std::vector<std::thread> threads;
    threads.reserve(input_texts.size());

    for (int i = 0; i < input_texts.size(); ++i) {
        auto doc = std::to_string(i);
        docs.push_back(doc);
        threads.emplace_back(&InvertedIndex::add_text, this, static_cast<size_t>(i), input_texts[i]);
    }

    for (auto& thread : threads) {
        if (thread.joinable()) thread.join();
    }
    std::cout << "Database updated, texts loaded: " << docs.size() << std::endl;
}

bool InvertedIndex::add_document(size_t doc_id, const std::string& filename, std::vector<std::string>& loading_errors) {

    if (HTTPFetcher::is_url(filename)) {
        auto text = HTTPFetcher::get_text(filename);
        if (text.empty()) {
            std::stringstream error_str;
            error_str << "add_document failed: could not fetch content from url '" << filename << "'.";
            loading_errors[doc_id] = error_str.str();
            return false;
        }
        std::stringstream content(text);
        index_doc(doc_id, content);

    } else {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::stringstream error_str;
            error_str << "add_document failed: could not open file '" << filename << "'.";
            loading_errors[doc_id] = error_str.str();
            return false;
        }

        index_doc(doc_id, file);
        file.close();
    }

    docs_access.lock();
    docs[doc_id] = filename;
    docs_access.unlock();
    return true;
}

bool InvertedIndex::add_text(size_t doc_id, const std::string& text) {
    std::stringstream content(text);
    index_doc2(doc_id, content);
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

//// Indexing

void InvertedIndex::clear() {
    freq_dictionary.clear();
    docs.clear();
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
    size_t n = docs.size();
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
    PUT_NUM(output, uint, docs.size());

    for (auto& doc : docs) output << doc << '\0';

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
    docs.resize(docs_amount);

    for (uint i = 0; i < docs_amount; ++i) std::getline(input, docs[i], '\0');

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





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

//// InvertedIndex class

InvertedIndex::InvertedIndex(const InvertedIndex& other) {
    wait_for_indexation(true);
    freq_dictionary = other.freq_dictionary;
}

InvertedIndex::~InvertedIndex() {
    if (indexation_handler_thread != nullptr) {
        interrupt_indexation = true;
        if (indexation_handler_thread->joinable()) indexation_handler_thread->join();
        delete indexation_handler_thread;
    }
};

const std::vector<DocInfo> &InvertedIndex::get_docs_info() const {
    return docs_info;
}

bool InvertedIndex::docs_loaded(const std::vector<std::string>& input_docs) const {
    auto size = docs_info.size();
    for (int i = 0; i < size; ++i) {
        auto& filepath = docs_info[i].filepath;
        if (!filepath.empty() && filepath != input_docs[i]) return false;
    }
    return true;
}

int InvertedIndex::indexation_in_progress() const {
    int amount = 0;
    for (auto& doc_info : docs_info) {
        if (doc_info.indexing_in_progress) return ++amount;
    }
    return amount;
}

void InvertedIndex::wait_for_indexation(bool interrupt) {
    interrupt_indexation = interrupt;
    if (interrupt && indexation_in_progress()) {
        std::cout << "Indexation in progress. Interrupting..." << std::endl;
    }

    if (indexation_handler_thread != nullptr) {
        if (indexation_handler_thread->joinable()) indexation_handler_thread->join();
        delete indexation_handler_thread;
        indexation_handler_thread = nullptr;
    }

    assert(indexation_in_progress() == 0);
    interrupt_indexation = false;
}

void InvertedIndex::indexation_queue_handler() {
    indexation_handler_active = true;
    std::cout << "Indexation handler started." << std::endl;

    std::map<size_t, std::thread*> indexation_threads;

    while (!indexation_queue.empty() || !indexation_threads.empty()) {
        int available = max_indexation_threads - indexation_in_progress();
        for (; available > 0 && !indexation_queue.empty(); --available) {
            size_t doc_id = indexation_queue.back();
            indexation_queue.pop_back();
            auto& doc_info = docs_info[doc_id];
            if (doc_info.indexing_in_progress) {
                ++available;
                continue;
            }
            doc_info.indexing_in_progress = true;
            indexation_threads[doc_id] = new std::thread(&InvertedIndex::add_document, this, doc_id, doc_info.filepath);
        }

        if (!indexation_threads.empty()) {
            auto front = indexation_threads.begin();
            if (front->second->joinable()) {
                front->second->join();
                delete front->second;
            }
            indexation_threads.erase(front);
        }
    }

    indexation_handler_active = false;
}

void InvertedIndex::extend_document_base(const std::map<size_t,std::string>& docs_by_id, bool wait_for_completion) {
    docs_access.lock();
    size_t max_id = 0;
    for (auto& pair : docs_by_id) {
        auto doc_id = pair.first;
        if (doc_id < docs_info.size()) flush_doc_index(doc_id);
        max_id = std::max(max_id, doc_id);
    }
    if (docs_info.size() <= max_id) docs_info.resize(max_id + 1);
    docs_access.unlock();

    GlobalLocks::cout.lock();
    std::cout << "Starting indexation of " << docs_by_id.size() << " files." << std::endl;
    GlobalLocks::cout.unlock();

    for (auto& pair : docs_by_id) {
        auto& doc_info = docs_info[pair.first];
        if (doc_info.indexing_in_progress) continue;
        doc_info.filepath = pair.second;
        doc_info.indexed = false;
        doc_info.awaits_indexation = true;
        indexation_queue.push_back(pair.first);
    }

    if (!indexation_handler_active) {
        std::cout << "Starting handler..." << std::endl;
        if (indexation_handler_thread != nullptr) {
            if (indexation_handler_thread->joinable()) indexation_handler_thread->join();
            delete indexation_handler_thread;
            indexation_handler_thread = nullptr;
        }
        indexation_handler_thread = new std::thread(&InvertedIndex::indexation_queue_handler, this);
    }

    if (wait_for_completion) {
        std::cout << "Waiting for completion..." << std::endl;
        wait_for_indexation(false);
        std::cout << "Indexation completed." << std::endl;
    }
}

void InvertedIndex::update_document_base(const std::vector<std::string>& input_docs, bool wait_for_completion) {
    wait_for_indexation(true);

    std::map<size_t,std::string> docs_by_id;
    for (int i = 0; i < input_docs.size(); ++i) docs_by_id[i] = input_docs[i];

    clear();
    extend_document_base(docs_by_id, wait_for_completion);
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
    docs_access.lock();
    DocInfo doc_info = docs_info[doc_id];
    docs_access.unlock();

    doc_info.filepath = filename;
    doc_info.error_text.clear();
    std::stringstream error_str;

    auto set_info = [&doc_info](bool indexed, bool in_progress, bool error) {
        doc_info.indexed = indexed;
        doc_info.indexing_in_progress = in_progress;
        doc_info.indexing_error = error;
        doc_info.index_date = std::time(nullptr);
    };

    set_info(false, true, false);
    doc_info.awaits_indexation = false;

    std::stringstream content;
    bool have_text;

    if (HTTPFetcher::is_url(filename)) {
        std::string html;
        have_text = HTTPFetcher::get_html(filename, html);
        if (have_text) HTTPFetcher::get_text(html, content);
    } else {
        have_text = PathHelper::get_text(filename, content);
        if (have_text && HTTPFetcher::is_html(filename)) {
            HTTPFetcher::get_text(content.str(), content);
        }
    }

    if (have_text) {
        indexation_method == IndependentDicts ? independent_index_doc(doc_id, content) : index_doc(doc_id, content);
        if (interrupt_indexation) {
            flush_doc_index(doc_id);
            doc_info.indexing_error = true;
            error_str << "Indexation interrupted by user.";
        };
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

    docs_access.lock();
    docs_info[doc_id] = doc_info;
    docs_access.unlock();
    report_indexation_finish(filename, doc_info);
    return !doc_info.indexing_error;
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

//// Indexation methods

void InvertedIndex::clear() {
    if (!indexation_queue.empty()) {
        std::cout << "Indexation tasks in progress detected, waiting for completion..." << std::endl;
        if (indexation_handler_thread->joinable()) indexation_handler_thread->join();
        std::cout << "Done." << std::endl;
    }

    freq_dict_access.lock();
    docs_access.lock();

    freq_dictionary.clear();
    docs_info.clear();

    freq_dict_access.unlock();
    docs_access.unlock();
}

void InvertedIndex::remove_documents(const std::vector<size_t> &ids) {
    wait_for_indexation();

    std::map<size_t,bool> remove_necessary;
    for (auto val : ids) remove_necessary[val] = true;

    int shift_back = 0;
    int new_count = 0;
    for (size_t i = 0; i < docs_info.size(); ++i) {
        if (remove_necessary[i]) {
            flush_doc_index(i);
            ++shift_back;
            continue;
        }

        ++new_count;
        if (shift_back > 0) {
            size_t new_id = i - shift_back;
            docs_info[new_id] = docs_info[i];
            rebase_doc_index(i, new_id);
        }
    }
    docs_info.resize(new_count);
}

void InvertedIndex::rebase_doc_index(size_t old_id, size_t new_id) {
    freq_dict_access.lock();
    for (auto& word_index : freq_dictionary) {
        auto& index = word_index.second.index;
        auto old = index.find(old_id);
        if (old == index.end()) continue;
        old->second.doc_id = new_id;
        index[new_id] = old->second;
        index.erase(old);
    }
    freq_dict_access.unlock();
}

void InvertedIndex::flush_doc_index(size_t doc_id) {
    freq_dict_access.lock();
    for (auto& word_index : freq_dictionary) {
        auto& item = word_index.second;
        item.access.lock();
        item.index.erase(doc_id);
        item.access.unlock();
    }
    freq_dict_access.unlock();
}

std::vector<std::string> InvertedIndex::split_by_non_letters(std::string& word, int min_part_size) {
    auto is_letter = [](unsigned char v){
        return v >= 'a' && v <= 'z' || v >= 'A' && v <= 'Z' || v > 127;
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

//// Indexation separate access method

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
        auto parts = InvertedIndex::split_by_non_letters(word);
        if (parts.empty()) {
            count_word(word, doc_id);
        } else {
            for (auto& part : parts) count_word(part, doc_id);
        }
        if (interrupt_indexation) return;
    }
}

//// Indexation independent dicts method

void InvertedIndex::merge_dict(std::map<std::string, WordIndex>& destination, std::map<std::string, WordIndex>& source) {
    for (auto& word_index : source) {
        word_index.second.access.lock();
        auto& word = word_index.first;
        auto& index = word_index.second.index;
        auto& dest_index = destination[word].index;
        for (auto& entry : index) {
            dest_index[entry.first] = entry.second;
        }
        word_index.second.access.unlock();
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
void InvertedIndex::index_doc(std::map<std::string, WordIndex>& dict, size_t doc_id, StreamT& stream) {
    std::string word;
    while (stream >> word) {
        count_word(word, doc_id);
        auto parts = InvertedIndex::split_by_non_letters(word);
        if (!parts.empty() && parts[0] != word) {
            for (auto& part : parts) count_word(dict, part, doc_id);
        }
        if (interrupt_indexation) return;
    }
}

template<typename StreamT>
void InvertedIndex::independent_index_doc(size_t doc_id, StreamT& stream) {
    std::map<std::string, WordIndex> dict;
    index_doc(dict, doc_id, stream);
    if (interrupt_indexation) return;
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

void InvertedIndex::dump_index(std::ofstream& output) {
    wait_for_indexation();

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
    uint docs_amount = 0;
    for (auto& doc_info : docs_info) {
        if (!doc_info.filepath.empty()) ++docs_amount;
    }
    PUT_NUM(output, uint, docs_amount);

    if (docs_amount == 0) return;

    for (int i = 0; i < docs_info.size(); ++i) {
        auto& doc_info = docs_info[i];
        if (doc_info.filepath.empty()) continue;
        PUT_NUM(output, uint, i);
        PUT_NUM(output, time_t, doc_info.index_date);
        output << doc_info.filepath << '\0';
        output.flush();
    }

    PUT_NUM(output, uint, freq_dictionary.size());
    for (auto& word_index : freq_dictionary) {
        output << word_index.first << '\0';
        PUT_NUM(output, uint, word_index.second.index.size());

        for (auto& entry : word_index.second.index) {
            PUT_NUM(output, uint, entry.second.doc_id);
            PUT_NUM(output, uint, entry.second.count);
        }
        output.flush();
        if (output.bad()) std::cerr << "Unknown error while dumping index." << std::endl;
    }
}

#define READ_NUM_TO(STREAM, TYPENAME, BUF) STREAM.read((char*)(&BUF), sizeof(TYPENAME));
#define READ_NUM(STREAM, TYPENAME, BUF_NAME) TYPENAME BUF_NAME; READ_NUM_TO(STREAM, TYPENAME, BUF_NAME);

bool InvertedIndex::load_index(std::ifstream& input) {
    wait_for_indexation(true);

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

    for (uint i = 0; i < docs_amount; ++i) {
        READ_NUM(input, uint, doc_id);
        if (docs_info.size() < doc_id + 1) docs_info.resize(doc_id + 1);
        auto& doc_info = docs_info[doc_id];
        READ_NUM_TO(input, time_t, doc_info.index_date);
        std::getline(input, doc_info.filepath, '\0');
        doc_info.indexed = true;
    }

    READ_NUM(input, uint, dict_size);

    docs_amount = docs_info.size();
    for (uint i = 0; i < dict_size; ++i) {
        std::string word;
        std::getline(input, word, '\0');
        auto& word_index = freq_dictionary[word];

        READ_NUM(input, uint, docs_count);
        for (uint j = 0; j < docs_count; ++j) {
            READ_NUM(input, uint, doc_id);
            READ_NUM(input, uint, count);

            if (doc_id >= docs_amount) {
                clear();
                std::cerr << "Could not load previously saved index: index file is corrupted: incorrect document id detected." << std::endl;
                std::cerr << "Amount of documents: " << docs_amount << ", id found: " << doc_id << std::endl;
                return false;
            }

            auto& entry = word_index.index[static_cast<size_t>(doc_id)];
            entry.doc_id = static_cast<size_t>(doc_id);
            entry.count = static_cast<size_t>(count);
        }
    }

    return true;
}





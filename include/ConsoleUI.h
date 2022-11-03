#pragma once

#include <thread>

#include "ConverterJSON.h"
#include "SearchEngine.h"
#include "PathHelper.h"

struct ConsoleCommand {
    int code;
    std::string name;
    std::string description;

    void (*func)(std::string&);
};

namespace ConsoleUI {
    ConverterJSON converter;
    SearchServer server;

    void show_help(std::string& params);
    bool load_backed_index();
    void reload_config(const std::string& filepath);
    void form_index();
    void dump_index();
    void after_indexation();
    void process_requests(std::string &filepath);
    std::string index_path(const std::string& config_path);
    std::string service_file_path(const std::string& name);
    void reindex_files();
    std::vector<size_t> parse_ids(std::string& str_ids);
    void init_commands(std::map<std::string, void (*)(std::string&)> &map);
}

void ConsoleUI::after_indexation() {
    auto& docs_info = ConsoleUI::server.get_docs_info();

    int config_total = converter.get_text_documents().size();
    int base_total = docs_info.size();
    int indexed_total = 0;
    int index_errors = 0;
    for (auto& doc_info : docs_info) {
        if (doc_info.indexed) ++indexed_total;
        if (doc_info.indexing_error) ++index_errors;
    }

    std::cout << "Indexation have been completed." << std::endl
        << " Documents listed: " << config_total
        << ", documents loaded: " << base_total
        << ", documents indexed: " << indexed_total
        << ", errors: " << index_errors << std::endl;

    if (index_errors > 0) std::cout << "Use 'BaseStatus' command to generate 'status.json' file and inspect indexation errors." << std::endl;

    if (converter.auto_dump_index) dump_index();
    else std::cout << "Auto dump disabled. Dump index manually, using 'DumpIndex' command, if necessary." << std::endl;
}

bool ConsoleUI::load_backed_index() {
    std::string index_path = ConsoleUI::index_path(ConsoleUI::converter.get_config_path());
    std::ifstream file(index_path, std::ios::binary);

    if (!file.is_open()) {
        std::cerr << "Could not open file '" << index_path << "'." << std::endl;
        return false;
    }

    ConsoleUI::server.load_index(file);
    file.close();
    if (ConsoleUI::server.docs_loaded(ConsoleUI::converter.get_text_documents())) {
        std::cout << "Index have been loaded from '" << index_path << "'." << std::endl;
        return true;
    }

    ConsoleUI::server.clear_index();
    std::cout << "Index from '" << index_path << "' does not match files listed in configuration file." << std::endl;
    return false;
}

std::string ConsoleUI::service_file_path(const std::string& name) {
    std::stringstream path;
    path << ConsoleUI::converter.get_config_catalog();
    path << ConsoleUI::converter.get_config_name();
    path << "." << name;
    return path.str();
}

std::string ConsoleUI::index_path(const std::string& config_path) {
    std::string name = "index.bin";
    return service_file_path(name);
}

void ConsoleUI::dump_index() {
    std::string index_path = ConsoleUI::index_path(converter.get_config_path());
    std::ofstream file(index_path, std::ios::binary);
    if (file.is_open()) {
        ConsoleUI::server.dump_index(file);
        std::cout << "Index have been saved to '" << index_path << "'." << std::endl;
        file.close();
    } else {
        std::cout << "Could not open file '" << index_path << "' to dump index to." << std::endl;
    }
}

void ConsoleUI::reindex_files() {
    server.update_document_base(converter.get_text_documents(), false);
}

std::vector<size_t> ConsoleUI::parse_ids(std::string &str_ids) {
    std::stringstream params_parser(str_ids);
    std::string doc_id_str;
    std::vector<size_t> ids;
    while (params_parser >> doc_id_str) {
        try {
            size_t doc_id = std::stoll(doc_id_str);
            ids.push_back(doc_id);
        } catch(...) {
            std::cerr << "Invalid doc id '" << doc_id_str << "' passed to command." << std::endl;
        }
    }
    return ids;
}

void ConsoleUI::form_index() {
    server.clear_index();
    const std::vector<std::string>& docs = converter.get_text_documents();
    if (docs.empty()) return;

    std::string index_path = ConsoleUI::index_path(converter.get_config_path());

    bool reindex_necessary = true;

    if (PathHelper::file_exist(index_path) && !ConsoleUI::converter.auto_load_index_dump) {
        std::cout << "'" << index_path << "' detected, but dump auto load disabled. Load dump manually, using 'LoadIndex', if necessary." << std::endl;
    } else {
        reindex_necessary = !load_backed_index();
    }

    std::cout << "Documents listed: " << docs.size() << ", documents loaded into base: " << server.get_docs_info().size() << std::endl;

    if (!reindex_necessary) return;

    if (!ConsoleUI::converter.auto_reindex) {
        std::cout << "Auto reindex disabled. Start reindex manually, using 'ReindexFiles', if necessary." << std::endl;
        return;
    }

    std::cout << "Starting indexation." << std::endl;
    ConsoleUI::reindex_files();
}

void ConsoleUI::reload_config(const std::string &filepath) {
    if (converter.reload_config_file(filepath)) {
        server.max_indexation_threads = converter.max_indexation_threads;
        server.indexation_method = converter.use_independent_dicts_method ? InvertedIndex::IndependentDicts : InvertedIndex::SeparatedAccess;
        if (!filepath.empty()) form_index();
    }
}

void ConsoleUI::process_requests(std::string& filepath) {
    bool empty_filepath = filepath.empty();
    if (empty_filepath) filepath = "requests.json";

    auto requests = ConverterJSON::get_requests(filepath);
    if (requests.empty()) return;

    auto results = server.search(requests, converter.responses_limit);
    if (results.empty()) return;

    if (empty_filepath) {
        filepath = "answers.json";
        converter.put_answers(results, filepath);
    } else {
        std::string answers_path = PathHelper::catalog_from_filepath(filepath);
        answers_path.append("answers.json");
        converter.put_answers(results, answers_path);
    }
}

ConsoleCommand CONSOLE_COMMANDS[] = {
        {
                1,
                "ProcessRequests",
                "process 'requests.json' file in executable directory, results will be stored in 'answers.json' in executable directory;",
                [](std::string& params) {
                    std::string filepath;
                    ConsoleUI::process_requests(filepath);
                }
        },
        {
                2,
                "ProcessRequestsFrom",
                "process requests from file by path (taken as parameter), results will be stored in 'answers.json' in requests file directory;\n"
                "   Examples:\n"
                "   2 ../folder_name/filename.json\n"
                "   ProcessRequestsFrom C:/folder_name/filename.json",
                [](std::string& params) {
                    ConsoleUI::process_requests(params);
                }
        },
        {
                3,
                "ReloadConfig",
                "reload last loaded 'config.json';",
                [](std::string& params) {
                    ConsoleUI::reload_config("");
                }
        },
        {
                4,
                "ReloadConfigFrom",
                "reload config data from file by path (taken as parameter);\n"
                "   Examples:\n"
                "   4 filename.json\n"
                "   ReloadConfigFrom C:/filename.json",
                [](std::string& params) {
                    ConsoleUI::reload_config(params);
                }
        },
        {
                5,
                "SaveConfigTo",
                "saves current config into file by path (taken as parameter);\n"
                "   Examples:\n"
                "   5 filename.json\n"
                "   SaveConfigTo C:/filename.json",
                [](std::string& params) {
                    if (!params.empty()) ConsoleUI::converter.save_config_file(params);
                }
        },
        {
                6,
                "CheckFile",
                "check file by path, whether it valid file for search engine or not.\n"
                "   Examples:\n"
                "   6 filename.json\n"
                "   CheckFile C:/filename.json",
                [](std::string& params) {
                    ConverterJSON::check_json_file(params);
                }
        },
        {
                7,
                "DumpIndex",
                "dump index into file for future reload. Creates 'index.bin' in directory of current loaded configuration file.",
                [](std::string& params) {
                    ConsoleUI::dump_index();
                }
        },
        {
                8,
                "LoadIndex",
                "attempts to load index from 'index.bin' in directory of current loaded configuration file.",
                [](std::string& params) {
                    ConsoleUI::load_backed_index();
                }
        },
        {
                9,
                "ExtendIndex",
                "takes filepath to config.json as parameter, appends documents from passed file's 'files' section to current base.\n"
                "Files which already present in current base will keep their doc id, their index will be updated.\n"
                "   Examples:\n"
                "   9 filename.json\n"
                "   ExtendIndex C:/folder name/filename.json",
                [](std::string& params) {
                    std::vector<std::string> files;
                    if (!ConverterJSON::text_documents_from_json(params, files)) return;

                    size_t next_id = ConsoleUI::converter.get_text_documents().size();
                    std::map<size_t,std::string> docs_by_id;
                    for (auto file : files) {
                        docs_by_id[next_id] = file;
                        ++next_id;
                    }

                    ConsoleUI::converter.add_files(files);
                    ConsoleUI::server.extend_document_base(docs_by_id, false);
                }
        },
        {
                10,
                "ReindexFiles",
                "clear current index and reindex files from loaded configuration files.",
                [](std::string& params) {
                    ConsoleUI::reindex_files();
                }
        },
        {
                11,
                "ReindexFile",
                "takes document id as parameter (or few ids separated by spaces), attempts to reindex it.",
                [](std::string& params) {
                    auto ids = ConsoleUI::parse_ids(params);
                    if (ids.empty()) return;

                    std::map<size_t,std::string> docs_by_id;
                    auto all_files = ConsoleUI::converter.get_text_documents();
                    for (auto& doc_id : ids) {
                        if (doc_id < all_files.size()) docs_by_id[doc_id] = all_files[doc_id];
                    }

                    ConsoleUI::server.extend_document_base(docs_by_id, false);
                }
        },
        {
                12,
                "BaseStatus",
                "saves status of loaded/listed files to 'status.json' to directory of last loaded config.",
                [](std::string& params) {
                    std::string name = "status.json";
                    ConsoleUI::converter.files_status(ConsoleUI::service_file_path(name), ConsoleUI::server.get_docs_info());
                }
        },
        {
                13,
                "FileStatus",
                "takes document id as parameter (or few ids separated by spaces), saves its status to 'status_part.json' to directory of last loaded config.",
                [](std::string& params) {
                    auto ids = ConsoleUI::parse_ids(params);
                    if (ids.empty()) return;

                    std::string name = "status_part.json";
                    ConsoleUI::converter.files_status_by_ids(ConsoleUI::service_file_path(name), ConsoleUI::server.get_docs_info(), ids);
                }
        },
        {
                14,
                "AddFile",
                "takes filepath as parameter, adds it to the list of files. Does not start reindexing.",
                [](std::string& params) {
                    if (HTTPFetcher::is_http_link(params) || PathHelper::file_exist(params)) {
                        ConsoleUI::converter.add_file(params);
                        std::cout << "File '" << params << "' have been added." << std::endl;
                    } else {
                        std::cerr << "Could not open file '" << params << "'." << std::endl;
                    }
                }
        },
        {
                15,
                "RemoveFiles",
                "takes document id as parameter (or few ids separated by spaces), removes these documents from configuration file and index base.\n"
                "Does not remove files from filesystem.",
                [](std::string& params) {
                    auto ids = ConsoleUI::parse_ids(params);
                    if (ids.empty()) return;

                    ConsoleUI::converter.remove_files(ids);
                    ConsoleUI::server.remove_documents(ids);
                }
        },
        {
                16,
                "Help",
                "show this list",
                ConsoleUI::show_help
        },
};

void ConsoleUI::show_help(std::string& params) {
    std::cout << "Use numeric code or name of the command to execute it.\n"
                 "Commands are listed below in format [numeric code] - [name] - [description].\n"
                 "\n"
                 "0 - Exit - exit the program;\n"
                 "\n";

    for (auto &command: CONSOLE_COMMANDS) {
        std::cout << std::endl;
        std::cout << command.code << " - " << command.name << " - " << command.description << std::endl;
        std::cout << std::endl;
    }
}

void ConsoleUI::init_commands(std::map<std::string, void (*)(std::string&)> &map) {
    for (auto &command: CONSOLE_COMMANDS) {
        map[command.name] = command.func;
        map[std::to_string(command.code)] = command.func;
    }
}

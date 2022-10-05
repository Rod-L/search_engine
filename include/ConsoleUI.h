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
    bool file_exist(const std::string& filepath);
    bool load_backed_index(std::string& filepath);
    void reload_config(std::string &filepath);
    void form_index();
    void process_requests(std::string &filepath);
    std::string index_path(const std::string& config_path);
    std::string service_file_path(const std::string& name);
    void reindex_files();
    std::vector<size_t> parse_ids(std::string& str_ids);
    void init_commands(std::map<std::string, void (*)(std::string&)> &map);
}

bool ConsoleUI::file_exist(const std::string& filepath) {
    std::ifstream backed_index(filepath);
    if (backed_index.is_open()) {
        backed_index.close();
        return true;
    }
    return false;
}

bool ConsoleUI::load_backed_index(std::string& filepath) {
    std::ifstream backed_index(filepath);
    if (!backed_index.is_open()) return false;

    std::cout << "'" << filepath << "' detected, loading index from file." << std::endl;
    bool loaded = ConsoleUI::server.load_index(backed_index);
    backed_index.close();
    return loaded;
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

void ConsoleUI::reindex_files() {
    server.update_document_base(converter.get_text_documents(), converter.get_threads_limit());

    if (!ConsoleUI::converter.autodump_enabled()) {
        std::cout << "Auto dump disabled. Dump index manually, using 'DumpIndex' command, if necessary." << std::endl;
        return;
    }

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
    const std::vector<std::string>& docs = converter.get_text_documents();
    std::string index_path = ConsoleUI::index_path(converter.get_config_path());

    bool reindex_necessary = true;

    if (file_exist(index_path) && !ConsoleUI::converter.dump_autoload_enabled()) {
        std::cout << "'" << index_path << "' detected, but dump auto load disabled. Load dump manually, using 'LoadIndex', if necessary." << std::endl;

    } else if (load_backed_index(index_path)) {
        if (ConsoleUI::server.docs_loaded(docs)) {
            reindex_necessary = false;
        } else {
            std::cout << "Loaded index from file does not match listed files in 'config.json'." << std::endl;
            server.clear_index();
        }
    }

    if (!reindex_necessary) return;

    if (docs.empty()) return;

    if (!ConsoleUI::converter.autoreindex_enabled()) {
        std::cout << "Documents listed: " << docs.size() << ", documents loaded into base: " << server.get_docs_info().size() << std::endl;
        std::cout << "Auto reindex disabled. Start reindex manually, using 'ReindexFiles', if necessary." << std::endl;
        return;
    }

    std::cout << "Starting indexation." << std::endl;
    ConsoleUI::reindex_files();
}

void ConsoleUI::reload_config(std::string &filepath) {
    if (converter.reload_config_file(filepath)) {
        form_index();
    }
}

void ConsoleUI::process_requests(std::string& filepath) {
    bool empty_filepath = filepath.empty();
    if (empty_filepath) filepath = "requests.json";

    auto requests = ConverterJSON::get_requests(filepath);
    if (requests.empty()) return;

    auto results = server.search(requests, converter.get_responses_limit());
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
                    std::string filepath;
                    ConsoleUI::reload_config(filepath);
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
                    std::string index_path = ConsoleUI::index_path(ConsoleUI::converter.get_config_path());
                    std::ofstream file(index_path, std::ios::binary);
                    if (file.is_open()) {
                        ConsoleUI::server.dump_index(file);
                        file.close();
                        std::cout << "Index have been written to '" << index_path << "'." << std::endl;
                    } else {
                        std::cerr << "Could not open file '" << index_path << "'." << std::endl;
                    }
                }
        },
        {
                8,
                "LoadIndex",
                "attempts to load index from 'index.bin' in directory of current loaded configuration file.",
                [](std::string& params) {
                    std::string index_path = ConsoleUI::index_path(ConsoleUI::converter.get_config_path());
                    std::ifstream file(index_path);
                    if (file.is_open()) {
                        ConsoleUI::server.load_index(file);
                        file.close();
                        std::cout << "Index have been loaded from '" << index_path << "'." << std::endl;
                    } else {
                        std::cerr << "Could not open file '" << index_path << "'." << std::endl;
                    }
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
                    auto func = [](std::string filepath) {
                        std::vector<std::string> files;
                        if (!ConverterJSON::text_documents_from_json(filepath, files)) return;
                        ConsoleUI::server.extend_document_base(files, ConsoleUI::converter.get_threads_limit());
                    };
                    std::thread(func, params).detach();
                }
        },
        {
                10,
                "ReindexFiles",
                "clear current index and reindex files from loaded configuration files.",
                [](std::string& params) {
                    std::thread(&ConsoleUI::reindex_files).detach();
                }
        },
        {
                11,
                "ReindexFile",
                "takes document id as parameter (or few ids separated by spaces), attempts to reindex it.",
                [](std::string& params) {
                    auto ids = ConsoleUI::parse_ids(params);
                    if (ids.empty()) return;

                    auto func = [](std::vector<size_t> ids) {
                        std::vector<std::string> files;
                        files.reserve(ids.size());
                        auto all_files = ConsoleUI::converter.get_text_documents();
                        for (auto& doc_id : ids) {
                            if (doc_id < all_files.size()) files.push_back(all_files[doc_id]);
                        }
                        ConsoleUI::server.extend_document_base(files, ConsoleUI::converter.get_threads_limit());
                    };
                    std::thread(func, ids).detach();
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

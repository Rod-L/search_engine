#pragma once

#include "ConverterJSON.h"
#include "SearchEngine.h"

struct ConsoleCommand {
    int code;
    std::string name;
    std::string description;

    void (*func)();
};

namespace ConsoleUI {
    ConverterJSON converter;
    SearchServer server;

    void show_help();
    bool file_exist(const std::string& filepath);
    bool load_backed_index(std::string& filepath);
    void reload_config(std::string &filepath);
    void form_index();
    void process_requests(std::string &filepath);
    std::string input_filepath();
    std::string catalog_from_filepath(const std::string& filepath);
    std::string index_path(const std::string& config_path);
    void reindex_files();
    void init_commands(std::map<std::string, void (*)()> &map);
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

std::string ConsoleUI::index_path(const std::string& config_path) {
    return catalog_from_filepath(config_path) + "index.bin";
}

void ConsoleUI::reindex_files() {
    server.update_document_base(converter.get_text_documents());

    if (!ConsoleUI::converter.autodump_enabled()) {
        std::cout << "Auto dump disabled. Dump index manually, using 'DumpIndex' command, if necessary." << std::endl;
        return;
    }

    std::string index_path = ConsoleUI::index_path(converter.get_config_path());
    std::ofstream file(index_path);
    if (file.is_open()) {
        ConsoleUI::server.dump_index(file);
        std::cout << "Index have been saved to '" << index_path << "'." << std::endl;
        file.close();
    } else {
        std::cout << "Could not open file '" << index_path << "' to dump index to." << std::endl;
    }
}

void ConsoleUI::form_index() {
    const std::vector<std::string>& docs = converter.get_text_documents();
    std::string index_path = ConsoleUI::index_path(converter.get_config_path());

    bool reindex_necessary = true;

    if (file_exist(index_path) && !ConsoleUI::converter.dump_autoload_enabled()) {
        std::cout << "'" << index_path << "' detected, but dump auto load disabled. Load dump manually, using 'LoadIndex' if necessary." << std::endl;

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
        std::cout << "Documents listed: " << docs.size() << ", documets loaded into base: " << server.get_loaded_docs().size() << std::endl;
        std::cout << "Auto reindex disbaled. Start reindex manually, using 'ReindexFiles' if necessary." << std::endl;
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
    auto results = server.search(requests, converter.get_responses_limit());

    if (empty_filepath) {
        filepath = "answers.json";
        converter.put_answers(results, filepath);
    } else {
        std::string answers_path = catalog_from_filepath(filepath);
        answers_path.append("answers.json");
        converter.put_answers(results, answers_path);
    }
}

std::string ConsoleUI::catalog_from_filepath(const std::string& filepath) {
    auto slash_pos = filepath.rfind('/');
    if (slash_pos == std::string::npos) return "";
    return filepath.substr(0, slash_pos + 1);
}

std::string ConsoleUI::input_filepath() {
    std::string filepath;
    std::cin >> std::ws;
    std::getline(std::cin, filepath);
    return filepath;
}

ConsoleCommand CONSOLE_COMMANDS[] = {
        {
                1,
                "ProcessRequests",
                "process 'requests.json' file in executable directory, results will be stored in 'answers.json' in executable directory;",
                []() {
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
                []() {
                    std::string filepath = ConsoleUI::input_filepath();
                    ConsoleUI::process_requests(filepath);
                }
        },
        {
                3,
                "ReloadConfig",
                "reload last loaded 'config.json';",
                []() {
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
                []() {
                    std::string filepath = ConsoleUI::input_filepath();
                    ConsoleUI::reload_config(filepath);
                }
        },
        {
                5,
                "SaveConfigTo",
                "saves current config into file by path (taken as parameter);\n"
                "   Examples:\n"
                "   5 filename.json\n"
                "   SaveConfigTo C:/filename.json",
                []() {
                    std::string filepath = ConsoleUI::input_filepath();
                    if (!filepath.empty()) ConsoleUI::converter.save_config_file(filepath);
                }
        },
        {
                6,
                "CheckFile",
                "check file by path, whether it valid file for search engine or not.\n"
                "   Examples:\n"
                "   6 filename.json\n"
                "   CheckFile C:/filename.json",
                []() {
                    std::string filepath = ConsoleUI::input_filepath();
                    ConverterJSON::check_json_file(filepath);
                }
        },
        {
                7,
                "DumpIndex",
                "dump index into file for future reload. Creates 'index.bin' in directory of current loaded configuration file.",
                []() {
                    std::string index_path = ConsoleUI::index_path(ConsoleUI::converter.get_config_path());
                    std::ofstream file(index_path);
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
                []() {
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
                []() {
                    auto filepath = ConsoleUI::input_filepath();
                    std::vector<std::string> files;
                    if (!ConverterJSON::text_documents_from_json(filepath, files)) return;
                    ConsoleUI::server.extend_document_base(files);
                }
        },
        {
                10,
                "ReindexFiles",
                "clear current index and reindex files from loaded configuration files.",
                ConsoleUI::reindex_files
        },
        {
                11,
                "ReindexFile",
                "takes id of file from loaded configuration file as parameter, attempts to reindex it.",
                []() {
                    //TODO
                }
        },
        {
                12,
                "FilesStatus",
                "saves status of loaded/listed files to 'status.json' to directory of last loaded config.\n",
                []() {
                    auto filepath = ConsoleUI::converter.get_config_path();
                    auto catalog = ConsoleUI::catalog_from_filepath(filepath);
                    auto status_path = catalog + "status.json";
                    ConsoleUI::converter.files_status(status_path, ConsoleUI::server.get_loaded_docs());
                }
        },
        {
                13,
                "Help",
                "show this list",
                ConsoleUI::show_help
        },
};

void ConsoleUI::show_help() {
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

void ConsoleUI::init_commands(std::map<std::string, void (*)()> &map) {
    for (auto &command: CONSOLE_COMMANDS) {
        map[command.name] = command.func;
        map[std::to_string(command.code)] = command.func;
    }
}

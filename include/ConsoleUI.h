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
    bool load_backed_index(std::string& filepath);
    void reload_config(std::string &filepath);
    void form_index();
    void process_requests(std::string &filepath);
    std::string input_filepath();
    void init_commands(std::map<std::string, void (*)()> &map);
}

bool ConsoleUI::load_backed_index(std::string& filepath) {
    std::ifstream backed_index(filepath);
    if (!backed_index.is_open()) return false;

    std::cout << "'" << filepath << "' detected, loading index from file." << std::endl;
    bool loaded = ConsoleUI::server.load_index(backed_index);
    backed_index.close();
    return loaded;
}

void ConsoleUI::form_index() {
    const std::vector<std::string>& docs = converter.get_text_documents();
    std::string index_path = converter.get_config_path() + ".index.bin";

    bool reindex_necessary = true;
    if (load_backed_index(index_path)) {
        if (ConsoleUI::server.docs_loaded(docs)) {
            reindex_necessary = false;
        } else {
            std::cout << "Loaded index from file does not match listed files in 'config.json'." << std::endl;
            std::cout << "Starting reindexing. Dump index afterwards if necessary." << std::endl;
            server.clear_index();
        }
    }

    if (reindex_necessary && !docs.empty()) {
        ConsoleUI::server.update_document_base(docs);
        std::ofstream file(index_path);
        if (file.is_open()) {
            ConsoleUI::server.dump_index(file);
            file.close();
        }
    }
}

void ConsoleUI::reload_config(std::string &filepath) {
    if (converter.reload_config_file(filepath)) {
        form_index();
    }
}

void ConsoleUI::process_requests(std::string &filepath) {
    auto requests = ConverterJSON::get_requests(filepath);
    auto results = server.search(requests, converter.get_responses_limit());

    if (filepath.empty()) {
        converter.put_answers(results, filepath);
    } else {
        std::string answers_path = filepath;
        auto slash_pos = answers_path.rfind('/');
        answers_path.resize(slash_pos == std::string::npos ? 0 : slash_pos);
        answers_path.append("/answers.json");
        converter.put_answers(results, answers_path);
    }
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
                "dump index into file for future reload. Creates 'd_index.bin' in executable directory.",
                []() {
                    std::ofstream file("d_index.bin");
                    if (file.is_open()) {
                        ConsoleUI::server.dump_index(file);
                        file.close();
                        std::cout << "Index have been written to 'd_index.bin'." << std::endl;
                    } else {
                        std::cerr << "Could not open file 'd_index.bin'." << std::endl;
                    }
                }
        },
        {
                8,
                "LoadIndex",
                "attempts to load index from 'd_index.bin' in executable directory.",
                []() {
                    std::ifstream file("d_index.bin");
                    if (file.is_open()) {
                        ConsoleUI::server.load_index(file);
                        file.close();
                        std::cout << "Index have been loaded from 'd_index.bin'." << std::endl;
                    } else {
                        std::cerr << "Could not open file 'd_index.bin'." << std::endl;
                    }
                }
        },
        {
                9,
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

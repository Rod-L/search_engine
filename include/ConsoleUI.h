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
    void reload_config(std::string &filepath);
    void process_requests(std::string &filepath);
    std::string input_filepath();
}

void ConsoleUI::reload_config(std::string &filepath) {
    if (converter.reload_config_file(filepath)) {
        auto docs = converter.get_text_documents();
        if (!docs.empty()) server.update_document_base(docs);
    }
}

void ConsoleUI::process_requests(std::string &filepath) {
    auto requests = ConverterJSON::get_requests(filepath);
    auto results = server.search(requests, converter.get_responses_limit());
    converter.put_answers(results);
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
                "process requests from file by path (taken as parameter), results will be stored in 'answers.json' in executable directory;\n"
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
                "5 - SaveConfigTo - saves current config into file by path (taken as parameter);\n"
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
                "6 - CheckFile - check file by path, whether it valid file for search engine or not.\n"
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

void init_commands(std::map<std::string, void (*)()> &map) {
    for (auto &command: CONSOLE_COMMANDS) {
        map[command.name] = command.func;
        map[std::to_string(command.code)] = command.func;
    }
}

#include <iostream>

#include "ConverterJSON.h"
#include "SearchEngine.h"

ConverterJSON converter;
SearchServer server;

std::string help_str =
        "Exit - exit the program;\n"
        "\n"
        "ProcessRequests - process 'requests.json' file in executable directory, results will be stored in 'answers.json' in executable directory;\n"
        "\n"
        "ProcessRequestsFrom - process requests from file by path (taken as parameter), results will be stored in 'answers.json' in executable directory;\n"
        "Examples:\n"
        "ProcessRequestsFrom ../folder_name/filename.json\n"
        "ProcessRequestsFrom C:/folder_name/filename.json\n"
        "\n"
        "ReloadConfig - reload last loaded 'config.json';\n"
        "\n"
        "ReloadConfigFrom - reload config data from file by path (taken as parameter);\n"
        "Examples:\n"
        "ReloadConfigFrom filename.json\n"
        "ReloadConfigFrom C:/filename.json\n"
        "\n"
        "SaveConfigTo - saves current config into file by path (taken as parameter);\n"
        "Examples:\n"
        "SaveConfigTo filename.json\n"
        "SaveConfigTo C:/filename.json\n"
        "\n"
        "Help - show this list.\n";

void reload_config(std::string& filepath) {
    if (converter.reload_config_file(filepath)) {
        auto docs = converter.get_text_documents();
        if (!docs.empty()) server.update_document_base(docs);
    }
}

void process_requests(std::string& filepath) {
    auto requests = ConverterJSON::get_requests(filepath);
    auto results = server.search(requests, converter.get_responses_limit());
    converter.put_answers(results);
}

int main() {
    auto docs = converter.get_text_documents();
    if (!docs.empty()) server.update_document_base(docs);

    std::cout << "Enter 'Help' to get list of commands." << std::endl;
    std::string command;

    while (true) {
        std::cin >> command;

        if (command == "ProcessRequests") {
            std::string filepath;
            process_requests(filepath);

        } else if (command == "ProcessRequestsFrom") {
            std::string filepath;
            std::cin >> std::ws;
            std::getline(std::cin, filepath);
            process_requests(filepath);

        } else if (command == "ReloadConfig") {
            std::string filepath;
            reload_config(filepath);

        } else if (command == "ReloadConfigFrom") {
            std::string filepath;
            std::cin >> std::ws;
            std::getline(std::cin, filepath);
            reload_config(filepath);

        } else if (command == "SaveConfigTo") {
            std::string filepath;
            std::cin >> std::ws;
            std::getline(std::cin, filepath);
            if (!filepath.empty()) converter.save_config_file(filepath);

        } else if (command == "Help") {
            std::cout << help_str;

        } else if (command == "Exit") {
            return 0;

        } else {
            std::cout << "Unknown command: " << command << std::endl;
        }
    }
}




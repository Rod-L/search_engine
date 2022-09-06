#include <iostream>

#include "ConverterJSON.h"
#include "SearchEngine.h"

ConverterJSON converter;
SearchServer server;

std::string help_str =
        "Exit - exit the program;\n"
        "ProcessRequests - process 'requests.json' file in executable directory, results will be stored in 'answers.json' in executable directory;\n"
        "ProcessRequestsFrom - process requests from file by path (taken as parameter), results will be stored in 'answers.json' in executable directory;\n"
        "ReloadConfig - reload last loaded 'config.json';\n"
        "ReloadConfigFrom - reload config data from file by path (taken as parameter);\n"
        "SaveConfigTo - saves current config into file by path (taken as parameter);\n"
        "Help - show this list.\n";

void reload_config(std::string& filepath) {
    if (converter.reload_config_file(filepath)) {
        auto docs = converter.get_text_documents();
        if (!docs.empty()) server.update_document_base(docs);
        std::cout << "Config reloaded. Database contains " << docs.size() << " documents." << std::endl;
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
            std::getline(std::cin, filepath);
            process_requests(filepath);

        } else if (command == "ReloadConfig") {
            std::string filepath;
            reload_config(filepath);

        } else if (command == "ReloadConfigFrom") {
            std::string filepath;
            std::getline(std::cin, filepath);
            reload_config(filepath);

        } else if (command == "SaveConfigTo") {
            std::string filepath;
            std::getline(std::cin, filepath);
            converter.save_config_file(filepath);

        } else if (command == "Help") {
            std::cout << help_str;

        } else if (command == "Exit") {
            return 0;

        } else {
            std::cout << "Unknown command: " << command << std::endl;
        }
    }
}




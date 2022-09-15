#include <iostream>
#include <map>

#include "ConsoleUI.h"

int main() {
    auto docs = ConsoleUI::converter.get_text_documents();
    if (!docs.empty()) ConsoleUI::server.update_document_base(docs);

    std::map<std::string, void(*)()> commands;
    init_commands(commands);

    std::cout << "Enter 'Help' to get list of commands." << std::endl;
    std::string command;

    while (true) {
        std::cin >> command;

        auto func = commands.find(command);
        if (func == commands.end()) {
            if (command == "0" || command == "Exit") return 0;

            std::cout << "Unknown command: " << command << std::endl;
            continue;
        }

        func->second();
    }
}




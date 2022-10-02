#include <iostream>
#include <map>

#include "ConsoleUI.h"

int main() {
    std::map<std::string, void(*)()> commands;
    ConsoleUI::init_commands(commands);
    ConsoleUI::form_index();

    std::cout << "Enter 'Help' to get list of commands." << std::endl;
    std::string command;

    while (true) {
        std::cin >> command;

        auto func = commands.find(command);
        if (func == commands.end()) {
            if (command == "0" || command == "Exit") {
                std::cout << "Exiting..." << std::endl;
                return 0;
            }

            std::cout << "Unknown command: " << command << std::endl;
            continue;
        }

        func->second();
//        auto size = std::cin.rdbuf()->in_avail();
//        std::cin.ignore(size);
//        std::cin.clear();
    }
}




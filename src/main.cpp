#include <iostream>
#include <map>

#include "ConsoleUI.h"

int main(int argc, char *argv[]) {
    std::cout << "Input params:" << std::endl;
    std::cout << argc << std::endl;
    for (int i = 0; i < argc; ++i) std::cout << argv[i] << std::endl;
    std::cout << std::endl;

    std::map<std::string, void(*)(std::string&)> commands;
    ConsoleUI::init_commands(commands);
    ConsoleUI::form_index();

    std::cout << "Enter 'Help' to get list of commands." << std::endl;
    std::string line;
    std::string command;
    std::string params;

    while (true) {
        std::getline(std::cin, line);
        std::stringstream parser(line);
        parser >> command;
        parser >> std::ws;
        std::getline(parser, params);

        auto func = commands.find(command);
        if (func == commands.end()) {
            if (command == "0" || command == "Exit") {
                std::cout << "Exiting..." << std::endl;
                return 0;
            }

            std::cout << "Unknown command: " << command << std::endl;
            continue;
        }

        func->second(params);
    }
}




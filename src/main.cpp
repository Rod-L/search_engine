#include <iostream>
#include <map>

#include "ConsoleUI.h"

//#include <thread>
//
//void start_remote() {
//    system("search_engine");
//}

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
//            if (command == "exec") {
//                std::thread(start_remote).detach();
//                //system("search_engine.exe");
//                std::cout << "Instance booted" << std::endl;
//                continue;
//            }
//            if (command == "stop") {
//                std::cout << 0;
//            }
//            if (command == "stop2") {
//                std::cin.putback('0');
//            }

            if (command == "0" || command == "Exit") {
                std::cout << "Exiting..." << std::endl;
                return 0;
            }

            std::cout << "Unknown command: " << command << std::endl;
            continue;
        }

        func->second();
    }
}




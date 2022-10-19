#include <iostream>
#include <map>

#include "ConsoleUI.h"
#include "FilePipes.h"

bool process_command_line(const std::string& line, const std::map<std::string, void(*)(std::string&)>& commands) {
    std::string command;
    std::string params;

    std::stringstream parser(line);
    parser >> command;
    parser >> std::ws;
    std::getline(parser, params);

    if (command.empty()) return true;

    auto func = commands.find(command);
    if (func == commands.end()) {
        if (command == "0" || command == "Exit") {
            std::cout << "Exiting..." << std::endl;
            return false;
        }

        std::cout << "Unknown command: " << command << std::endl;
        return true;
    }

    func->second(params);
    return true;
}

void command_line_mode(const std::string& path, const std::map<std::string, void(*)(std::string&)>& commands) {
    std::string line;
    ConsoleUI::converter.create_templates();
    ConsoleUI::converter.reload_config_file(path);
    ConsoleUI::form_index();
    std::cout << "Enter 'Help' to get list of commands." << std::endl;
    while (true) {
        std::getline(std::cin, line);
        if (!process_command_line(line, commands)) return;
    }
}

void filepipe_mode(const std::string& path, const std::map<std::string, void(*)(std::string&)>& commands) {
    std::cout << "FILEPIPE mode started." << std::endl;
    std::cout << "pipe: " << path << std::endl;
    std::string line;
    InputFilePipe ipipe(path);
    while (true) {
        if (!ipipe.get(line)) {
            std::this_thread::sleep_for(std::chrono::milliseconds(250));
            continue;
        }

        std::stringstream pipe_commands(line);
        std::string command;
        while (!pipe_commands.eof()) {
            std::getline(pipe_commands, command);
            if (!command.empty()) std::cout << "Got command from pipe: " << command << std::endl;
            if (!process_command_line(command, commands)) {
                std::remove(path.c_str());
                return;
            }
        }
    }
}

/*
Command line params:
 [communication method] [filepath]

 Method - CONSOLE, filepath - optional - path to config for initial load.
 Method - FILEPIPE, filepath - required - path to pipe file to use for IPC.

 default: CONSOLE config.json
*/
int main(int argc, char *argv[]) {
    std::string mode;
    std::string path;

    if (argc < 2) {
        mode = "CONSOLE";
        path = "config.json";
    } else {
        mode = argv[1];
        if (mode == "CONSOLE") {
            path = (argc > 2 ? argv[2] : "config.json");
        } else if (mode == "FILEPIPE") {
            if (argc < 3) {
                std::cerr << "'FILEPIPE' working mode selected, but path to .pipe file have not been supplied!"
                          << std::endl;
                return 1;
            }
            path = argv[2];
        } else {
            std::cerr << "Unknown working mode supplied: " << mode << std::endl;
            std::cerr << "Allowed modes are: CONSOLE, FILEPIPE" << std::endl;
            return 1;
        }
    }

    std::map<std::string, void(*)(std::string&)> commands;
    ConsoleUI::init_commands(commands);

    if (mode == "CONSOLE") {
        command_line_mode(path, commands);
    } else {
        filepipe_mode(path, commands);
    }

    if (ConsoleUI::converter.auto_dump_index) ConsoleUI::dump_index();
}




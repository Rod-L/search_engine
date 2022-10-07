#include <iostream>
#include <thread>
#include <ctime>
#include "FilePipes.h"

int main(int argc, char *argv[]) {
    std::string buf;
    std::string pipe_name = "gui_" + std::to_string(std::time(nullptr)) + ".pipe";
    OutputFilePipe opipe(pipe_name);

    auto remote = [pipe_name](){
        std::string cmd = "search_engine.exe FILEPIPE " + pipe_name;
        system(cmd.c_str());
    };
    std::thread engine(remote);

    std::this_thread::sleep_for(std::chrono::seconds(8));
    std::cout << "Sending 'content' to pipe." << std::endl;
    opipe << "content";
    std::this_thread::sleep_for(std::chrono::seconds(8));
    std::cout << "Sending '0' to pipe." << std::endl;
    opipe << "0";
    std::this_thread::sleep_for(std::chrono::seconds(8));

    std::cout << "Waiting for engine to finish job." << std::endl;
    if (engine.joinable()) engine.join();
    std::cout << "Exiting." << std::endl;
    return 0;
}
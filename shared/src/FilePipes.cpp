#include "FilePipes.h"

//// OutputPipeInterface class

void OutputPipeInterface::set_filepath(const std::string& filepath) {
    executable_filepath = filepath;
};

const std::string& OutputPipeInterface::get_filepath() const {
    return executable_filepath;
};

//// FilePipeInterface class

void FilePipeInterface::lock(const std::string& my_token, const std::string& other_token) {
    while (true) {
        std::string current_token;
        std::ifstream block(block_filepath);
        if (!block.is_open()) break;
        block >> current_token;
        block.close();
        if (current_token != other_token) break;
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }

    std::ofstream block(block_filepath, std::ios::trunc);
    block << my_token;
    block.close();
}

void FilePipeInterface::unlock(const std::string& my_token) {
    std::ifstream block(block_filepath);
    std::string current_token;

    if (block.is_open()) {
        block >> current_token;
        block.close();
        if (current_token != my_token) return;
    }
    std::ofstream block2(block_filepath, std::ios::trunc);
    block2.close();
}

//// InputFilePipe class

InputFilePipe::InputFilePipe(const std::string& filepath) {
    pipe_filepath = filepath;
    block_filepath = PathHelper::catalog_from_filepath(filepath) + PathHelper::file_name(filepath) + ".lock";
};

InputFilePipe::~InputFilePipe() {
    std::remove(block_filepath.c_str());
    std::remove(pipe_filepath.c_str());
}

bool InputFilePipe::get(std::string& to) {
    if (pipe_filepath.empty()) return false;
    if (!PathHelper::file_exist(pipe_filepath)) {
        throw std::runtime_error("Cannot find pipe file: " + pipe_filepath);
    }

    lock("in", "out");

    std::ifstream pipe(pipe_filepath);
    if (!pipe.is_open()) {
        unlock("in");
        return false;
    }

    std::stringstream buf;
    buf << pipe.rdbuf();
    pipe.close();
    std::ofstream trunc(pipe_filepath, std::ios::trunc);
    trunc.close();
    unlock("in");

    to = buf.str();
    return !to.empty();
}

//// OutputFilePipe class

OutputFilePipe::OutputFilePipe(const std::string& filepath) {
    set_filepath(filepath);
};

void OutputFilePipe::set_filepath(const std::string& filepath) {
    if (executable_started) return;

    executable_filepath = filepath;

    std::stringstream name_plate;
    name_plate << PathHelper::catalog_from_filepath(filepath);
    name_plate << PathHelper::file_name(filepath);
    name_plate << "_" << std::time(nullptr) << "_" << std::rand() % 100;

    pipe_filepath = name_plate.str() + ".pipe";
    block_filepath = name_plate.str() + ".lock";
}

const std::string& OutputFilePipe::get_filepath() const {
    return executable_filepath;
}

bool OutputFilePipe::send(const std::string& content) {
    if (executable_filepath.empty()) return false;

    if (!executable_started) {
        std::ofstream pipe(pipe_filepath, std::ios::trunc);
        pipe.close();
        std::ofstream block(block_filepath, std::ios::trunc);
        block.close();
        executable_started = true;
        executable_running = true;
        auto remote = [this](){
            std::string cmd = executable_filepath + " FILEPIPE " + pipe_filepath;
            auto startup = std::time(nullptr);
            int ret = system(cmd.c_str());

            if (PathHelper::is_relative(executable_filepath)) {
                std::cout << "Could not open relative executable path: " << executable_filepath << std::endl;
                std::string orig_path = executable_filepath;
                std::string current_dir = PathHelper::current_dir();

                int attempts = 3;
                while (ret != 0 && attempts > 0 && std::time(nullptr) - startup < 2) {
                    --attempts;
                    orig_path = "../" + orig_path;
                    executable_filepath = PathHelper::combine_path(current_dir, orig_path);
                    std::cout << "Trying " << executable_filepath << std::endl;
                    cmd = executable_filepath + " FILEPIPE " + pipe_filepath;
                    startup = std::time(nullptr);
                    ret = system(cmd.c_str());
                }
            }

            if (ret != 0) std::cerr << executable_filepath << " stopped working with exit code " << ret << std::endl;
            executable_running = false;
        };
        std::thread(remote).detach();
    }

    if (!executable_running) {
        std::string error_message = "Executable '";
        error_message.append(executable_filepath);
        error_message.append("' stopped working unexpectedly.");
        throw std::runtime_error(error_message);
    }

    lock("out", "in");

    std::ofstream pipe(pipe_filepath, std::ios::app);
    if (!pipe.is_open()) {
        std::cerr << "Could not open output pipe file!" << std::endl;
        unlock("out");
        return false;
    }

    pipe << content << std::endl;
    pipe.close();

    unlock("out");
    return true;
}



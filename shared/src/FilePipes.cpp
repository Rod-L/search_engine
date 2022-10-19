#include "FilePipes.h"

void lock(const std::string& block_filepath, const std::string& my_token, const std::string& other_token) {
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

void unlock(const std::string& block_filepath, const std::string& token) {
    std::ifstream block(block_filepath);
    std::string current_token;

    block >> current_token;
    block.close();
    if (current_token != token) return;

    std::ofstream block2(block_filepath, std::ios::trunc);
    block2.close();
}

//// InputFilePipe class

InputFilePipe::InputFilePipe(const std::string& filepath) {
    pipe_filepath = filepath;
    block_filepath = filepath + ".lock";
    message_id = 0;
};

InputFilePipe::~InputFilePipe() {
    std::remove(block_filepath.c_str());
    std::remove(pipe_filepath.c_str());
}

bool InputFilePipe::get(std::string& to) {
    if (pipe_filepath.empty()) return false;

    lock(block_filepath, "in", "out");

    std::ifstream pipe(pipe_filepath);
    if (!pipe.is_open()) {
        unlock(block_filepath, "in");
        return false;
    }

    std::stringstream buf;
    buf << pipe.rdbuf();
    pipe.close();
    std::ofstream trunc(pipe_filepath, std::ios::trunc);
    trunc.close();
    unlock(block_filepath, "in");

    to = buf.str();
    return !to.empty();
}

//// OutputFilePipe class

OutputFilePipe::OutputFilePipe(const std::string& filepath) {
    set_filepath(filepath);
    message_id = 0;
};

void OutputFilePipe::set_filepath(const std::string& filepath) {
    pipe_filepath = filepath;
    block_filepath = filepath + ".lock";
}

const std::string& OutputFilePipe::get_filepath() const {
    return pipe_filepath;
}

bool OutputFilePipe::send(const std::string& content){
    if (pipe_filepath.empty()) return false;

    lock(block_filepath, "out", "in");

    std::ofstream pipe(pipe_filepath, std::ios::app);
    if (!pipe.is_open()) {
        std::cerr << "Could not open output pipe file!" << std::endl;
        unlock(block_filepath, "out");
        return false;
    }

    ++message_id;
    pipe << content << std::endl;
    pipe.close();

    unlock(block_filepath, "out");
    return true;
}



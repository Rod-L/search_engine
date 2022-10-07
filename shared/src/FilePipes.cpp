#include "FilePipes.h"

#define PIPE_EOF_TOKEN "\npipe_end"

//// InputFilePipe class

InputFilePipe::InputFilePipe(const std::string& filepath) {
    pipe_filepath = filepath;
    message_id = 0;
};

bool InputFilePipe::operator>>(std::string& to){
    std::ifstream pipe(pipe_filepath);
    if (!pipe.is_open()) return false;

    size_t inc_message_id;
    std::string str_message_id;
    try {
        getline(pipe, str_message_id);
        inc_message_id = std::stoll(str_message_id);
    } catch (...) {
        pipe.close();
        return false;
    }

    if (inc_message_id <= message_id) {
        pipe.close();
        return false;
    }

    std::stringstream buf;
    buf << pipe.rdbuf();
    pipe.close();
    std::string result = buf.str();

    auto content_size = result.size();
    auto eof_size = sizeof(PIPE_EOF_TOKEN);
    if (result.size() < eof_size) return false;

    auto file_end = result.substr(content_size - eof_size + 1, eof_size - 1);
    if (file_end != PIPE_EOF_TOKEN) return false;

    result.resize(content_size - eof_size + 1);
    to = result;
    message_id = inc_message_id;
    return true;
}

//// OutputFilePipe class

OutputFilePipe::OutputFilePipe(const std::string& filepath) {
    pipe_filepath = filepath;
    message_id = 0;
};

OutputFilePipe::~OutputFilePipe() {
    std::remove(pipe_filepath.c_str());
};

bool OutputFilePipe::operator<<(const std::string& content){
    std::ofstream pipe(pipe_filepath, std::ios::trunc);
    if (!pipe.is_open()) return false;

    ++message_id;
    pipe << message_id << std::endl;
    pipe << content << PIPE_EOF_TOKEN;
    pipe.close();
    return true;
}

#pragma once

#include <sstream>
#include <fstream>

class InputFilePipe {
public:
    InputFilePipe() = delete;
    InputFilePipe(const std::string& filepath);
    bool operator>>(std::string& to);
private:
    std::string pipe_filepath;
    size_t message_id;
};

class OutputFilePipe {
public:
    OutputFilePipe() = delete;
    explicit OutputFilePipe(const std::string& filepath);
    OutputFilePipe(const OutputFilePipe& other) = delete;
    operator=(const OutputFilePipe& other) = delete;
    ~OutputFilePipe();
    bool operator<<(const std::string& content);
private:
    std::string pipe_filepath;
    size_t message_id;
};

#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>
#include "PathHelper.h"

//// Generic

class InputPipeInterface {
public:
    virtual ~InputPipeInterface() {};
    virtual bool get(std::string& to) = 0;
};

class OutputPipeInterface {
public:
    virtual ~OutputPipeInterface() {};
    virtual bool send(const std::string& content) = 0;
    virtual void set_filepath(const std::string& filepath) {
        executable_filepath = filepath;
    };
    virtual const std::string& get_filepath() const {
        return executable_filepath;
    };
protected:
    bool executable_started = false;
    bool executable_running = false;
    std::string executable_filepath;
};

//// File

class FilePipeInterface {
public:
    virtual ~FilePipeInterface() {};
protected:
    void lock(const std::string& my_token, const std::string& other_token);
    void unlock(const std::string& my_token);
    std::string pipe_filepath;
    std::string block_filepath;
};

class InputFilePipe: InputPipeInterface, FilePipeInterface {
public:
    InputFilePipe() = delete;
    explicit InputFilePipe(const std::string& filepath);

    ~InputFilePipe();
    bool get(std::string& to) override;
};

class OutputFilePipe: public OutputPipeInterface, public FilePipeInterface {
public:
    OutputFilePipe() = default;
    explicit OutputFilePipe(const std::string& filepath);

    ~OutputFilePipe() = default;
    bool send(const std::string& content) override;
    void set_filepath(const std::string& filepath) override;
    const std::string& get_filepath() const override;
};
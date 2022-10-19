#pragma once

#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>

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
        pipe_filepath = filepath;
    };
    virtual const std::string& get_filepath() const {
        return pipe_filepath;
    };
protected:
    std::string pipe_filepath;
};

class InputFilePipe: public InputPipeInterface {
public:
    InputFilePipe() = delete;
    explicit InputFilePipe(const std::string& filepath);

    ~InputFilePipe();
    bool get(std::string& to) override;
private:
    std::string pipe_filepath;
    std::string block_filepath;
    size_t message_id;
};

class OutputFilePipe: public OutputPipeInterface {
public:
    OutputFilePipe() = default;
    explicit OutputFilePipe(const std::string& filepath);

    ~OutputFilePipe() = default;
    bool send(const std::string& content) override;
    void set_filepath(const std::string& filepath) override;
    const std::string& get_filepath() const override;
private:
    std::string pipe_filepath;
    std::string block_filepath;
    size_t message_id;
};
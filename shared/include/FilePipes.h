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

    /**
     * Reads content from pipe
     * @param to string acceptor to put read content to
     * @return true, if operation was successful
     */
    virtual bool get(std::string& to) = 0;
};

class OutputPipeInterface {
public:
    virtual ~OutputPipeInterface() {};

    /**
     * Sends content to pipe
     * @param content text to send to pipe
     * @return true, if operation was successful
     */
    virtual bool send(const std::string& content) = 0;

    /**
     * Sets path to executable file to communicate with.
     * @param filepath path to executable file
     */
    virtual void set_filepath(const std::string& filepath);

    /**
     * @return path to current executable
     */
    virtual const std::string& get_filepath() const;
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
    /**
     * Locks file pipe for usage, if pipe have already been locked by other process, waits until it's freed.
     * @param my_token token to set as sign of pipe being locked by this process
     * @param other_token token to consider as sign of pipe being locked by other process
     */
    void lock(const std::string& my_token, const std::string& other_token);

    /**
     * Unlocks pipe for usage, if it was blocked by this process
     * @param my_token token to consider pipe as being locked by this process
     */
    void unlock(const std::string& my_token);
    std::string pipe_filepath;
    std::string block_filepath;
};

class InputFilePipe: InputPipeInterface, FilePipeInterface {
public:
    InputFilePipe() = delete;
    InputFilePipe(const InputFilePipe& other) = delete;
    InputFilePipe& operator=(const InputFilePipe& other) = delete;
    explicit InputFilePipe(const std::string& filepath);
    ~InputFilePipe();

    /**
     * Reads content from file pipe
     * @param to string acceptor to put read content to
     * @return true, if operation was successful
     */
    bool get(std::string& to) override;
};

class OutputFilePipe: public OutputPipeInterface, public FilePipeInterface {
public:
    OutputFilePipe() = default;
    OutputFilePipe(const OutputFilePipe& other) = delete;
    OutputFilePipe& operator=(const OutputFilePipe& other) = delete;
    explicit OutputFilePipe(const std::string& filepath);
    ~OutputFilePipe() = default;

    /**
     * Sends text to stdin stream of child process
     * @param content text to send
     * @return true, if content have been sent successfully
     */
    bool send(const std::string& content) override;

    /**
     * Sets path to executable file. Has no effect if message exchange has already started (i.e. send command was used at least once)
     * @param filepath path to executable file
     */
    void set_filepath(const std::string& filepath) override;

    /**
     * @return path to current executable
     */
    const std::string& get_filepath() const override;
};
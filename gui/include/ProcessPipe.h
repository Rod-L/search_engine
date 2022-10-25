#pragma once

#include <QProcess>
#include "FilePipes.h"

class ProcessPipe: public OutputPipeInterface {
public:
    ProcessPipe() = default;
    explicit ProcessPipe(const std::string& filepath);
    ~ProcessPipe();
    ProcessPipe(const ProcessPipe& other);
    ProcessPipe& operator=(const ProcessPipe& other);

    /**
     * Sends text to stdin stream of child process
     * @param content text to send
     * @return true, if content have been sent successfully
     */
    bool send(const std::string& content) override;
private:
    void stop_child_process();
    void copy_from(const ProcessPipe& other);

    QProcess* child_process;
};

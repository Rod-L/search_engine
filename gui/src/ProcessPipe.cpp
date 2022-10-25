#include "ProcessPipe.h"

ProcessPipe::ProcessPipe(const std::string& filepath) {
    executable_filepath = filepath;
}

ProcessPipe::~ProcessPipe() {
    stop_child_process();
}

ProcessPipe::ProcessPipe(const ProcessPipe& other) {
    copy_from(other);
}

ProcessPipe& ProcessPipe::operator=(const ProcessPipe& other) {
    if (this == &other) return *this;
    copy_from(other);
    return *this;
}

bool ProcessPipe::send(const std::string& content) {
    if (executable_filepath.empty()) return false;

    if (!executable_started) {
        executable_started = true;
        child_process = new QProcess;
        child_process->setProcessChannelMode(QProcess::ForwardedChannels);
        child_process->start(QString::fromStdString(executable_filepath), {"CONSOLE"}, QIODevice::ReadWrite);
        child_process->waitForStarted();
    }

    executable_running = child_process->state() != QProcess::ProcessState::NotRunning;
    if (!executable_running) {
        std::string error_message = "Executable '";
        error_message.append(executable_filepath);
        error_message.append("' stopped working unexpectedly.");
        throw std::runtime_error(error_message);
    }

    child_process->write(content.c_str(), content.size());
    child_process->write("\n", 1);
    child_process->waitForBytesWritten();
    return true;
}

void ProcessPipe::stop_child_process() {
    if (child_process == nullptr) return;

    if (child_process->state() == QProcess::ProcessState::NotRunning) {
        delete child_process;
        child_process = nullptr;
        return;
    }

    child_process->closeWriteChannel();
    child_process->terminate();
    child_process->waitForFinished(5000);
    if (child_process->state() != QProcess::ProcessState::NotRunning) {
        child_process->kill();
    }
    delete child_process;
    child_process = nullptr;
}

void ProcessPipe::copy_from(const ProcessPipe& other) {
    stop_child_process();
    executable_running = false;
    executable_started = false;
    executable_filepath = other.executable_filepath;
}


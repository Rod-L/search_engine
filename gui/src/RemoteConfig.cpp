#include "RemoteConfig.h"

//// Class ProcessPipe

ProcessPipe::ProcessPipe(const std::string& filepath) {
    pipe_filepath = filepath;
};

ProcessPipe::~ProcessPipe() {
    if (child_process != nullptr && child_process->state() != QProcess::ProcessState::NotRunning) {
        child_process->closeWriteChannel();
        child_process->terminate();
        child_process->waitForFinished(5000);
        if (child_process->state() != QProcess::ProcessState::NotRunning) {
            child_process->kill();
        }
        delete child_process;
    }
};

bool ProcessPipe::send(const std::string& content) {
    if (child_process == nullptr || child_process->state() == QProcess::ProcessState::NotRunning) {
        if (child_process != nullptr) delete child_process;
        if (pipe_filepath.empty()) return false;
        child_process = new QProcess;
        child_process->setProcessChannelMode(QProcess::ForwardedChannels);
        child_process->start(QString::fromStdString(pipe_filepath), {"CONSOLE"}, QIODevice::ReadWrite);
        child_process->waitForStarted();
    }
    child_process->write(content.c_str(), content.size());
    child_process->write("\n", 1);
    child_process->waitForBytesWritten();
    return true;
};

//// Class RemoteEngine

RemoteEngine::RemoteEngine(RemoteMode mode) {
    remote_running = true;
    remote_mode = mode;
    if (mode == RemoteMode::FilePipe) {
        pipe = std::make_unique<OutputFilePipe>();
        pipe->set_filepath("gui_" + std::to_string(std::time(nullptr)) + "_" + std::to_string(std::rand() % 100) + ".pipe");
        remote = new std::thread(&RemoteEngine::start_remote, this);
    } else if (mode == RemoteMode::Process) {
        pipe = std::make_unique<ProcessPipe>();
        pipe->set_filepath("search_engine");
    } else {
        throw std::runtime_error("Incorrect work mode supplied to RemoteEngine object.");
    }
}

RemoteEngine::~RemoteEngine() {
    if (remote_mode == RemoteMode::FilePipe && remote != nullptr && remote->joinable()) {
        pipe->send("0");
        remote->join();
        delete remote;
    } else if (remote_mode == RemoteMode::Process) {
        pipe->send("0");
    }
}

int RemoteEngine::indexation_ongoing() {
    int total = 0;
    for (auto& doc_info : current_status) {
        if (doc_info.indexing_in_progress) ++total;
    }
    return total;
}

const std::string& RemoteEngine::get_filepath() {
    return config_filepath;
}

//// commands

bool RemoteEngine::process_request(const std::string& request, std::vector<RequestAnswer>& acceptor) {
    if (!operable()) return false;

    nlohmann::json request_json;
    request_json["requests"] = nlohmann::json::array({request});

    std::string answers_filepath = catalog + "answers.json";

    std::string request_filepath = catalog + "requests.json";
    std::ofstream file(request_filepath);
    if (file.is_open()) {
        file << request_json.dump();
        file.close();
        std::remove(answers_filepath.c_str());
        pipe->send("ProcessRequestsFrom " + request_filepath);
    } else {
        std::cerr << "Could not create requests file '" << request_filepath << "'." << std::endl;
    }

    if (!wait_for_file(answers_filepath)) return false;

    std::ifstream answers_file(answers_filepath);
    if (!answers_file.is_open()) {
        std::cerr << "Could not open '" << answers_filepath << "'." << std::endl;
        return false;
    }

    nlohmann::json answers;
    try {
        answers = nlohmann::json::parse(answers_file);
        answers_file.close();
    } catch(...) {
        answers_file.close();
        std::cerr << "Error while parsing '" << answers_filepath << "'." << std::endl;
        return false;
    }

    for (auto& entry : answers["answers"]) {
        if (entry["result"].get<bool>()) {
            if (entry.contains("relevance")) {
                for (auto& doc : entry["relevance"]) {
                    acceptor.emplace_back(doc["docid"].get<size_t>(), doc["rank"].get<float>());
                }
            } else {
                acceptor.emplace_back(entry["docid"].get<size_t>(), entry["rank"].get<float>());
            }
        }
    }

    return true;
}

bool RemoteEngine::load_config(const std::string& filepath) {
    if (!remote_running) {
        if (remote != nullptr) delete remote;
        remote_running = true;
        remote = new std::thread(&RemoteEngine::start_remote, this);
    }
    if (load_file(filepath)) {
        current_status.clear();
        pipe->send("ReloadConfigFrom " + filepath);
        return true;
    }
    return false;
}

void RemoteEngine::reload_config() {
    if (!operable()) return;
    load_file(config_filepath);
    pipe->send("ReloadConfig");
}

void RemoteEngine::save_config(const std::string& filepath) {
    if (!operable()) return;
    std::stringstream commands;
    commands << "SaveConfigTo " << filepath << std::endl;
    pipe->send(commands.str());
};

void RemoteEngine::dump_index() {
    if (!operable()) return;
    pipe->send("DumpIndex");
};

void RemoteEngine::load_index() {
    if (!operable()) return;
    pipe->send("LoadIndex");
};

void RemoteEngine::reindex(const std::vector<size_t>& docs) {
    if (!operable()) return;

    if (docs.empty()) {
        pipe->send("ReindexFiles");
    } else {
        std::stringstream commands;
        commands << "ReindexFile ";
        for (size_t doc_id : docs) {
            commands << doc_id << ' ';
        }
        pipe->send(commands.str());
    }
};

bool RemoteEngine::status(const std::vector<size_t>& docs, std::vector<DocInfo>& acceptor) {
    if (!operable()) return false;

    std::string status_filepath;
    bool full_update = false;

    if (docs.empty()) {
        full_update = true;
        status_filepath = catalog + file_name + ".status.json";
        std::remove(status_filepath.c_str());
        pipe->send("BaseStatus");
    } else {
        status_filepath = catalog + file_name + ".status_part.json";
        std::remove(status_filepath.c_str());
        std::stringstream commands;
        commands << "FileStatus ";
        for (size_t doc_id : docs) {
            commands << std::to_string(doc_id) << ' ';
        }
        commands << std::endl;
        pipe->send(commands.str());
    }

    if (!wait_for_file(status_filepath)) return false;

    std::ifstream status_file(status_filepath);
    if (!status_file.is_open()) {
        std::cerr << "Could not open '" << status_filepath << "'." << std::endl;
        return false;
    }

    nlohmann::json status;
    try {
        status = nlohmann::json::parse(status_file);
        status_file.close();
    } catch(...) {
        status_file.close();
        std::cerr << "Error while parsing '" << status_filepath << "'." << std::endl;
        return false;
    }

    int entries_count = status["doc_id"].size();
    acceptor.resize(entries_count);
    for (int i = 0; i < entries_count; ++i) {
        auto& doc_info = acceptor[i];
        doc_info.doc_id = status["doc_id"][i].get<int>();
        doc_info.indexed = status["indexed"][i].get<bool>();
        doc_info.indexing_in_progress = status["indexing_in_progress"][i].get<bool>();
        doc_info.indexing_error = status["indexing_error"][i].get<bool>();

        doc_info.index_date = status["index_date"][i].get<time_t>();
        doc_info.error_text = status["error_text"][i].get<std::string>();
        doc_info.filepath = status["filepath"][i].get<std::string>();
    }

    if (full_update || acceptor.size() >= current_status.size()) {
        current_status = acceptor;
    } else {
        for (auto& doc_info : acceptor) {
            current_status[doc_info.doc_id] = doc_info;
        }
    }
    return true;
};

void RemoteEngine::add_files(const std::vector<std::string>& files) {
    if (!operable()) return;

    if (files.empty()) return;
    std::stringstream commands;
    for (auto& file : files) {
        commands << "AddFile " << file << std::endl;
    }
    pipe->send(commands.str());
}

void RemoteEngine::remove_files(const std::vector<size_t>& docs) {
    if (!operable()) return;

    if (docs.empty()) return;
    std::stringstream command;
    command << "RemoveFiles";
    for (auto& doc : docs) {
        command << ' ' << doc;
    }
    pipe->send(command.str());
}

//// Private members

bool RemoteEngine::operable() const {
    return remote_running && !config_filepath.empty();
}

void RemoteEngine::start_remote() {
    remote_running = true;
    std::string command = "search_engine FILEPIPE " + pipe->get_filepath();
    system(command.c_str());
    remote_running = false;
}

bool RemoteEngine::wait_for_file(const std::string& filepath, int timeout_s) {
    std::ifstream file(filepath);
    time_t timeout = std::time(nullptr) + static_cast<time_t>(timeout_s);
    while(true) {
        if (std::time(nullptr) > timeout) return false;
        if (file.is_open()) return true;
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        file.open(filepath);
    }
}

bool RemoteEngine::load_file(const std::string& filepath) {
    nlohmann::json config;

    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Could not open file '" << filepath << "'." << std::endl;
        return false;
    }

    bool parsed = false;
    try {
        config = nlohmann::json::parse(file);
        parsed = true;
    } catch(nlohmann::json::exception& exc) {
        std::cerr << "Error while parsing '" << filepath << "':" << std::endl;
        std::cerr << exc.what() << std::endl;
    } catch (...) {
        std::cerr << "Unknown exception while parsing '" << filepath << "'." << std::endl;
    }

    file.close();
    if (!parsed) return false;

    if (!config.contains("config")) {
        std::cerr << "'" << filepath << "' is not valid file for search engine." << std::endl;
        return false;
    }

    auto& settings = config["config"];
    catalog = PathHelper::catalog_from_filepath(filepath);
    file_name = PathHelper::file_name(filepath);
    name = settings.contains("name") ? settings["name"].get<std::string>() : file_name;

    max_responses = settings.contains("max_responses") ? settings["max_responses"].get<int>() : 5;
    max_indexation_threads = settings.contains("max_indexation_threads") ? settings["max_indexation_threads"].get<int>() : 8;

    auto choose = [](const nlohmann::json& settings, const std::string& field_name){
        return settings.contains(field_name) && settings[field_name].get<bool>();
    };
    auto_reindex = choose(settings, "auto_reindex");
    auto_dump_index = choose(settings, "auto_dump_index");
    auto_load_index_dump = choose(settings, "auto_load_index_dump");
    relative_to_config_folder = choose(settings, "relative_to_config_folder");

    config_filepath = filepath;
    return true;
};

#include "RemoteConfig.h"

//// struct RequestAnswer

bool RequestAnswer::operator==(const RequestAnswer& other) const {
    return doc_id == other.doc_id && std::abs(rank - other.rank) < 0.0001;
};

//// Class RemoteEngine

RemoteEngine::RemoteEngine(RemoteMode mode, const std::string& executable_path) {
    start(mode, executable_path);
}

RemoteEngine::~RemoteEngine() {
    exit();
}

RemoteEngine::RemoteEngine(const RemoteEngine& other) {
    copy_from(other);
}

RemoteEngine& RemoteEngine::operator=(const RemoteEngine& other) {
    if (this == &other) return *this;
    copy_from(other);
    return *this;
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

void RemoteEngine::start(RemoteMode mode, const std::string& executable) {
    exit();
    remote_mode = mode;
    executable_path = executable;

    if (remote_mode == RemoteMode::FilePipe) {
        pipe = new OutputFilePipe;
    } else if (remote_mode == RemoteMode::Process) {
        pipe = new ProcessPipe;
    } else {
        throw std::runtime_error("Invalid remote mode supplied to remote engine.");
    }
    pipe->set_filepath(executable_path);
}

void RemoteEngine::exit() {
    if (pipe == nullptr) return;
    pipe->send("0");
    delete pipe;
    pipe = nullptr;
}

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

    acceptor.clear();
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
    if (load_config_file(filepath)) {
        current_status.clear();
        pipe->send("ReloadConfigFrom " + filepath);
        return true;
    }
    return false;
}

void RemoteEngine::reload_config() {
    if (!operable()) return;
    load_config_file(config_filepath);
    pipe->send("ReloadConfig");
}

void RemoteEngine::save_config(const std::string& filepath, bool wait_for_completion) {
    if (!operable()) return;
    if (wait_for_completion) std::remove(filepath.c_str());
    std::stringstream commands;
    commands << "SaveConfigTo " << filepath << std::endl;
    pipe->send(commands.str());
    if (wait_for_completion) wait_for_file(filepath);
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

bool RemoteEngine::full_status() {
    if (!operable()) return false;

    std::string status_filepath = catalog + file_name + ".status.json";
    std::remove(status_filepath.c_str());
    pipe->send("BaseStatus");

    if (!wait_for_file(status_filepath)) return false;
    current_status.clear();
    return parse_status_file(status_filepath);
}

bool RemoteEngine::files_status(const std::vector<size_t>& docs, std::vector<DocInfo>* acceptor) {
    if (!operable()) return false;

    std::string status_filepath = catalog + file_name + ".status_part.json";
    std::remove(status_filepath.c_str());

    std::stringstream commands;
    commands << "FileStatus ";
    for (size_t doc_id : docs) commands << std::to_string(doc_id) << ' ';
    commands << std::endl;

    pipe->send(commands.str());
    if (!wait_for_file(status_filepath)) return false;
    bool success = parse_status_file(status_filepath);

    if (acceptor != nullptr) {
        for (auto doc_id : docs) {
            if (doc_id < current_status.size()) acceptor->push_back(current_status[doc_id]);
        }
    }

    return success;
}

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
    return pipe != nullptr && !config_filepath.empty();
}

void RemoteEngine::copy_from(const RemoteEngine& other) {
    exit();

    executable_path = other.executable_path;
    remote_mode     = other.remote_mode;
    config_filepath = other.config_filepath;
    file_name       = other.file_name;
    catalog         = other.catalog;
    name            = other.name;
    max_responses   = other.max_responses;
    current_status  = other.current_status;
    auto_reindex    = other.auto_reindex;
    auto_dump_index = other.auto_dump_index;
    max_indexation_threads  = other.max_indexation_threads;
    auto_load_index_dump    = other.auto_load_index_dump;
    relative_to_config_folder       = other.relative_to_config_folder;
    use_independent_dicts_method    = other.use_independent_dicts_method;

    if (other.pipe != nullptr) start(remote_mode, executable_path);
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

bool RemoteEngine::load_config_file(const std::string& filepath) {
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
    use_independent_dicts_method = choose(settings, "use_independent_dicts_method");

    config_filepath = filepath;
    return true;
};

bool RemoteEngine::parse_status_file(const std::string& filepath) {
    std::ifstream status_file(filepath);
    if (!status_file.is_open()) {
        std::cerr << "Could not open '" << filepath << "'." << std::endl;
        return false;
    }

    nlohmann::json status;
    try {
        status = nlohmann::json::parse(status_file);
        status_file.close();
    } catch(...) {
        status_file.close();
        std::cerr << "Error while parsing '" << filepath << "'." << std::endl;
        return false;
    }


    int entries_count = status["doc_id"].size();

    if (entries_count > 0) {
        size_t max_doc_id = 0;
        for (int i = 0; i < entries_count; ++i) max_doc_id = std::max(max_doc_id, status["doc_id"][i].get<size_t>());
        if (current_status.size() <= max_doc_id) current_status.resize(max_doc_id + 1);
    } else {
        return true;
    }

    for (int i = 0; i < entries_count; ++i) {
        size_t doc_id = status["doc_id"][i].get<size_t>();
        auto& doc_info = current_status[doc_id];

        doc_info.doc_id = status["doc_id"][i].get<int>();
        doc_info.indexed = status["indexed"][i].get<bool>();
        doc_info.indexing_in_progress = status["indexing_in_progress"][i].get<bool>();
        doc_info.indexing_error = status["indexing_error"][i].get<bool>();

        doc_info.index_date = status["index_date"][i].get<time_t>();
        doc_info.error_text = status["error_text"][i].get<std::string>();
        doc_info.filepath = status["filepath"][i].get<std::string>();
    }

    return true;
}


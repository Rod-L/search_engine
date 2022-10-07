#include "ConverterJSON.h"

using json = nlohmann::json;

ConverterJSON::ConverterJSON(const std::string& config_file) {
    load_config_file(config_file);
}

void ConverterJSON::create_templates() {
    // requests.json
    std::ifstream requests_file("requests.json");
    if (!requests_file.is_open()) {
        std::ofstream def_requests("requests.json");
        json requests = {
                {"requests", {"request1", "request2", "request3"}}
        };
        def_requests << requests.dump(2);
        def_requests.close();
    } else {
        requests_file.close();
    }

    // config.json
    std::ifstream config("config.json");
    if (config.is_open()) {
        config.close();
    } else {
        std::ofstream new_config("config.json");
        if (!new_config.is_open()) {
            throw std::runtime_error(
                    "Executable directory does not contain valid 'config.json' file.\n"
                    "Could not create default file on startup. Create the file and restart the program.");
        }

        auto def_config_json = ConverterJSON::default_config();
        load_config_json(def_config_json);

        int indentLevel = 2;
        new_config << def_config_json.dump(indentLevel);
        new_config.close();

        std::cout<< "Executable directory does not contain valid 'config.json' file.\n"
                    "Template file have been created." << std::endl;
    }
}

bool ConverterJSON::check_json_file(std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cout << "Cannot open '" << filepath << "'." << std::endl;
        return false;
    }

    json content;
    bool parsed = ConverterJSON::protected_parse_json(file, content, filepath);
    file.close();

    if (!parsed) {
        std::cout << "Could not parse '" << filepath << "' as json object." << std::endl;
        return false;
    }

    if (ConverterJSON::config_json_valid(content)) {
        std::cout << "'" << filepath << "' is valid configuration file for search engine." << std::endl;
        return true;
    }

    if (ConverterJSON::requests_json_valid(content)) {
        std::cout << "'" << filepath << "' is valid requests file for search engine." << std::endl;
        return true;
    }

    std::cout << "'" << filepath << "' is valid json file, but does not fit any format supported by search engine." << std::endl;
    return false;
}

bool ConverterJSON::config_json_valid(json& config) {
    return config.contains("files") && config["files"].is_array() && config.contains("config");
}

bool ConverterJSON::requests_json_valid(json& requests) {
    return requests.contains("requests") && requests["requests"].is_array();
}

bool ConverterJSON::protected_parse_json(std::ifstream& file, json& acceptor, const std::string& filepath) {
    try {
        acceptor = json::parse(file);
    } catch(json::exception& exc) {
        std::cerr << "Error while parsing '" << filepath << "':" << std::endl;
        std::cerr << exc.what() << std::endl;
        return false;
    } catch (...) {
        std::cerr << "Unknown exception while parsing '" << filepath << "'." << std::endl;
        return false;
    }
    return true;
}

void ConverterJSON::load_config_json(json& config) {
    files.clear();
    files.reserve(config["files"].size());
    for (int doc_id = 0; doc_id < config["files"].size(); ++doc_id) {
        std::string filename = config["files"][doc_id];
        config_files.push_back(filename);
        if (HTTPFetcher::is_url(filename)) {
            files.push_back(filename);
        } else if (relative_to_config_folder && PathHelper::is_relative(filename)) {
            files.push_back(PathHelper::combine_path(config_catalog, filename));
        } else {
            files.push_back(filename);
        }
    }

    if (files.empty()) {
        std::cout << "List of files to index in the 'config.json' is empty. "
                     "Update 'config.json' and run 'ReloadConfig' command." << std::endl;
    }

    auto& settings = config["config"];
    responses_limit = settings.contains("max_responses") ? static_cast<int>(settings["max_responses"]) : 5;
    max_indexation_threads = settings.contains("max_indexation_threads") ? static_cast<int>(settings["max_indexation_threads"]) : 8;

    auto choose = [](const json& settings, const std::string& name){
        return settings.contains(name) && settings[name];
    };
    auto_reindex = choose(settings, "auto_reindex");
    auto_dump_index = choose(settings, "auto_dump_index");
    auto_load_index_dump = choose(settings, "auto_load_index_dump");
    store_html_web_files = choose(settings, "store_html_web_files");
    relative_to_config_folder = choose(settings, "relative_to_config_folder");
}

bool ConverterJSON::load_config_file(const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cout << "Could not open '" << filepath << "' config file." << std::endl;
        return false;
    }
    bool loaded = load_config_file(file, filepath);
    file.close();
    return loaded;
}

bool ConverterJSON::load_config_file(std::ifstream& file, const std::string& filepath) {
    json config;
    bool parsed = ConverterJSON::protected_parse_json(file, config, filepath);
    if (!parsed) return false;

    if (config_json_valid(config)) {
        config_filepath = filepath;
        config_catalog = PathHelper::catalog_from_filepath(filepath);
        config_name = PathHelper::file_name(filepath);

        load_config_json(config);
        std::cout << "Config loaded from '" << filepath << "', " << files.size() << " documents listed." << std::endl;
        return true;
    }

    std::cout << filepath << " is not valid configuration file for search engine." << std::endl;
    return false;
}

bool ConverterJSON::reload_config_file(const std::string& filepath) {
    if (!filepath.empty()) config_filepath = filepath;
    if (config_filepath.empty()) {
        std::cerr << "Configuration file have not been selected." << std::endl;
        return false;
    }
    return load_config_file(config_filepath);
}

void ConverterJSON::save_config_file(std::string& filepath) const {
    if (filepath.empty()) filepath = "config.json";

    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << filepath << " - cannot open file." << std::endl;
        return;
    }

    json config;
    config["config"] = {
            {"name", "SkillboxSearchEngine"},
            {"version", "0.1"},
            {"max_responses", responses_limit},
            {"max_indexation_threads", max_indexation_threads},
            {"auto_dump_index", auto_dump_index},
            {"auto_load_index_dump", auto_load_index_dump},
            {"auto_reindex", auto_reindex},
            {"store_html_web_files", store_html_web_files},
            {"relative_to_config_folder", relative_to_config_folder}
    };

    config["files"] = config_files;
    int IndentLevel = 2;
    file << config.dump(IndentLevel);
    file.close();
    std::cout << "Current configuration saved to: " << filepath << std::endl;
}

json ConverterJSON::default_config() {
    json config;

    config["config"] = {
            {"name", "SkillboxSearchEngine"},
            {"version", "0.1"},
            {"max_responses", 5},
            {"max_indexation_threads", 8},
            {"auto_dump_index", true},
            {"auto_load_index_dump", true},
            {"auto_reindex", false},
            {"relative_to_config_folder", true},
            {"store_html_web_files", false}
    };

    config["files"] = json::array();

    return config;
}

const std::vector<std::string>& ConverterJSON::get_text_documents() const {
    return files;
}

bool ConverterJSON::text_documents_from_json(const std::string& filepath, std::vector<std::string>& acceptor) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cout << "Could not open '" << filepath << "' config file." << std::endl;
        return false;
    }

    json config;
    bool parsed = ConverterJSON::protected_parse_json(file, config, filepath);
    if (!parsed) return false;

    if (!ConverterJSON::config_json_valid(config)) {
        std::cout << "'" << filepath << "' is not valid config file." << std::endl;
        return false;
    }

    bool relative_paths = config.contains("relative_to_config_folder") && config["relative_to_config_folder"];

    acceptor.reserve(config["files"].size());
    for (auto& doc : config["files"]) {
        if (relative_paths && PathHelper::is_relative(doc)) {
            acceptor.push_back(PathHelper::combine_path(filepath, doc));
        } else {
            acceptor.push_back(doc);
        }
    }

    return true;
}

std::vector<std::string> ConverterJSON::get_requests(const std::string& filepath) {
    std::vector<std::string> result;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Could not open " << filepath << " file." << std::endl;
        return result;
    }

    json requests;
    bool parsed = ConverterJSON::protected_parse_json(file, requests, filepath);
    file.close();
    if (!parsed) return result;

    if (!requests.contains("requests")) {
        std::cerr << filepath << "is invalid: 'requests' array have not been found." << std::endl;
        return result;
    }

    for (auto& request : requests["requests"]) result.push_back(request);
    std::cout << "Got " << result.size() << " requests from '" << filepath << "'." << std::endl;
    return result;
}

json ConverterJSON::prepare_responses_list(std::vector<RelativeIndex> list) const {
    auto relevance = json::array();
    int responses_count = 0;
    for (auto& rel_index : list) {
        relevance.push_back(json{{"docid", rel_index.doc_id}, {"rank", rel_index.rank}});
        ++responses_count;
        if (responses_count >= responses_limit) break;
    }
    return relevance;
}

void ConverterJSON::put_answers(std::vector<std::vector<RelativeIndex>> answers, std::string& filepath) const {
    if (filepath.empty()) filepath = "answers.json";

    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Could not open '" << filepath << "' file to write answers to." << std::endl;
        return;
    }

    auto request_id_repr = [](int id){
        std::string repr = "Request000";
        std::string str_id = std::to_string(id);

        int id_length = static_cast<int>(str_id.size());
        int template_length = static_cast<int>(repr.size());

        if (id_length > 3) {
            repr.reserve(template_length + id_length - 3);
            for (int i = 3; i < id_length; ++i) repr.append("0");
        }

        for (int i = id_length - 1; i >= 0; --i) {
            repr[template_length - 1 - i] = str_id[i];
        }

        return repr;
    };

    json json_answers;
    int requestId = 1;

    for (auto& list : answers) {
        json entry;
        bool have_results = !list.empty();
        entry["result"] = have_results;
        if (list.size() == 1) {
            entry["docid"] = list[0].doc_id;
            entry["rank"] = list[0].rank;
        } else if (have_results) {
            entry["relevance"] = prepare_responses_list(list);
        }
        json_answers[request_id_repr(requestId)] = entry;
        ++requestId;
    }

    json result;
    result["answers"] = json_answers;

    int indentLevel = 2;
    file << result.dump(indentLevel);
    file.close();
    std::cout << "Results have been written to '" << filepath << "'" << std::endl;
}

void init_status_json(json& status) {
    status["timestamp"] = std::time(nullptr);

    status["doc_id"] = json::array();
    status["in_config"] = json::array();
    status["in_base"] = json::array();
    status["id_mismatch"] = json::array();

    status["indexed"] = json::array();
    status["indexing_in_progress"] = json::array();
    status["indexing_error"] = json::array();
    status["index_date"] = json::array();
    status["error_text"] = json::array();
    status["filepath"] = json::array();
}

void ConverterJSON::add_status_entry(json& status, int line_id, size_t doc_id, const std::vector<DocInfo>& docs_info) const {
    bool in_config = doc_id < files.size();
    bool in_base = doc_id < docs_info.size();
    bool id_mismatch = false;

    if (!in_config && !in_base) return;
    if (in_config && in_base) id_mismatch = files[doc_id] != docs_info[doc_id].filepath;

    status["in_config"][line_id] = in_config;
    status["in_base"][line_id] = in_base;
    status["id_mismatch"][line_id] = id_mismatch;
    status["doc_id"][line_id] = doc_id;

    if (in_base) {
        auto& doc_info = docs_info[doc_id];
        status["indexed"][line_id] = doc_info.indexed;
        status["indexing_in_progress"][line_id] = doc_info.indexing_in_progress;
        status["indexing_error"][line_id] = doc_info.indexing_error;
        status["index_date"][line_id] = doc_info.index_date;
        status["error_text"][line_id] = doc_info.error_text;
        status["filepath"][line_id] = doc_info.filepath;
    } else {
        status["indexed"][line_id] = false;
        status["indexing_in_progress"][line_id] = false;
        status["indexing_error"][line_id] = false;
        status["index_date"][line_id] = 0;
        status["error_text"][line_id] = "";
        status["filepath"][line_id] = files[doc_id];
    }
}

void ConverterJSON::write_status_report(const json& report, const std::string& filepath) const {
    std::ofstream file(filepath);
    if (file.is_open()) {
        file << report.dump(2);
        file.close();
        std::cout << "Status report saved to '" << filepath << "'." << std::endl;
    } else {
        std::cerr << "Could not open file '" << filepath << "'." << std::endl;
    }
}

void ConverterJSON::files_status_by_ids(const std::string& filepath, const std::vector<DocInfo>& docs_info, const std::vector<size_t>& ids) const {
    nlohmann::json status;
    init_status_json(status);

    for (int i = 0; i < ids.size(); ++i) {
        add_status_entry(status, i, ids[i], docs_info);
    }

    write_status_report(status, filepath);
}

void ConverterJSON::files_status(const std::string& filepath, const std::vector<DocInfo>& docs_info) const {
    nlohmann::json status;
    init_status_json(status);

    int border = std::max(files.size(), docs_info.size());
    for (int i = 0; i < border; ++i) {
        add_status_entry(status, i, i, docs_info);
    }

    write_status_report(status, filepath);
}

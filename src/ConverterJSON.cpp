#include "ConverterJSON.h"

using json = nlohmann::json;

bool ConverterJSON::config_json_valid(json& config) {
    return config.contains("files") && config.contains("config") && config["config"].contains("max_responses");
}

void ConverterJSON::load_config_json(json& config) {
    files.clear();
    for (auto& filename : config["files"]) {
        files.push_back(filename);
    }

    if (files.empty()) {
        std::cout << "List of files to index in the 'config.json' is empty. "
                     "Update 'config.json' and run 'ReloadConfig' command." << std::endl;
    }

    responses_limit = config["config"]["max_responses"];
}

bool ConverterJSON::load_config_file(std::string& filepath) {
    std::ifstream file(config_filepath);

    if (!file.is_open()) {
        std::cout << "Could not open '" << filepath << "' config file." << std::endl;
        return false;
    }

    auto config = json::parse(file);
    file.close();

    if (config_json_valid(config)) {
        load_config_json(config);
        return true;
    }

    std::cout << config_filepath << " is not valid configuration file for search engine." << std::endl;
    return false;
}

ConverterJSON::ConverterJSON(std::string config_file): config_filepath(config_file) {
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

    if (load_config_file(config_file)) return;

    std::ofstream new_config(config_filepath);
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
                "Template file have been created.\n"
                "Fill 'files' section of 'config.json' and run 'ReloadConfig' command." << std::endl;

}

bool ConverterJSON::reload_config_file(std::string& filepath) {
    if (!filepath.empty()) config_filepath = filepath;
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
            {"max_responses", responses_limit}
    };

    config["files"] = files;
    int IndentLevel = 2;
    file << config.dump(IndentLevel);
    file.close();
}

json ConverterJSON::default_config() {
    json config;

    config["config"] = {
            {"name", "SkillboxSearchEngine"},
            {"version", "0.1"},
            {"max_responses", 5}
    };

    config["files"] = json::array();

    return config;
}

const std::vector<std::string>& ConverterJSON::get_text_documents() const {
    return files;
}

int ConverterJSON::get_responses_limit() const {
    return responses_limit;
}

std::vector<std::string> ConverterJSON::get_requests(std::string& filepath) {
    if (filepath.empty()) filepath = "requests.json";

    std::vector<std::string> result;
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Could not open 'requests.json' file." << std::endl;
        return result;
    }

    auto requests = json::parse(file);
    file.close();
    if (!requests.contains("requests")) {
        std::cerr << filepath << "is invalid: 'requests' array have not been found." << std::endl;
        return result;
    }

    for (auto& request : requests["requests"]) result.push_back(request);
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

void ConverterJSON::put_answers(std::vector<std::vector<RelativeIndex>> answers) const {
    std::ofstream file("answers.json");
    if (!file.is_open()) {
        std::cerr << "Could not open 'answers.json' file." << std::endl;
        return;
    }

    auto request_id_repr = [](int id){
        std::string repr = "Request000";
        std::string str_id = std::to_string(id);

        int id_length = static_cast<int>(str_id.size());
        int template_length = static_cast<int>(repr.size());
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
    std::cout << "Results have been written to 'answers.json'" << std::endl;
}
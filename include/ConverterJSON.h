#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <exception>

#include "nlohmann/json.hpp"
#include "RelativeIndex.h"

class ConverterJSON {

public:
    explicit ConverterJSON(std::string config_file = "config.json");

    /**
    * Loads configuration from file, if one exist and fits 'config.json' format
    * @param filepath - path to file to load config from. If empty string passed, file from 'config_filepath' object field will be loaded.
    * @return true if file was loaded successfully.
    */
    bool reload_config_file(std::string& filepath);

    /**
    * Saves configuration to file
    * @param filepath - path to file to save current config to. If empty string passed, path from 'config_filepath' object field will be taken.
    */
    void save_config_file(std::string& filepath) const;

    /**
    * @return list of file names from 'files' section of last loaded 'config.json'
    */
    [[nodiscard]] const std::vector<std::string>& get_text_documents() const;

    /**
    * @return max amount of responses for one request ('max_responses' field from last loaded 'config.json')
    */
    [[nodiscard]] int get_responses_limit() const;

    /**
    * @param filepath path to file to load requests from, if empty string passed, 'requests.json' will be used
    * @return list of queries from 'requests.json'
    */
    static std::vector<std::string> get_requests(std::string& filepath);

    /**
    * Writes results of requests to 'answers.json' file in the executable directory
    */
    void put_answers(std::vector<std::vector<RelativeIndex>> answers) const;

private:
    /** Contains maximum amount of responses */
    int responses_limit;
    /** Contains name of loaded config file */
    std::string config_filepath;
    /** Contains text document names loaded from config file */
    std::vector<std::string> files;

    /** @return true if passed json fits 'config.json' format */
    static bool config_json_valid(nlohmann::json& config);

    /** Loads configuration from json object */
    void load_config_json(nlohmann::json& config);

    /** Loads configuration from file, if one exist and fits 'config.json' format */
    bool load_config_file(std::string& filepath);

    [[nodiscard]] nlohmann::json prepare_responses_list(std::vector<RelativeIndex> list) const;

    /**
    * Generates and returns default config.
    * @return default config in form of json object
    */
    static nlohmann::json default_config();
};

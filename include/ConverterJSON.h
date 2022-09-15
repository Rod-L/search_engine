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
    * Checks whether file is supported by search engine or not. Sends status info to std::cout
    * @param filepath - path to file for check.
    * @return true if file supported by search engine
    */
    static bool check_json_file(std::string& filepath);

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
    const std::vector<std::string>& get_text_documents() const;

    /**
    * @return max amount of responses for one request ('max_responses' field from last loaded 'config.json')
    */
    int get_responses_limit() const;

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

    /** @return true if passed json fits 'requests.json' format */
    static bool requests_json_valid(nlohmann::json& requests);

    /** @return true if passed json fits 'config.json' format */
    static bool config_json_valid(nlohmann::json& config);

    /**
    * Attempts to parse file as json object, errors are written to std::cerr stream
    * @return true if passed file was successfully parsed as json object */
    static bool protected_parse_json(std::ifstream& file, nlohmann::json& acceptor, std::string& filepath);

    /** Loads configuration from json object */
    void load_config_json(nlohmann::json& config);

    /** Loads configuration by filepath, if one exist and fits 'config.json' format */
    bool load_config_file(std::string& filepath);

    /**
    * Loads configuration from file, if one fits 'config.json' format
    * @param file - valid input file stream, is_open check is not performed
    * @param filepath - path to file being loaded, used for status/error messages
    * @return true, if config file was loaded successfully
    * */
    bool load_config_file(std::ifstream& file, std::string& filepath);

    nlohmann::json prepare_responses_list(std::vector<RelativeIndex> list) const;

    /**
    * Generates and returns default config.
    * @return default config in form of json object
    */
    static nlohmann::json default_config();
};

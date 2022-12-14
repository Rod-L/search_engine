#pragma once

#include <iostream>
#include <vector>
#include <fstream>
#include <exception>

#include "nlohmann/json.hpp"
#include "RelativeIndex.h"
#include "DocInfoStruct.h"
#include "PathHelper.h"
#include "HTTPTextFetcher.h"

class ConverterJSON {

public:
    /** Contains maximum amount of responses */
    int responses_limit;

    /** Indexation threads amount limit */
    int max_indexation_threads;

    /** flags loaded from config file */
    bool auto_reindex;
    bool auto_dump_index;
    bool auto_load_index_dump;
    bool relative_to_config_folder;
    bool use_independent_dicts_method;

    ConverterJSON() = default;

    /**
    * Initializes object by passed config file. Equivalent of:
    *   ConverterJSON conv;
    *   conv.reload_config_file(filepath);
    * @param config_file configuration file to use for initialization
    */
    explicit ConverterJSON(const std::string& config_file);

    /**
    * Creates 'config.json' and 'requests.json' templates in the executable directory
    */
    void create_templates();

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
    bool reload_config_file(const std::string& filepath);

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
    * @return list of file names from 'files' section of passed file. File must fit 'config.json' requirements
    */
    static bool text_documents_from_json(const std::string& filepath, std::vector<std::string>& acceptor);

    /**
    * @param filepath path to file to load requests from, if empty string passed, 'requests.json' will be used
    * @return list of queries from 'requests.json'
    */
    static std::vector<std::string> get_requests(const std::string& filepath);

    /**
    * Writes results of requests to 'answers.json' file in the executable directory
    */
    void put_answers(const std::vector<std::vector<RelativeIndex>>& answers, std::string& filepath) const;

    /**
    * Dumps status of index base to json file by passed filepath
    * @param filepath path to the file to dump info to
    * @param docs_info status of documents in index base
    */
    void files_status(const std::string& filepath, const std::vector<DocInfo>& docs_info) const;

    /**
    * Dumps status of selected documents from index base to json file by passed filepath
    * @param filepath path to the file to dump info to
    * @param docs_info status of documents in index base
    * @param ids indexes of documents which info should be dumped
    */
    void files_status_by_ids(const std::string& filepath, const std::vector<DocInfo>& docs_info, const std::vector<size_t>& ids) const;

    /**
    * @return filepath to loaded configuration file
    */
    const std::string& get_config_path() const;

    /**
    * @return catalog of loaded configuration file
    */
    const std::string& get_config_catalog() const;

    /**
    * @return name without extension of loaded configuration file
    */
    const std::string& get_config_name() const;

    /**
    * Adds file to configuration's list of files, if file have not been added yet
    * @param filepath path to file
    * @return Id which file acquired in index base
    */
    size_t add_file(const std::string& filepath);

    /**
    * Adds passed files to configuration's list of files.
    * If file with same name already presented, double entry won't be created
    * @param new_files list of files to add to database
    */
    void add_files(const std::vector<std::string>& new_files);

    /**
    * Removes files from configuration's list of files.
    * @param ids
    */
    void remove_files(const std::vector<size_t>& ids);

private:

    /** Contains name of loaded config file */
    std::string config_filepath;
    std::string config_catalog;
    std::string config_name; // no extension
    std::string project_name;

    /** Contains text document names loaded from config file */
    std::vector<std::string> config_files;
    /** Contains converted document names loaded from config file */
    std::vector<std::string> files;

    /** @return true if passed json fits 'requests.json' format */
    static bool requests_json_valid(nlohmann::json& requests);

    /** @return true if passed json fits 'config.json' format */
    static bool config_json_valid(nlohmann::json& config);

    /**
    * Attempts to parse file as json object, errors are written to std::cerr stream
    * @return true if passed file was successfully parsed as json object */
    static bool protected_parse_json(std::ifstream& file, nlohmann::json& acceptor, const std::string& filepath);

    /** Loads configuration from json object */
    void load_config_json(nlohmann::json& config);

    /** Loads configuration by filepath, if one exist and fits 'config.json' format */
    bool load_config_file(const std::string& filepath);

    /**
    * Loads configuration from file, if one fits 'config.json' format
    * @param file - valid input file stream, is_open check is not performed
    * @param filepath - path to file being loaded, used for status/error messages
    * @return true, if config file was loaded successfully
    * */
    bool load_config_file(std::ifstream& file, const std::string& filepath);

    nlohmann::json prepare_responses_list(const std::vector<RelativeIndex>& list) const;

    /**
    * Generates and returns default config.
    * @return default config in form of json object
    */
    static nlohmann::json default_config();

    void write_status_report(const nlohmann::json& report, const std::string& filepath) const;

    void init_status_json(nlohmann::json& status) const;

    void add_status_entry(nlohmann::json& status, int line_id, size_t doc_id, const std::vector<DocInfo>& docs_info) const;
};

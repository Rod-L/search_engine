#pragma once

#include <iostream>
#include <thread>
#include <memory>

#include "nlohmann/json.hpp"
#include "FilePipes.h"
#include "ProcessPipe.h"
#include "PathHelper.h"

//// struct RequestAnswer

struct RequestAnswer{
    size_t doc_id;
    float rank;

    RequestAnswer(): doc_id(0), rank(0) {};
    RequestAnswer(size_t _doc_id, float _rank): doc_id(_doc_id), rank(_rank) {};
    bool operator==(const RequestAnswer& other) const;
};

//// struct DocInfo

struct DocInfo {
    bool indexed;
    bool indexing_in_progress;
    bool indexing_error;
    int doc_id;
    time_t index_date;
    std::string error_text;
    std::string filepath;
};

//// Class RemoteEngine

class RemoteEngine {
public:
    enum RemoteMode {
        Process,
        FilePipe
    };

    std::string name;
    int max_responses;
    int max_indexation_threads;
    bool auto_reindex;
    bool auto_dump_index;
    bool auto_load_index_dump;
    bool relative_to_config_folder;
    bool use_independent_dicts_method;
    std::vector<DocInfo> current_status;

    RemoteEngine() = default;
    RemoteEngine(RemoteMode mode, const std::string& executable_path);
    ~RemoteEngine();
    RemoteEngine(const RemoteEngine& other);
    RemoteEngine& operator=(const RemoteEngine& other);

    /**
     * Counts amount of indexation tasks in progress by last loaded status.
     * Does not update status, thus consequential calls will return same value, unless full_status() called.
     * @return amount of indexation tasks in progress
     */
    int indexation_ongoing();

    /**
     * @return path to loaded configuration file
     */
    const std::string& get_filepath();

    //// commands

    /**
     * Stops current and attempts to start new remote engine in passed mode
     * @param mode FilePipe or Process
     * @param executable_path path to executable file to launch
     */
    void start(RemoteMode mode, const std::string& executable_path);

    /**
     * Stops current remote engine
     */
    void exit();

    /**
     * @param request text of request
     * @param acceptor array, where result of request should be placed
     * @return true, if operation was successful
     */
    bool process_request(const std::string& request, std::vector<RequestAnswer>& acceptor);

    /**
     * Loads configuration from given file
     * @param filepath path to file to load configuration from
     * @return true, if operation was successful
     */
    bool load_config(const std::string& filepath);

    /**
     * Reloads configuration from last loaded file, forces remote engine to reload configuration file as well
     * Auto reindex and auto load/unload of index base is not performed.
     */
    void reload_config();

    /**
     * Saves current configuration by passed filepath
     * @param filepath path to save configuration to
     * @param wait_for_completion if true, function won't return until operation is complete
     */
    void save_config(const std::string& filepath, bool wait_for_completion = false);

    /**
     * Saves current index base into directory of loaded configuration file
     */
    void dump_index();

    /**
     * Attempts to load index base from
     */
    void load_index();

    /**
     * Flushes index of documents passed in docs array and performs indexation from scratch
     * @param docs IDs of documents to reindex
     */
    void reindex(const std::vector<size_t>& docs);

    /**
     * Retrieves status of index base from remote search engine and updates current_status field of RemoteEngine object
     * @return true if operation completed successfully
     */
    bool full_status();

    /**
     * Retrieves status of documents listed in docs array, updates current_status field of RemoteEngine object
     * if acceptor parameter is not null, acceptor array will contain copies of status entries of updated documents
     * @param docs IDs of documents to update status of
     * @param acceptor array to retrieve entries of updated documents
     * @return true if operation completed successfully
     */
    bool files_status(const std::vector<size_t>& docs, std::vector<DocInfo>* acceptor = nullptr);

    /**
     * Adds files to configuration, indexation won't be performed automatically
     * @param files array of file paths or HTTP links to add
     */
    void add_files(const std::vector<std::string>& files);

    /**
     * Flushes index of passed files and removes them from configuration file
     * @param docs IDs of documents to remove
     */
    void remove_files(const std::vector<size_t>& docs);

private:
    RemoteMode remote_mode;
    std::string executable_path;
    std::string config_filepath;
    std::string file_name;
    std::string catalog;

    OutputPipeInterface* pipe = nullptr;

    bool operable() const;
    void copy_from(const RemoteEngine& other);
    static bool wait_for_file(const std::string& filepath, int timeout_s = 4);
    bool load_config_file(const std::string& filepath);
    bool parse_status_file(const std::string& filepath);
};


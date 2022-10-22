#pragma once

#include <iostream>
#include <thread>
#include <memory>

#include "nlohmann/json.hpp"
#include "FilePipes.h"
#include "PathHelper.h"
#include <QProcess>

enum RemoteMode {
    Process,
    FilePipe
};

//// struct RequestAnswer

struct RequestAnswer{
    size_t doc_id;
    float rank;

    RequestAnswer(): doc_id(0), rank(0) {};
    RequestAnswer(size_t _doc_id, float _rank): doc_id(_doc_id), rank(_rank) {};
    bool operator==(const RequestAnswer& other) const;
};

//// struct DocsInfo

struct DocInfo {
    bool indexed;
    bool indexing_in_progress;
    bool indexing_error;
    int doc_id;
    time_t index_date;
    std::string error_text;
    std::string filepath;
};

//// Class ProcessPipe

class ProcessPipe: public OutputPipeInterface {
public:
    ProcessPipe() = default;
    explicit ProcessPipe(const std::string& filepath);

    ~ProcessPipe();
    bool send(const std::string& content) override;

private:
    QProcess* child_process;
};

//// Class RemoteEngine

class RemoteEngine {
public:
    std::unique_ptr<OutputPipeInterface> pipe;

    std::string name;
    int max_responses;
    int max_indexation_threads;
    bool auto_reindex;
    bool auto_dump_index;
    bool auto_load_index_dump;
    bool relative_to_config_folder;
    bool use_independent_dicts_method;
    std::vector<DocInfo> current_status;

    RemoteEngine() = delete;
    RemoteEngine(RemoteMode mode);
    ~RemoteEngine();

    int indexation_ongoing();
    const std::string& get_filepath();

    //// commands

    bool process_request(const std::string& request, std::vector<RequestAnswer>& acceptor);
    bool load_config(const std::string& filepath);
    void reload_config();
    void save_config(const std::string& filepath, bool wait_for_completion = false);
    void dump_index();
    void load_index();
    void reindex(const std::vector<size_t>& docs);
    bool status(const std::vector<size_t>& docs, std::vector<DocInfo>* acceptor = nullptr);
    void add_files(const std::vector<std::string>& files);
    void remove_files(const std::vector<size_t>& docs);

private:
    RemoteMode remote_mode;
    std::string config_filepath;
    std::string file_name;
    std::string catalog;

    bool operable() const;
    static bool wait_for_file(const std::string& filepath, int timeout_s = 4);
    bool load_file(const std::string& filepath);
};


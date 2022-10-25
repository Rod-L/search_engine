#include <catch2/catch_test_macros.hpp>
#include "nlohmann/json.hpp"
#include "RemoteConfig.h"

using json = nlohmann::json;

TEST_CASE("RemoteConfig_General") {
    RemoteEngine premote(RemoteEngine::Process, "search_engine");
    premote.exit();

    RemoteEngine fremote(RemoteEngine::FilePipe, "search_engine");
    fremote.exit();
}

//// Base functions

void make_text_file(const std::string& filepath, const std::string& text) {
    std::ofstream text_file(filepath);
    REQUIRE(text_file.is_open());

    text_file << text;
    text_file.close();
}

std::string make_test_db(const std::string& db_name, const std::vector<std::string>& texts) {
    std::string config_filepath = db_name + ".json";
    std::ofstream test_config_file(config_filepath);
    REQUIRE(test_config_file.is_open());

    json test_config;
    test_config["config"] = {
            {"name", db_name},
            {"max_responses", 5},
            {"max_indexation_threads", 8},
            {"auto_reindex", false},
            {"auto_dump_index", false},
            {"auto_load_index_dump", false},
            {"relative_to_config_folder", true}
    };

    std::vector<std::string> filenames;
    filenames.reserve(texts.size());
    for (int i = 0; i < texts.size(); ++i) {
        std::string filename = db_name + "_" + std::to_string(i) + ".txt";
        filenames.push_back(filename);
        make_text_file(filename, texts[i]);
    }

    test_config["files"] = filenames;
    test_config_file << test_config.dump(2);
    return config_filepath;
}

void delete_test_db(const std::string& config_filepath) {
    std::ifstream test_config_file(config_filepath);
    if (!test_config_file.is_open()) return;

    json test_config = json::parse(test_config_file);
    test_config_file.close();
    std::remove(config_filepath.c_str());

    for (const std::string& filename : test_config["files"]) {
        std::remove(filename.c_str());
    }

    auto index_path = PathHelper::file_name(config_filepath) + ".index.bin";
    std::remove(index_path.c_str());
}

//// Test commands

void test_all_commands(RemoteEngine* remote) {

    auto wait_reindex = [remote](const std::vector<size_t>& docs) {
        remote->reindex(docs);
        REQUIRE((docs.empty() ? remote->full_status() : remote->files_status(docs)));
        while (remote->indexation_ongoing()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            REQUIRE((docs.empty() ? remote->full_status() : remote->files_status(docs)));
        }
    };

    auto config_filepath = make_test_db("test_all_commands", {
        "text1 text1 text1",
        "text2 text2 text2",
        "text3 text3 text3"
    });

    // load_config
    REQUIRE(remote->load_config(config_filepath));

    // reindex & status
    wait_reindex({});

    // process_request
    std::vector<RequestAnswer> answers;
    std::vector<RequestAnswer> expected = {{0, 1.0}, {1, 1.0}};
    REQUIRE(remote->process_request("text1 text2", answers));
    REQUIRE(answers == expected);

    // dump and load index
    remote->dump_index();
    remote->load_index();

    // reprocess requests to be sure, index remains consistent
    REQUIRE(remote->process_request("text1 text2", answers));
    REQUIRE(answers == expected);

    // save and reload config
    std::string saved_config = "test_all_commands_copy.json";
    remote->save_config(saved_config, true);
    REQUIRE(remote->load_config(saved_config));

    wait_reindex({});
    REQUIRE(remote->process_request("text1 text2", answers));
    REQUIRE(answers == expected);
    std::remove(saved_config.c_str());

    // fall back to previous config and recheck dumped index consistency.
    REQUIRE(remote->load_config(config_filepath));
    remote->load_index();
    REQUIRE(remote->process_request("text1 text2", answers));
    REQUIRE(answers == expected);

    // add / remove files
    remote->full_status();
    int old_size = remote->current_status.size();
    std::string additional_file = "test_all_commands_addition.txt";
    make_text_file(additional_file, "addition");
    remote->add_files({additional_file});
    remote->full_status();
    REQUIRE(remote->current_status.size() == old_size + 1);

    size_t add_doc_id = remote->current_status.size() - 1;
    wait_reindex({add_doc_id});

    std::vector<RequestAnswer> add_expected = {{add_doc_id, 1.0}};
    REQUIRE(remote->process_request("addition", answers));
    REQUIRE(answers == add_expected);
    REQUIRE(remote->process_request("text1 text2", answers));
    REQUIRE(answers == expected);

    remote->remove_files({add_doc_id});
    REQUIRE(remote->process_request("addition", answers));
    REQUIRE(answers.empty());
    REQUIRE(remote->process_request("text1 text2", answers));
    REQUIRE(answers == expected);
    remote->full_status();
    REQUIRE(remote->current_status.size() == old_size);
    remote->exit();

    delete_test_db(config_filepath);
}

TEST_CASE("RemoteConfig_AllCommands") {
    SECTION("File pipe mode") {
        RemoteEngine remote(RemoteEngine::FilePipe, "search_engine");
        test_all_commands(&remote);
    }
    SECTION("Child process mode") {
        RemoteEngine remote(RemoteEngine::Process, "search_engine");
        test_all_commands(&remote);
    }
}
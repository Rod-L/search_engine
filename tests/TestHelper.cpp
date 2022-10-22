#include "TestHelper.h"

void make_test_files(const std::vector<std::string>& texts, std::vector<std::string>& acceptor) {
    acceptor.resize(texts.size());
    for (int i = 0; i < texts.size(); ++i) {
        std::string filepath = "test_tmp_" + std::to_string(i) + ".txt";
        std::ofstream text_file(filepath);
        REQUIRE(text_file.is_open());

        text_file << texts[i];
        text_file.close();
        acceptor[i] = filepath;
    }
}

void remove_test_files(std::vector<std::string>& filenames) {
    for (auto& file : filenames) std::remove(file.c_str());
}

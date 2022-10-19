#pragma once

#include <string>
#include <ctime>

struct DocInfo {
    bool indexed;
    bool indexing_in_progress;
    bool indexing_error;
    bool awaits_indexation;
    time_t index_date;
    std::string error_text;
    std::string filepath;
};
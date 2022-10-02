#pragma once

#include <mutex>

namespace GlobalLocks {
    std::mutex cout;
    std::mutex cerr;
}
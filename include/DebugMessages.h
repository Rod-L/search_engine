#pragma once

#ifdef __DEBUG__
#include <mutex>

inline namespace DEBUG_LOCKS {
    inline std::mutex cout;
}

#define DEBUG_MSG(MSG) {        \
    DEBUG_LOCKS::cout.lock();   \
    std::cout << MSG;           \
    DEBUG_LOCKS::cout.unlock();};

#define DEBUG_MSG_COND(BOOL_V, MSG1, MSG2) { \
    DEBUG_LOCKS::cout.lock();              \
    if (BOOL_V) {                          \
        std::cout << MSG1;                 \
    } else {                               \
        std::cout << MSG2;                 \
    }                                      \
    DEBUG_LOCKS::cout.unlock();            \
}

#else

#define DEBUG_MSG(MSG) {};
#define DEBUG_MSG_COND(BOOL, MSG1, MSG2) {};

#endif


cmake_minimum_required(VERSION 3.22)
project(search_engine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# include necessary libraries into executable
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static")

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/ConverterJSON.cpp
        src/InvertedIndex.cpp
        src/SearchEngine.cpp
        shared/src/FilePipes.cpp
        shared/src/PathHelper.cpp
        shared/src/HTTPTextFetcher.cpp)

#target_compile_definitions(search_engine PRIVATE __DEBUG__) # uncomment to see debug messages

target_include_directories(${PROJECT_NAME} PUBLIC include shared/include)

include(FetchContent)

##############################################
# nlohmann_json

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.1/json.tar.xz)
FetchContent_MakeAvailable(json)

target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

##############################################
# CPR - http support

set(BUILD_CPR_TEST OFF)
set(CMAKE_USE_OPENSSL OFF)
SET(CURL_ZLIB OFF CACHE STRING "" FORCE)

FetchContent_Declare(cpr
        GIT_REPOSITORY https://github.com/libcpr/cpr.git
        GIT_TAG 871ed52d350214a034f6ef8a3b8f51c5ce1bd400) # The commit hash for 1.9.0.
FetchContent_MakeAvailable(cpr)

target_link_libraries(${PROJECT_NAME} PRIVATE cpr::cpr)

##############################################
# GUI

add_subdirectory(gui)

##############################################
# Catch2 - uncomment for testing

enable_testing()
add_subdirectory(tests)



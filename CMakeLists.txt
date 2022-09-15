cmake_minimum_required(VERSION 3.22)
project(search_engine)

set(CMAKE_CXX_STANDARD 17)

add_executable(${PROJECT_NAME}
        src/main.cpp
        src/ConverterJSON.cpp
        src/InvertedIndex.cpp
        src/SearchEngine.cpp)

target_include_directories(${PROJECT_NAME} PUBLIC include)

include(FetchContent)

##############################################
# nlohmann_json

FetchContent_Declare(json URL https://github.com/nlohmann/json/releases/download/v3.11.1/json.tar.xz)
FetchContent_MakeAvailable(json)

target_link_libraries(${PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

##############################################
# Catch2 - uncomment for testing

enable_testing()
add_subdirectory(tests)



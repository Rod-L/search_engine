cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 17)
set(TESTS_PROJECT_NAME tests)
set(CATCH_CONFIG_ENABLE_BENCHMARKING true)

############################################
# Build the test framework
Include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.0.1
)

FetchContent_MakeAvailable(Catch2)
add_executable(${TESTS_PROJECT_NAME}
        TestInvertedIndex.cpp
        TestSearchEngine.cpp
        ../src/ConverterJSON.cpp
        ../src/InvertedIndex.cpp
        ../src/SearchEngine.cpp
        ../shared/src/FilePipes.cpp
        ../shared/src/PathHelper.cpp
        ../shared/src/HTTPTextFetcher.cpp)

target_link_libraries(${TESTS_PROJECT_NAME} PRIVATE Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(${TESTS_PROJECT_NAME})

##############################################
# nlohmann_json

target_link_libraries(${TESTS_PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

##############################################
# CPR - http support

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_libraries(${TESTS_PROJECT_NAME} PRIVATE cpr::cpr)

############################################


target_include_directories(${TESTS_PROJECT_NAME} PUBLIC ../include ../shared/include)

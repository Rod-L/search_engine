cmake_minimum_required(VERSION 3.22)
project(gui)

set(CMAKE_CXX_STANDARD 17)
set(GUI_PROJECT_NAME gui)

############################################
# Build the test framework
add_executable(gui
        ../shared/src/FilePipes.cpp
        src/main.cpp)

##############################################
# nlohmann_json

target_link_libraries(${GUI_PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

##############################################
# CPR - http support

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_libraries(${GUI_PROJECT_NAME} PRIVATE cpr::cpr)

############################################

target_include_directories(${GUI_PROJECT_NAME} PUBLIC include ../shared/include)

cmake_minimum_required(VERSION 3.22)

set(CMAKE_CXX_STANDARD 17)

add_executable(gui_tests
        ../../shared/src/FilePipes.cpp
        ../../shared/src/PathHelper.cpp
        ../../shared/src/HTTPTextFetcher.cpp
        ../src/RemoteConfig.cpp
        #../src/GUIWindow.cpp
        #../include/GUIWindow.h
        TestRemoteConfig.cpp)

##############################################
# Qt
find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

target_link_libraries(gui_tests PUBLIC Qt5::Core)
target_link_libraries(gui_tests PUBLIC Qt5::Gui)
target_link_libraries(gui_tests PUBLIC Qt5::Widgets)

############################################
# Build the test framework
Include(FetchContent)

FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG        v3.0.1
)

FetchContent_MakeAvailable(Catch2)
target_link_libraries(gui_tests PRIVATE Catch2::Catch2WithMain)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(CTest)
include(Catch)
catch_discover_tests(gui_tests)

##############################################
# nlohmann_json

target_link_libraries(gui_tests PRIVATE nlohmann_json::nlohmann_json)

##############################################
# CPR - http support

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_libraries(gui_tests PRIVATE cpr::cpr)

############################################

target_include_directories(gui_tests PUBLIC ../include ../../shared/include)

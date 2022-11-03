cmake_minimum_required(VERSION 3.22)
project(gui)

set(CMAKE_CXX_STANDARD 17)
set(GUI_PROJECT_NAME gui)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# include necessary libraries into executable
set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_EXE_LINKER_FLAGS "-static")

# Setup qt installation path, select necessary build system
set(CMAKE_PREFIX_PATH "E:\\Qt5\\5.15.2\\mingw81_64\\lib\\cmake")

add_executable(gui
        ../shared/src/FilePipes.cpp
        ../shared/src/PathHelper.cpp
        ../shared/src/HTTPTextFetcher.cpp
        src/main.cpp
        src/RemoteConfig.cpp
        src/GUIWindow.cpp
        src/ProcessPipe.cpp
        include/GUIWindow.h)

##############################################
# nlohmann_json

target_link_libraries(${GUI_PROJECT_NAME} PRIVATE nlohmann_json::nlohmann_json)

##############################################
# CPR - http support

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
target_link_libraries(${GUI_PROJECT_NAME} PRIVATE cpr::cpr)

##############################################
# Catch2 - uncomment for testing

add_subdirectory(tests)

############################################

target_include_directories(${GUI_PROJECT_NAME} PUBLIC include ../shared/include)

##############################################
# QT 5

find_package(Qt5 COMPONENTS
        Core
        Gui
        Widgets
        REQUIRED)

target_link_libraries(${GUI_PROJECT_NAME} PUBLIC Qt5::Core)
target_link_libraries(${GUI_PROJECT_NAME} PUBLIC Qt5::Gui)
target_link_libraries(${GUI_PROJECT_NAME} PUBLIC Qt5::Widgets)

if (WIN32)
    if (DEFINED CMAKE_PREFIX_PATH OR DEFINED QT_INSTALL_PATH)
        MESSAGE(STATUS "QT_INSTALL_PATH set to${QT_INSTALL_PATH}")
        MESSAGE(STATUS "CMAKE_PREFIX_PATH set to ${CMAKE_PREFIX_PATH}")
    else()
        MESSAGE(FATAL_ERROR "Neither CMAKE_PREFIX_PATH nor QT_INSTALL_PATH have been set. Unable to find Qt installation path.")
    endif()

    set(DEBUG_SUFFIX)
    set(QT_INSTALL_PATH "${CMAKE_PREFIX_PATH}")
    if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
        set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        if (NOT EXISTS "${QT_INSTALL_PATH}/bin")
            set(QT_INSTALL_PATH "${QT_INSTALL_PATH}/..")
        endif ()
    endif ()
    if (EXISTS "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E make_directory
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/plugins/platforms/qwindows${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>/plugins/platforms/")
    endif ()
    foreach (QT_LIB Core Gui Widgets)
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy
                "${QT_INSTALL_PATH}/bin/Qt5${QT_LIB}${DEBUG_SUFFIX}.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
    endforeach (QT_LIB)
endif ()
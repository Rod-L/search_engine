Simple index and search engine with support of multithread indexing and runtime index extension.

Dependencies:\
https://github.com/nlohmann/json \
https://github.com/libcpr/cpr \
Test framework: https://github.com/catchorg/Catch2.git \
Three above fetched and installed automatically by CMake

Optional GUI subproject requires Qt 5.15: https://www.qt.io

Features:\
auto fetch and index texts from http links;\
flexible index base: add and remove documents on the fly without loss of previously made index data;\
fast and compact index base binary dump / load mechanism;\
engine is designed to be used as a child process of other application, thus can be easily embed into existing solution;


Quick start
1. Browse CMakeLists.txt in root folder of the project: enable / disable desired features (gui, tests, debug info) according to comments.
2. Build search_engine.

If you're willing to use gui:
1. Build gui subproject.
2. Launch gui, create configuration, add documents, make search requests.

If you're willing to use engine in console mode:
1. Find 'config.json' file in the executable directory and fill its 'files' section with absolute or relative paths to files you want to present in database for future search.
2. In 'search_engine' console use 'ReloadConfig' and 'ReindexFiles' commands.
3. Find 'requests.json' file in the executable directory and fill its 'requests' section with arbitrary requests according to template.
4. Run 'ProcessRequests' command to generate 'answers.json' file.

If you're willing to use engine as child process of host application, refer to documentation and 'RemoteEngine' class of gui subproject as example.

\
Examples.

*project contains 'examples' folder with configuration files and sample texts inside its subfolders.*

If you use engine in console mode: 
1. Choose example and put its files into 'search_engine' executable directory. 
2. Launch search engine and use 'ProcessRequests' command to see example in use.

If you use gui subproject:
1. Select 'Open configuration' in 'File' context menu.
2. Inspect 'Documents' tab and make search requests to see engine in work.
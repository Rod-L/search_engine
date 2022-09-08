Simple console index and search engine with support of multithread indexing and runtime index extension.

Dependencies:\
https://github.com/nlohmann/json \
https://github.com/catchorg/Catch2.git \
Both fetched and installed automatically by CMake


How to:

*'requests.json' and 'config.json' templates will be created in the executable directory on first startup.*

1. Build project and launch 'search_engine.exe'.
2. Find 'config.json' file in the executable directory and fill its 'files' section with absolute or relative paths to files you want to present in database for future search.
3. In 'search_engine' console use 'ReloadConfig' command to reindex files.
4. Find 'requests.json' file in the executable directory and fill its 'requests' section with arbitrary requests according to template.
5. Run 'ProcessRequests' command to generate 'answers.json' file.

\
How to use Examples:

*project contains 'examples' folder with configuration files and sample texts inside its subfolders.* 
1. Once 'search_engine.exe' is built, choose example and put it's files into executable directory. 
2. Launch search engine and use 'ProcessRequests' command to see example in use.
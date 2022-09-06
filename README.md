Simple console index and search engine with support of multithread indexing and runtime index extension.

Dependencies:\
https://github.com/nlohmann/json \
https://github.com/catchorg/Catch2.git \
Both fetched and installed automatically by CMake


How to:

*'requests.json' and 'config.json' templates will be created in the executable directory on first startup.*

1. Build project with MingW compiler and run 'search_engine.exe'.
2. Fill 'config.json' 'files' section with absolute or relative paths to files you want to present in database for future search.
3. In 'search_engine' console use 'ReloadConfig' command to reindex files.
4. Fill 'requests.json' with arbitrary requests according to template.
5. Run 'ProcessRequests' command to generate 'answers.json' file.
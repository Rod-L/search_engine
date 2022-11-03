## Available search engine settings

Settings listed below are specific for each configuration. \
If you use console engine, you'll have to manualy change its settings in corresponding configuration file. \
Gui subproject allows to change them via user interface.

### auto_dump_index
default : true \
If set to true, index base will be automatically dumped to index.bin file in the configuration directory after each completion of indexation task.

### auto_load_index_dump
default : true \
If set to true, engine will attempt to find and load previously dumped index on configuration startup.

### auto_reindex
default : false \
Engine will automatically start indexation on configuration startup, if none documents are indexed.

### max_indexation_threads
default : 8 \
Max amout of thread to use for indexation tasks.

### max_responses
default : 5 \
Max amount of results to write to "answers.json"

### relative_to_config_folder
default : true \
Relative file paths, listed in "files" section of  configuration file, are relative to configuration file directory

### use_independent_dicts_method
default : false \
Use an alternative indexing method: higher memory consumption, but the more indexing threads, the higher the performance.
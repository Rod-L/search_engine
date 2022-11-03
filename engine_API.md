## Search engine API

Commands available for search engine are listed below in format:\
[numeric code] - [name]\
[description]

Each command can be called by either numeric code or name, i.e.: '0' and 'Exit' are equal commands and will invoke similar behavior.\
Some commands have parameters, as stated in their description. Command and parameter should be separated by space or line feed symbol. In case of passing file path, quotes are not necessary, even if path contain spaces.

#### 0 - Exit
Exit the program.


#### 1 - ProcessRequests
Process 'requests.json' file in executable directory, results will be stored in 'answers.json' in executable directory.


#### 2 - ProcessRequestsFrom
Process requests from file by path (taken as parameter), results will be stored in 'answers.json' in requests file directory;
```
2 ../folder name/filename.json
ProcessRequestsFrom C:/folder name/filename.json
```


#### 3 - ReloadConfig
Reload last loaded 'config.json'.


#### 4 - ReloadConfigFrom
Reload config data from file by path (taken as parameter).
```
4 filename.json
ReloadConfigFrom C:/filename.json
```


#### 5 - SaveConfigTo
Saves current config into file by path (taken as parameter);
```
5 filename.json
SaveConfigTo C:/filename.json
```

#### 6 - CheckFile
Check file by path, whether it valid file for search engine or not.
```
6 filename.json
CheckFile C:/filename.json
```


#### 7 - DumpIndex
Dump index into file for future reload. Creates 'index.bin' in directory of current loaded configuration file.


#### 8 - LoadIndex
Attempts to load index from 'index.bin' in directory of current loaded configuration file.


#### 9 - ExtendIndex
Takes filepath to config.json as parameter, appends documents from passed file's 'files' section to current base.
Files which already present in current base will keep their doc id, their index will be updated.
```
9 filename.json
ExtendIndex C:/folder name/filename.json
```


#### 10 - ReindexFiles
Clear current index and reindex files from loaded configuration file.


#### 11 - ReindexFile
Takes document id as parameter (or few ids separated by spaces), attempts to reindex it.
```
11 1 4 0
ReindexFile 1 4 0
```

#### 12 - BaseStatus
Saves status of loaded/listed files to 'status.json' to directory of last loaded config.


#### 13 - FileStatus
Takes document id as parameter (or few ids separated by spaces), saves its status to 'status_part.json' to directory of last loaded config.
```
13 1 4 0
FileStatus 1 4 0
```

#### 14 - AddFile
Takes filepath as parameter, adds it to the list of files. Does not start reindexing.
```
14 filename.json
AddFile C:/filename.json
```

#### 15 - RemoveFiles
Takes document id as parameter (or few ids separated by spaces), removes these documents from configuration file and index base.\
Does not remove files from filesystem.
```
15 1 4 0
RemoveFiles 1 4 0
```

#### 16 - Help
Show this list

# Mesh

## TODO
- [ ] Remove dependencies on logcpp and tinyxml2 and replace with libQ
- [ ] Fix the Makefile and the include paths
- [ ] Turn ip, udp, and tcp into one file
- [ ] Design a backend agnostic C++ interface (maybe with typedef)
- [ ] Turn netmesh into mesh and make it backend agnostic
- [ ] Turn the ip file into a proper backend
- [ ] Create a wiringPi backend
- [ ] Implement shm as a backend
- [ ] Update README with up-to-date information
- [ ] Finally can set the repo to public again

## TO INSTALL
```
git clone --recurse-submodules https://github.com/IQuesadilla/mesh.git
```

## Public Commands

```
int connect()
    success : return 0
    failure : return errno

int disconnect()
    success : return 0
    failure : return errno

vector<uint8> getFile(string)
    success : return vector of binary data
            : if file empty, return vector of one uint8 '~'
    failure : return empty vector (size = 0)
    arg1    : name of file

vector<string> getFileList()
    success : return vector of all file names
            : if no files available, return vector of one string "~"
    failure : return empty vector (size = 0)

int createFile(string)
    success : return 0
            : update internal list of local files
    failure : return errno

int removeFile(string)
    success : return 0
            : update internal list of local files
    failure : return errno
```

## Private Commands

```
map<string,vector<string> > getNetFileLists()
    success : return map with device name as key and vector of file names as data
            : remove any old file cache
    failure : return empty map (size = 0)

vector<string> getNetFileListFrom(string)
    success : vector of file names
            : if empty, return vector of one string "~"
            : update internal file cache
    failure : return empty vector (size = 0)
    arg1    : name of device to request from

vector<uint8> getNetFileData(string,string)
    success : return vector of binary data
            : if file empty, return vector of one uint8 '~'
    failure : return empty vector (size = 0)
    arg1    : name of device to request from
    arg2    : name of file to request

int putNetFileList(string)
    success : return 0
    failure : return errno
    arg1    : name of device to send to

int putNetFileData(string,vector<uint8>)
    return  : errno or 0 for success
    arg1    : name of device to send to
    arg2    : vector of binary data of file

int updateLocalFile(string,vector<uint8>)
    success : return 0
            : reset local file with data
    failure : return errno
    arg1    : name of file to reset
    arg2    : data to reset to

int resetLocalFiles(map<string,vector<uint8> >)
    success : return 0
            : reset all local files
    failure : return errno
    arg1    : files to reset local files with
```

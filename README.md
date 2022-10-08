# File Mesh

## Public Commands

int connect()
    success : return 0
    failure : return errno

int disconnect()
    success : return 0
    failure : return errno

vector<char> getFile(string)
    success : return vector of binary data
            : if file empty, return vector of one char '~'
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


## Private Commands

map<string,vector<string> > getFileLists()
    success : return map with device name as key and vector of file names as data
            : update internal cache file lists
    failure : return empty map (size = 0)

vector<string> getFileListFrom(string)
    success : vector of file names
            : if empty, return vector of one string "~"
    failure : return empty vector (size = 0)
    arg1    : name of device to request from

vector<char> getFileData(string,string)
    success : return vector of binary data
            : if file empty, return vector of one char '~'
    failure : return empty vector (size = 0)
    arg1    : name of device to request from
    arg2    : name of file to request

int putFileList(string)
    success : return 0
    failure : return errno
    arg1    : name of device to send to

int putFileData(string,vector<char>)
    return  : errno or 0 for success
    arg1    : name of device to send to
    arg2    : vector of binary data of file

int updateLocalFiles(string,vector<char>)
    success : return 0
            : reset local file with data
    failure : return errno
    arg1    : name of file to reset
    arg2    : data to reset to
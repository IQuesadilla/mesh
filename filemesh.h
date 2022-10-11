#ifndef FILEMESH_H
#define FILEMESH_H
#pragma once

#include "netmesh.h"

// testing

class filemesh
{
public:
    filemesh();
    ~filemesh();

    int connect(std::string name);
    int disconnect();

    std::vector<uint8_t> getFile(std::string path);
    std::vector<std::string> getFileList();

    int createFile(std::string path, std::vector<uint8_t> data);
    int removeFile(std::string path);
    int updateFile(std::string path, std::vector<uint8_t> data);

private:
    std::map<std::string,std::vector<std::string> > getNetFileLists();

    std::vector<std::string> getNetFileListFrom(std::string name);
    int putNetFileList(std::string name);

    std::vector<uint8_t> getNetFileData(std::string name, std::string path);
    int putNetFileData(std::string name, std::vector<uint8_t> data);

    int updateLocalFile(std::string path, std::vector<uint8_t> data);
    int resetLocalFiles(std::map<std::string,std::vector<uint8_t> > files);

    std::map<std::string,std::vector<uint8_t> > localfiles;
    std::map<std::string,std::vector<std::string> > filecache;
    std::shared_ptr<netmesh> _netmesh;
};

#endif

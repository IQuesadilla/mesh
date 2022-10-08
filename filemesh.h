#ifndef FILEMESH_H
#define FILEMESH_H
#pragma once

#include "netmesh.h"

class filemesh
{
public:
    filemesh();
    ~filemesh();

    int connect();
    int disconnect();

    std::vector<uint8_t> getFile(std::string path);
    std::vector<std::string> getFileList();

    int createFile(std::string path);
    int removeFile(std::string path);

private:
    std::map<std::string,std::vector<std::string> > getFileLists();

    std::vector<std::string> getFileListFrom(std::string name);
    int putFileList(std::string name);

    std::vector<uint8_t> getFileData(std::string name, std::string path);
    int putFileData(std::string name, std::vector<uint8_t> data);

    int updateLocalFile(std::string path, std::vector<uint8_t> data);
    int resetLocalFiles(std::map<std::string,std::vector<uint8_t> > files);
};

#endif
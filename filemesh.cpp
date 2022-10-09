#include "filemesh.h"

filemesh::filemesh()
{
    _netmesh.reset(new netmesh());
}

filemesh::~filemesh()
{
    delete _netmesh.get();
}

int filemesh::connect(std::string name)
{
    return _netmesh->initserver(name);
}

int filemesh::disconnect()
{
    return _netmesh->killserver();
}

std::vector<uint8_t> filemesh::getFile(std::string path)
{
    auto localit = localfiles.find(path);
    if (localit != localfiles.end())
        return localit->second;

    std::string name;
    auto netfiles = getNetFileLists();
    for (auto &x : netfiles)
        for (auto &y : x.second)
            if (y == path)
                return getNetFileData (x.first, path);

    return *new std::vector<uint8_t>();
}

std::vector<std::string> filemesh::getFileList()
{
    std::vector<std::string> list;
    for (auto &x : localfiles)
        list.push_back(x.first);

    for (auto &x : getNetFileLists())
        std::copy(x.second.begin(),x.second.end(),back_inserter(list));

    if (list.empty())
        return std::vector<std::string>{"~"};
    return list;
}

int filemesh::createFile(std::string path, std::vector<uint8_t> data)
{
    auto localit = localfiles.find(path);
    if (localit == localfiles.end())
        return EEXIST;

    localfiles.insert(std::pair(path,data));

    auto localit = localfiles.find(path);
    if (localit == localfiles.end())
        return ENOSPC;
}

int filemesh::removeFile(std::string path)
{
    auto localit = localfiles.find(path);
    if (localit == localfiles.end())
        return ENOENT;
    localfiles.erase(localit);
    return EXIT_SUCCESS;
}

int filemesh::updateFile(std::string path, std::vector<uint8_t> data)
{
    auto localit = localfiles.find(path);
    if (localit == localfiles.end())
        return ENOENT;
    localfiles[path] = data;
    return EXIT_SUCCESS;
}
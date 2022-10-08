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
    auto netfiles = getFileLists();
    for (auto &x : netfiles)
        for (auto &y : x.second)
            if (y == path)
                return getFileData (x.first, path);

    return *new std::vector<uint8_t>();
}
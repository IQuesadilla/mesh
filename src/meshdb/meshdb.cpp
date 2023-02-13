#include "meshdb.h"

meshdb::meshdb(std::shared_ptr<netmesh> mesh1)
{
    mesh = mesh1;

    mesh->registerUDP("meshdb",recvcallback,this);
}

void meshdb::recvcallback(std::string from, netmesh::netdata* data, void* ptr)
{
    ((meshdb*)ptr)->internalrecvcallback(from,data);
}

void meshdb::internalrecvcallback(std::string from, netmesh::netdata* data)
{
    ;
}
#include "../netmesh/netmesh.h"

class meshdb
{
public:
    meshdb(std::shared_ptr<netmesh> mesh1);

    void insert();

    static void recvcallback(std::string from, netmesh::netdata* data, void* ptr);

private:
    void internalrecvcallback(std::string from, netmesh::netdata* data);
    
    std::shared_ptr<netmesh> mesh;
};
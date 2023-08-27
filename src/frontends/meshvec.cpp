#include "meshvec.h"

meshvec::meshvec(std::string servname, std::shared_ptr<netmesh> ptr)
{
    themesh = ptr;
    _servname = servname;
    themesh->registerUDP(_servname,receiveCallback,this);
}

meshvec::~meshvec()
{
    ;
}

int meshvec::set(int start, int size, char *raw)
{
    return _set(start, size, raw);
}

int meshvec::get(int start, int size, char *raw)
{
   return  _get(start, size, raw);
}

void meshvec::receiveCallback(std::string name, netmesh::netdata* data, void* ptr)
{
    ((meshvec*)ptr)->_receiveCallback(name, data);
}

int meshvec::_set(int start, int size, char *raw)
{
    auto log = themesh->getLogger()->function("_set");
    if ( updateCache(start, size, raw) )
        return -1;

    if ( updateOnline(start, size, raw) )
        return -1;
}

int meshvec::_get(int start, int size, char *raw)
{
    auto log = themesh->getLogger()->function("_get");
    if ( loadOnline(start, size, raw) )
    {
        // LoadOnline failed
    }

    if ( loadCache(start, size, raw) )
        return -1;
}

void meshvec::_receiveCallback(std::string name, netmesh::netdata* data)
{
    int start, size;
    char *raw;

    // Parse incoming data

    updateCache(start, size, raw);
    recvlock.unlock();
}

int meshvec::updateOnline(int start, int size, char *raw)
{
    auto log = themesh->getLogger()->function("updateOnline");
}

int meshvec::updateCache(int start, int size, char *raw)
{
    auto log = themesh->getLogger()->function("updateCache");
}

int meshvec::loadOnline(int start, int size, char *raw)
{
    auto log = themesh->getLogger()->function("loadOnline");

    //netmesh::netdata tosend;
    // Generate message to send

    //themesh->serviceBroadcast(_servname, &tosend);

    recvlock.try_lock();
    recvlock.lock();
}

int meshvec::loadCache(int start, int size, char *raw)
{
    auto log = themesh->getLogger()->function("loadCache");
}
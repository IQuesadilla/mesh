#ifndef MESHVEC_H
#define MESHVEC_H
#pragma once

#include <array>

#include "../netmesh/netmesh.h"

#define VECSIZE 1024

typedef std::array<char,VECSIZE> shvec;

class meshvec
{
public:
    meshvec(std::string servname, std::shared_ptr<netmesh> ptr);
    ~meshvec();

    char& operator[](int index);

    int set(int start, int size, char *raw);
    int get(int start, int size, char *raw);

    static void receiveCallback(std::string name, netmesh::netdata* data,void* ptr);

private:
    int _set(int start, int size, char *raw);
    int _get(int start, int size, char *raw);

    void _receiveCallback(std::string name, netmesh::netdata* data);

    int updateOnline(int start, int size, char *raw);
    int updateCache(int start, int size, char *raw);

    int loadOnline(int start, int size, char *raw);
    int loadCache(int start, int size, char *raw);

    shvec cache;
    std::mutex recvlock;
    std::string _servname;
    std::shared_ptr<netmesh> themesh;
};

#endif
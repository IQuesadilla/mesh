#ifndef MESHVEC_H
#define MESHVEC_H
#pragma once

#include <array>

#include "netmesh.h"

#define VECSIZE 1024

typedef std::array<char,VECSIZE> shvec;

class meshvec
{
public:
    meshvec(std::shared_ptr<netmesh> ptr);
    ~meshvec();

    shvec get();
    void set(shvec);

private:
    shvec cache;
    std::shared_ptr<netmesh> themesh;
};

#endif
#ifndef IP_UDP_H
#define IP_UDP_H
#pragma once

#include "ip.h"
#include <string>

class udp : public ip
{
public:
    udp() {};

    bool initSocket(uint setport);

    int bindaddr(std::string addr = "0.0.0.0");

private:
    //int fd;
};

#endif
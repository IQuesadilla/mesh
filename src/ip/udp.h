#ifndef IP_UDP_H
#define IP_UDP_H
#pragma once

#include "ip.h"
#include <string>

class udp : public ip
{
public:
    udp() {};

    bool initSocket(uint setport, std::string addr = "0.0.0.0");
    bool testconn(uint setport);

private:
    //int fd;
};

#endif
#ifndef IP_UDP_H
#define IP_UDP_H
#pragma once

#include "ip.h"
#include <string>

class udp : public ip
{
public:
    //udp() { fd = -1; };
    udp(std::shared_ptr<logcpp> logobj, uint port = 0);

    bool initSocket(uint setport);

    int bindaddr(std::string addr = "0.0.0.0");

private:
    //int fd;
};

#endif
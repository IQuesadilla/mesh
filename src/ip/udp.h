#ifndef IP_UDP_H
#define IP_UDP_H
#pragma once

#include "ip.h"
#include <string>

class udp : public ip
{
public:
    //udp() { fd = -1; };
    udp(std::shared_ptr<logcpp> logobj, in_addr_t addr, int port = 0);

    bool initSocket();

private:
    //int fd;
};

#endif
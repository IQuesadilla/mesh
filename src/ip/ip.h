#ifndef IP_IP_H
#define IP_IP_H
#pragma once

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <poll.h>
#include <vector>
#include <string>
#include <memory>
#include "../../logcpp/logcpp.h"

struct packet
{
    const char *raw;
    ssize_t length;
    in_addr_t addr;
};

class ip
{
public:
    //ip() { fd = -1; }
    //ip(uint port) { initSocket(port); }
    ~ip() { close(fd); }

    void setLogcpp(std::shared_ptr<logcpp> logptr);

    virtual bool initSocket() = 0;

    bool send(const packet raw);
    const packet recv();

    int getport();
    int bindaddr(in_addr_t addr, int port = 0);

    pollfd topoll(short int events);

protected:
    int fd;
    std::shared_ptr<logcpp> logobj;
};

#endif
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

struct packet
{
    const char *raw;
    size_t length;
    in_addr_t addr;
};

class ip
{
public:
    ip() { fd = -1; }
    ip(uint port) { initSocket(port); }
    ~ip() { close(fd); }

    virtual bool initSocket(uint port) = 0;

    bool send(const packet raw);
    const packet recv();

    pollfd *topoll(short int events);

protected:
    int fd;
    int port;
};

#endif
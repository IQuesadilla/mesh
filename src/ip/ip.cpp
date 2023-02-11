#include "ip.h"

#include <iostream>
#include <cstring>
#include <thread>
#include <sys/ioctl.h>
#include <poll.h>

void ip::setLogcpp(std::shared_ptr<logcpp> logptr)
{
    logobj = logptr;
    auto log = logobj->function("ip::setlogcpp");
}

bool ip::send(const packet raw)
{
    auto log = logobj->function("ip::send");
    log << "Length: " << raw.length << logcpp::loglevel::VALUE;
    log << "Address: " << inet_ntoa( in_addr{raw.addr} ) << logcpp::loglevel::VALUE;
    log << "Port: " << ntohs(getport()) << logcpp::loglevel::VALUE;

    sockaddr_in who;
    memset (&who, '\0', sizeof(who));

    who.sin_family = AF_INET;
    who.sin_port = getport();
    who.sin_addr.s_addr = raw.addr;

    ssize_t bytes_sent = sendto(fd,raw.raw,raw.length,0,(sockaddr*)&who,sizeof(who));
    if (bytes_sent == raw.length)
        return true;
    else
        return false;
}

const packet ip::recv()
{
    auto log = logobj->function("ip::recv");
    packet toreturn;

    int size;
    ioctl(fd, FIONREAD, &size);
    log << "Size: " << size << logcpp::loglevel::VALUE;

    toreturn.raw = (char*)malloc(size);

    log << "Waiting for data to receive" << logcpp::loglevel::NOTE;

    struct sockaddr_in fromaddr;
    socklen_t fromaddrlen = sizeof(fromaddr);

    int tempcount = recvfrom(fd,(void*)toreturn.raw,size,MSG_DONTWAIT,(struct sockaddr*)&fromaddr,&fromaddrlen);
    log << "tempcount: " << tempcount << logcpp::loglevel::VALUE;
    log << "errno: " << errno << logcpp::loglevel::VALUE;
    log << "fd: " << fd << logcpp::loglevel::VALUE;
    log << "From: " << inet_ntoa(fromaddr.sin_addr) << logcpp::loglevel::VALUE;

    toreturn.length = tempcount;
    toreturn.addr = fromaddr.sin_addr.s_addr;

    return toreturn;
}

int ip::getport()
{
    sockaddr_in sin;
    socklen_t len = sizeof(sin);
    getsockname(fd,(struct sockaddr *)&sin, &len);
    return sin.sin_port;
}

int ip::bindaddr(in_addr_t addr, int port /*= 0*/)
{
    auto log = logobj->function("ip::bindaddr");
    sockaddr_in ipaddr;
    memset (&ipaddr, '\0', sizeof(ipaddr));

    ipaddr.sin_family = AF_INET;
    ipaddr.sin_addr.s_addr = addr;
    ipaddr.sin_port = port;
    log << "Address to bind: " << inet_ntoa(ipaddr.sin_addr) << logcpp::loglevel::VALUE;

    errno = 0;
    if (bind(fd, (const struct sockaddr *)&ipaddr, sizeof(ipaddr)) < 0)
    {
        log << "Failed to bind socket" << logcpp::loglevel::WARNING;
        log << "Errno: " << errno << logcpp::loglevel::VALUE;
        return 1;
    }
    log << "Port: " << getport() << logcpp::loglevel::VALUE;
    return 0;
}

pollfd ip::topoll(short int events)
{
    auto log = logobj->function("ip::topoll");
    pollfd toreturn;
    toreturn.fd = fd;
    toreturn.events = events;
    toreturn.revents = 0;
    return toreturn;
}
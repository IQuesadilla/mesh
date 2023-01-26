#include "ip.h"

#include <iostream>
#include <cstring>
#include <thread>
#include <sys/ioctl.h>
#include <poll.h>

#define RECVBLKSIZE 4

bool ip::send(const packet raw)
{
    std::cout << "Log: (send)" << std::endl;
    std::cout << "Value: Length: " << raw.length << std::endl;
    std::cout << "Value: Address: " << inet_ntoa( in_addr{raw.addr} ) << std::endl;
    std::cout << "Value: Port: " << port << std::endl;

    sockaddr_in who;
    memset (&who, '\0', sizeof(who));

    who.sin_family = AF_INET;
    who.sin_port = htons(port);
    who.sin_addr.s_addr = raw.addr;

    ssize_t bytes_sent = sendto(fd,raw.raw,raw.length,0,(sockaddr*)&who,sizeof(who));
    return true;
}

const packet ip::recv()
{
    std::cout << "Log: (recv)" << std::endl;
    packet toreturn;

    int size;
    ioctl(fd, FIONREAD, &size);
    std::cout << "Value: Size: " << size << std::endl;

    toreturn.raw = (char*)malloc(size);
    int total = 0;

    std::cout << "Log: Waiting for data to receive" << std::endl;

    struct sockaddr_in *fromaddr = new struct sockaddr_in;
    socklen_t fromaddrlen;

    int tempcount = recvfrom(fd,(void*)toreturn.raw,size,MSG_DONTWAIT,(struct sockaddr*)fromaddr,&fromaddrlen);
    std::cout << "Value: tempcount: " << tempcount << std::endl;
    std::cout << "Value: errno: " << errno << std::endl;
    std::cout << "Value: fd: " << fd << std::endl;
    std::cout << "Value: From: " << inet_ntoa(fromaddr->sin_addr) << std::endl;

    toreturn.length = tempcount;

    return toreturn;
}

pollfd *ip::topoll(short int events)
{
    pollfd *toreturn = new pollfd;
    toreturn->fd = fd;
    toreturn->events = events;
    toreturn->revents = 0;
    return toreturn;
}
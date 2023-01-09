#include "udp.h"

#include <cstring>
#include <iostream>

bool udp::initSocket(uint setport, std::string addr /*= "0.0.0.0"*/)
{
    std::cout << "Log: (initSocket)" << std::endl;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        std::cout << "Error: Failed to open socket" << std::endl;
        return errno;
    }

    std::cout << "Value: fd: " << fd << std::endl;

    //std::cout << bcsock << std::endl;
    const int trueFlag = 1;
    if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&trueFlag,sizeof(trueFlag)) < 0)
    {
        std::cout << "Error: Failed to setsockopt" << std::endl;
        return errno;
    }

    port = setport;

    sockaddr_in udpaddr;
    memset (&udpaddr, '\0', sizeof(udpaddr));

    udpaddr.sin_family = AF_INET;
    udpaddr.sin_addr.s_addr = inet_addr(addr.c_str()); // TODO might not work? untested
    udpaddr.sin_port = htons(port);

    if (bind(fd, (const struct sockaddr *)&udpaddr, sizeof(udpaddr)) < 0)
    {
        std::cout << "Error: Failed to bind socket" << std::endl;
        return errno;
    }
    return 0;
}

bool udp::testconn(uint setport)
{
    std::cout << "Log: (testconn)" << std::endl;
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        std::cout << "Error: Failed to open socket" << std::endl;
        return errno;
    }

    std::cout << "Value: fd: " << fd << std::endl;

    //std::cout << bcsock << std::endl;
    const int trueFlag = 1;
    if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&trueFlag,sizeof(trueFlag)) < 0)
    {
        std::cout << "Error: Failed to setsockopt" << std::endl;
        return errno;
    }

    port = setport;
    return true;
}
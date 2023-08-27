#include "udp.h"

#include <cstring>
#include <iostream>

udp::udp(std::shared_ptr<logcpp> logobj, in_addr_t addr, int port /*= 0*/)
{
    setLogcpp(logobj);
    initSocket();
    
    if (port < 0)
        return;

    bindaddr(addr, port);

}

bool udp::initSocket()
{
    auto log = logobj->function("udp::initSocket");
    if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        log << "Failed to open socket" << logcpp::loglevel::ERROR;
        return errno;
    }

    log << "fd: " << fd << logcpp::loglevel::VALUE;

    /*
    //std::cout << bcsock << std::endl;
    const int trueFlag = 1;
    if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&trueFlag,sizeof(trueFlag)) < 0)
    {
        std::cout << "Error: Failed to setsockopt" << std::endl;
        return errno;
    }
    */

    //port(setport);
    return true;
}

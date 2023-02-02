#include "../../src/ip/ip.h"
#include "../../src/ip/udp.h"

#include <vector>
#include <thread>
#include <iostream>

#define PORT 1739

void server(ip *socket)
{
    std::cout << "Log: (server)" << std::endl;
    for (int i = 0; i < 5; ++i)
    {
        auto p = socket->topoll(POLLIN);
        poll(&p,1,-1);

        packet raw = socket->recv();
        if (raw.length > 0)
            std::cout << "Value: " << std::string(raw.raw,raw.length) << std::endl;
        
        free((void*)raw.raw);
    }
    std::cout << "Log: Closing server" << std::endl;
}

void client(ip *socket)
{
    std::cout << "Log: (client)" << std::endl;
    const std::string data = "It works! Yaaaayeee";
    packet raw;
    raw.raw = data.c_str();
    raw.length = data.length();
    raw.addr = inet_addr("127.0.0.1");


    for (int i = 0; i < 5; ++i)
    {
        socket->send(raw);
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    }

    socket->~ip();
    std::cout << "Log: Closing client" << std::endl;
}

int main(int argc, char *argv[])
{
    std::shared_ptr<logcpp> logobj;
    logobj.reset(new logcpp(logcpp::vlevel::DEFAULT));
    auto log = logobj->function("main");

    udp serversocket(logobj), clientsocket(logobj);
    serversocket.initSocket(PORT);
    clientsocket.initSocket(PORT);
    serversocket.bindaddr();

    std::thread serverthread(server,&serversocket);
    std::thread clientthread(client,&clientsocket);

    serverthread.join();
    clientthread.join();

    //client(&clientsocket);
    //server(&serversocket);

    return 0;
}
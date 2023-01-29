#ifndef NETMESH_H
#define NETMESH_H
#pragma once

#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <memory.h>
#include <poll.h>
#include <fcntl.h>
#include <error.h>
#include <ifaddrs.h>
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>
#include <sstream>
#include <map>
#include <memory>
#include <functional>
#include "tinyxml2/tinyxml2.h"
#include "src/ip/udp.h"
#include "logcpp/logcpp.h"

#define BCPORT 1999
#define TCPPORT 1998
#define UPDATETIME 2000
#define TIMEOUTTIME 10000

/*
    TODO: Add proper logging to ip classes
    TODO: Build out a larger protocol
        - Request mesh info, anyone answers
        - Broadcast internal value update
            - Update time
            - Timeout time
        - Leaving the mesh
*/

class netmesh
{
public:
    typedef std::vector<char> netdata;
    enum iptype
    {
        UDP,
        TCP
    };

    netmesh(std::shared_ptr<logcpp> log);
    ~netmesh();

    int initserver(std::string name, std::string mesh);
    int killserver();

    void enableLogging(std::shared_ptr<logcpp> log);

    std::vector<std::string> findAvailableMeshes();

    int initBroadcastSocket(std::string addr);
    //int initListenSocket(std::string myaddr = "0.0.0.0");

    bool isConnected();

    int serviceSend(std::string devname, std::string servname, netdata *data);

    std::string returnDevices();
    int updateDeviceList();

    uint16_t registerUDP(std::string servname, std::function<void(std::string,netdata*)> fn);

    bool getBroadcastAlive() { return flags.broadcastalive; }
    bool setBroadcastAlive(bool in) { return (flags.broadcastalive = in); }
    std::string getName() { return myName; }
    std::string setName(std::string in) { return (myName = in); }

    static void run(netmesh *mynetmesh) { mynetmesh->updateThread(); };

private:
    struct service
    {
        uint16_t port;
        iptype ipt;
    };

    struct device
    {
        in_addr_t address;
        std::chrono::_V2::system_clock::time_point timeout;
        std::map<std::string,service> services;
    };

    struct myservice
    {
        std::string name;
        iptype ipt;
        std::shared_ptr<ip> connptr;
        std::function<void(std::string,netdata*)> callback;
    };


    int broadcastAlive();
    
    std::pair<std::string,device> parseUpdate(const char *xml);
    tinyxml2::XMLPrinter *generateUpdate();
    //int checkforconn();
    uint16_t findAvailablePort();
    bool pollAll(std::chrono::milliseconds timeout);
    void updateThread();

    struct
    {
        bool broadcastalive;
    } flags;


    //int listensock; // TODO Initialize udpsock
    std::shared_ptr<udp> bcsock;
    std::chrono::_V2::system_clock::time_point last_update;
    std::string myName;
    bool connected;
    sockaddr_in bcaddr;
    std::map<std::string, device> devices;
    std::map<uint16_t,myservice> myservices;
    std::shared_ptr<udp> sockGeneral;
    std::shared_ptr<logcpp> logobj;
};

#endif
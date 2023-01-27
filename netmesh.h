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

#define BCADDR "192.168.123.255"

#define BCPORT 1999
#define TCPPORT 1998
#define UDPPORT 1997
#define DEVICE_TIMEOUT 1000
#define POLLDELAY 50
#define UPDATETIME 2000
#define TIMEOUTTIME 10000

#define AFREQ 56320
#define AFORMAT AUDIO_S32MSB
#define ACHANNELS 2
#define ASAMPLES 64
#define POLLCOUNT UPDATETIME / POLLDELAY

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

    int initUpdateThread();
    int initBroadcastSocket(std::string addr);
    //int initListenSocket(std::string myaddr = "0.0.0.0");

    bool isConnected();

    int conntodevice(std::string devname);

    int sendraw(std::string to, netdata *data);
    int recvraw(std::string from, netdata *data);

    std::string returnDevices();
    int updateDeviceList(std::chrono::milliseconds timeout);

    uint16_t registerUDP(std::string servname, std::function<void(char*,int)> fn);

    bool getBroadcastAlive() { return flags.broadcastalive; }
    bool setBroadcastAlive(bool in) { return (flags.broadcastalive = in); }
    bool getEnableUpdateThread() { return flags.enableupdatethread; }
    bool setEnableUpdateThread(bool in) { return (flags.enableupdatethread = in); }
    std::string getName() { return myName; }
    std::string setName(std::string in) { return (myName = in); }

private:
    struct service
    {
        uint16_t port;
        iptype ip;
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
        std::shared_ptr<ip> connptr;
        std::function<void(char*,int)> callback;
    };


    int broadcastAlive();
    
    std::pair<std::string,device> parseUpdate(const char *xml);
    tinyxml2::XMLPrinter *generateUpdate(std::map<std::string,service> services);
    //int checkforconn();
    uint16_t findAvailablePort();
    void pollAll(std::chrono::milliseconds timeout);
    friend void updateThread(netmesh *mynetmesh);

    struct
    {
        bool broadcastalive;
        bool enableupdatethread;
    } flags;


    //int listensock; // TODO Initialize udpsock
    std::shared_ptr<udp> bcsock;
    std::chrono::_V2::system_clock::time_point last_update;
    std::string myName;
    bool connected;
    sockaddr_in bcaddr;
    std::map<std::string, device> devices;
    std::map<uint16_t,myservice> myservices;
    std::thread myUpdateThread;
    std::shared_ptr<udp> sockGeneral;
    std::shared_ptr<logcpp> logobj;
};

#endif
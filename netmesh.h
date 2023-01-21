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
#define UPDATETIME 200
#define TIMEOUTTIME 1000
#define BUFFLEN 2000
#define MAXNAMELEN 20

#define AFREQ 56320
#define AFORMAT AUDIO_S32MSB
#define ACHANNELS 2
#define ASAMPLES 64
#define POLLCOUNT UPDATETIME / POLLDELAY

class netmesh
{
public:
    typedef std::vector<char> netdata;

    netmesh(std::shared_ptr<logcpp> log);
    ~netmesh();

    int initserver(std::string name, std::string mesh);
    int killserver();

    void enableLogging(std::shared_ptr<logcpp> log);

    std::vector<std::string> findAvailableMeshes();

    int initUpdateThread();
    int initBroadcastSocket(std::string addr);
    int initListenSocket(std::string myaddr = "0.0.0.0");

    bool isConnected() { return connected; }

    int conntodevice(std::string devname);

    int sendraw(std::string to, netdata *data);
    int recvraw(std::string from, netdata *data);

    std::string returnDevices();
    int updateDeviceList(std::chrono::milliseconds timeout);

    uint16_t registerUDP(std::function<void(short int)> fn);

    bool getBroadcastAlive() { return flags.broadcastalive; }
    bool setBroadcastAlive(bool in) { return (flags.broadcastalive = in); }
    bool getEnableUpdateThread() { return flags.enableupdatethread; }
    bool setEnableUpdateThread(bool in) { return (flags.enableupdatethread = in); }
    std::string getName() { return myName; }
    std::string setName(std::string in) { return (myName = in); }

private:
    int broadcastAlive();
    int checkforconn();
    uint16_t findAvailablePort();
    void pollAll(std::chrono::milliseconds timeout);
    static void updateThread(netmesh *mynetmesh);

    struct device
    {
        in_addr_t address;
        // std::shared_ptr<ip> devconn;
        std::chrono::_V2::system_clock::time_point timeout;
    };

    struct connection
    {
        std::string devname;
        std::shared_ptr<ip> connptr;
        std::function<void(short int)> callback;
    };

    struct
    {
        bool broadcastalive;
        bool enableupdatethread;
    } flags;

    int listensock, udpsock; // TODO Initialize udpsock
    std::shared_ptr<udp> bcsock;
    std::chrono::_V2::system_clock::time_point last_update;
    std::string myName;
    bool connected;
    sockaddr_in bcaddr;
    std::map<std::string, device> devices;
    std::map<uint16_t,connection> connections;
    std::thread myUpdateThread;
    std::shared_ptr<udp> sockGeneral;
    std::shared_ptr<logcpp> logobj;
};

#endif
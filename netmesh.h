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
#include <thread>
#include <mutex>
#include <vector>
#include <chrono>
#include <sstream>
#include <map>
#include <memory>
#include "tinyxml2/tinyxml2.h"
#include "src/ip/udp.h"

#define BCADDR "192.168.123.255"
#define MYADDR "192.168.123."

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

    netmesh();
    ~netmesh() {}

    int initserver(std::string name);
    int killserver();

    int initUpdateThread();
    int initBroadcastSocket();
    int initListenSocket();

    bool isConnected() { return connected; }

    int conntodevice(std::string devname);

    int sendraw(std::string to, netdata *data);
    int recvraw(std::string from, netdata *data);

    std::string returnDevices();
    int updateDeviceList();

    bool getBroadcastAlive() { return flags.broadcastalive; }
    bool setBroadcastAlive(bool in) { return (flags.broadcastalive = in); }
    bool getEnableUpdateThread() { return flags.enableupdatethread; }
    bool setEnableUpdateThread(bool in) { return (flags.enableupdatethread = in); }
    std::string getName() { return myName; }
    std::string setName(std::string in) { return (myName = in); }

private:
    int broadcastAlive();
    int checkforconn();
    static void updateThread(netmesh *mynetmesh);

    struct device
    {
        in_addr_t address;
        //std::shared_ptr<ip> devconn;
        std::chrono::_V2::system_clock::time_point timeout;
    };

    struct connection
    {
        std::string devname;
        int connsock;
    };

    struct
    {
        bool broadcastalive;
        bool enableupdatethread;
    } flags;

    int bcsock, listensock, udpsock; // TODO Initialize udpsock
    std::string myName;
    bool connected;
    sockaddr_in bcaddr;
    std::map<std::string, device> devices;
    std::vector<connection> connections;
    std::thread myUpdateThread;
    std::shared_ptr<udp> sockGeneral;
};

#endif
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
#include "tinyxml2.h"

#define BCADDR "10.42.0.255"
#define MYADDR "10.42.0.104"

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

class mesh
{
public:
    mesh();
    ~mesh() {}

    struct message
    {
        std::string name;
        std::shared_ptr<std::vector<uint8_t> > data;
    };

    std::mutex dumbshitsafety;
    std::vector<uint8_t> dumbshit;
    int cocksafety = 0;

    int initserver(std::string name);
    int killserver();

    int initUpdateThread();
    int initBroadcastSocket();
    int initListenSocket();
    int initUDPSocket();

    bool isConnected() {return connected;}
    //std::string returnErrorMessage(ERRCODES err);

    int conntodevice(std::string devname);
    int sendTCP(message value);
    message recieveTCP(std::string name);

    int sendUDP(message value);
    message receiveUDP();
    message receiveUDP(std::string from);

    std::string returnDevices();
    int updateDeviceList();

    bool getBroadcastAlive() {return flags.broadcastalive;}
    bool setBroadcastAlive(bool in) {return (flags.broadcastalive = in);}
    bool getEnableUpdateThread() {return flags.enableupdatethread;}
    bool setEnableUpdateThread(bool in) {return (flags.enableupdatethread = in);}
    std::string getName() {return myName;}
    std::string setName(std::string in) {return (myName = in);}
private:

    int broadcastAlive();
    int checkforconn();
    static void updateThread(mesh *mymesh);

    struct device
    {
        std::string name;
        in_addr_t address;
        std::chrono::_V2::system_clock::time_point timeout;
    };

    struct connection
    {
        std::string devname;
        int connsock;
    };

    struct {
        bool broadcastalive;
        bool enableupdatethread;
    } flags;

    int bcsock, listensock, udpsock; // TODO Initialize udpsock
    std::string myName;
    bool connected;
    sockaddr_in bcaddr;
    std::vector<device> devices;
    std::vector<connection> connections;
    //std::map<std::string,int> receivebuffer;
    std::thread myUpdateThread;
};

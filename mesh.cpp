#include "mesh.h"

mesh::mesh()
{
    setBroadcastAlive(true);
    setEnableUpdateThread(true);
}

int mesh::initserver(std::string name)
{
    if (connected == true)
        return 0;

    connected = false;
    setName(name);
    initBroadcastSocket();
    initListenSocket();
    initUDPSocket();

    myName = name;
    connected = true;
    if (getEnableUpdateThread()) initUpdateThread();
    return 0;
}

int mesh::killserver()
{
    connected = false;
    if (getEnableUpdateThread()) myUpdateThread.join();
    return 0;
}

int mesh::initUpdateThread()
{
    myUpdateThread = std::thread(updateThread,this);
    setEnableUpdateThread(true);
    return 0;
}

int mesh::initBroadcastSocket()
{
    connected = false;
    if ((bcsock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        return errno;

    const int trueFlag = 1;
    if (setsockopt(bcsock,SOL_SOCKET,SO_REUSEADDR,&trueFlag,sizeof(trueFlag)) < 0)
        return errno;
    if (setsockopt(bcsock,SOL_SOCKET,SO_BROADCAST,&trueFlag,sizeof(trueFlag)) < 0)
        return errno;

    memset (&bcaddr, '\0', sizeof(bcaddr));

    bcaddr.sin_family = AF_INET;
    bcaddr.sin_addr.s_addr = inet_addr(BCADDR);
    bcaddr.sin_port = htons(BCPORT);

    if (bind(bcsock, (const struct sockaddr *)&bcaddr, sizeof(bcaddr)) < 0)
        return errno;

    connected = true;
    return 0;
}

int mesh::initListenSocket()
{
    if ((listensock = socket(AF_INET,SOCK_STREAM,0)) < 0)
        return errno;

    fcntl(listensock,F_SETFL,fcntl(listensock,F_GETFL) | O_NONBLOCK);

    const int trueFlag = 1;
    if (setsockopt(listensock,SOL_SOCKET,SO_REUSEADDR,&trueFlag,sizeof(trueFlag)) < 0)
        return errno;

    sockaddr_in listenaddr;
    memset (&listenaddr, '\0', sizeof(listenaddr));

    listenaddr.sin_family = AF_INET;
    listenaddr.sin_addr.s_addr = inet_addr(MYADDR);//htonl(INADDR_ANY);
    listenaddr.sin_port = htons(TCPPORT);

    if (bind(listensock, (const struct sockaddr *)&listenaddr, sizeof(listenaddr)) < 0)
        return errno;
    return 0;
}

int mesh::initUDPSocket()
{
    if ((udpsock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        return errno;

    //std::cout << bcsock << std::endl;
    const int trueFlag = 1;
    if (setsockopt(udpsock,SOL_SOCKET,SO_REUSEADDR,&trueFlag,sizeof(trueFlag)) < 0)
        return errno;

    sockaddr_in listenaddr, udpaddr;
    memset (&udpaddr, '\0', sizeof(udpaddr));

    udpaddr.sin_family = AF_INET;
    udpaddr.sin_addr.s_addr = htonl(INADDR_ANY); // TODO might not work? untested
    udpaddr.sin_port = htons(UDPPORT);

    if (bind(udpsock, (const struct sockaddr *)&udpaddr, sizeof(udpaddr)) < 0)
        return errno;
    return 0;
}

/*
std::string mesh::returnErrorMessage(ERRCODES err)
{
    switch (err)
    {
        case NOERROR:       return ""; break;//"No Error Detected"; break;
        case SOCKET:        return "Error Creating Socket"; break;
        case SETSOCKOPT:    return "Error Setting the Socket Option"; break;
        case BIND:          return "Error Binding the Socket"; break;
        default:            return "Not a Mesh Defined Error Code"; break;
    }
}
*/
int mesh::sendTCP(mesh::message value)
{
    int sendsock = -1;
    for (auto &x : connections)
        sendsock = (x.devname == value.name) ? x.connsock : sendsock;
    if (sendsock == -1)
        return 1;

    std::vector<uint8_t> prefix {'-','-','<'};
    value.data.insert(value.data.begin(),prefix.begin(),prefix.end());
    int count = send (sendsock, value.data.data(), value.data.size(),0);
    if (count == -1)
        return 1;
    return 0;
}

mesh::message mesh::recieveTCP(std::string name)
{
    int recvsock = -1;
    for (auto &x : connections)
        recvsock = (x.devname == name) ? x.connsock : recvsock;
    if (recvsock == -1)
        return {std::string(),std::vector<uint8_t>()};

    char *recvdata = (char*)malloc(BUFFLEN);
    std::vector<uint8_t> recvbuff;
    int count = recv (recvsock, recvdata, BUFFLEN, MSG_DONTWAIT);
    if (count == -1)
        return {std::string(),std::vector<uint8_t>()};
    recvbuff.insert(recvbuff.end(),&recvdata[0],&recvdata[count]);

    std::vector<int8_t> prefix {'-','-','<'};
    if (!std::equal(prefix.begin(),prefix.end(),recvbuff.begin()))
        return {*new std::string,*new std::vector<uint8_t>};

    return {name,recvbuff};
}

int mesh::sendUDP(mesh::message value)
{
    in_addr_t sendaddr = -1;
    for (auto &x : devices)
        sendaddr = (x.name == value.name) ? x.address : sendaddr;
    if (sendaddr == -1)
        return -1;

    sockaddr_in udpaddr;
    memset (&udpaddr,'\0',sizeof(udpaddr));
    udpaddr.sin_family = AF_INET;
    udpaddr.sin_addr.s_addr = sendaddr;
    udpaddr.sin_port = htons(UDPPORT);

    std::vector<uint8_t> prefix {'-','-','<'};
    value.data.insert(value.data.begin(),prefix.begin(),prefix.end());
    int count = sendto (udpsock, value.data.data(), value.data.size(),0,(const struct sockaddr *)&udpaddr,sizeof(udpaddr));
    return count - 3;
}

mesh::message mesh::receiveUDP()
{
    std::string recvname;
    std::vector<uint8_t> recvbuff;
    char *recvdata = (char*)malloc(BUFFLEN);

    sockaddr_in recvaddr;
    socklen_t recvaddrlen = sizeof(recvaddr);
    int count = 0;
    while (true)
    {
        int tempcount = recvfrom (udpsock, recvdata, BUFFLEN, MSG_DONTWAIT, (struct sockaddr *) &recvaddr, &recvaddrlen);
        if (tempcount == -1)
            break;
        recvbuff.insert(recvbuff.end(),&recvdata[0],&recvdata[tempcount]);
    }

    if (recvbuff.size() <= 0)
        return {std::string(),std::vector<uint8_t>()};

    std::vector<int8_t> prefix {'-','-','<'};
    if (!std::equal(prefix.begin(),prefix.end(),recvbuff.begin()))
        return {std::string(),std::vector<uint8_t>()};

    
    std::cout << recvbuff.size() << " " << std::flush;

    for (auto &d : devices)
        recvname = (recvaddr.sin_addr.s_addr == d.address) ? d.name : "";

    return {recvname,recvbuff};
}

std::string mesh::returnDevices()
{
    std::stringstream ss;

    ss << "Connected Devices: " << std::endl;
    for (auto &x : devices)
        ss  << "Name: " << x.name << std::endl\
            << "  Address: " << x.address << std::endl\
            << "  Timeout: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - x.timeout).count() << std::endl;

    return ss.str();
}

int mesh::broadcastAlive()
{
    std::string message = "++<"+myName;
    int count = sendto (bcsock, message.c_str(), message.length()+1, MSG_CONFIRM, (const struct sockaddr *) &bcaddr, sizeof(bcaddr));
    if (count == -1)
        std::cout << "oof" << std::endl;
    return 0;
}

int mesh::updateDeviceList()
{
    char *recvbuff = (char*)malloc(BUFFLEN),
         *namebuff = (char*)malloc(BUFFLEN),
         *addrbuff = (char*)malloc(BUFFLEN);
    int count = 0;
    while (1)
    {
        sockaddr_in recvaddr;
        socklen_t recvaddrlen = sizeof(recvaddr);
        int count = recvfrom (bcsock, recvbuff, BUFFLEN, MSG_DONTWAIT, (sockaddr *) &recvaddr, &recvaddrlen);
        if (count == -1)
            {break;}

        recvbuff[count] = '\0';
        sscanf(recvbuff,"++<%s",namebuff);
        std::string namestring = namebuff;
        if (namestring == myName)
            return 0;
        auto settimeout = std::chrono::system_clock::now();

        bool foundDevice = false;
        for (auto &x : devices)
        {
            if (x.name != namestring)
                continue;
            
            x.address = recvaddr.sin_addr.s_addr;
            x.timeout = settimeout;
            foundDevice = true;
        }

        if (!foundDevice)
        {
            devices.push_back({ namestring,
                                recvaddr.sin_addr.s_addr,
                                settimeout});
            std::cout << "Device Added!" << std::endl << returnDevices();
        }
    }

    for (int i = devices.size()-1; i >= 0; --i)
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - devices[i].timeout) >= std::chrono::milliseconds(TIMEOUTTIME))
            devices.erase(devices.begin()+i);

    return 0;
}

int mesh::checkforconn()
{
    if (listen(listensock,10) > -1)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
            return 0;
        std::cout << "something fucked up " << errno << std::endl;
        std::string name;
        sockaddr_in newaddr;
        socklen_t newaddrlen = sizeof(newaddr);
        int newfd = accept(listensock,(sockaddr*)&newaddr,&newaddrlen);
        for (auto &x : devices)
            name = (newaddr.sin_addr.s_addr == x.address) ? x.name : name;
        connections.push_back({name,newfd});
    }
    return 0;
}

void mesh::updateThread(mesh *mymesh)
{
    while (mymesh->isConnected())
    {
        for (int i = 0; i < POLLCOUNT; ++i)
        {
            mymesh->updateDeviceList();
            //mymesh->receiveUDP();
            //mymesh->checkforconn();
            std::this_thread::sleep_for(std::chrono::milliseconds(POLLDELAY));
        }

        if (mymesh->getBroadcastAlive()) mymesh->broadcastAlive();
        //std::cout << returnDevices();
    }
}
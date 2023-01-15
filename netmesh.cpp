#include "netmesh.h"

netmesh::netmesh(std::shared_ptr<logcpp> logptr)
{
    enableLogging(logptr);

    auto log = logobj->function("netmesh");
    setBroadcastAlive(true);
    setEnableUpdateThread(true);
}

int netmesh::initserver(std::string name, std::string mesh)
{
    auto log = logobj->function("initserver");
    if (connected == true)
        return 0;

    connected = false;
    setName(name);
    initBroadcastSocket(mesh);

    sockGeneral.reset(new udp());
    sockGeneral->initSocket(UDPPORT);
    sockGeneral->bindaddr();

    myName = name;
    connected = true;
    if (getEnableUpdateThread())
        initUpdateThread();
    return 0;
}

int netmesh::killserver()
{
    auto log = logobj->function("killserver");
    connected = false;
    if (getEnableUpdateThread())
        myUpdateThread.join();
    return 0;
}

void netmesh::enableLogging(std::shared_ptr<logcpp> log)
{
    logobj = log;
    auto log = logobj->function("enableLogging");
}

std::vector<std::string> netmesh::findAvailableMeshes()
{
    auto log = logobj->function("findAvailableMeshes");
    std::vector<std::string> to_return;

    struct ifaddrs * ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_ifu.ifu_broadaddr == NULL)
            continue;

        std::string tempbcaddr = inet_ntoa(((sockaddr_in*)ifa->ifa_ifu.ifu_broadaddr)->sin_addr);
        //std::cout << "Log: Available mesh: " << tempbcaddr<< std::endl;
        to_return.push_back(tempbcaddr);
    }

    return to_return;
}

int netmesh::initUpdateThread()
{
    auto log = logobj->function("initUpdateThread");
    myUpdateThread = std::thread(updateThread, this);
    setEnableUpdateThread(true);
    return 0;
}

int netmesh::initBroadcastSocket(std::string addr)
{
    auto log = logobj->function("initBroadcastSocket");
    connected = false;
    if ((bcsock = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
        return errno;

    const int trueFlag = 1;
    if (setsockopt(bcsock, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof(trueFlag)) < 0)
        return errno;
    if (setsockopt(bcsock, SOL_SOCKET, SO_BROADCAST, &trueFlag, sizeof(trueFlag)) < 0)
        return errno;

    memset(&bcaddr, '\0', sizeof(bcaddr));

    bcaddr.sin_family = AF_INET;
    bcaddr.sin_addr.s_addr = inet_addr(addr.c_str());
    bcaddr.sin_port = htons(BCPORT);

    if (bind(bcsock, (const struct sockaddr *)&bcaddr, sizeof(bcaddr)) < 0)
        return errno;

    connected = true;
    return 0;
}

int netmesh::initListenSocket(std::string myaddr /*= "0.0.0.0"*/)
{
    auto log = logobj->function("initListenSocket");
    if ((listensock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        return errno;

    fcntl(listensock, F_SETFL, fcntl(listensock, F_GETFL) | O_NONBLOCK);

    const int trueFlag = 1;
    if (setsockopt(listensock, SOL_SOCKET, SO_REUSEADDR, &trueFlag, sizeof(trueFlag)) < 0)
        return errno;

    sockaddr_in listenaddr;
    memset(&listenaddr, '\0', sizeof(listenaddr));

    listenaddr.sin_family = AF_INET;
    listenaddr.sin_addr.s_addr = inet_addr(myaddr.c_str());
    listenaddr.sin_port = htons(TCPPORT);

    if (bind(listensock, (const struct sockaddr *)&listenaddr, sizeof(listenaddr)) < 0)
        return errno;
    return 0;
}

int netmesh::sendraw(std::string to, netdata *data)
{
    auto log = logobj->function("sendraw");
    std::cout << "Log: (sendraw)" << std::endl;
    std::cout << "Value: to: " << to << std::endl;
    if (devices.find(to) != devices.end())
    {
        packet temppack;
        temppack.addr = devices.at(to).address;
        temppack.raw = data->data();
        temppack.length = data->size();

        sockGeneral->send(temppack);
    }
    else
    {
        std::cout << "Warning: Tried to send data to invalid name" << std::endl;
    }
    return 0;
}

int netmesh::recvraw(std::string from, netdata *data)
{
    auto log = logobj->function("recvraw");
    std::cout << "Log: (recvraw)" << std::endl;
    std::cout << "Value: from: " << from << std::endl;
    if (devices.find(from) != devices.end())
    {
        poll(sockGeneral->topoll(POLLIN),1,-1);
        packet temppack = sockGeneral->recv();
        data->resize(0);
        data->insert(data->begin(),temppack.raw[0],temppack.raw[temppack.length]);
    }
    else
    {
        std::cout << "Warning: Tried to recv data from invalid name" << std::endl;
    }
    return 0;
}

std::string netmesh::returnDevices()
{
    auto log = logobj->function("returnDevices");
    std::stringstream ss;

    ss << "Connected Devices: " << std::endl;
    for (auto &x : devices)
        ss << "Name: " << x.first << std::endl
           << "  Address: " << x.second.address << std::endl
           << "  Timeout: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - x.second.timeout).count() << std::endl;

    return ss.str();
}

int netmesh::broadcastAlive()
{
    auto log = logobj->function("broadcastAlive");
    std::string message = "++<" + myName;
    int count = sendto(bcsock, message.c_str(), message.length() + 1, MSG_CONFIRM, (const struct sockaddr *)&bcaddr, sizeof(bcaddr));
    if (count == -1)
        std::cout << "oof" << std::endl;
    return 0;
}

int netmesh::updateDeviceList()
{
    auto log = logobj->function("updateDeviceList");
    char *recvbuff = (char *)malloc(BUFFLEN),
         *namebuff = (char *)malloc(BUFFLEN),
         *addrbuff = (char *)malloc(BUFFLEN);
    while (1)
    {
        sockaddr_in recvaddr;
        socklen_t recvaddrlen = sizeof(recvaddr);
        int count = recvfrom(bcsock, recvbuff, BUFFLEN, MSG_DONTWAIT, (sockaddr *)&recvaddr, &recvaddrlen);
        if (count == -1)
        {
            break;
        }

        recvbuff[count] = '\0';
        sscanf(recvbuff, "++<%s", namebuff);
        std::string namestring = namebuff;
        if (namestring == myName)
            return 0;
        auto settimeout = std::chrono::system_clock::now();

        if (devices.find(namestring) != devices.end())
        {
            devices.at(namestring).address = recvaddr.sin_addr.s_addr;
            devices.at(namestring).timeout = settimeout;
        }
        else
        {
            device tempdev;
            tempdev.address = recvaddr.sin_addr.s_addr;
            tempdev.timeout = settimeout;

            devices.insert(std::make_pair(namestring, tempdev));

            std::cout << "Device Added!" << std::endl
                      << returnDevices();
        }
    }

    for (auto it = devices.cbegin(); it != devices.cend();)
    {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - it->second.timeout) >= std::chrono::milliseconds(TIMEOUTTIME))
        {
            std::cout << "Log: Removing device from mesh" << std::endl;
            std::cout << "Value: Device: " << it->first << std::endl;
            devices.erase(it++);
            std::cout << "Log: Successfully removed device from mesh" << std::endl;
        }
        else
        {
            ++it;
        }
    }

    return 0;
}

int netmesh::checkforconn()
{
    auto log = logobj->function("checkforconn");
    if (listen(listensock, 10) > -1)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
            return 0;
        std::cout << "something fucked up " << errno << std::endl;
        std::string name;
        sockaddr_in newaddr;
        socklen_t newaddrlen = sizeof(newaddr);
        int newfd = accept(listensock, (sockaddr *)&newaddr, &newaddrlen);
        for (auto &x : devices)
            name = (newaddr.sin_addr.s_addr == x.second.address) ? x.first : name;
        connections.push_back({name, newfd});
    }
    return 0;
}

void netmesh::updateThread(netmesh *mynetmesh)
{
    while (mynetmesh->isConnected())
    {
        for (int i = 0; i < POLLCOUNT; ++i)
        {
            mynetmesh->updateDeviceList();
            std::this_thread::sleep_for(std::chrono::milliseconds(POLLDELAY));
        }

        if (mynetmesh->getBroadcastAlive())
            mynetmesh->broadcastAlive();
    }
}

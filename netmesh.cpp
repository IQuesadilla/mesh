#include "netmesh.h"

void updateThread(netmesh *mynetmesh)
{
    auto log = mynetmesh->logobj->function("updateThread");
    while (mynetmesh->isConnected())
    {
        auto polltimeout = std::chrono::system_clock::now().time_since_epoch();
        auto temptimeout = std::chrono::milliseconds(UPDATETIME);
        while (mynetmesh->updateDeviceList(temptimeout))
        {
            if ( !mynetmesh->isConnected() )
                return;
                
            temptimeout = std::chrono::milliseconds(UPDATETIME) - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch() - polltimeout);
        }

        if (mynetmesh->getBroadcastAlive())
            mynetmesh->broadcastAlive();

        
    }
}



netmesh::netmesh(std::shared_ptr<logcpp> logptr)
{
    enableLogging(logptr);

    auto log = logobj->function("netmesh");
    setBroadcastAlive(true);
    setEnableUpdateThread(true);
}

netmesh::~netmesh()
{
    auto log = logobj->function("~netmesh");
    if (connected)
    {
        killserver();
    }
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
    if (myUpdateThread.joinable())
    {
        log << "Waiting until update thread finshes" << logcpp::loglevel::NOTE;
        myUpdateThread.join();
    }

    return 0;
}

void netmesh::enableLogging(std::shared_ptr<logcpp> log)
{
    logobj = log;
    auto templog = logobj->function("enableLogging");
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
        //log << "Available mesh: " << tempbcaddr << logcpp::loglevel::NOTE;
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

    bcaddr.sin_addr.s_addr = inet_addr(addr.c_str());

    bcsock.reset(new udp());

    log << "Running initSocket" << logcpp::loglevel::NOTE;
    bcsock->initSocket(BCPORT);

    const int trueFlag = 1;
    if (setsockopt(bcsock->topoll(0)->fd, SOL_SOCKET, SO_BROADCAST, &trueFlag, sizeof(trueFlag)) < 0)
        return errno;

    bcsock->bindaddr();

    connected = true;
    return 0;
}

/*
int netmesh::initListenSocket(std::string myaddr /*= "0.0.0.0"*)
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
*/

bool netmesh::isConnected()
{
    auto log = logobj->function("isConnected");
    return connected;
}

int netmesh::sendraw(std::string to, netdata *data)
{
    auto log = logobj->function("sendraw");
    log << "to: " << to << logcpp::loglevel::VALUE;
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
        log << "Tried to send data to invalid name" << logcpp::loglevel::WARNING;
    }
    return 0;
}

int netmesh::recvraw(std::string from, netdata *data)
{
    auto log = logobj->function("recvraw");
    log << "from: " << from << logcpp::loglevel::VALUE;
    if (devices.find(from) != devices.end())
    {
        poll(sockGeneral->topoll(POLLIN),1,-1);
        packet temppack = sockGeneral->recv();
        data->clear();
        data->insert(data->begin(),&temppack.raw[0],&temppack.raw[temppack.length]);
    }
    else
    {
        log << "Tried to recv data from invalid name" << logcpp::loglevel::WARNING;
    }
    return 0;
}

std::string netmesh::returnDevices()
{
    auto log = logobj->function("returnDevices");
    std::stringstream ss;

    ss << "Connected Devices: " ;
    for (auto &x : devices)
        ss << "Name: " << x.first << std::endl
           << "  Address: " << x.second.address << std::endl
           << "  Timeout: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - x.second.timeout).count() ;

    return ss.str();
}

int netmesh::updateDeviceList(std::chrono::milliseconds timeout)
{
    auto log = logobj->function("updateDeviceList");

    log << "The VALUE " << int(timeout.count()) << logcpp::loglevel::VALUE;
    auto fds = bcsock->topoll(POLLIN);
    int nfds = poll(fds,1,int(timeout.count()));
    log << "fdval: " << fds->revents << logcpp::loglevel::VALUE;
    if (nfds < 1)
        return 0;

    auto resp = bcsock->recv();
    int count = resp.length;
    auto dev = parseUpdate(resp.raw);

    log << "NAME " << dev.first << logcpp::loglevel::VALUE;
    if (dev.first == myName)
        return 1;
    auto settimeout = std::chrono::system_clock::now();

    bool newdev = false;
    if (devices.find(dev.first) != devices.end())
    {
        newdev = true;
    }
    
    devices[dev.first] = dev.second;

    if (newdev)
        log << "Device Added!" << "\n"
                    << returnDevices() << logcpp::loglevel::NOTE;

    for (auto it = devices.cbegin(); it != devices.cend();)
    {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - it->second.timeout) >= std::chrono::milliseconds(TIMEOUTTIME))
        {
            log << "Removing device from mesh" << logcpp::loglevel::NOTE;
            log << "Device: " << it->first << logcpp::loglevel::VALUE;
            devices.erase(it++);
            log << "Successfully removed device from mesh" << logcpp::loglevel::NOTE;
        }
        else
        {
            ++it;
        }
    }

    return 1;
}

uint16_t netmesh::registerUDP(std::string servname, std::function<void(char*,int)> fn)
{
    auto log = logobj->function("registerUDP");
    uint16_t port = findAvailablePort();

    std::shared_ptr<udp> conn;
    conn.reset(new udp(port));
    conn->bindaddr();

    myservice newconn;
    newconn.name = servname;
    newconn.connptr = conn;
    newconn.callback = fn;
    myservices.insert(std::make_pair(port,newconn));
    return port;
}

int netmesh::broadcastAlive()
{
    auto log = logobj->function("broadcastAlive");
    tinyxml2::XMLPrinter *print = generateUpdate();

    packet data;
    data.raw = print->CStr();
    data.length = print->CStrSize()+1;
    data.addr = bcaddr.sin_addr.s_addr;

    if (bcsock->send(data))
        log << "oof" << logcpp::loglevel::NOTE;
    return 0;
}

std::pair<std::string,netmesh::device> netmesh::parseUpdate(const char *xml)
{
    auto log = logobj->function("parseUpdate");

    device toreturn;
    tinyxml2::XMLDocument doc;
    doc.Parse(xml);

    tinyxml2::XMLElement *dev = doc.RootElement();
    std::string devname = dev->FindAttribute("name")->Value();
    //toreturn.timeout = std::chrono::milliseconds( dev->FindAttribute("timeout")->IntValue() );

    auto serv = dev->FirstChildElement("serv");
    if ( serv )
        do
        {
            std::pair<std::string,service> tempserv;
            tempserv.first = serv->FindAttribute("name")->Value();
            tempserv.second.port = serv->FindAttribute("port")->IntValue();

            auto tempiptype = std::string(serv->FindAttribute("ip")->Value());
            if (tempiptype == "udp")
                tempserv.second.ipt = iptype::UDP;
            else if (tempiptype == "tcp")
                tempserv.second.ipt = iptype::TCP;
            else
                log << "Invalid IP Type: " << tempserv.second.ipt << logcpp::loglevel::ERROR;

            toreturn.services.insert(tempserv);
        }
        while ( serv = serv->NextSiblingElement("serv") );
    
    log << "Device Name: " << devname << logcpp::loglevel::VALUE;
    //log << "Device Timeout: " << toreturn.timeout << std::endl;

    for (auto &x : toreturn.services)
    {
        log << "Service Name: " << x.first << logcpp::loglevel::VALUE;
        log << "Service Port: " << x.second.port << logcpp::loglevel::VALUE;
        log << "Service IP: " << x.second.ipt << logcpp::loglevel::VALUE;
    }

    return std::make_pair(devname,toreturn);
}

tinyxml2::XMLPrinter *netmesh::generateUpdate()
{
    auto log = logobj->function("generateUpdate");

    tinyxml2::XMLDocument doc;
    auto dev = doc.NewElement("dev");
    dev->SetAttribute("name",myName.c_str());
    dev->SetAttribute("timeout",TIMEOUTTIME);
    doc.InsertFirstChild(dev);

    for (auto &x : myservices)
    {
        auto serv = dev->InsertNewChildElement("serv");
        serv->SetAttribute("name",x.second.name.c_str());
        serv->SetAttribute("port",x.first);
        if (x.second.ipt == iptype::UDP)
            serv->SetAttribute("ip","udp");
        else if (x.second.ipt == iptype::TCP)
            serv->SetAttribute("ip","tcp");
        else
            log << "Invalid IP Type: " << x.second.ipt << logcpp::loglevel::ERROR;
    }

    tinyxml2::XMLPrinter *print = new tinyxml2::XMLPrinter(0,true);
    doc.Print( print );
    return print;
}

/*
int netmesh::checkforconn()
{
    auto log = logobj->function("checkforconn");
    if (listen(listensock, 10) > -1)
    {
        if (errno == EWOULDBLOCK || errno == EAGAIN)
            return 0;
        log << "something fucked up " << errno << logcpp::loglevel::VALUE;
        std::string name;
        sockaddr_in newaddr;
        socklen_t newaddrlen = sizeof(newaddr);
        int newfd = accept(listensock, (sockaddr *)&newaddr, &newaddrlen);
        for (auto &x : devices)
            name = (newaddr.sin_addr.s_addr == x.second.address) ? x.first : name;
        //myservices.push_back({name, newfd});
    }
    return 0;
}
*/

uint16_t netmesh::findAvailablePort()
{
    uint16_t toreturn;
    return toreturn;
}

void netmesh::pollAll(std::chrono::milliseconds timeout)
{
    int size = myservices.size();
    pollfd fds[size];
    int i = 0;
    for (auto &x : myservices)
    {
        fds[i++] = *x.second.connptr->topoll(POLLIN);
    }

    int count = poll(fds,size,timeout.count());

    auto it = myservices.begin();
    for (int j = 0; j < size; j++)
    {
        auto revent = fds[j].revents;
        if (revent != 0)
        {
            //it->second.callback(revent); // Add recv support and pass data as param
        }

        it++;
    }
}
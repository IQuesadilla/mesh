#include "netmesh.h"

void netmesh::updateThread()
{
    auto log = logobj->function("updateThread");
    while (isConnected())
    {
        auto polltimeout = std::chrono::system_clock::now().time_since_epoch();
        auto temptimeout = std::chrono::milliseconds(UPDATETIME);
        while (pollAll(temptimeout))
        {
            if ( !isConnected() )
                return;
                
            temptimeout = std::chrono::milliseconds(UPDATETIME) - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch() - polltimeout);
        }

        if (getBroadcastAlive())
            broadcastAlive();
    }
}



netmesh::netmesh(std::shared_ptr<logcpp> logptr)
{
    enableLogging(logptr);

    auto log = logobj->function("netmesh");
    setBroadcastAlive(true);
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
    sockGeneral->initSocket(0);
    //sockGeneral->bindaddr();

    myName = name;
    connected = true;

    return 0;
}

int netmesh::killserver()
{
    auto log = logobj->function("killserver");
    connected = false;

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

int netmesh::serviceSend(std::string devname, std::string servname, netdata *data)
{
    auto log = logobj->function("serviceSend");

    if (devices.find(devname) == devices.end())
    {
        log << "Tried to send data to an invalid device name" << logcpp::loglevel::WARNING;
        return -1;
    }

    packet tosend;
    tosend.addr = devices.at(devname).address;
    tosend.raw = data->data();
    tosend.length = data->size();

    for (auto &x : myservices)
        if (x.second.name == servname)
            x.second.connptr->send(tosend);

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

int netmesh::updateDeviceList()
{
    auto log = logobj->function("updateDeviceList");

    auto resp = bcsock->recv();
    int count = resp.length;
    auto dev = parseUpdate(resp.raw);

    log << "NAME " << dev.first << logcpp::loglevel::VALUE;
    if (dev.first == myName)
        return 1;
    auto settimeout = std::chrono::system_clock::now();

    bool newdev = false;
    if (devices.find(dev.first) == devices.end())
    {
        newdev = true;
    }
    
    dev.second.address = resp.addr;
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

uint16_t netmesh::registerUDP(std::string servname, std::function<void(std::string,netdata*)> fn)
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
    newconn.ipt = iptype::UDP;
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
    toreturn.timeout = std::chrono::system_clock::now();

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError errorid = doc.Parse(xml,strlen(xml));
    if ( errorid != tinyxml2::XMLError::XML_SUCCESS )
    {
        log << "Failed to parse input " << errorid << logcpp::loglevel::ERROR;
        return std::make_pair("",device());
    }

    tinyxml2::XMLElement *dev = doc.RootElement();
    std::string devname = dev->FindAttribute("name")->Value();
    log << "Devname: " << devname << logcpp::loglevel::NOTE;
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
    auto log = logobj->function("findAvailablePort");

    uint16_t toreturn;
    toreturn = 2000;
    return toreturn;
}

bool netmesh::pollAll(std::chrono::milliseconds timeout)
{
    auto log = logobj->function("pollAll");

    int size = myservices.size() + 1;
    pollfd fds[size];

    fds[0] = *bcsock->topoll(POLLIN);

    int i = 1;
    for (auto &x : myservices)
    {
        fds[i++] = *x.second.connptr->topoll(POLLIN);
    }

    log << "Num of Services: " << size << logcpp::loglevel::VALUE;

    int count = poll(fds,size,timeout.count());
    log << "Polled Count: " << count << logcpp::loglevel::VALUE;

    if (count < 1)
    {
        return false;
    }

    if (fds[0].revents == POLLIN)
    {
        updateDeviceList();
        --count;
    }

    auto it = myservices.begin();
    for (int j = 1; j < size && count > 0; j++)
    {
        log << "DATA WAS RECIEVED FROM SOMETHING OTHER THAN UPDATEDEVICELIST!!! =====" << logcpp::loglevel::NOTE;

        auto revent = fds[j].revents;
        if (revent != 0)
        {
            log << "Found a service with an event" << logcpp::loglevel::VALUE;
            auto data = it->second.connptr->recv();

            std::string devname;
            for (auto &x : devices)
                if (x.second.address == data.addr)
                    devname = x.first;
            
            std::vector<char> ipdata ( data.raw, &data.raw[data.length] );
            it->second.callback(devname,&ipdata);
            --count;
        }

        it++;
    }

    return true;
}

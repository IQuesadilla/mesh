#include "netmesh.h"
#include <unistd.h>
#include <sys/ioctl.h>

netmesh::netmesh(std::shared_ptr<logcpp> logptr)
{
    setLogger(logptr);

    auto log = logobj->function("netmesh");
    setBroadcastAlive(true);
    setUserPtr(nullptr);

    connected = false;
}

netmesh::~netmesh()
{
    auto log = logobj->function("~netmesh");
    if (connected)
    {
        killserver();
    }
}

int netmesh::initserver(std::string name,
                        ifaddrs mesh,
                        int bcport /*= 1999*/,
                        int tcpport /*= 1998*/,
                        int updatetime /*= 2000*/,
                        int timeouttime /*= 10000*/)
{
    auto log = logobj->function("initserver");
    if (connected == true)
        return 0;

    flags.bcport = bcport;
    flags.tcpport = tcpport;
    flags.updatetime = updatetime;
    flags.timeouttime = timeouttime;

    setName(name);
    initBroadcastSocket( ((sockaddr_in*)mesh.ifa_ifu.ifu_broadaddr)->sin_addr.s_addr );

    log << "Mesh IP: " << inet_ntoa(((sockaddr_in*)mesh.ifa_addr)->sin_addr) << logcpp::loglevel::VALUE;
    sockGeneral.reset(new udp( logobj, ((sockaddr_in*)mesh.ifa_addr)->sin_addr.s_addr) );
    //sockGeneral->bindaddr();

    myaddr = ((sockaddr_in*)mesh.ifa_addr)->sin_addr.s_addr;

    freeifaddrs(ifAddrStruct);

    int pipes[2];
    if (pipe(pipes) < 0)
        return -1;

    rfifo = pipes[0];
    wfifo = pipes[1];

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

std::vector<ifaddrs> netmesh::findAvailableMeshes()
{
    auto log = logobj->function("findAvailableMeshes");
    std::vector<ifaddrs> to_return;

    ifAddrStruct=NULL;
    struct ifaddrs * ifa=NULL;

    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_ifu.ifu_broadaddr == NULL)
            continue;

        if (ifa->ifa_addr->sa_family != AF_INET)
            continue;

        //log << "Available mesh: " << tempbcaddr << logcpp::loglevel::NOTE;
        to_return.push_back(*ifa);
    }

    return to_return;
}

int netmesh::initBroadcastSocket(in_addr_t addr)
{
    auto log = logobj->function("initBroadcastSocket");

    bcaddr = addr;

    log << "Creating bc socket" << logcpp::loglevel::NOTE;
    bcsock.reset(new udp(logobj,0,-1));

    const int trueFlag = 1;
    if (setsockopt(bcsock->topoll(0).fd, SOL_SOCKET, SO_BROADCAST, &trueFlag, sizeof(trueFlag)) < 0)
        return errno;

    log << "Binding to port: " << flags.bcport << logcpp::loglevel::VALUE;
    bcsock->bindaddr(addr,flags.bcport);

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
           << "  Timeout: " << 
            std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()
                - x.second.timeout).count() ;

    return ss.str();
}

uint16_t netmesh::registerUDP(std::string servname, std::function<void(std::string,netdata*,void*)> fn)
{
    auto log = logobj->function("registerUDP");

    std::shared_ptr<udp> conn;
    conn.reset(new udp(logobj,myaddr));

    myservice newconn;
    newconn.name = servname;
    newconn.connptr = conn;
    newconn.callback = fn;
    newconn.ipt = iptype::UDP;
    myservices.insert(std::make_pair(conn->getport(),newconn));
    return conn->getport();
}

std::shared_ptr<logcpp> netmesh::getLogger()
{
    auto templog = logobj->function("getLogger");
    return logobj;
}

void netmesh::setLogger(std::shared_ptr<logcpp> log)
{
    logobj = log;
    auto templog = logobj->function("setLogger");
}

bool netmesh::getBroadcastAlive()
{
    return flags.broadcastalive;
}

bool netmesh::setBroadcastAlive(bool in)
{
    return (flags.broadcastalive = in);
}

std::string netmesh::getName()
{
    return myName;
}

std::string netmesh::setName(std::string in)
{
    return (myName = in);
}

void *netmesh::getUserPtr()
{
    return userptr;
}

void netmesh::setUserPtr(void *ptr)
{
    userptr = ptr;
}

void netmesh::setIntMsgHandler(std::function<void(std::string,void*)> fn)
{
    intMsgHandler = fn;
}
std::function<void(std::string,void*)> netmesh::getIntMsgHandler()
{
    return intMsgHandler;
}

void netmesh::interruptMsg(std::string message)
{
    write(wfifo,message.c_str(),message.length()+1);
}

void netmesh::runOnce()
{
    auto log = logobj->function("runOnce");
    updateThread();
};

void netmesh::runForever()
{
    auto log = logobj->function("runForever");
    while ( isConnected() ) updateThread();
}

int netmesh::broadcastAlive()
{
    auto log = logobj->function("broadcastAlive");
    tinyxml2::XMLPrinter *print = generateUpdate();

    packet data;
    data.raw = print->CStr();
    data.length = print->CStrSize()+1;
    data.addr = bcaddr;

    if (bcsock->send(data))
        log << "oof" << logcpp::loglevel::NOTE;
    return 0;
}

int netmesh::parseUpdate(packet input)
{
    auto log = logobj->function("parseUpdate");

    device newdev;
    newdev.address = input.addr;
    newdev.timeout = std::chrono::system_clock::now();

    tinyxml2::XMLDocument doc;
    tinyxml2::XMLError errorid = doc.Parse(input.raw,input.length);
    if ( errorid != tinyxml2::XMLError::XML_SUCCESS )
    {
        log << "Failed to parse input " << errorid << logcpp::loglevel::ERROR;
        return -1;
    }

    tinyxml2::XMLElement *root = doc.RootElement();
    if ( !root )
    {
        log << "Invalid root element" << root->Name() << logcpp::loglevel::ERROR;
        return -1;
    }

    do
    {
        if (std::string(root->Name()) == "dev")
        {
            std::string devname = root->FindAttribute("name")->Value();
            log << "Devname: " << devname << logcpp::loglevel::NOTE;
            //toreturn.timeout = std::chrono::milliseconds( dev->FindAttribute("timeout")->IntValue() );

            auto serv = root->FirstChildElement("serv");
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

                    newdev.services.insert(tempserv);
                }
                while ( serv = serv->NextSiblingElement("serv") );
            
            log << "Device Name: " << devname << logcpp::loglevel::VALUE;
            //log << "Device Timeout: " << newdev.timeout << std::endl;

            for (auto &x : newdev.services)
            {
                log << "Service Name: " << x.first << logcpp::loglevel::VALUE;
                log << "Service Port: " << x.second.port << logcpp::loglevel::VALUE;
                log << "Service IP Type: " << x.second.ipt << logcpp::loglevel::VALUE;
            }

            updateDeviceList( devname, newdev );
        }
        else
        {
            log << "Invalid root string: " << root->Name() << logcpp::loglevel::ERROR;
            return -1;
        }
    }
    while ( root = root->NextSiblingElement() );

    return -1;
}

tinyxml2::XMLPrinter *netmesh::generateUpdate()
{
    auto log = logobj->function("generateUpdate");

    tinyxml2::XMLDocument doc;
    auto dev = doc.NewElement("dev");
    dev->SetAttribute("name",myName.c_str());
    dev->SetAttribute("timeout",flags.timeouttime);
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

int netmesh::updateDeviceList(std::string devname, netmesh::device devobj)
{
    auto log = logobj->function("updateDeviceList");

        log << "NAME " << devname << logcpp::loglevel::VALUE;
    if (devname == myName)
        return 1;
    auto settimeout = std::chrono::system_clock::now();

    bool newdev = false;
    if (devices.find(devname) == devices.end())
    {
        newdev = true;
    }
    
    devices[devname] = devobj;

    if (newdev)
        log << "Device Added!" << "\n"
                    << returnDevices() << logcpp::loglevel::NOTE;

    for (auto it = devices.cbegin(); it != devices.cend();)
    {
        if (std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()
                - it->second.timeout) >= std::chrono::milliseconds(flags.timeouttime))
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
/*
bool netmesh::findAvailablePort(std::shared_ptr<ip> netobj)
{
    auto log = logobj->function("findAvailablePort");

    /*
    int port = flags.bcport;
    do {
        log << "Attempting port " << port << logcpp::loglevel::VALUE;
        netobj->port(port++);
    } while(netobj->bindaddr());
    *-/
   netobj->port(0);
   netobj->bindaddr();
    return true;
}
*/
bool netmesh::pollAll(std::chrono::milliseconds timeout)
{
    auto log = logobj->function("pollAll");

    int internalFunctionCount = 2;
    int size = myservices.size() + internalFunctionCount;
    pollfd fds[size];

    fds[0] = bcsock->topoll(POLLIN);
    fds[1].fd = rfifo;
    fds[1].events = POLLIN;

    int i = internalFunctionCount;
    for (auto &x : myservices)
    {
        fds[i++] = x.second.connptr->topoll(POLLIN);
    }

    log << "Num of Services: " << size << logcpp::loglevel::VALUE;

    int count = -1;
    do {
        errno = 0;
        count = poll(fds,size,timeout.count());
    } while (errno == 4);
    log << "Polled Count: " << count << logcpp::loglevel::VALUE;

    if (count < 1)
    {
        log << "Errno: " << errno << logcpp::loglevel::ERROR;
        return false;
    }

    if (fds[0].revents == POLLIN)
    {
        auto resp = bcsock->recv();

        parseUpdate(resp);
        --count;
    }
    else if (fds[1].revents == POLLIN)
    {
        // Read data from fifo and run callback with data
        int fifosize;
        ioctl(rfifo, FIONREAD, &fifosize);
        char buff[fifosize];
        read(rfifo,buff,fifosize);

        getIntMsgHandler()(std::string(buff,fifosize),getUserPtr());
        --count;
    }

    auto it = myservices.begin();
    for (int j = internalFunctionCount; j < size && count > 0; j++)
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
            it->second.callback(devname,&ipdata,getUserPtr());
            --count;
        }

        it++;
    }

    return true;
}

void netmesh::updateThread()
{
    auto log = logobj->function("updateThread");

    auto polltimeout = std::chrono::system_clock::now().time_since_epoch();
    auto temptimeout = std::chrono::milliseconds(flags.updatetime);
    while (pollAll(temptimeout))
    {
        if ( !isConnected() )
            return;
            
        temptimeout = std::chrono::milliseconds(flags.updatetime)
            - std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()
            - polltimeout);
    }

    if (getBroadcastAlive())
        broadcastAlive();
}
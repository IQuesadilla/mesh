#include "netmesh.h"

netmesh::netmesh()
{
    setBroadcastAlive(true);
    setEnableUpdateThread(true);
}

int netmesh::initserver(std::string name)
{
    if (connected == true)
        return 0;

    connected = false;
    setName(name);
    initBroadcastSocket();
    //initListenSocket();
    initUDPSocket();

    myName = name;
    connected = true;
    if (getEnableUpdateThread()) initUpdateThread();
    return 0;
}

int netmesh::killserver()
{
    connected = false;
    if (getEnableUpdateThread()) myUpdateThread.join();
    return 0;
}

int netmesh::initUpdateThread()
{
    myUpdateThread = std::thread(updateThread,this);
    setEnableUpdateThread(true);
    return 0;
}

int netmesh::initBroadcastSocket()
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

int netmesh::initListenSocket()
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

int netmesh::initUDPSocket()
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
std::string netmesh::returnErrorMessage(ERRCODES err)
{
    switch (err)
    {
        case NOERROR:       return ""; break;//"No Error Detected"; break;
        case SOCKET:        return "Error Creating Socket"; break;
        case SETSOCKOPT:    return "Error Setting the Socket Option"; break;
        case BIND:          return "Error Binding the Socket"; break;
        default:            return "Not a netmesh Defined Error Code"; break;
    }
}
*//*
int netmesh::sendTCP(tinyxml2::XMLNode *value, std::string name)
{
    int sendsock = -1;
    for (auto &x : connections)
        sendsock = (x.devname == name) ? x.connsock : sendsock;
    if (sendsock == -1)
        return 1;

    std::vector<uint8_t> prefix {'-','-','<'};
    value.data->insert(value.data->begin(),prefix.begin(),prefix.end());
    int count = send (sendsock, value.data->data(), value.data->size(),0);
    if (count == -1)
        return 1;
    return 0;
}

netmesh::message netmesh::recieveTCP(std::string name)
{
    int recvsock = -1;
    for (auto &x : connections)
        recvsock = (x.devname == name) ? x.connsock : recvsock;
    if (recvsock == -1)
        return {NULL,NULL};//{std::string(),std::vector<uint8_t>()};

    char *recvdata = (char*)malloc(BUFFLEN);
    std::shared_ptr<std::vector<uint8_t> > recvbuff;
    int count = recv (recvsock, recvdata, BUFFLEN, MSG_DONTWAIT);
    if (count == -1)
        return {NULL,NULL}; //{std::string(),std::vector<uint8_t>()};
    recvbuff->insert(recvbuff->end(),&recvdata[0],&recvdata[count]);

    std::vector<int8_t> prefix {'-','-','<'};
    if (!std::equal(prefix.begin(),prefix.end(),recvbuff->begin()))
        return {NULL,NULL};// {*new std::string,*new std::vector<uint8_t>};

    return {name,recvbuff};
}
*/
int netmesh::sendUDP(tinyxml2::XMLNode *value)
{
    // Verify that the input node name is "msg" according to spec
    if (value->Value() != "msg")
        return -1;

    // Clone the node into a new document
    tinyxml2::XMLDocument doc;
    tinyxml2::XMLNode *recvnode = value->DeepClone(&doc);
    doc.InsertFirstChild(recvnode);

    // Get the name of the device to send to
    char * namebuff;
    doc.FirstChildElement("msg")->QueryStringAttribute("name",&namebuff);
    std::string name = namebuff;

    tinyxml2::XMLElement *dataElement = nullptr;
    dataElement = doc.FirstChildElement("msg")->FirstChildElement("data");

    tinyxml2::XMLPrinter dataElementPrinter;
    for (auto it = dataElement->FirstChild(); it != nullptr; it = it->NextSibling())
        it->Accept(&dataElementPrinter);

    if (dataElementPrinter.CStrSize() == 0)
        return -1;

    in_addr_t sendaddr = -1;
    for (auto &x : devices)
        sendaddr = (x.name == name) ? x.address : sendaddr;
    if (sendaddr == -1)
        return -1;

    sockaddr_in udpaddr;
    memset (&udpaddr,'\0',sizeof(udpaddr));
    udpaddr.sin_family = AF_INET;
    udpaddr.sin_addr.s_addr = sendaddr;
    udpaddr.sin_port = htons(UDPPORT);    in_addr_t sendaddr = -1;
    for (auto &x : devices)
        sendaddr = (x.name == name) ? x.address : sendaddr;
    if (sendaddr == -1)
        return -1;

    sockaddr_in udpaddr;
    memset (&udpaddr,'\0',sizeof(udpaddr));
    udpaddr.sin_family = AF_INET;
    udpaddr.sin_addr.s_addr = sendaddr;
    udpaddr.sin_port = htons(UDPPORT);
     
    std::cout << "Sending XML data, " << dataElementPrinter.CStrSize() << " bytes" << std::endl;
    int count = sendto (udpsock, dataElementPrinter.CStr(), dataElementPrinter.CStrSize(),0,(const struct sockaddr *)&udpaddr,sizeof(udpaddr));
    std::cout << "Sent " << count << " bytes of [" << dataElementPrinter.CStr() << "]" << std::endl;
    return count;
}

int netmesh::receiveUDP(tinyxml2::XMLDocument *target, tinyxml2::XMLNode **data)
{
    std::string recvname;
    std::shared_ptr<std::vector<uint8_t> > recvbuff;
    recvbuff.reset(new std::vector<uint8_t>(BUFFLEN));
    sockaddr_in recvaddr;
    socklen_t recvaddrlen = sizeof(recvaddr);

    int totalcount = 0;
    int tempcount = recvfrom (udpsock, recvbuff->data(), BUFFLEN, 0, (struct sockaddr *) &recvaddr, &recvaddrlen);
    while (tempcount == -1 || tempcount == 0)
    {
        int tempsize = recvbuff->size();
        recvbuff->resize(recvbuff->size() + BUFFLEN);
        tempcount = recv (udpsock, recvbuff->data() + tempsize, BUFFLEN, MSG_DONTWAIT);
        totalcount += tempcount;
    }

    std::cout << "Bytes received: " << tempcount << " of [" << recvbuff->data() << "] "  << strlen((const char*)recvbuff->data()) << std::endl;

    *data = target->NewElement("msg");
    tinyxml2::XMLNode *datanode = target->NewElement("data");
    tinyxml2::XMLNode *timenode = target->NewElement("time");
    (*data)->InsertFirstChild(datanode);
    (*data)->InsertAfterChild(datanode, timenode);

    tinyxml2::XMLDocument doc;
    auto parseerror = doc.Parse((const char*)recvbuff->data(),tempcount);
    if (parseerror == tinyxml2::XML_SUCCESS)
    {
        std::cout << "Successfully parsed" << std::endl;
        *data = doc.FirstChild()->FirstChild()->DeepClone(target);
    }
    else if (parseerror == tinyxml2::XML_ERROR_PARSING)
    {
        tinyxml2::XMLNode *datanode = doc.NewElement("data");
    }
    //if (doc.Error() == true)
    //{
    //    doc.PrintError();
    //    return -1;
    //.}

/*
    //tinyxml2::XMLPrinter printer;
    //doc.Print(&printer);
    std::string printer = doc->FirstChildElement()->Value();

    std::cout << "Ready to print" << std::endl;*/
/*
    //std::vector<uint8_t> prefix {'-','-','<'};
    if (!std::equal(prefix.begin(),prefix.end(),recvbuff->begin()))
        return {std::string(),NULL};
    else
        recvbuff->erase(recvbuff->begin(),recvbuff->begin()+3);
    
    //std::cout << recvbuff->size() << " " << std::flush;
*//*
    std::cout << "Creating vector of data " << printer << std::endl;
    std::shared_ptr<std::vector<uint8_t> > outdata;
    outdata.reset(new std::vector<uint8_t>(printer.begin(),printer.end()));
    outdata->push_back('\0');
    std::cout << "Created vector of size " << outdata->size() << std::endl;
    //outdata->resize(printer.CStrSize()+1);
    //std::copy(printer.CStr(),printer.CStr()+printer.CStrSize(),outdata->data());


    for (auto &d : devices)
        recvname = (recvaddr.sin_addr.s_addr == d.address) ? d.name : "";

    std::cout << "Recieved from " << recvname << std::endl;
    return {recvname,outdata};*/
}
/*
tinyxml2::XMLNode *netmesh::receiveUDP(tinyxml2::XMLDocument *target, std::string from)
{
    auto buff = receiveUDP(target);
    if (buff.name == from)
        return buff;
    else nullptr;
}*/

std::string netmesh::returnDevices()
{
    std::stringstream ss;

    ss << "Connected Devices: " << std::endl;
    for (auto &x : devices)
        ss  << "Name: " << x.name << std::endl\
            << "  Address: " << x.address << std::endl\
            << "  Timeout: " << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - x.timeout).count() << std::endl;

    return ss.str();
}

int netmesh::broadcastAlive()
{
    std::string message = "++<"+myName;
    int count = sendto (bcsock, message.c_str(), message.length()+1, MSG_CONFIRM, (const struct sockaddr *) &bcaddr, sizeof(bcaddr));
    if (count == -1)
        std::cout << "oof" << std::endl;
    return 0;
}

int netmesh::updateDeviceList()
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

int netmesh::checkforconn()
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

void netmesh::updateThread(netmesh *mynetmesh)
{
    while (mynetmesh->isConnected())
    {
        for (int i = 0; i < POLLCOUNT; ++i)
        {
            mynetmesh->updateDeviceList();
            //mynetmesh->receiveUDP();
            //mynetmesh->checkforconn();
            std::this_thread::sleep_for(std::chrono::milliseconds(POLLDELAY));
        }

        if (mynetmesh->getBroadcastAlive()) mynetmesh->broadcastAlive();
        //std::cout << returnDevices();
    }
}

#include "../../src/netmesh/netmesh.h"
#include <signal.h>

void mycallback(std::string name, netmesh::netdata* data, void* userptr)
{
    std::shared_ptr<logcpp> logobj = *(std::shared_ptr<logcpp>*)userptr;
    auto log = logobj->function("mycallback");

    log << "Running the callback" << logcpp::loglevel::NOTE;
    log << "Name: " << name << logcpp::loglevel::VALUE;
    log << "Length: " << data->size() << logcpp::loglevel::VALUE;
    log << "Data: " << std::string(data->begin(),data->end()) << logcpp::loglevel::VALUE;
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Not enough arguments" << std::endl;
        return -1;
    }

    std::shared_ptr<logcpp> logobj;
    logobj.reset(new logcpp(logcpp::vlevel::DEFAULT));
    auto log = logobj->function("main");

    log << "Creating shared pointer for netmesh" << logcpp::loglevel::NOTE;
    std::shared_ptr<netmesh> mesh1;

    log << "Resetting with new netmesh object" << logcpp::loglevel::NOTE;
    mesh1.reset(new netmesh(logobj));

    mesh1->setUserPtr(&logobj);

    log << "Finding available meshes" << logcpp::loglevel::NOTE;
    auto avail = mesh1->findAvailableMeshes();
    int n = 0;
    for (auto x : avail)
    {
        std::cout << n++ << ": " << x.ifa_name << std::endl;
    }

    std::cin >> n;
    std::cout << "Log: Selected " << avail[n].ifa_name << " mesh" << std::endl;

    log << "Running init server with defaults" << logcpp::loglevel::NOTE;
    mesh1->initserver(std::string(argv[1]), avail[n]);

    mesh1->registerUDP("serv1",mycallback);

    std::thread mythread (&netmesh::runForever,mesh1.get());

    log << "Delaying for 10s to allow for another device" << logcpp::loglevel::NOTE;
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    const std::string datastring = mesh1->getName() + " : Ayo! It works DAWGS! lfg";
    std::vector<char> datavec (datastring.begin(),datastring.end());

    log << "Sending some data" << logcpp::loglevel::NOTE;
    mesh1->serviceSend(argv[2], "serv1", &datavec);

    log << "Delaying for 10s to give time for other to send" << logcpp::loglevel::NOTE;
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    mesh1->killserver();
    mythread.join();

    return 0;
}
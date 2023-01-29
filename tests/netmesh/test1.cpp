#include "../../netmesh.h"

void mycallback(std::string name,netmesh::netdata* data)
{
    std::cout << "Running the callback" << std::endl;
    std::cout << "Name: " << name << std::endl;
    std::cout << "Length: " << data->size() << std::endl;
    std::cout << "Data: " << std::string(data->begin(),data->end()) << std::endl;
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
    log << "Creating shared ptr" << logcpp::loglevel::NOTE;
    std::shared_ptr<netmesh> mesh1;

    log << "Resetting with new netmesh object" << logcpp::loglevel::NOTE;
    mesh1.reset(new netmesh(logobj));

    log << "Finding available meshes" << logcpp::loglevel::NOTE;
    auto avail = mesh1->findAvailableMeshes();
    int n = 0;
    for (auto x : avail)
    {
        std::cout << n++ << ": " << x << std::endl;
    }

    std::cin >> n;
    std::cout << "Log: Selected " << avail[n] << " mesh" << std::endl;

    log << "Running init server with defaults" << logcpp::loglevel::NOTE;
    mesh1->initserver(std::string(argv[1]), avail[n]);

    mesh1->registerUDP("serv1",mycallback);

    std::thread mythread (netmesh::run,mesh1.get());

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
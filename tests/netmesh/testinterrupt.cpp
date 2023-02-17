#include "../../src/netmesh/netmesh.h"
#include <signal.h>

std::unique_ptr<netmesh> mesh1;
std::string who;
int doloopcount;

void alarm_interrupt(int sig, siginfo_t *si, void *uc)
{
    auto log = mesh1->getLogger()->function("alarm_interrupt");
    mesh1->interruptMsg("It works! ijk");
}

void mycallback(std::string name, netmesh::netdata* data, void* userptr)
{
    std::shared_ptr<logcpp> logobj = *(std::shared_ptr<logcpp>*)userptr;
    auto log = logobj->function("mycallback");

    log << "Running the callback" << logcpp::loglevel::NOTE;
    log << "Name: " << name << logcpp::loglevel::VALUE;
    log << "Length: " << data->size() << logcpp::loglevel::VALUE;
    log << "Data: " << std::string(data->begin(),data->end()) << logcpp::loglevel::VALUE;
}

void myalarmcallback(std::string data, void* userptr)
{
    std::shared_ptr<logcpp> logobj = *(std::shared_ptr<logcpp>*)userptr;
    auto log = logobj->function("myalarmcallback");
    log << "Running my alarm callback" << logcpp::loglevel::NOTE;
    log << "DATA RECV: " << data << logcpp::loglevel::VALUE;
    
    const std::string datastring = mesh1->getName() + " : Ayo! It works DAWGS! lfg";
    std::vector<char> datavec (datastring.begin(),datastring.end());

    log << "Sending some data" << logcpp::loglevel::NOTE;
    mesh1->serviceSend(who, "serv1", &datavec);

    if (--doloopcount == 0)
    {
        mesh1->killserver();
    }
}

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Not enough arguments" << std::endl;
        return -1;
    }

    who = argv[2];
    doloopcount = 10;

    std::shared_ptr<logcpp> logobj;
    logobj.reset(new logcpp(logcpp::vlevel::DEBUG));
    auto log = logobj->function("main");

    log << "Resetting with new netmesh object" << logcpp::loglevel::NOTE;
    mesh1.reset(new netmesh(logobj));

    mesh1->setIntMsgHandler(myalarmcallback);
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
    std::cout << "Value: myip: " << inet_ntoa( ((sockaddr_in*)(avail[n].ifa_addr))->sin_addr ) << std::endl;

    log << "Running init server with defaults" << logcpp::loglevel::NOTE;
    mesh1->initserver(std::string(argv[1]), avail[n]);


    sigevent sev;
    timer_t ti;
    itimerspec its;
    struct sigaction sa;

    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = alarm_interrupt;
    sigemptyset(&sa.sa_mask);
    if (sigaction(SIGRTMIN, &sa, NULL) == -1)
        log << "Failed to set sigaction" << logcpp::loglevel::ERROR;

    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_signo = SIGRTMIN;
    sev.sigev_value.sival_ptr = &ti;
    if (timer_create(CLOCK_REALTIME, &sev, &ti) == -1)
        log << "Failed to create timer" << logcpp::loglevel::ERROR;

    long freq_nanosecs = 2000000000;
    its.it_value.tv_sec = freq_nanosecs / 1000000000;
    its.it_value.tv_nsec = freq_nanosecs % 1000000000;
    its.it_interval.tv_sec = its.it_value.tv_sec;
    its.it_interval.tv_nsec = its.it_value.tv_nsec;
    if (timer_settime(ti, 0, &its, NULL) == -1)
        log << "Failed to set timer " << errno << logcpp::loglevel::ERROR;

    mesh1->registerUDP("serv1",mycallback,nullptr);
    mesh1->runForever();

    timer_delete(ti);

    return 0;
}
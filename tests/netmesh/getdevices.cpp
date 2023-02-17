#include "../../src/netmesh/netmesh.h"

int main (int argc, char **argv)
{
    std::shared_ptr<logcpp> logobj;
    logobj.reset(new logcpp(logcpp::vlevel::DEBUG));

    auto log = logobj->function("main");

    std::shared_ptr<netmesh> temp;
    temp.reset(new netmesh(logobj));

    std::cout << "Log: Finding available meshes" << std::endl;
    auto avail = temp->findAvailableMeshes();
    int n = 0;
    for (auto x : avail)
    {
        std::cout << n++ << ": " << x.ifa_addr << std::endl;
    }

    std::cin >> n;

    std::cout << "Log: Selected " << avail[n].ifa_addr << std::endl;

    temp->setBroadcastAlive(false);
    temp->initserver("",avail[n]);

    char buffer;
    bool doLoop = true;
    while (doLoop)
    {
        std::cin >> buffer;

        switch (buffer)
        {
            case 'g': std::cout << temp->returnDevices(); break;
            case 'q': doLoop = false; break;
        }
    }

    temp->killserver();
}

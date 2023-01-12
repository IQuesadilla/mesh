#include "../../netmesh.h"

int main (int argc, char **argv)
{
    netmesh *temp = new netmesh();

    std::cout << "Log: Finding available meshes" << std::endl;
    auto avail = temp->findAvailableMeshes();
    int n = 0;
    for (auto x : avail)
    {
        std::cout << n++ << ": " << x << std::endl;
    }

        std::cin >> n;

    std::cout << "Log: Selected " << avail[n] << std::endl;

    temp->setBroadcastAlive(false);
    temp->initBroadcastSocket(avail[n]);
    temp->initUpdateThread();

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
    delete temp;
}

#include "netmesh.h"

int main (int argc, char **argv)
{
    mesh *temp = new mesh();
    temp->setBroadcastAlive(false);
    temp->initBroadcastSocket();
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

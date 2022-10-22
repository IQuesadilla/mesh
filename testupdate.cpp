#include "netmesh.h"

int main (int argc, char **argv)
{
    if (argc < 2)
        return 1;
    
    mesh *temp = new mesh();
    temp->initserver(argv[1]);

    while (1)
    {
        std::cout << temp->returnDevices();
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    temp->killserver();
    delete temp;
}
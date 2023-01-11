#include "../../netmesh.h"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cout << "Not enough arguments" << std::endl;
        return -1;
    }

    std::cout << "Log: (main)" << std::endl;
    std::cout << "Log: Creating sharedptr" << std::endl;
    std::shared_ptr<netmesh> mesh1;

    std::cout << "Log: Resetting with new netmesh object" << std::endl;
    mesh1.reset(new netmesh());

    std::cout << "Log: Running init server with defaults" << std::endl;
    mesh1->initserver(std::string(argv[1]));

    std::cout << "Log: Delaying for 10s to allow for another device" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    const std::string datastring = "Ayo! It works DAWGS! lfg";
    std::vector<char> datavec (datastring.begin(),datastring.end());

    std::cout << "Log: Sending some data" << std::endl;
    mesh1->sendraw(argv[2],&datavec);

    std::cout << "Log: Receiving some data" << std::endl;
    mesh1->recvraw(argv[2],&datavec);

    std::cout << "Log: Closing mesh" << std::endl;
    mesh1->killserver();

    std::cout << "Log: Finished" << std::endl;
}
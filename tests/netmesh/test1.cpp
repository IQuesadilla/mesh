#include "../../netmesh.h"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        std::cerr << "Not enough arguments" << std::endl;
        return -1;
    }

    std::shared_ptr<logcpp> logobj;
    logobj.reset(new logcpp());

    auto log = logobj->function("main");
    log << "Creating shared ptr";
    std::shared_ptr<netmesh> mesh1;

    log << "Resetting with new netmesh object";
    mesh1.reset(new netmesh(logobj));

    log << "Finding available meshes";
    auto avail = mesh1->findAvailableMeshes();
    int n = 0;
    for (auto x : avail)
    {
        std::cout << n++ << ": " << x << std::endl;
    }

    std::cin >> n;
    std::cout << "Log: Selected " << avail[n] << " mesh" << std::endl;

    log << "Running init server with defaults";
    mesh1->initserver(std::string(argv[1]), avail[n]);

    log << "Delaying for 10s to allow for another device";
    std::this_thread::sleep_for(std::chrono::milliseconds(10000));

    const std::string datastring = "Ayo! It works DAWGS! lfg";
    std::vector<char> datavec (datastring.begin(),datastring.end());

    log << "Sending some data";
    mesh1->sendraw(argv[2],&datavec);

    log << "Receiving some data";
    mesh1->recvraw(argv[2],&datavec);

    log << "Closing mesh";
    mesh1->killserver();

    log << "Finished";
}
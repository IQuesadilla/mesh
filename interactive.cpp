#include "mesh.h"

int main (int argc, char **argv)
{
    mesh *temp = new mesh();
    temp->initserver(argv[1]);

    std::string buffer;
    bool doLoop = true;
    while (doLoop)
    {
        std::cin >> buffer;

        if (buffer == "quit")
        {
            doLoop = false;
        }
        else if (buffer == "set")
        {
            std::cin >> buffer;
            if (buffer == "BroadcastAlive")
            {
                std::cin >> buffer;
                if (buffer == "true")
                    temp->setBroadcastAlive(true);
                else if (buffer == "false")
                    temp->setBroadcastAlive(false);
                else
                    std::cout << buffer << " is not a valid value" << std::endl;
            }
        }
        else if (buffer == "get")
        {
            //
        }
        else if (buffer == "connect")
        {
            //
        }
        else if (buffer == "send")
        {
            std::string datastring;
            std::cin >> buffer >> datastring;
            std::shared_ptr<std::vector<uint8_t> > databuff;
            databuff.reset(new std::vector<uint8_t>(datastring.begin(),datastring.end()));
            databuff->push_back('\0');
            temp->sendUDP({buffer,databuff});
        }
        else if (buffer == "recv")
        {
            auto value = temp->receiveUDP();
            std::cout << value.name << ' ' << (char*)value.data->data() << '!' << std::endl;
        }
        else
        {
            std::cout << "Improper usage..." << std::endl;
        }
    }

    temp->killserver();
    delete temp;
}

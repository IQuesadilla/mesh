#include "wire.h"

int main( int argc, char *argv[] )
{
    init_shm(false);
    std::string line;
    while (1)
    {
        char value = 0;
        wire_recv(&value);

        if (value == 3)
        {
            std::cout << "Recived line: " << line << std::endl;
            line = "";
        }
        else
        {
            line += value;
        }
    }

    return 0;
}
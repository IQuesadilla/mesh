#include "frontends/benchmark.h"
#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <thread>

#define CURRENT_BACKEND shm_backend
#include "backends/shm.h"

const std::string mbuff = "\
Lorem ipsum dolor sit amet, consectetur adipiscing elit, \
sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. \
Ut enim ad minim veniam, quis nostrud exercitation ullamco \
laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure \
dolor in reprehenderit in voluptate velit esse cillum dolore \
eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat \
non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\
";

void server_thread(mesh_backend* mesh)
{
    auto log = mesh->logobj->function("server_thread");
    while( mesh->isConnected() )
    {
        mesh->run_cycle();
        
        if ( mesh->to_send.empty() && mesh->doSend )
        {
            //log << mesh->_ct->timestamp() << " ID: " << mesh->ID << " IT: " << mesh->recvCount << " SEND MSGID: " << mesh->msgcount << libQ::loglevel::VALUEDEBUG;

            mesh->to_send = mbuff;
        }
    }
}

benchmark::benchmark()
{
    std::cout << "Running automated test suite!!!\n";
    std::cout << "This will run the shm backend at " << SPEED_COUNT << " speeds and return the results." << std::endl;

}

uint32_t benchmark::get_send_data( char *data, uint32_t len )
{
    int count = ( len + cPos > mLen) ? mLen - cPos : len;

    memcpy ( data, &mbuff.c_str()[cPos], count );
    cPos += count;

    return count;
}

void benchmark::put_recv_data( char *data, uint32_t len, uint64_t devID )
{
    auto log = logobj->function("recv_callback");
    auto result = std::string(data,len);
    bool success = ( result == mbuff );

    log << "From: " << devID << " Received: " << result << libQ::loglevel::VALUEDEBUG;

    _results.push_back(success); // Needs to be specified which speed and which device
    ++recvCount;
    return;
}

void benchmark::update()
{
    //myshm *meshes[DEVICE_COUNT];

    for (int speed = 0; speed < SPEED_COUNT; ++speed)
    {
        int delay = 3 * SPEEDS[speed];
        std::cout << "Running test at " << 1000.f / float(delay) << " bps" << std::endl;

        for (int devid = 0; devid < DEVICE_COUNT; ++devid)
        {
            auto newobj = new CURRENT_BACKEND( delay, 10, devid + 1, std::make_shared<libQ::log>(&logobj) );
            _backends->push_back( newobj );
        }

        while ( recvCount != extremelyRUN_COUNT )
        {
            std::this_thread::sleep_for(std::chrono::nanoseconds(500));
        }

        for (int devid = 0; devid < DEVICE_COUNT; ++devid)
            (*_backends)[devid]->disconnect();

        for (int devid = 0; devid < DEVICE_COUNT; ++devid)
        {
            //(*_backend_threads[devid]->join();
            delete (*_backends)[devid];
        }
    }

    std::cout << "\n\n\n\n\n\nRESULTS!!!" << std::endl;
    for (int test = 0; test < SPEED_COUNT; ++test)
    {
        std::cout << "Speed " << SPEEDS[test] << ": " << std::endl;

        for (int devid = 0; devid < DEVICE_COUNT; ++devid)
        {
            std::cout << " ID: " << devid + 1 << std::endl << "  ";
            for (int run = 0; run < RUN_COUNT; ++run)
            {
                std::cout << _results[test][0][run];
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    return;
}
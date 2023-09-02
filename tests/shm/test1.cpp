#include "backends/shm.h"
#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <thread>

#define RUN_COUNT 3
#define DEVICE_COUNT 4
#define SPEEDS 100

const std::string mbuff = "\
Lorem ipsum dolor sit amet, consectetur adipiscing elit, \
sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. \
Ut enim ad minim veniam, quis nostrud exercitation ullamco \
laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure \
dolor in reprehenderit in voluptate velit esse cillum dolore \
eu fugiat nulla pariatur. Excepteur sint occaecat cupidatat \
non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.\
";

class myshm : public shm_backend
{
public:
    myshm( int delayms, int buffsize, int ID, std::vector<bool> *results, std::shared_ptr<libQ::log> log ) : shm_backend( delayms, buffsize, ID )
    {
        cPos = 0;
        mLen = mbuff.length();
        _results = results;

        _log = log;
        _log->setPrefix(std::string(std::string("ID: ") + std::to_string(ID)).c_str());

        recvCount = 0;
        doLoop = true;
        doSend = true;
    }

    ~myshm()
    {
        auto log = _log->function("~myshm");
        //delete current_thread.get();
    }

    void kill()
    {
        doLoop = false;
    }

    void waitForDeath()
    {
        current_thread->join();
    }

    uint32_t send_callback( char *data, uint32_t len )
    {
        int count = ( len + cPos > mLen) ? mLen - cPos : len;

        memcpy ( data, &mbuff.c_str()[cPos], count );
        cPos += count;

        return count;
    }

    void recv_callback( char *data, uint32_t len, uint64_t devID )
    {
        auto log = _log->function("recv_callback");
        auto result = std::string(data,len);
        bool success = ( result == mbuff );

        log << _ct->timestamp() << " From: " << devID << " Received: " << result << libQ::loglevel::VALUEDEBUG;

        _results->push_back(success);
        ++recvCount;
        return;
    }

    std::shared_ptr<std::thread> current_thread;

    unsigned int cPos;
    unsigned int mLen;
    std::vector<bool> *_results;

    int recvCount;
    bool doLoop;
    bool doSend;
};

void server_thread(myshm *mesh)
{
    auto log = mesh->_log->function("server_thread");
    while( mesh->doLoop )
    {
        mesh->loop();
        
        if ( mesh->to_send.empty() && mesh->doSend )
        {
            //log << mesh->_ct->timestamp() << " ID: " << mesh->ID << " IT: " << mesh->recvCount << " SEND MSGID: " << mesh->msgcount << libQ::loglevel::VALUEDEBUG;

            mesh->to_send = mbuff;
        }
    }
}

int main()
{
    libQ::log logobj(libQ::vlevel::DEBUG);
    int speeds[] = {SPEEDS};
    int speed_count = sizeof(speeds)/sizeof(*speeds);

    std::cout << "Running automated test suite!!!\n";
    std::cout << "This will run the shm backend at " << speed_count << " speeds and return the results." << std::endl;

    std::vector<bool> results[speed_count][DEVICE_COUNT];
    myshm *meshes[DEVICE_COUNT];

    for (int speed = 0; speed < speed_count; ++speed)
    {
        int delay = 3 * speeds[speed];
        std::cout << "Running test at " << 1000.f / float(delay) << " bps" << std::endl;

        for (int devid = 0; devid < DEVICE_COUNT; ++devid)
        {
            meshes[devid] = new myshm( delay, 10, devid + 1, &results[speed][devid], std::make_shared<libQ::log>(&logobj) );
            meshes[devid]->current_thread.reset( new std::thread(server_thread, meshes[devid]) );
        }

        while ( meshes[0]->recvCount != RUN_COUNT )
        {
            std::this_thread::sleep_for(std::chrono::nanoseconds(500));
        }

        for (int devid = 0; devid < DEVICE_COUNT; ++devid)
            meshes[devid]->kill();

        for (int devid = 0; devid < DEVICE_COUNT; ++devid)
        {
            meshes[devid]->waitForDeath();
            delete meshes[devid];
        }
    }

    std::cout << "\n\n\n\n\n\nRESULTS!!!" << std::endl;
    for (int test = 0; test < speed_count; ++test)
    {
        std::cout << "Speed " << speeds[test] << ": " << std::endl;

        for (int devid = 0; devid < DEVICE_COUNT; ++devid)
        {
            std::cout << " ID: " << devid + 1 << std::endl << "  ";
            for (int run = 0; run < RUN_COUNT; ++run)
            {
                std::cout << results[test][0][run];
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }

    return 0;
}
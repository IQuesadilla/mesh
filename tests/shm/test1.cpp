#include "backends/shm.h"
#include <cstring>
#include <string>
#include <iostream>
#include <thread>

#define RUN_COUNT 2
#define TEST_COUNT 1

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
    myshm( int delayms, int buffsize, int ID, bool *results, std::shared_ptr<libQ::log> log ) : shm_backend( delayms, buffsize, ID )
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

        _results[recvCount] = success;
        ++recvCount;
        return;
    }

    unsigned int cPos;
    unsigned int mLen;
    bool *_results;

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

    std::cout << "Running automated test suite!!!\n";
    std::cout << "This will run the shm backend at " << TEST_COUNT << " speeds and return the results." << std::endl;

    bool results[TEST_COUNT][2][RUN_COUNT];

    for (int i = 0; i < TEST_COUNT; ++i)
    {
        int delay = 100 * (TEST_COUNT-i);
        std::cout << "Running test at " << 1000.f / float(delay) << " bps" << std::endl;
        myshm mesh1( delay, 10, 2, results[i][0], std::make_shared<libQ::log>(&logobj) );
        myshm mesh2( delay, 10, 3, results[i][1], std::make_shared<libQ::log>(&logobj) );
        
        std::thread server1 (server_thread, &mesh1);

        //std::this_thread::sleep_for(std::chrono::milliseconds(33));

        std::thread server2 (server_thread, &mesh2);

        while ( mesh1.doSend || mesh2.doSend )
        {
            if ( mesh1.recvCount == RUN_COUNT) mesh2.doSend = false;
            if ( mesh2.recvCount == RUN_COUNT) mesh1.doSend = false;

            std::this_thread::sleep_for(std::chrono::nanoseconds(500));
        }

        mesh1.doLoop = false;
        mesh2.doLoop = false;

        server1.join();
        server2.join();
    }

    std::cout << "\n\n\n\n\n\nRESULTS!!!" << std::endl;
    for (int test = 0; test < TEST_COUNT; ++test)
    {
        std::cout << "Test " << test << ": \n";
        for (int run = 0; run < RUN_COUNT; ++run)
        {
            std::cout << results[test][0][run] << results[test][1][run] << ' ';
        }
        std::cout << std::endl;
    }

    return 0;
}
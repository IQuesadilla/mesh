#include "backends/shm.h"
#include <cstring>
#include <string>
#include <iostream>
#include <thread>

#define RUN_COUNT 3
#define TEST_COUNT 10

const std::string mbuff = "\
This is long.\
";

class myshm : public shm
{
public:
    myshm( int delayms, int buffsize, bool *results ) : shm( delayms, buffsize )
    {
        cPos = 0;
        mLen = mbuff.length();
        _results = results;

        recvCount = 0;
        doLoop = true;
        doSend = true;
    }

    int send_callback( char *data, size_t len )
    {
        int count = ( len + cPos > mLen) ? mLen - cPos : len;

        memcpy ( data, &mbuff.c_str()[cPos], count );
        cPos += count;

        return count;
    }

    void recv_callback( char *data, size_t len )
    {
        auto result = std::string(data,len);
        bool success = ( result == mbuff );

        stdout_mutex->lock();
        std::cout << TIMESTAMP << " ID: " << ID << " Received: " << result << std::endl;
        stdout_mutex->unlock();

        _results[recvCount] = success;
        ++recvCount;
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
    while( mesh->doLoop )
    {
        mesh->loop();
        
        if ( mesh->to_send.empty() && mesh->doSend )
        {
            mesh->stdout_mutex->lock();
            std::cout << TIMESTAMP << " ID: " << mesh->ID << " IT: " << mesh->recvCount << " SEND MSGID: " << mesh->msgcount << std::endl;
            mesh->stdout_mutex->unlock();

            mesh->to_send = mbuff;
        }
    }
}

int main()
{
    std::cout << "Running automated test suite!!!\n";
    std::cout << "This will run the shm backend at " << TEST_COUNT << " speeds and return the results." << std::endl;

    bool results[TEST_COUNT][2][RUN_COUNT*2];

    std::shared_ptr<std::mutex> stdout_mutex;
    stdout_mutex.reset(new std::mutex());

    for (int i = 0; i < TEST_COUNT; ++i)
    {
        int delay = 10 * (10-i);
        std::cout << "Running test at " << 1000.f / float(delay) << " bps" << std::endl;
        myshm mesh1( delay, 10, results[i][0] );
        myshm mesh2( delay, 10, results[i][1] );

        mesh1.stdout_mutex = stdout_mutex;
        mesh2.stdout_mutex = stdout_mutex;
        mesh1.ID = 1;
        mesh2.ID = 155;
        
        std::thread server1 (server_thread, &mesh1);

        //std::this_thread::sleep_for(std::chrono::milliseconds(33));

        std::thread server2 (server_thread, &mesh2);

        while ( mesh1.recvCount < RUN_COUNT || mesh2.recvCount < RUN_COUNT )
        {
            if ( mesh1.recvCount == RUN_COUNT) mesh2.doSend = false;
            if ( mesh2.recvCount == RUN_COUNT) mesh1.doSend = false;

            std::this_thread::sleep_for(std::chrono::milliseconds(33));
        }

        std::cout << "................................................." << std::endl;

        mesh1.doLoop = false;
        mesh2.doLoop = false;

        server1.join();
        server2.join();
    }

    std::cout << "\n\n\n\n\n\nRESULTS!!!" << std::endl;
    for (int test = 0; test < TEST_COUNT; ++test)
    {
        std::cout << "Test 1: \n";
        for (int run = 0; run < RUN_COUNT; ++run)
        {
            std::cout << results[test][0][run] << results[test][1][run] << ' ';
        }
    }
    std::cout << std::endl;
}
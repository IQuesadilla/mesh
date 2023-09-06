// This file is intended to have a main and simply bridge a frontend to a backend
// This file should get renamed as bridge already means something
// As defines, this file should accept one frontend and any number of backends
// The frontend does not need to know which backends are being used

#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <thread>

#include <log.h>
#include <cycle_timer.h>

#include "frontend.h"
#include "frontends/benchmark.h"

#include "backend.h"
#include "backends/shm.h"

#define CURRENT_FRONTEND benchmark
#define CURRENT_BACKEND shm_backend

#define BUFFSIZE 10

class myBackendClass : public CURRENT_BACKEND
{
    
public:
    myBackendClass( int ID, int buffsize, std::shared_ptr<libQ::log> log ) : shm_backend( ID, buffsize )
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

    ~myBackendClass()
    {
        auto log = _log->function("~myBackendClass");
        //delete current_thread.get();
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

        log << "From: " << devID << " Received: " << result << libQ::loglevel::VALUEDEBUG;

        _results->push_back(success);
        ++recvCount;
        return;
    }
};

class myFrontendClass : public CURRENT_FRONTEND
{
public:
    myFrontendClass()
    {
        ;
    }
};

int main()
{
    std::shared_ptr<libQ::log> logobj;
    logobj.reset( new libQ::log(libQ::vlevel::DEBUG) );
    logobj->function("main");

    int buffsize = BUFFSIZE;

    std::shared_ptr<frontend> myFrontend;
    myFrontend.reset(new myFrontendClass() );

    std::shared_ptr<mesh_backend> myBackend;
    myBackend.reset(new myBackendClass(1,buffsize,logobj) );




}
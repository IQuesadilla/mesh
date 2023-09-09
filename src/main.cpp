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

#define BUFFSIZE 10

int main()
{
    std::shared_ptr<libQ::log> logobj;
    logobj.reset( new libQ::log(libQ::vlevel::DEBUG) );
    logobj->function("main");

    int buffsize = BUFFSIZE;

    std::shared_ptr<frontend> myFrontend;
    myFrontend.reset(new CURRENT_FRONTEND() );

//    std::shared_ptr<mesh_backend> myBackend;
//    myBackend.reset(new myBackendClass(1,buffsize,logobj) );

    while ( myFrontend->doLoop() )
    {
        myFrontend->run_cycle();
    }

}
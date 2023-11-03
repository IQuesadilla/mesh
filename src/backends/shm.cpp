#include "backend.h"

#include "cycle_timer.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <fstream>
#include <memory>

#include <iostream>

#define DELAYMS 50
#define SHMKEY 0x1234

struct backend_ptr
{
    std::shared_ptr<libQ::cycle_timer> _ct;
    uint8_t *shmptr;
};

#define myshm ((backend_ptr*)_usrptr)

void backend::BackendInit()
{
    int sid;
    sid = shmget( SHMKEY, 1024, 0644|IPC_CREAT );

    _usrptr = new backend_ptr;

    HeaderCycleCount = 2; // Replace
    BodyCycleCount = 24;

    myshm->shmptr = (uint8_t*)shmat( sid, NULL, 0 );
    *myshm->shmptr = 0;

    myshm->_ct.reset(new libQ::cycle_timer(DELAYMS));
}

void backend::WriteBodyByte()
{
    switch ( CurrentCycle % 3 )
    {
    case 2: // Perform the write on the first part of the cycle
        if ( (BodyByteBuffer>>(CurrentCycle>>1)) & 0b1 )
        {
            (*myshm->shmptr) = 1;
        }
        break;
    case 1: // This is for reading, do nothing
        break;
    case 0: // Clear the memory buffer to prepare for the next write
        (*myshm->shmptr) = 0;
        break;
    }
}

void backend::ReadBodyByte() // Incomplete
{
   switch ( CurrentCycle % 3 )
    {
    case 2: // This is for writing, do nothing
        break;
    case 1: // Perform the write on the middle part of the cycle
        // Perform a read and set some bits accordingly
        break;
    case 0: // Clear the memory buffer to prepare for the next write
        (*myshm->shmptr) = 0;
        break;
    }
}
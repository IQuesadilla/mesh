#include "backends/shm.h"

#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <fstream>

#include <iostream>

#define DELAYMS 50
#define BUFFSIZE 10
#define SHMKEY 0x1234

shm_backend::shm_backend() : mesh_backend(DELAYMS,BUFFSIZE,200)
{
    init_shm(SHMKEY);
}

shm_backend::shm_backend(int delayms, int buffsize, int ID) : mesh_backend(delayms,buffsize,ID)
{
    init_shm(SHMKEY);
}

void shm_backend::init_shm( key_t key )
{
    int sid;

    sid = shmget( SHMKEY, 1024, 0644|IPC_CREAT );

    //stdout_mutex->lock();
    //std::cout << "errno: " << errno << std::endl;
    //stdout_mutex->unlock();
    //errno = 0;

    shmptr = (uint8_t*)shmat( sid, NULL, 0 );
    *shmptr = 0;

    _ct.reset(new libQ::cycle_timer(_delayms));
}

void shm_backend::wire_clear_bit()
{
    _ct->delay_until(2,3);
    *shmptr = 0;
    return;
}

void shm_backend::wire_send_bit(uint8_t bit)
{
    // Wait for nothing, should run new_cycle first

    if (bit)
        (*shmptr) = bit;

    return;
}

uint8_t shm_backend::wire_recv_bit()
{
    _ct->delay_until(1,3); // Wait for 1/3 of interval

    return (*shmptr);
}
#pragma once
#ifndef BACKEND_SHM
#define BACKEND_SHM

#include "backend.h"

class shm_backend : public mesh_backend
{
public:
    shm_backend( int ID, int buffsize );
    shm_backend( int delayms, int buffsize, int ID );

private:
    uint8_t *shmptr;

    void init_shm( key_t key );

    void wire_clear_bit();
    void wire_send_bit( uint8_t bit );
    uint8_t wire_recv_bit();
};

#endif
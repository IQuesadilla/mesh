#pragma once
#ifndef BACKEND_SHM
#define BACKEND_SHM

#include <cstddef>
#include <cstdint>
#include <sys/types.h>
#include <memory>

#define DELAYMS 50
#define BUFFSIZE 10
#define SHMKEY 0x1234

#include <mutex>
#include <bitset>

#define TIMESTAMP uint16_t(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count())

class shm
{
public:
    shm();
    shm( int delayms, int buffsize );
    ~shm();

    //void sendmsg();
    //int recvmsg();

    void loop();

    virtual int send_callback( char *data, size_t len ) = 0;
    virtual void recv_callback( char *data, size_t len ) = 0;

    bool isRecieving;
    bool isSending;
    uint16_t msgcount;

    std::string to_send;

    std::shared_ptr<std::mutex> stdout_mutex;
    int ID;

private:
    uint8_t *shmptr;
    int _delayms, _buffsize;

    void init_shm( key_t key );

    void wait_for_next_send();
    void wait_for_next_recv();
    void wait_for_next_clear();

    void wire_clear_bit();
    void wire_send_bit( bool bit );
    bool wire_recv_bit();

    void wire_send( char byte );
    void wire_recv( char *byte );

};

typedef shm backend;

#endif
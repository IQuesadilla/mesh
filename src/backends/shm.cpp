#include "backends/shm.h"

#include <iostream>
#include <thread>
#include <fstream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <cstring>

shm::shm()
{
    init_shm(SHMKEY);

    _delayms = DELAYMS;
    _buffsize = BUFFSIZE;
}

shm::shm(int delayms, int buffsize)
{
    init_shm(SHMKEY);

    _delayms = delayms;
    _buffsize = buffsize;
}

shm::~shm()
{
    ;
}
/*
void shm::sendmsg()
{
    while ( isRecieving ) 
        std::this_thread::sleep_for( std::chrono::milliseconds(2) );

    isSending = true;
    if ( wire_send( msgcount ) )
    {
        stdout_mutex->lock();
        std::cout << "ID: " << ID << " FAILED! Now looping..." << std::endl;
        stdout_mutex->unlock();
        isSending = false;
        std::this_thread::sleep_for( std::chrono::milliseconds(5) );
        sendmsg();
        return;
    }

    int len = _buffsize;
    while ( len == _buffsize )
    {
        char buff[_buffsize];
        len = send_callback(buff, _buffsize);
        
        for (int i = 0; i < len; ++i)
            wire_send( buff[i] );
    }

    wire_send(3);

    msgcount = 1;
    isSending = false;
}

int shm::recvmsg()
{
    char cmsgid = 0;
    wire_recv( &cmsgid );
    int msgid = cmsgid;

    if ( isSending ) return -1;
    isRecieving = true;

    stdout_mutex->lock();
    std::cout << "Started recieving a message" << std::endl;
    stdout_mutex->unlock();

    int i;
    do
    {
        char buffer[_buffsize];
        for ( i = 0; i < _buffsize; ++i )
        {
            char value = 0;
            wire_recv( &value );

            if ( value == 3 ) break;
            else buffer[i] = value;
        }

        recv_callback(buffer, i);
    } while ( i == _buffsize );

    ++msgcount;
    isRecieving = false;
    return msgid;
}
*/
void shm::loop()
{
    uint16_t recv_QueuePosition = 0;
    int8_t QueueBit = 15;
    uint32_t msglen = to_send.length();

    if ( msglen > 0 )
    {
        wire_send_bit(1);

        stdout_mutex->lock();
        std::cout << TIMESTAMP << " ID: " << ID << " Attempting to send some data" << std::endl;
        stdout_mutex->unlock();

        wait_for_next_recv();
        wire_clear_bit();

        for ( ; QueueBit >= 0; --QueueBit )
        {
            stdout_mutex->lock();
            std::cout << TIMESTAMP << " ID: " << ID << " SEND QueueBit " << int(QueueBit) << std::endl;
            stdout_mutex->unlock();

            bool value = ID & 0b1<<QueueBit;
            wire_send_bit( value );
            //std::cout << value << std::flush;
            bool inbit = wire_recv_bit();
            recv_QueuePosition = (recv_QueuePosition)|(inbit)<<QueueBit;
            wire_clear_bit();
            if ( inbit && !value )
            {
                stdout_mutex->lock();
                std::cout << TIMESTAMP << " ID: " << ID << " Bit conflict! - expected " << value << " but got " << inbit << " at QueueBit " << int(QueueBit) << std::endl;
                stdout_mutex->unlock();
                break;
            }
        }

        --QueueBit;

        stdout_mutex->lock();
        std::cout << TIMESTAMP << " ID: " << ID << " QueueBit " << int(QueueBit) << std::endl;
        stdout_mutex->unlock();

        if ( QueueBit < -1 )
        {
            wire_send( ((char*)(&msglen))[2] );
            wire_send( ((char*)(&msglen))[1] );
            wire_send( ((char*)(&msglen))[0] );

            stdout_mutex->lock();
            std::cout << TIMESTAMP << " ID: " << ID << " Sending " << std::bitset<24>(msglen) << " bytes of data" << std::endl;
            stdout_mutex->unlock();

            for (uint32_t i = 0; i < msglen; ++i)
                wire_send(to_send[i]);

            to_send.erase(0,msglen);
            return;
        }
    }
    else
    {
        stdout_mutex->lock();
        std::cout << TIMESTAMP << " ID: " << ID << " Testing for start bit" << std::endl;
        stdout_mutex->unlock();
        if ( !wire_recv_bit() ) return;
    }

    stdout_mutex->lock();
    std::cout << TIMESTAMP << " ID: " << ID << " Attempting to receive some data, QueueBit = " << int(QueueBit) << std::endl;
    stdout_mutex->unlock();

    for ( ; QueueBit >= 0; --QueueBit )
    {
        stdout_mutex->lock();
        std::cout << TIMESTAMP << " ID: " << ID << " RECV QueueBit " << int(QueueBit) << std::endl;
        stdout_mutex->unlock();

        bool inbit = wire_recv_bit();
        recv_QueuePosition = (recv_QueuePosition)|(inbit)<<QueueBit;
    }

    stdout_mutex->lock();
    std::cout << TIMESTAMP << " ID: " << ID << " Receiving data from Queue Position " << recv_QueuePosition << std::endl;
    stdout_mutex->unlock();

    uint32_t recvlen = 0;
    wire_recv( &((char*)(&recvlen))[2] );
    wire_recv( &((char*)(&recvlen))[1] );
    wire_recv( &((char*)(&recvlen))[0] );

    stdout_mutex->lock();
    std::cout << TIMESTAMP << " ID: " << ID << " Receiving " << std::bitset<24>(recvlen) << " bytes of data" << std::endl;
    stdout_mutex->unlock();

    char buffer[recvlen];
    for (uint32_t i = 0; i < recvlen; ++i)
        wire_recv( &buffer[i] );
    recv_callback(buffer,recvlen);
    //wire_clear_bit();
    //wait_for_next_recv();
}

void shm::init_shm( key_t key )
{
    int sid;

    sid = shmget( SHMKEY, 1024, 0644|IPC_CREAT );

    //stdout_mutex->lock();
    //std::cout << "errno: " << errno << std::endl;
    //stdout_mutex->unlock();
    errno = 0;

    shmptr = (uint8_t*)shmat( sid, NULL, 0 );
    *shmptr = 0;

    isRecieving = false;
    isSending = false;
    msgcount = 1;
}

void shm::wait_for_next_send()
{
    using namespace std::chrono;

    auto now = system_clock::now();
    auto since_epoch = duration_cast<milliseconds>(now.time_since_epoch());

    // Calculate the next tenth of a second mark
    auto next_tenth_of_second = ((since_epoch / milliseconds(_delayms)) * milliseconds(_delayms)).count();

    auto next_time_point = system_clock::time_point(milliseconds(next_tenth_of_second));

    std::this_thread::sleep_until(next_time_point);
}

void shm::wait_for_next_recv()
{
    using namespace std::chrono;

    auto now = system_clock::now();
    auto since_epoch = duration_cast<milliseconds>(now.time_since_epoch());

    // Calculate the next tenth of a second mark
    auto next_tenth_of_second = ((since_epoch / milliseconds(_delayms) + 1) * milliseconds(_delayms)).count();

    auto next_time_point = system_clock::time_point(milliseconds(next_tenth_of_second) + milliseconds(_delayms / 3));

    std::this_thread::sleep_until(next_time_point);
}

void shm::wait_for_next_clear()
{
    using namespace std::chrono;

    auto now = system_clock::now();
    auto since_epoch = duration_cast<milliseconds>(now.time_since_epoch());

    // Calculate the current tenth of a second mark
    auto next_tenth_of_second = ((since_epoch / milliseconds(_delayms)) * milliseconds(_delayms)).count();

    auto next_time_point = system_clock::time_point(milliseconds(next_tenth_of_second) + ( milliseconds(_delayms / 3) * 2 ));

    std::this_thread::sleep_until(next_time_point);
}

void shm::wire_clear_bit()
{
    wait_for_next_clear();
    *shmptr = 0;
    return;
}

void shm::wire_send_bit(bool bit)
{
    wait_for_next_send();

    if ( !(*shmptr) )
        *shmptr = ( (bit) ? 1 : 0 );

    return;
}

bool shm::wire_recv_bit()
{
    wait_for_next_recv();

    return (*shmptr) > 0;
}

void shm::wire_send(char byte)
{
    for (int i = 7; i >= 0; --i)
    {
        bool value = byte & 0b1<<i;
        wire_send_bit( value );
        wait_for_next_recv();
        wire_clear_bit();
    }

    return;
}

void shm::wire_recv(char *byte)
{
    *byte = 0;
    for (int i = 7; i >= 0; --i)
    {
        bool value = wire_recv_bit();
        *byte = (*byte)|((value)<<i);
    }

    stdout_mutex->lock();
    std::cout << TIMESTAMP << " ID: " << ID << " Recv byte " << std::bitset<8>(*byte) << std::endl;
    stdout_mutex->unlock();
}
#include "backends/shm.h"
#include "backend.h"

#include <thread>
#include <fstream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <errno.h>
#include <cstring>
#include <bitset>
#include <iostream>

mesh_backend::mesh_backend(int delayms, int buffsize, int ID)
{
    _delayms = delayms;
    _buffsize = buffsize;

    _ID = ID;
    _QueuePosition = ID;

    decrement_start_delay = true;
    waiting_for_start = false; // This exists only for cleanliness in the log
    start_delay = 4200; // Should be around the size of a header but for shm would need to be technically max data length
}

mesh_backend::~mesh_backend()
{
    auto log = _log->function("~mesh_backend");
}

void mesh_backend::loop()
{
    auto log = _log->function("loop",waiting_for_start);

    _ct->new_cycle();
    if ( !to_send.empty() && !start_delay )
    {
        waiting_for_start = false;
        wire_send();
    }
    else
    {
        uint8_t value = wire_recv_bit();

        if ( !waiting_for_start )
        {
            log << " Waiting for start bit" << libQ::loglevel::NOTEDEBUG;
            waiting_for_start = true;
        }
        
        if ( value && start_delay )
        {
            log << "Disable automatic start" << libQ::loglevel::NOTEDEBUG;
            decrement_start_delay = false;
        }
        
        if ( value == 255 )
        {
            waiting_for_start = false;
            start_delay = 0;
            wire_recv();
        }

        if ( start_delay && decrement_start_delay)
            --start_delay;
    }
}

void mesh_backend::wire_send()
{
    auto log = _log->function("wire_send");

    uint16_t recv_QueuePosition = 0;
    int8_t QueueBit = 15;
    uint32_t msglen = to_send.length();


    wire_send_bit(255);

    log << " Attempting to send the header" << libQ::loglevel::NOTEDEBUG;
    
    wire_recv_bit();
    wire_clear_bit();

    for ( ; QueueBit >= 0; --QueueBit )
    {
        bool value = _QueuePosition & 0b1<<QueueBit;
        _ct->new_cycle();
        wire_send_bit( value );
        //log << value << std::flush;
        bool inbit = wire_recv_bit();
        recv_QueuePosition = (recv_QueuePosition)|(inbit)<<QueueBit;
        wire_clear_bit();
        if ( inbit && !value )
        {
            log << " Bit conflict! - expected " << value << " but got " << inbit << " at QueueBit " << int(QueueBit) << libQ::loglevel::NOTEDEBUG;
            wire_recv(QueueBit - 1, recv_QueuePosition);
            return;
        }
    }

    wire_send_byte( ((char*)(&msglen))[2] );
    wire_send_byte( ((char*)(&msglen))[1] );
    wire_send_byte( ((char*)(&msglen))[0] );

    log << " Sending " << std::bitset<24>(msglen).to_string() << " bytes of data" << libQ::loglevel::NOTEDEBUG;

    for (uint32_t i = 0; i < msglen; ++i)
        wire_send_byte(to_send[i]);

    to_send.erase(0,msglen);
    _QueuePosition = 1;
}

void mesh_backend::wire_recv(int8_t QueueBit, uint16_t inQueuePosition)
{
    auto log = _log->function("wire_recv");
    log << " Attempting to receive some data, QueueBit = " << int(QueueBit) << libQ::loglevel::NOTEDEBUG;

    for ( ; QueueBit >= 0; --QueueBit )
    {
        _ct->new_cycle();
        bool inbit = wire_recv_bit();
        inQueuePosition = (inQueuePosition)|(inbit)<<QueueBit;
    }

    log << " Receiving data from Queue Position " << inQueuePosition << libQ::loglevel::NOTEDEBUG;

    uint32_t recvlen = 0;
    wire_recv_byte( &((char*)(&recvlen))[2] );
    wire_recv_byte( &((char*)(&recvlen))[1] );
    wire_recv_byte( &((char*)(&recvlen))[0] );

    log << " Receiving " << std::bitset<24>(recvlen).to_string() << " bytes of data" << libQ::loglevel::NOTEDEBUG;

    char buffer[recvlen];
    for (uint32_t i = 0; i < recvlen; ++i)
        wire_recv_byte( &buffer[i] );
    recv_callback(buffer,recvlen,inQueuePosition); // Replace recv_QueuePosition with ID once adding new devices is implemented
    ++_QueuePosition;
}

void mesh_backend::wire_send_byte(char byte)
{
    for (int i = 7; i >= 0; --i)
    {
        bool value = byte & 0b1<<i;
        _ct->new_cycle();
        wire_send_bit( value );
        wire_recv_bit();
        wire_clear_bit();
    }

    return;
}

void mesh_backend::wire_recv_byte(char *byte)
{
    *byte = 0;
    for (int i = 7; i >= 0; --i)
    {
        _ct->new_cycle();
        bool value = wire_recv_bit();
        *byte = (*byte)|((value)<<i);
    }
}
#ifndef MESH_BACKEND
#define MESH_BACKEND
#pragma once

#include <string>
#include <memory>
#include <cycle_timer.h>
#include <log.h>
#include <mutex>

class mesh_backend
{
public:
    enum header_types
    {
        MESSAGE,
        NEW_DEVICE,
        PING,
        EMERGENCY
    };

    virtual ~mesh_backend();

    void loop();

    virtual uint32_t send_callback( char *data, uint32_t len ) = 0;
    virtual void recv_callback( char *data, uint32_t len, uint64_t devID ) = 0;

    std::string to_send;

    

    std::shared_ptr<libQ::cycle_timer> _ct;
    std::shared_ptr<libQ::log> _log;

protected:
    int _delayms, _buffsize;
    int _ID;
    uint8_t _QueuePosition;
    uint16_t start_delay;
    bool decrement_start_delay, waiting_for_start;

    mesh_backend(int delayms, int buffsize, int ID);

    void wire_recv(int8_t QueueBit = 7, uint16_t inQueuePosition = 0);

    bool wire_send_header(header_types type = MESSAGE);
    void wire_send_body(const char *raw, uint16_t msglen);

    virtual void wire_clear_bit() = 0;
    virtual void wire_send_bit( uint8_t bit ) = 0;
    virtual uint8_t wire_recv_bit() = 0;

    virtual void wire_send_byte( char byte );
    virtual void wire_recv_byte( char *byte );
};

#endif
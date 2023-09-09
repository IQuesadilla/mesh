#ifndef FRONTEND_H
#define FRONTEND_H
#pragma once

#include "log.h"
#include "event_timer.h"

#include "backend.h"

#include <memory>
#include <vector>

class frontend
{
public:
    frontend();

    bool doLoop();

    virtual uint32_t get_send_data( char *data, uint32_t len ) = 0;
    virtual void put_recv_data( char *data, uint32_t len, uint64_t devID ) = 0;

    void send(int msgID);

    void run_cycle();

protected:
    virtual void update() = 0;

    std::shared_ptr<libQ::log> logobj;
    std::shared_ptr<libQ::event_timer<mesh_backend*> > backend_timer;

    std::shared_ptr<std::vector<mesh_backend*> > _backends;

};

#endif
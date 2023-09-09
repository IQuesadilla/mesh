#ifndef FRONTEND_BENCHMARK_H
#define FRONTEND_BENCHMARK_H
#pragma once

#define RUN_COUNT 3
#define DEVICE_COUNT 4
#define SPEEDS (int[]){1}
#define SPEED_COUNT (sizeof(SPEEDS)/sizeof(*SPEEDS))

#include "frontend.h"

#include <thread>

class benchmark : public frontend
{
public:
    benchmark();

    uint32_t get_send_data( char *data, uint32_t len );
    void put_recv_data( char *data, uint32_t len, uint64_t devID );

private:
    unsigned int cPos;
    unsigned int mLen;
    std::vector<bool> _results[SPEED_COUNT][DEVICE_COUNT];

    //std::vector<std::thread*> _backend_threads;

    int recvCount;
    bool doLoop;
    bool doSend;

    void update();
};

#endif
#ifndef FRONTEND_BENCHMARK_H
#define FRONTEND_BENCHMARK_H
#pragma once

#include "frontend.h"

class benchmark : public frontend
{
public:
    benchmark();

private:
    std::shared_ptr<std::thread> current_thread;

    unsigned int cPos;
    unsigned int mLen;
    std::vector<bool> *_results;

    int recvCount;
    bool doLoop;
    bool doSend;
};

#endif
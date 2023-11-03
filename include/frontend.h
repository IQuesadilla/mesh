#ifndef MESH_FRONTEND
#define MESH_FRONTEND

#include "../libQ/include/libQ.h"
#include "../libQ/include/cfifo.h"

class frontend
{
public:
    frontend(cfifo *wfifoptr, cfifo *rfifoptr);

    cfifo* GetReadFifo();
    cfifo* GetWriteFifo();

private:
    // Specific Functions
    void FrontendInit();

    cfifo *_write_fifo;
    cfifo *_read_fifo;
    void *_usrptr;
};

#endif
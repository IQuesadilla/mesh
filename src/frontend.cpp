#include "frontend.h"

frontend::frontend(cfifo *wfifoptr, cfifo *rfifoptr)
{
    _write_fifo = wfifoptr;
    _read_fifo = rfifoptr;

    FrontendInit();
}


cfifo* frontend::GetReadFifo()
{
    return _read_fifo;
}

cfifo* frontend::GetWriteFifo()
{
    return _write_fifo;
}
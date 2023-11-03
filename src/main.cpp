#include "frontend.h"
#include "backend.h"

#include "libQ.h"

#ifndef __AVR__

#include <cstring>
#include <string>
#include <iostream>
#include <vector>
#include <thread>

#include <log.h>

int main()
{
    std::shared_ptr<libQ::log> logobj;
    logobj.reset( new libQ::log(libQ::vlevel::DEBUG) );
    logobj->function("main");

    int buffsize = BUFFSIZE;

    std::shared_ptr<frontend> myFrontend;
    myFrontend.reset(new CURRENT_FRONTEND() );

//    std::shared_ptr<mesh_backend> myBackend;
//    myBackend.reset(new myBackendClass(1,buffsize,logobj) );

    while ( myFrontend->doLoop() )
    {
        myFrontend->run_cycle();
    }

}

#else

#include "avr/generic.h"

void start()
{
	setBits(PORTB, 0b100);
	for (uint i = 10000; i > 0; ++i)
	{
		nop();
		nop();
		nop();
	}
	resetBits(PORTB, 0b100);

	const uint cfifo_buffer_len = 512;
	char _write_cfifo_buffer[cfifo_buffer_len];
	cfifo _write_stackcfifo = cfifo(_write_cfifo_buffer,cfifo_buffer_len);
	char _read_cfifo_buffer[cfifo_buffer_len];
	cfifo _read_stackcfifo = cfifo(_read_cfifo_buffer,cfifo_buffer_len);

	frontend stackfrontend(&_write_stackcfifo,&_read_stackcfifo);
	backend stackbackend(&_write_stackcfifo,&_read_stackcfifo);

	EnableInterrupts();

	while (1) { nop(); }
}

#endif
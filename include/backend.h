#ifndef MESH_BACKEND
#define MESH_BACKEND
#pragma once

#include <string>
#include <memory>
#include <cycle_timer.h>
#include <log.h>
#include <event_timer.h>
#include <mutex>

#include "frontend.h"

class mesh_backend : public libQ::etimer_child
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

    void run_cycle();

    void disconnect();
    bool isConnected();

    void etimer_update();
    
    std::shared_ptr<frontend> _frontend;

    std::shared_ptr<libQ::cycle_timer> _ct;
    std::shared_ptr<libQ::log> logobj;

protected:
    int _delayms, _buffsize;
    int _ID;
    uint8_t _QueuePosition;
    uint16_t start_delay;
    bool decrement_start_delay, waiting_for_start;

    mesh_backend(int delayms, int buffsize, int ID, std::shared_ptr<libQ::log> _log);

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

// 8/31/23
// Brainstorming
// All data to be sent gets loaded into a buffer through send_callback
// and then frontend runs the timer and scheduler to make sure the send-recv-clear
// cycle happens according to the proper timing. Since the data is already in a
// buffer, it is up to the individual backend to decide whether it needs
// to send data bitwise, bytewise, or in one big chunk. If sending bitwise,
// wire_send_bit, wire_recv_bit, and wire_clear_bit are the funcions to use; 
// if sending bytewise then wire_send_byte and wire_recv_byte are the functions
// to use, and wire_*_bit should all do nothing. If sending as one large chunk,
// wire_send and wire_recv are the best commands to use. Setting up scheduling
// for all of these is going to be incredibly difficult, as this is meant to be
// able to run on a microcontroller, meaning no multithreading. The part that
// makes scheduling extra difficult is that not all backends send data at the
// same rate, meaning sometimes the bit-timings are going to be different. The
// only potential method I can think of is to have each backend keep track of when
// the next event is to occur and then constantly poll all of them to see if the
// time has come. This method could also remove the need to have separate functions
// for the different transfer methods, as it is up to the backend to come up with
// timing anyway, so the frontend does not care whether to call a bit-wise function
// or a byte-wise function. 

// Option 1
// Each backend keeps track of when the next important even should take place, and
// the frontend constantly runs update on the entire list of backends. The backend's
// update function should check if the important time has come, and then it sends
// data appropriately, whether it's bit-wise or byte-wise.
// Cons: constantly polling

// Option 2
// Each backend registers the next important time with some sort of background
// scheduler, which then sleeps until the next important time, runs the appropriate
// update function, gets the next important time for that backend, and then sleeps again.
// This could be mixed with the polling method depending on whether running on a
// microcontroller or not. Avoiding constant polling on a PC is important, but
// a microcontroller needs to be doing something constantly anyway.
// Cons: requires a fairly complicated scheduler, could easily be added to libQ though
// Implementation:
// libQ will have a scheduler class. It will be a list of objects of class type
// scheduler_child. backend will inherit from scheduler_child, which will have a
// next_time value and a pure virtual scheduler_update function, which backend will
// overload. After every scheduler_update, scheduler could either insert the new time
// into a sorted list or it could run a minimum function. I believe running minimum 
// would almost always be faster, especially since it is zero-copy. Frontend will own
// the scheduler and will be in charge of running wait_for_next_event. Hopefully, 
// this allows many backends to run simultaneously without the need for threading.

// Option 3
// Each backend keeps track of the next important event, and then the frontend
// simply finds the nearest value and waits until then, via either sleeping the thread
// on a threaded system or polling on a microcontroller.
// Cons: requires recalculating the mininumum every time
// Implementation: 
// When update is run on a device, it updates its internal next_event_time value.
// Frontend then loops through all of the backends and finds which one is the lowest.
// Then, frontend sleeps until that time and then runs that backend's update function.

// Current Winner: Option 3

// Option 3 is basically a version of Option 2 where frontend does the task of the
// scheduler. While normally I would be all for the idea of keeping things doing only
// what they are designed to do and creating a new system to do the extra tasks, I think
// letting frontend be in charge of scheduling would be much simpler.

// But would it? Both options require having a list of objects, each which has a time
// and an update function. The calculation and sleep functions are the same for both.
// It is just a matter of 1. letting frontend do it because creating an entire scheduler
// is a lot of work for something that it actually relatively simple. Creating a scheduler
// class the way I described might also cause some compilation, but I'm not sure on how
// inheritance works. Or 2. Creating a scheduler because reuable code is always better,
// plus if the scheduling does ever get more complicated, frontend isn't trying to bear
// the weight of the extra complication.

// I think would prefer to create a scheduler if there aren't any compilation issues or
// inheritance issues. As I said, reusable code is always better. Segmenting the code
// should also help narrow down issues during debugging, as the whole system isn't just
// nested code inside nested code. 

// This also means backend needs to be completely reworked. The current procedural
// approach no longer works, as each bit event needs to be ran individually. This
// means setting up an internal state machine that runs the send/recv algorithm instead.
// This will also allow for better unexpected header detection. This means that if
// this device gets out of sync with the sending device and starts recieving a header
// when it was expecting more body, this can be accommodated for by simply updating
// the state of the internal state machine to clear the old body and start reading
// the new header. 

// 9/9/23
// The scheduler has been built and it is exactly as simple as I suspected. 

// Turns out, it was far more complicated than I expected. It probably would've been
// much faster and better to just implement it in frontend, but hindsight is 20/20.
// Eventually, I just gave in and used templates, which ultimately made the whole
// thing a million times simpler. Hopefully, I am ready to implement it in frontend...

// Left to do:
// Completely rework backend so that is runs as a state machine, alternating updates
// between performing a send, a recv, and a clear. Frontend update also needs to be
// reworked in the same way. Benchmark update currently is just the main function from
// the threaded approach, which does not work. Ultimately, it doesn't need to do much,
// just facilitate the removing of old backends once they are finished and the adding
// of new backends for the next test. The results screen can happen during the benchmark
// destructor, update just ends the loop. Both update functions need to be performed
// fast, so no blocking operations in either. Both need to just set flags and move on.
// Cycle_timer needs to be reworked to simply change from delaying until the new cycle
// to just returning the time_point that it should occur at. This is important for
// event_timer, as cycle_timer will just determine the time of the next event.
// get_send_data in benchmark also needs to be fixed to actually get new data every
// time, as well as being in need of fixing the _results.push_back statement.

// The first main focus should be to reimplement backend. Once one backend is able to
// send data, just use threads to make sure that two backends can send a recieve. This
// can (for now) just be implemented in main. Once two backends are able to send and
// receive at the same time, then frontend should be worked on. Just make sure benchmark
// tests an arbitrary number of backends an arbitrary number of times at an arbitary
// speed.

// Once benchmark is working again, it's time to implement the rest of backend. Right
// now, backend only supports sending a receiving from pre-existing devices. However,
// it actually needs to be able to support adding new devices to the mesh, it needs to
// support emergency messages, it needs to support checksums, and it needs to support
// keeping track of device IDs and Queue Positions so that it can translate between 
// the two.

// Once that is done, it is time for more frontend work. A frontend built using Qt6
// and QML is the plan, but it may be easier at first to implement something using
// ncurses. Regardless, once that is in place, it might be time to develop some new
// frontends and backends. First, a frontend/backend combo is needed to send all data
// over either UART or I2C. Then, FINALLY, work can be started on a microcontroller
// real physical implementation. The first physical implementation will just use DIO
// to send the data as fast as possible that way.

// Interjection: At this point, it may also be a good time to finalize a software
// protocol. As of right now, its been basically decided that it's going to be XML,
// but no format or standard has been decided on. This will warrant many hours of
// headaches and discussions with myself and with other people. 

// Once the DIO implementation is fully complete, it could finally be time to build
// REAL hardware. Like, using an FPGA, take USB commands and convert them into mesh
// commands and back. Using dedicated hardware, send data over a wire (or wirelessly).
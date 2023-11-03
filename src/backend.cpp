#include "backend.h"

extern void FrontendPing();

backend::backend(cfifo *wfifoptr, cfifo *rfifoptr)
{
    _write_fifo = wfifoptr;
    _read_fifo = rfifoptr;

    MsgQueueCount = 0;
    CurrentCycle = 0;

    BackendInit();
}

void backend::ClockTick()
{
    //WritingBody = true; // Force condition temporarily
    switch (_state)
    {
    case PollingForStartBit:
        // This wont be quite as simple as it seems. As the start bit could be multiple clock ticks,
        // this needs to account for that, as to not write a start bit halfway through reading one.
        // This doesn't check whether to send the message as NewDev or Emergency, for now this
        // will be simply omitted, since no way to do indicate that is implemented yet anyway.
        if ( MsgQueueCount )
        {
            WriteStartBit();

            HeaderValue = QueuePos;
            CurrentCycle = 8 * 2;
            _state = WritingHeader;
        }
        else
        {
            // Something of the sort
            //if ( ReadStartBit() ) // If the start bit is 1
            {
                HeaderValue = 0;
                CurrentCycle = 8 * 2;
                _state = ReadingHeader;
            }
        }
        break;
    case WritingEmergencyHeader:
        if ( CurrentCycle )
        {
            HeaderValue = 0xFF;
            WriteHeaderBit();
        }
        else
        {
            CurrentCycle = 8 * 2;
            _state = WritingHeader;
        }
        break;
    case WritingNewDeviceHeader:
        if ( CurrentCycle )
        {
            ReadHeaderBit();
        }
        else
        {
            if ( HeaderValue )
            {
                _state = ReadingBodyLength;
            }
            else
            {
                // This is the part where it sends the Serial ID, and code for this doesn't exist yet
                // It might just jump to WritingHeader with a higher initial CurrentCycle, but after
                // that I don't know it would know to send the New Device Information instead of a body
                // CurrentCycle = 8 * HeaderCycleCount;
                //_state = ; 
            }
        }
        break;
    case WritingHeader:
        if ( CurrentCycle )
        {
            uint OldHeaderValue = HeaderValue;
            WriteHeaderBit();
            ReadHeaderBit();
            if ( HeaderValue > OldHeaderValue )
            {
                _state = ReadingHeader;
            }
        }
        else
        {
            CurrentCycle = 12;
            _state = WritingBodyLength;
        }
        break;
    case ReadingHeader:
        if ( CurrentCycle )
        {
            ReadHeaderBit();
        }
        else
        {
            CurrentCycle = 12;
            _state = ReadingBodyLength;
        }
        break;
    case WritingBodyLength:
        if ( CurrentCycle )
        {
            WriteBodyLength();
        }
        else
        {
            CurrentCycle = BodyLength;
            _state = WritingBodyContent;
        }
        break;
    case ReadingBodyLength:
        if ( CurrentCycle )
        {
            ReadBodyLength();
        }
        else
        {
            CurrentCycle = BodyLength;
            _state = ReadingBodyContent;
        }
        break;
    case WritingBodyContent:
        if ( CurrentCycle )
        {
            WriteBodyByte();
        }
        else
        {
            // Setup for next message
        }
        break;
    case ReadingBodyContent:
        if ( CurrentCycle )
        {
            ReadBodyByte();
        }
        else
        {
            FrontendPing();
        }
        break;
    }
}

void backend::QueueMessage()
{
    ++MsgQueueCount;
}

cfifo* backend::GetReadFifo()
{
    return _read_fifo;
}

cfifo* backend::GetWriteFifo()
{
    return _write_fifo;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*
void mesh_backend::run_cycle()
{
    _ct->new_cycle();
    if ( !to_send.empty() )
    {
        if ( wire_send_header() )
        {
            wire_send_body( to_send.c_str(), to_send.length() );
            to_send.erase();
        }
    }
    else
    {
        uint8_t value = wire_recv_bit();

        if ( value == 255 )
        {
            wire_recv();
        }
    }
}

void mesh_backend::wire_recv(int8_t QueueBit, uint16_t inQueuePosition)
{
    log << "Attempting to receive some data, QueueBit = " << int(QueueBit) << libQ::loglevel::NOTEDEBUG;

    for ( ; QueueBit >= 0; --QueueBit )
    {
        _ct->new_cycle();
        bool inbit = wire_recv_bit();
        inQueuePosition = (inQueuePosition)|(inbit)<<QueueBit;
    }

    log << "Receiving data from Queue Position " << inQueuePosition << libQ::loglevel::NOTEDEBUG;

    uint32_t recvlen = 0;
    wire_recv_byte( &((char*)(&recvlen))[1] );
    wire_recv_byte( &((char*)(&recvlen))[0] );

    log << "Receiving " << std::bitset<16>(recvlen).to_string() << " bytes of data" << libQ::loglevel::NOTEDEBUG;

    char buffer[recvlen];
    for (uint32_t i = 0; i < recvlen; ++i)
        wire_recv_byte( &buffer[i] );
    recv_callback(buffer,recvlen,inQueuePosition); // Replace recv_QueuePosition with ID once adding new devices is implemented
    ++_QueuePosition;
}

bool mesh_backend::wire_send_header(header_types type)
{
    int8_t QueueBit = 7;
    uint8_t inQueuePosition = 0;
    
    wire_send_bit(255);
    wire_recv_bit();
    wire_clear_bit();

    for ( ; QueueBit >= 0; --QueueBit )
    {
        bool value = _QueuePosition & 0b1<<QueueBit;
        _ct->new_cycle();
        wire_send_bit( value );
        //log << value << std::flush;
        bool inbit = wire_recv_bit();
        inQueuePosition = (inQueuePosition)|(inbit)<<QueueBit;
        wire_clear_bit();
        if ( inbit && !value )
        {
            log << "Bit conflict! - expected " << value << " but got " << inbit << " at QueueBit " << int(QueueBit) << libQ::loglevel::NOTEDEBUG;
            wire_recv(QueueBit - 1, inQueuePosition);
            return false;
        }
    }

    return true;
}

void mesh_backend::wire_send_body(const char *raw, uint16_t msglen)
{
    wire_send_byte( ((char*)(&msglen))[1] );
    wire_send_byte( ((char*)(&msglen))[0] );

    log << "Sending " << std::bitset<16>(msglen).to_string() << " bytes of data" << libQ::loglevel::NOTEDEBUG;

    for (uint32_t i = 0; i < msglen; ++i)
        wire_send_byte(raw[i]);

    _QueuePosition = 1;
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
}*/
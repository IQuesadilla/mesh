#include "backend.h"

#include "avr/generic.h"

backend *_this_backend;

void backend::BackendInit()
{
    _this_backend = this;

    DDRB = 0b10000111;
	PORTB = 0b00000000;
	DDRD = 0b00000000;
	PORTD = 0b00000000; 

	switch ( (PIND>>5) & 0b11 )
	{
		case 0b00:
			SerialID = 0b01010101;
		break;
		case 0b01:
			SerialID = 0b00110011;
		break;
		case 0b10:
			SerialID = 0b00001111;
		break;
	}

    setBits(EICRA, 0b00000011);
	setBits(EIMSK, 0b00000001);
}

void backend::WriteStartBit()
{
    ;
}

void backend::WriteHeaderBit()
{
    setBits(PORTB, 0b00000100);

    if ( CurrentCycle & 0b1 )
        return;

    if ( HeaderValue>>((CurrentCycle>>1)-1) & 0b1 )
    {
        setBits(PORTB, 0b00000011);
    }
    else
    {
        resetBits(PORTB, 0b00000011);
    }
}

void backend::ReadHeaderBit()
{
    resetBits(PORTB, 0b00000100);

    if ( !( CurrentCycle & 0b1) )
        return;

    if (PIND & 0b10000000)
    { 
        setBits(PORTB, 0b10000000);
        setBits( HeaderValue, 0b1<<(CurrentCycle>>1) );
    }
    else
    {
        resetBits(PORTB, 0b10000000);
        resetBits( HeaderValue, 0b1<<(CurrentCycle>>1) );
    }
}

uint CurrentBodyBit;
char BodyByteBuffer;

void backend::WriteBodyByte()
{
    if ( CurrentCycle == BodyLength )
    {
        _write_fifo->getch(&BodyByteBuffer);
        CurrentBodyBit = 8;
    }
    else if ( !CurrentBodyBit )
    {
        --CurrentCycle;
        CurrentBodyBit = 8;
        if ( CurrentCycle )
            _write_fifo->getch(&BodyByteBuffer);
    }

    if ( !(CurrentBodyBit & 0b1) )
    {
        setBits(PORTB, 0b00000100);

        if ( (BodyByteBuffer>>(CurrentBodyBit>>1)) & 0b1 )
        {
            setBits(PORTB, 0b00000011);
        }
        else
        {
            resetBits(PORTB, 0b00000011);
        }
    }
    else resetBits(PORTB, 0b00000100);
}

void backend::ReadBodyByte() // Incomplete
{
    if ( !CurrentBodyBit )
    {
        --CurrentCycle;
        CurrentBodyBit = 8;
    }

    if ( !(CurrentBodyBit & 0b1) ) setBits(PORTB, 0b00000100);
    else
    {
        if (PIND & 0b10000000)
        { 
            setBits(PORTB, 0b10000000);
            BodyByteBuffer;
        }
        else
        {
            resetBits(PORTB, 0b10000000);
            BodyByteBuffer;
        }
    }
}

void backend::ClearOutput()
{
    resetBits(PORTB, 0b00000111);
}

EXTERNAL void INT rising_interrupt()
{
	_this_backend->ClockTick();
}

EXTERNAL void INT timer_interrupt()
{
	return;
}

void BackendPing()
{
    _this_backend->QueueMessage();
}
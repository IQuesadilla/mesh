#ifndef MESH_BACKEND
#define MESH_BACKEND

#include "libQ.h"
#include "cfifo.h"

class backend
{
public:
    backend(cfifo *wfifoptr, cfifo *rfifoptr);

    // Universal Functions
    void ClockTick();

    void QueueMessage();

    cfifo* GetReadFifo();
    cfifo* GetWriteFifo();

private:
    struct {
        uint ID;
        uint QueuePos;
    } DeviceList[100];

    enum CurrentState
    {
        PollingForStartBit,
        WritingEmergencyHeader,
        WritingNewDeviceHeader,
        WritingHeader,
        ReadingHeader,
        WritingBodyLength,
        ReadingBodyLength,
        WritingBodyContent,
        ReadingBodyContent,
    } _state;

    // Specific Functions
    void BackendInit();
    void WriteStartBit();

    /// @brief Writes a single Header Bit to the line
    void WriteHeaderBit();
    /// @brief Reads a single Header Bit from the line
    void ReadHeaderBit();

    void WriteBodyByte();
    void ReadBodyByte();

    void WriteBodyLength();
    void ReadBodyLength();

    void ClearOutput();

    // Variables
    uint DeviceCount;
    uint SerialID;
    uint QueuePos;

    uint MsgQueueCount;
    uint CurrentCycle;
    // For Read/Write HeaderBit, CycleCount is the number of cycles for the entire header section
    //     AKA, CycleCount = BitsInSection * HeaderCycleCount - 1
    // For Read/Write BodyByte, CycleCount is the number of cycles for a single byte
    //     AKA, CycleCount = BodyCycleCount - 1

    uint HeaderValue;
    uint BodyLength;

    cfifo *_write_fifo;
    cfifo *_read_fifo;
    void *_usrptr;
};

#endif
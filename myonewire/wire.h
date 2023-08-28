#include <iostream>
#include <thread>
#include <fstream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>

#define DELAYMS 50
#define BUFFSIZE 10
#define SHMKEY 0x1234

class owm
{
public:
    owm()
    {
        int sid;

        sid = shmget( SHMKEY, 1024, 0644|IPC_CREAT );

        std::cout << "errno: " << errno << std::endl;
        errno = 0;

        shmptr = (uint8_t*)shmat( sid, NULL, 0 );

        std::cout << "errno: " << errno << std::endl;

        isRecieving = false;
        isSending = false;
        msgcount = 1;
    }

    ~owm() {}

    void sendmsg()
    {
        while ( isRecieving ) 
            std::this_thread::sleep_for( std::chrono::milliseconds(2) );

        isSending = true;
        if ( wire_send( msgcount ) )
        {
            std::cout << "FAILED! Now looping..." << std::endl;
            sendmsg();
            return;
        }

        int len = BUFFSIZE;
        while ( len == BUFFSIZE )
        {
            char buff[BUFFSIZE];
            len = send_callback(buff, BUFFSIZE);
            
            for (int i = 0; i < len; ++i)
                wire_send( buff[i] );
        }

        wire_send(3);

        msgcount = 1;
        isSending = false;
    }

    int recvmsg()
    {
        char cmsgid = 0;
        wire_recv( &cmsgid );
        int msgid = cmsgid;

        if ( isSending ) return -1;
        isRecieving = true;

        int i;
        do
        {
            char buffer[BUFFSIZE];
            for ( i = 0; i < BUFFSIZE; ++i )
            {
                char value = 0;
                wire_recv( &value );

                if ( value == 3 ) break;
                else buffer[i] = value;
            }

            recv_callback(buffer, i);
        } while ( i == BUFFSIZE );

        ++msgcount;
        isRecieving = false;
        return msgid;
    }

    virtual int send_callback( char *data, size_t len ) = 0;
    virtual void recv_callback( char *data, size_t len ) = 0;

    bool isRecieving;
    bool isSending;
    int msgcount;

private:
    uint8_t *shmptr;

    void wait_for_next_send()
    {
        using namespace std::chrono;

        auto now = system_clock::now();
        auto since_epoch = duration_cast<milliseconds>(now.time_since_epoch());

        // Calculate the next tenth of a second mark
        auto next_tenth_of_second = ((since_epoch / milliseconds(DELAYMS) + 1) * milliseconds(DELAYMS)).count();

        auto next_time_point = system_clock::time_point(milliseconds(next_tenth_of_second));

        std::this_thread::sleep_until(next_time_point);
    }

    void wait_for_next_recv()
    {
        using namespace std::chrono;

        auto now = system_clock::now();
        auto since_epoch = duration_cast<milliseconds>(now.time_since_epoch());

        // Calculate the next tenth of a second mark
        auto next_tenth_of_second = ((since_epoch / milliseconds(DELAYMS) + 1) * milliseconds(DELAYMS)).count();

        auto next_time_point = system_clock::time_point(milliseconds(next_tenth_of_second) + milliseconds(DELAYMS / 3));

        std::this_thread::sleep_until(next_time_point);
    }

    void wait_for_next_clear()
    {
        using namespace std::chrono;

        auto now = system_clock::now();
        auto since_epoch = duration_cast<milliseconds>(now.time_since_epoch());

        // Calculate the current tenth of a second mark
        auto next_tenth_of_second = ((since_epoch / milliseconds(DELAYMS)) * milliseconds(DELAYMS)).count();

        auto next_time_point = system_clock::time_point(milliseconds(next_tenth_of_second) + ( milliseconds(DELAYMS / 3) * 2 ));

        std::this_thread::sleep_until(next_time_point);
    }

    void wire_clear_bit()
    {
        wait_for_next_clear();
        *shmptr = 0;
        return;
    }

    bool wire_send_bit(bool bit)
    {
        wire_clear_bit();
        wait_for_next_send();

        //if ( bit )
        //{
        //    if ( !shmptr )
                *shmptr = ( (bit) ? 1 : 0 );
        //}

        return ( *shmptr > 0 ) != bit;
    }

    bool wire_recv_bit()
    {
        wait_for_next_recv();
        return *shmptr;
    }

    bool wire_send(uint8_t byte)
    {
        wire_send_bit(1);
        //std::cout << "Send [";
        
        for (int i = 7; i >= 0; --i)
        {
            bool value = byte & 0b1<<i;
            if ( wire_send_bit( value ) )
            {
                //std::cout << "#]" << std::endl;
                return true;
            }

            //std::cout << value;
        }

        wire_send_bit(0);
        //std::cout << "]\n";

        return false;
    }

    void wire_recv(char *byte)
    {
        while ( !wire_recv_bit() );
        if ( isSending ) return;

        //std::cout << "Recv [";
        for (int i = 7; i >= 0; --i)
        {
            bool value = wire_recv_bit();

            //std::cout << value;

            *byte = (*byte)|(value)<<i;
        }

        //wire_clear_bit();

        //std::cout << "]\n";
    }

};
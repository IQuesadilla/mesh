#include "wire.h"
#include <string>
#include <thread>
#include <cstring>

class myowm : public owm
{
public:
    int send_callback( char *data, size_t len )
    {
        int ret_len = send_buffer.length();
        if ( ret_len > len ) ret_len = len;

        memcpy (data, send_buffer.c_str(), ret_len);
        send_buffer.erase( 0, ret_len );

        return ret_len; 
    }

    void recv_callback( char *data, size_t len )
    {
        recv_buffer.append( data, len );
    }

    std::string send_buffer;
    std::string recv_buffer;
};

void server_thread(myowm *mesh)
{
    while (1)
    {
        std::cout << "MSGID: " << mesh->msgcount << ", Enter a value: " << std::flush;
        std::getline( std::cin, mesh->send_buffer );

        mesh->sendmsg();
    }
}

void client_thread(myowm *mesh)
{
    while (1)
    {
        int msgid = mesh->recvmsg();
        if ( msgid < 0 ) continue;

        std::cout << "MSGID: " << msgid << ", Recived line: " << mesh->recv_buffer << "\nMSGID: " << mesh->msgcount << ", Enter a value: " << std::flush;
        mesh->recv_buffer = "";
    }
}

int main( int argc, char *argv[] )
{
    myowm mesh;
    
    std::thread server (server_thread, &mesh);
    std::thread client (client_thread, &mesh);

    server.join();
    client.join();

    return 0;
}

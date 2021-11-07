#include "mesh.h"
#include <SDL2/SDL.h>

#define CLIENTNAME "audioclient"
#define SERVERNAME "audioserver"
#define MAX_RECORDING_DEVICES 10

void audioPlaybackCallback(void* userdata, Uint8* stream, int len)
{
    auto tempmesh = ((mesh*)userdata);

    //std::string buffer ((char*)stream,len);
    mesh::message messagebuffer = tempmesh->receiveUDP();
    if (messagebuffer.data.size() <= 0)
        return;
    if (messagebuffer.data.size() < len) len = messagebuffer.data.size();

    for (int i = 0; i < len; ++i)
        if(messagebuffer.data[i] == '-' || messagebuffer.data[i] == '<')
            {messagebuffer.data.erase(messagebuffer.data.begin() + i); --len;}

    //char *tempbuffer = (char*)malloc(len+1);
    //SDL_memset(tempbuffer, '\0', len+1);
    //SDL_memset(&tempbuffer[len/2], 255, len/2);
    SDL_memcpy(stream, messagebuffer.data.data(), len);
    //std::cout << "Buffered value: " << tempbuffer << std::endl;
    //SDL_MixAudioFormat(stream, messagebuffer.data.data(), AUDIO_U8, len, SDL_MIX_MAXVOLUME*0.75f);
    //((mesh*)userdata)->dumbshit.erase(((mesh*)userdata)->dumbshit.begin(),((mesh*)userdata)->dumbshit.begin()+len);
    //std::cout << len << ' ' << std::flush;
    //memset(0,0,10000);
    return;
}

int initaudio(mesh *tempmesh)
{
    SDL_Init(SDL_INIT_AUDIO);

    //Get capture device count
    int gRecordingDeviceCount = SDL_GetNumAudioDevices( SDL_FALSE );

    //No recording devices
    if( gRecordingDeviceCount < 1 )
    {
        printf( "Unable to get audio capture device! SDL Error: %s\n", SDL_GetError() );
        return -1;
    }

    //Number of available devices

    //Recieved audio spec
    SDL_AudioSpec gReceivedRecordingSpec;
    SDL_AudioDeviceID recordingDeviceId;

    //Cap recording device count
    if( gRecordingDeviceCount > MAX_RECORDING_DEVICES )
    {
        gRecordingDeviceCount = MAX_RECORDING_DEVICES;
    }

    //Render device names
    int index;
    std::stringstream promptText;
    for( int i = 0; i < gRecordingDeviceCount; ++i )
    {
        //Get capture device name
        promptText.str( "" );
        promptText << i << ": " << SDL_GetAudioDeviceName( i, SDL_FALSE );

        std::cout << promptText.str().c_str() << std::endl;
    }
    std::cin >> index;

    if( index < gRecordingDeviceCount )
    {
        //Default audio spec
        SDL_AudioSpec desiredRecordingSpec;
        SDL_zero(desiredRecordingSpec);
        desiredRecordingSpec.freq = 28160;
        desiredRecordingSpec.format = AUDIO_U8;
        desiredRecordingSpec.channels = 1;
        desiredRecordingSpec.samples = 64;
        desiredRecordingSpec.callback = audioPlaybackCallback;
        desiredRecordingSpec.userdata = (void*)tempmesh;

        //Open recording device
        recordingDeviceId = SDL_OpenAudioDevice( SDL_GetAudioDeviceName( index, SDL_FALSE ), SDL_FALSE, &desiredRecordingSpec, &gReceivedRecordingSpec, 0 );
    }
    return recordingDeviceId;
}

int main (int argc, char **argv)
{
    mesh *temp = new mesh();
    SDL_AudioDeviceID audioid = initaudio(temp);
    std::cout << audioid << std::endl;
    temp->initserver(CLIENTNAME);
    std::cout << "Server initialized" << std::endl;

    SDL_UnlockAudioDevice(audioid);
    std::cout << "Audio device locked" << std::endl;
    SDL_PauseAudioDevice(audioid,SDL_FALSE);
    std::cout << "Audio device unpaused" << std::endl;
    while (temp->isConnected())
    {
        //SDL_PauseAudioDevice(audioid, SDL_TRUE);
        //SDL_LockAudioDevice(audioid);
        //auto tempdumbshit = temp->receiveUDP();
        //char *tempstring = (char*)malloc (tempdumbshit.data.size()+1);
        //SDL_memset (tempstring,'\0',tempdumbshit.data.size()+1);
        //SDL_memcpy(tempstring,tempdumbshit.data.data(),tempdumbshit.data.size());
        //std::cout << "True Value: " << tempstring << std::endl;
        //temp->dumbshit.insert(temp->dumbshit.end(),tempdumbshit.data.begin(),tempdumbshit.data.end());
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        //SDL_UnlockAudioDevice(audioid);
        //SDL_PauseAudioDevice(audioid, SDL_FALSE);
    }
    std::cout << "Done loopung" << std::endl;

    temp->killserver();
    delete temp;
}

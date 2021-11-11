#include "mesh.h"
#include <SDL2/SDL.h>

#define CLIENTNAME "audioclient"
#define SERVERNAME "audioserver"
#define MAX_RECORDING_DEVICES 10

void audioRecordingCallback(void* userdata, Uint8* stream, int len)
{
    std::vector<uint8_t> buffer;
    uint8_t *tempstream = stream;//(uint8_t*)malloc(len);
    //SDL_MixAudioFormat(tempstream, stream, AUDIO_U8, len, SDL_MIX_MAXVOLUME*0.75f);
    //sprintf(tempstream,"PENIS!");
    buffer.insert(buffer.end(),&tempstream[0],&tempstream[len]);
    int count = ((mesh*)userdata)->sendUDP({CLIENTNAME,buffer});
    if (count != -1)
        std::cout << count << ' ' << std::flush;
    //free(tempstream);
    //memset(0,0,10000);
    return;
}

int initaudio(mesh *tempmesh)
{
    SDL_Init(SDL_INIT_AUDIO);

    //Get capture device count
    int gRecordingDeviceCount = SDL_GetNumAudioDevices( SDL_TRUE );

    //No recording devices
    if( gRecordingDeviceCount < 1 )
    {
        printf( "Unable to get audio capture device! SDL Error: %s\n", SDL_GetError() );
        return -1;
    }

    //Number of available devices

    //Recieved audio spec
    SDL_AudioSpec gReceivedRecordingSpec;
    SDL_AudioSpec gReceivedPlaybackSpec;
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
        promptText << i << ": " << SDL_GetAudioDeviceName( i, SDL_TRUE );

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
        desiredRecordingSpec.callback = audioRecordingCallback;
        desiredRecordingSpec.userdata = (void*)tempmesh;

	std::cout << "Requested Spec: " << std::endl\
		<< "    Freq: " << desiredRecordingSpec.freq << std::endl\
		<< "    Format: " << desiredRecordingSpec.format << std::endl\
		<< "    Channels: " << desiredRecordingSpec.channels << std::endl\
		<< "    Samples: " << desiredRecordingSpec.samples << std::endl;

        //Open recording device
        recordingDeviceId = SDL_OpenAudioDevice( SDL_GetAudioDeviceName( index, SDL_TRUE ), SDL_TRUE, &desiredRecordingSpec, &gReceivedRecordingSpec, 0 );

	std::cout << "Recieved Spec: " << std::endl\
		<< "    Freq: " << gReceivedRecordingSpec.freq << std::endl\
		<< "    Format: " << gReceivedRecordingSpec.format << std::endl\
		<< "    Channels: " << gReceivedRecordingSpec.channels << std::endl\
		<< "    Samples: " << gReceivedRecordingSpec.samples << std::endl;
    }
    return recordingDeviceId;
}

int main (int argc, char **argv)
{
    mesh *temp = new mesh();
    SDL_AudioDeviceID audioid = initaudio(temp);
    std::cout << audioid << std::endl;
    temp->initserver(SERVERNAME);
    std::cout << "Server initialized" << std::endl;

    SDL_UnlockAudioDevice(audioid);
    std::cout << "Audio device locked" << std::endl;
    SDL_PauseAudioDevice(audioid,SDL_FALSE);
    std::cout << "Audio device unpaused" << std::endl;
    while (temp->isConnected())
    {
        //SDL_PauseAudioDevice(audioid, SDL_TRUE);
        //SDL_LockAudioDevice(audioid);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        //SDL_UnlockAudioDevice(audioid);
        //SDL_PauseAudioDevice(audioid, SDL_FALSE);
    }
    std::cout << "Done loopung" << std::endl;

    temp->killserver();
    delete temp;
}

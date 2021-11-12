#include "mesh.h"
#include <SDL2/SDL.h>

#define CLIENTNAME "audioclient"
#define SERVERNAME "audioserver"
#define MAX_RECORDING_DEVICES 10

void audioPlaybackCallback(void* userdata, Uint8* stream, int len)
{
    auto tempmesh = ((mesh*)userdata);

    if (tempmesh->dumbshit.size() == 0)
    {
        SDL_memset(stream,'\0',len);
    }

    //std::string buffer ((char*)stream,len);
    if (tempmesh->dumbshit.size() < len)
    {
        len = tempmesh->dumbshit.size();
        //std::cout << "Is the server sending enough DATA?!?!?" << std::endl;
    }

    //char *tempbuffer = (char*)malloc(len+1);
    //SDL_memset(tempbuffer, '\0', len+1);
    //SDL_memset(&tempbuffer[len/2], 255, len/2);
    tempmesh->dumbshitsafety.lock();
    SDL_memcpy(stream, tempmesh->dumbshit.data(), len);
    tempmesh->dumbshit.erase(tempmesh->dumbshit.begin(),tempmesh->dumbshit.begin()+len);
    //std::cout << tempmesh->dumbshit.size() << std::endl;

    //if (tempmesh->cocksafety == 1000)
    //{
    //if (tempmesh->dumbshit.size() > 512)
    //    tempmesh->dumbshit.erase(tempmesh->dumbshit.begin(),tempmesh->dumbshit.end()-512);
    //    tempmesh->cocksafety = 0;
    //}
    //else ++tempmesh->cocksafety;
    tempmesh->dumbshitsafety.unlock();
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
        desiredRecordingSpec.freq = AFREQ;
        desiredRecordingSpec.format = AFORMAT;
        desiredRecordingSpec.channels = ACHANNELS;
        desiredRecordingSpec.samples = ASAMPLES;
        desiredRecordingSpec.callback = audioPlaybackCallback;
        desiredRecordingSpec.userdata = (void*)tempmesh;

	std::cout << "Requested Spec: " << std::endl\
		<< "    Freq: " << desiredRecordingSpec.freq << std::endl\
		<< "    Format: " << desiredRecordingSpec.format << std::endl\
		<< "    Channels: " << desiredRecordingSpec.channels << std::endl\
		<< "    Samples: " << desiredRecordingSpec.samples << std::endl;

        //Open recording device
        recordingDeviceId = SDL_OpenAudioDevice( SDL_GetAudioDeviceName( index, SDL_FALSE ), SDL_FALSE, &desiredRecordingSpec, &gReceivedRecordingSpec, 0 );

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
    temp->initserver(CLIENTNAME);
    std::cout << "Server initialized" << std::endl;

    SDL_UnlockAudioDevice(audioid);
    std::cout << "Audio device locked" << std::endl;
    SDL_PauseAudioDevice(audioid,SDL_FALSE);
    std::cout << "Audio device unpaused" << std::endl;
    while (temp->isConnected())
    {
        //std::this_thread::sleep_for(std::chrono::milliseconds(50));
        //SDL_PauseAudioDevice(audioid, SDL_TRUE);
        //SDL_LockAudioDevice(audioid);
        //auto tempdumbshit = temp->receiveUDP();
        //char *tempstring = (char*)malloc (tempdumbshit.data.size()+1);
        //SDL_memset (tempstring,'\0',tempdumbshit.data.size()+1);
        //SDL_memcpy(tempstring,tempdumbshit.data.data(),tempdumbshit.data.size());
        //std::cout << "True Value: " << tempstring << std::endl;
        //temp->dumbshit.insert(temp->dumbshit.end(),tempdumbshit.data.begin(),tempdumbshit.data.end());
        mesh::message messagebuffer = temp->receiveUDP();
        if (messagebuffer.name.length() == 0)
        {
            //std::cout << "No data recieved" << std::endl;
            //std::this_thread::sleep_for(std::chrono::microseconds(10));
            continue;
        }
        std::this_thread::sleep_for(std::chrono::microseconds(50));

        std::vector<int8_t> prefix {'-','-','<'};
        for (int i = 0; i < messagebuffer.data->size()-3; ++i)
            if (std::equal(prefix.begin(),prefix.end(),messagebuffer.data->begin()+i))//(messagebuffer.data->at(i) == '-' || messagebuffer.data->at(i) == '<')
                messagebuffer.data->erase(messagebuffer.data->begin() + i,messagebuffer.data->begin() + i + 3);
        temp->dumbshitsafety.lock();
        temp->dumbshit.insert(temp->dumbshit.end(),messagebuffer.data->begin(),messagebuffer.data->end());
        temp->dumbshitsafety.unlock();
        messagebuffer.data.reset();
        //SDL_UnlockAudioDevice(audioid);
        //SDL_PauseAudioDevice(audioid, SDL_FALSE);
    }
    std::cout << "Done loopung" << std::endl;

    temp->killserver();
    delete temp;
}

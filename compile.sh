#!/bin/sh

g++ -o ./bin/audio_server audio_server.cpp mesh.cpp -lSDL2 -lpthread -std=c++17
g++ -o ./bin/audio_client audio_client.cpp mesh.cpp -lSDL2 -lpthread -std=c++17

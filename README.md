# Mesh

## Synopsis
Mesh is a protocol designed to accomodate communication between devices where each device is of equal status and there is no central hub.
Data flow is handled automatically with a Queue Position and requiring all data to be OR'd together during transmission, so that a value of 1 from any device overrides any 0's. 
All communication is sent to all devices connected to the Mesh, however methods of encryption can be implemented.
For more information, read Concept.pdf included in the documentation folder.

## TODO
- [ ] Remove dependencies on logcpp and tinyxml2 and replace with libQ
- [ ] Mix meshfs and meshfs fuse into one file
- [ ] Fix the Makefile and the include paths
- [ ] Turn ip, udp, and tcp into one file
- [ ] Design a backend agnostic C++ interface
- [ ] Turn netmesh into mesh and make it backend agnostic
- [ ] Turn the ip file into a proper backend
- [ ] Create a wiringPi backend
- [x] Implement shm as a backend
- [ ] Update README with up-to-date information
- [ ] "make" command currently fails since no core application has been decided upon

Rework the entire structure so that if connecting to a microcontroller over serial for example, the microcontroller has a serial frontend and a DIO backend, while the computer has any other frontend and a serial backend. The whole project would get compiled twice, once for the microcontroller and once for the computer, but this allows for easy mixing of frontends and backends. This is called a bridged configuration. Plus, this allows setting up different kinds of frontends for microcontrollers, such as sensors and whatnot, allowing the microcontroller to act completely independently. It also allows for dedicated hardware to fall right into the place of the microcontroller, meaning the computer can operate as it always has been regardless of what is connected to the other end of the serial line. It also doesn't need to be serial, for example an I2C backend would be nice as well.

The backend should manage the QueuePosition to DeviceID conversion, completely out of sight of the frontend. All that the frontend should care about is DeviceIDs. However, XML parsing should happen in the frontend, and communicate any configuration back to the backend. In the case of a bridged configuration, in general, the microcontroller backend should manage the conversion.

## TO INSTALL
```
git clone --recurse-submodules https://github.com/IQuesadilla/mesh.git
make -C libQ/
make install -C libQ/
make
```

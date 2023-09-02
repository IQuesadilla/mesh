# Mesh

## Synopsis
Mesh is a protocol designed to accomodate communication between devices where each device is of equal status and there is no central hub.
Data flow is handled automatically with a Queue Position and requiring all data to be OR'd together during transmission, so that a value of 1 from any device overrides any 0's. 
All communication is sent to all devices connected to the Mesh, however methods of encryption can be implemented.
For more information, read Concept.pdf included in the documentation folder.

## TODO
- [ ] Remove dependencies on logcpp and tinyxml2 and replace with libQ
- [ ] Fix the Makefile and the include paths
- [ ] Turn ip, udp, and tcp into one file
- [ ] Design a backend agnostic C++ interface
- [ ] Turn netmesh into mesh and make it backend agnostic
- [ ] Turn the ip file into a proper backend
- [ ] Create a wiringPi backend
- [x] Implement shm as a backend
- [ ] Update README with up-to-date information
- [ ] "make" command currently fails since no core application has been decided upon

## TO INSTALL
```
git clone --recurse-submodules https://github.com/IQuesadilla/mesh.git
make -C libQ/
make install -C libQ/
make
```

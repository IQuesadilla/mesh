CXX = g++
CFLAGS = -std=c++17 -Wall -fPIC -fno-exceptions -O3 -Iinclude/ -IlibQ/include/
OBJFLAGS = -o $@ -c $< $(CFLAGS)

LIBDIR = lib/
SOFLAGS = -o $@ $^ -shared
SHARED = $(LIBDIR)/libnetmesh.so

TBIN = tests/bin/
TESTFLAGS = -o $@ $^ $(CFLAGS) -LlibQ/ -lQ
NETMESHTESTS = $(TBIN)/netmesh_testthreaded $(TBIN)/netmesh_testinterrupt $(TBIN)/netmesh_test_getdevices
MESHFSTESTS = $(TBIN)/meshfs_test1 $(TBIN)/meshfs_test2

.PHONY: all shared tests
all: $(SHARED) $(TESTS)
shared: $(SHARED)
tests: $(TBIN)/shm_test1


#------Tests------
$(TBIN)/udp_test1: tests/ip/test1.cpp ip_ip.o ip_udp.o logcpp/logcpp.o
	$(CXX) $(TESTFLAGS) -lpthread

$(TBIN)/meshfs_test1: tests/meshfs/test1.cpp meshfs.o
	$(CXX) $(TESTFLAGS) 

$(TBIN)/meshfs_test2: tests/meshfs/test2.cpp meshfs.o
	$(CXX) $(TESTFLAGS) -D_FILE_OFFSET_BITS=64 -lpthread -lfuse3

$(TBIN)/netmesh_testthreaded: tests/netmesh/testthreaded.cpp netmesh.o ip_ip.o ip_udp.o logcpp/logcpp.o tinyxml2/tinyxml2.o
	$(CXX) $(TESTFLAGS) -lpthread

$(TBIN)/netmesh_testinterrupt: tests/netmesh/testinterrupt.cpp netmesh.o ip_ip.o ip_udp.o logcpp/logcpp.o tinyxml2/tinyxml2.o
	$(CXX) $(TESTFLAGS) -lrt

$(TBIN)/netmesh_test_getdevices: tests/netmesh/getdevices.cpp netmesh.o ip_ip.o ip_udp.o logcpp/logcpp.o tinyxml2/tinyxml2.o
	$(CXX) $(TESTFLAGS) -lpthread

$(TBIN)/shm_test1: tests/shm/test1.cpp backend_shm.o backend.o
	$(CXX) $(TESTFLAGS) -lpthread


#------Shared Objects------
$(LIBDIR)/libfilemesh.so: filemesh.o netmesh.o meshfs.o
	$(CXX) $(SOFLAGS)

$(LIBDIR)/libmeshfs.so: meshfs.o
	$(CXX) $(SOFLAGS)

$(LIBDIR)/libnetmesh.so: netmesh.o
	$(CXX) $(SOFLAGS)


#------Objects------
filemesh.o: filemesh.cpp filemesh.h
	$(CXX) $(OBJFLAGS)

meshfs.o: meshfs.cpp meshfs.h
	$(CXX) $(OBJFLAGS)

netmesh.o: src/netmesh/netmesh.cpp src/netmesh/netmesh.h
	$(CXX) $(OBJFLAGS)

ip_udp.o: src/ip/udp.cpp src/ip/udp.h src/ip/ip.h
	$(CXX) $(OBJFLAGS)

ip_ip.o: src/ip/ip.cpp src/ip/ip.h
	$(CXX) $(OBJFLAGS)

backend_shm.o: src/backends/shm.cpp include/backends/shm.h
	$(CXX) $(OBJFLAGS)

backend.o: src/backend.cpp include/backend.h
	$(CXX) $(OBJFLAGS)


#------Clean Up------
clean:
	-rm *.o
	-rm $(TBIN)/*
	-rm $(LIBDIR)/*

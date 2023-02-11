CXX = g++
CFLAGS = -std=c++17 -Wall -fPIC -fno-exceptions -O3
OBJFLAGS = -o $@ -c $< $(CFLAGS)
TESTFLAGS = -o ./tests/bin/$@ $^ $(CFLAGS)
SOFLAGS = -o $(LIBDIR)/$@ $^ -shared
SHARED = libnetmesh.so
NETMESHTESTS = netmesh_testthreaded netmesh_testinterrupt netmesh_test_getdevices
MESHFSTESTS = meshfs_test1 meshfs_test2
TESTS = udp_test1 $(NETMESHTESTS) $(MESHFSTESTS)
LIBDIR = ./lib/

all: $(SHARED) $(TESTS)

shared: $(SHARED)

tests: $(TESTS)

#heyo
udp_test1: tests/ip/test1.cpp ip_ip.o ip_udp.o logcpp/liblogcpp.o
	$(CXX) $(TESTFLAGS) -lpthread

meshfs_test1: tests/meshfs/test1.cpp meshfs.o
	$(CXX) $(TESTFLAGS) 

meshfs_test2: tests/meshfs/test2.cpp meshfs.o
	$(CXX) $(TESTFLAGS) -D_FILE_OFFSET_BITS=64 -lpthread -lfuse3

netmesh_testthreaded: tests/netmesh/testthreaded.cpp netmesh.o ip_ip.o ip_udp.o logcpp/liblogcpp.o tinyxml2/tinyxml2.o
	$(CXX) $(TESTFLAGS) -lpthread

netmesh_testinterrupt: tests/netmesh/testinterrupt.cpp netmesh.o ip_ip.o ip_udp.o logcpp/liblogcpp.o tinyxml2/tinyxml2.o
	$(CXX) $(TESTFLAGS) -lrt

netmesh_test_getdevices: tests/netmesh/getdevices.cpp netmesh.o ip_ip.o ip_udp.o logcpp/liblogcpp.o tinyxml2/tinyxml2.o
	$(CXX) $(TESTFLAGS) -lpthread

libfilemesh.so: filemesh.o netmesh.o meshfs.o
	$(CXX) $(SOFLAGS)

filemesh.o: filemesh.cpp filemesh.h
	$(CXX) $(OBJFLAGS)

libmeshfs.so: meshfs.o
	$(CXX) $(SOFLAGS)

meshfs.o: meshfs.cpp meshfs.h
	$(CXX) $(OBJFLAGS)

libnetmesh.so: netmesh.o
	$(CXX) $(SOFLAGS)

netmesh.o: src/netmesh/netmesh.cpp src/netmesh/netmesh.h
	$(CXX) $(OBJFLAGS)

ip_udp.o: src/ip/udp.cpp src/ip/udp.h src/ip/ip.h
	$(CXX) $(OBJFLAGS)

ip_ip.o: src/ip/ip.cpp src/ip/ip.h
	$(CXX) $(OBJFLAGS)

logcpp/liblogcpp.o:
	make -C logcpp

tinyxml2/tinyxml2.o:
	make -C tinyxml2

install: $(TARGET)
	cp $(TARGET) /usr/local/lib 

clean:
	-rm *.o
	-rm tests/bin/*
	-rm $(LIBDIR)/*

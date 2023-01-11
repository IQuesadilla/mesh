CXX = g++
CFLAGS = -o $@ -c $< -std=c++17 -fPIC 
TARGET = libfilemesh.so
TESTS = meshfs_test1 meshfs_test2 udp_test1
LIBDIR = ./lib/
LIBS = -L$(LIBDIR) -lpthread

all: $(TARGET)

tests: $(TESTS)

#heyo
udp_test1: tests/ip/test1.cpp ip_ip.o ip_udp.o
	$(CXX) -o ./tests/bin/$@ $^ -std=c++17 -lpthread

meshfs_test1: tests/meshfs/test1.cpp meshfs.o
	$(CXX) -o ./tests/bin/$@ $^ -std=c++17

meshfs_test2: tests/meshfs/test2.cpp meshfs.o
	$(CXX) -o ./tests/bin/$@ $^ -std=c++17 -D_FILE_OFFSET_BITS=64 -lpthread -lfuse3

netmesh_test1: tests/netmesh/test1.cpp netmesh.o ip_ip.o
	$(CXX) -o ./tests/bin/$@ $^ -std=c++17 -lpthread

netmesh_test_getdevices: tests/netmesh/getdevices.cpp netmesh.o ip_ip.o
	$(CXX) -o ./tests/bin/$@ $^ -std=c++17 -lpthread

libfilemesh.so: filemesh.o netmesh.o meshfs.o
	$(CXX) -o $(LIBDIR)/$@ $^ -shared

filemesh.o: filemesh.cpp filemesh.h
	$(CXX) $(CFLAGS)

libmeshfs.so: meshfs.o
	$(CXX) -o $(LIBDIR)/$@ $< -shared

meshfs.o: meshfs.cpp meshfs.h
	$(CXX) $(CFLAGS)

libnetmesh.so: netmesh.o
	$(CXX) -o $(LIBDIR)/$@ $< -shared

netmesh.o: netmesh.cpp netmesh.h tinyxml2/libtinyxml2.a ip_udp.o ip_ip.o
	$(CXX) $(CFLAGS)

ip_udp.o: src/ip/udp.cpp src/ip/udp.h src/ip/ip.h
	$(CXX) $(CFLAGS)

ip_ip.o: src/ip/ip.cpp src/ip/ip.h
	$(CXX) $(CFLAGS)

interactive: interactive.cpp libmesh.so install
	$(CXX) -o $@ $< $(LIBS)

install: $(TARGET)
	cp $(TARGET) /usr/local/lib 

clean:
	-rm *.o
	-rm tests/bin/*
	-rm $(LIBDIR)/*
	-rm $(TARGET)

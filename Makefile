# This will build all of the listed frontends, each with support for all of the listed backends
BACKENDS_ENABLED = shm
FRONTENDS_ENABLED = benchmark

CXX = g++
AR = ar
CFLAGS = -std=c++17 -Wall -fPIC -fno-exceptions -O3 -Iinclude/ -IlibQ/include/

# Setup -L and -Wl,-rpath to search for files in /usr/local/lib/, ../lib/, and possibly ./lib/

ODIR = obj/
OBJFLAGS = -o $@ -c $< $(CFLAGS)

LIBDIR = lib/
SOFLAGS = -o $@ $^ -shared
ARFLAGS = # Needs to be filled in

TBIN = tests/bin/
BINFLAGS = -o $@ $^ $(CFLAGS) -lQ -lmesh

#NETMESHTESTS = $(TBIN)/netmesh_testthreaded $(TBIN)/netmesh_testinterrupt $(TBIN)/netmesh_test_getdevices
#MESHFSTESTS = $(TBIN)/meshfs_test1 $(TBIN)/meshfs_test2

BACKEND = src/backend.cpp include/backend.h
BACKEND_OBJECTS = $(OBJDIR)/backend.o $(foreach backend, $(BACKENDS_ENABLED), $(ODIR)/backend_$(backend).o)

FRONTEND = src/frontend.cpp include/frontend.h
FRONTEND_OBJECTS = $(OBJDIR)/frontend.o $(foreach frontend, $(FRONTENDS_ENABLED), $(ODIR)/frontend_$(frontend).o)

.PHONY: all tests frontends
all: frontends
tests: 
frontends: $(foreach frontend, $(FRONTENDS_ENABLED), bin/$(frontend))


#------Installing libQ------
lib/libQ.so: libQ/Makefile
	$(MAKE) -C libQ/
	cp libQ/libQ.so lib/

libQ/Makefile:
	git submodule init
	git submodule sync
	git submodule update


#------Frontends------
bin/benchmark: src/main.cpp $(ODIR)/frontend_benchmark.o $(LIBDIR)/libmesh.so
	$(CXX) $(BINFLAGS) -lpthread

#------Tests------
#$(TBIN)/udp_test1: tests/ip/test1.cpp ip_ip.o ip_udp.o logcpp/logcpp.o
#	$(CXX) $(BINFLAGS) -lpthread

#$(TBIN)/meshfs_test1: tests/meshfs/test1.cpp meshfs.o
#	$(CXX) $(BINFLAGS) 

#$(TBIN)/meshfs_test2: tests/meshfs/test2.cpp meshfs.o
#	$(CXX) $(BINFLAGS) -D_FILE_OFFSET_BITS=64 -lpthread -lfuse3

#$(TBIN)/netmesh_testthreaded: tests/netmesh/testthreaded.cpp netmesh.o ip_ip.o ip_udp.o logcpp/logcpp.o tinyxml2/tinyxml2.o
#	$(CXX) $(BINFLAGS) -lpthread

#$(TBIN)/netmesh_testinterrupt: tests/netmesh/testinterrupt.cpp netmesh.o ip_ip.o ip_udp.o logcpp/logcpp.o tinyxml2/tinyxml2.o
#	$(CXX) $(BINFLAGS) -lrt

#$(TBIN)/netmesh_test_getdevices: tests/netmesh/getdevices.cpp netmesh.o ip_ip.o ip_udp.o logcpp/logcpp.o tinyxml2/tinyxml2.o
#	$(CXX) $(BINFLAGS) -lpthread

#$(TBIN)/shm_test1: tests/shm/test1.cpp backend_shm.o backend.o
#	$(CXX) $(BINFLAGS) -lpthread


#------Shared Objects------
#$(LIBDIR)/libfilemesh.so: filemesh.o netmesh.o meshfs.o
#	$(CXX) $(SOFLAGS)

#$(LIBDIR)/libmeshfs.so: meshfs.o
#	$(CXX) $(SOFLAGS)

#$(LIBDIR)/libnetmesh.so: netmesh.o
#	$(CXX) $(SOFLAGS)

$(LIBDIR)/libmesh.so: $(BACKEND_OBJECTS)
	$(CXX) $(SOFLAGS)


#------Archive Objects------
$(LIBDIR)/libmesh.a: $(BACKEND_OBJECTS)
	$(AR) $(ARFLAGS)


#------Frontend Objects------
$(ODIR)/frontend_benchmark.o: src/frontends/benchmark.cpp include/frontends/benchmark.h
	$(CXX) $(OBJFLAGS)

$(ODIR)/frontend.o: $(FRONTEND)
	$(CXX) $(OBJFLAGS)

#$(ODIR)/filemesh.o: filemesh.cpp filemesh.h
#	$(CXX) $(OBJFLAGS)

#$(ODIR)/meshfs.o: meshfs.cpp meshfs.h
#	$(CXX) $(OBJFLAGS)

#$(ODIR)/netmesh.o: src/netmesh/netmesh.cpp src/netmesh/netmesh.h
#	$(CXX) $(OBJFLAGS)

#$(ODIR)/ip_udp.o: src/ip/udp.cpp src/ip/udp.h src/ip/ip.h
#	$(CXX) $(OBJFLAGS)

#$(ODIR)/ip_ip.o: src/ip/ip.cpp src/ip/ip.h
#	$(CXX) $(OBJFLAGS)


#------Backend Objects------
$(ODIR)/backend_shm.o: src/backends/shm.cpp include/backends/shm.h $(BACKEND)
	$(CXX) $(OBJFLAGS)

$(ODIR)/backend.o: $(BACKEND)
	$(CXX) $(OBJFLAGS)


#------Clean Up------
clean:
	-rm $(ODIR)/*
	-rm $(TBIN)/*
	-rm $(LIBDIR)/*

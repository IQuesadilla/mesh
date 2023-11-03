# This will build all of the listed frontends, each with support for all of the listed backends
BACKEND_ENABLED = shm
FRONTEND_ENABLED = benchmark

CXX = g++
AR = ar
CFLAGS = -std=c++17 -Wall -fPIC -fno-exceptions -O3 -Iinclude/ -IlibQ/include/

AVR_CXX=avr-g++
AVR_CFLAGS=-fno-exceptions -ffunction-sections -fdata-sections -Werror -nostartfiles -Wno-misspelled-isr -Os
AVR_LDSCRIPT=-Wl,-T,temp.ld -Wl,-Map,output.map
AVR_LIBQ=-I./libQ/include -I./include

# Setup -L and -Wl,-rpath to search for files in /usr/local/lib/, ../lib/, and possibly ./lib/

BUILDABLETYPES = %.c %.cpp %.s %.S %.o %.a

ODIR = obj/
OBJFLAGS = -o $@ -c $(filter $(BUILDABLETYPES),$^) $(CFLAGS)

LIBDIR = lib/
SOFLAGS = -o $@ $(filter $(BUILDABLETYPES),$^) -shared
ARFLAGS = rcs

LIBS = mesh Q
CLIBS = $(foreach lib, $(LIBS), -l$(lib))

TBIN = tests/bin/
BINFLAGS = -o $@ $(filter $(BUILDABLETYPES),$^) $(CFLAGS) -L$(LIBDIR) $(CLIBS)

LIBQ_VERIFY = .git/modules/libQ/HEAD

#NETMESHTESTS = $(TBIN)/netmesh_testthreaded $(TBIN)/netmesh_testinterrupt $(TBIN)/netmesh_test_getdevices
#MESHFSTESTS = $(TBIN)/meshfs_test1 $(TBIN)/meshfs_test2

BACKEND = include/backend.h $(LIBQ_VERIFY)

FRONTEND = include/frontend.h $(LIBQ_VERIFY)

.PHONY: all tests frontends
all: frontends
tests: 
#frontends: $(foreach frontend, $(FRONTENDS_ENABLED), bin/$(frontend))


#------Installing libQ------
$(LIBDIR)/libQ.so: libQ/lib/libQ.so
	cp $< $@

$(LIBDIR)/libQ.a: libQ/lib/libQ.a
	cp $< $@

libQ/lib/libQ.so: $(LIBQ_VERIFY)
	$(MAKE) -C libQ/ lib/libQ.so

libQ/lib/libQ.a: $(LIBQ_VERIFY)
	$(MAKE) -C libQ/ lib/libQ.a

$(LIBQ_VERIFY):
	git submodule init
	git submodule sync
	git submodule update


#------Frontends------
#bin/benchmark: src/main.cpp $(ODIR)/frontend_benchmark.o $(MAKELIBS)
#	$(CXX) $(BINFLAGS) -lpthread

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
$(LIBDIR)/libmesh.so: $(BACKEND_OBJECTS)
	$(CXX) $(SOFLAGS)

#------Archive Objects------
$(LIBDIR)/libmesh.a: $(BACKEND_OBJECTS)
	$(AR) $(ARFLAGS)


#------Frontend Objects------
$(ODIR)/frontend_benchmark.o: src/frontends/benchmark.cpp $(FRONTEND)
	$(CXX) $(OBJFLAGS)

$(ODIR)/frontend.o: src/frontend.cpp $(FRONTEND)
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
$(ODIR)/backend_shm.o: src/backends/shm.cpp $(BACKEND)
	$(CXX) $(OBJFLAGS)

$(ODIR)/backend.o: src/backend.cpp $(BACKEND)
	$(CXX) $(OBJFLAGS)


#------Clean Up------
clean:
	-rm $(ODIR)/*
	-rm $(TBIN)/*
	-rm $(LIBDIR)/*

bin/main.elf: src/main.cpp src/backend.cpp include/backend.h src/backends/bitbang.cpp src/frontend.cpp include/frontend.h src/frontends/uart.cpp libQ/src/avr/bootloader.c temp.S temp.ld libQ/include/avr/m328p_defines.h libQ/include/avr/generic.h libQ/include/cfifo.h libQ/src/cfifo.cpp
	$(AVR_CXX) $(AVR_CFLAGS) $(AVR_LDSCRIPT) $(AVR_LIBQ) -mmcu=atmega328p -o $@ src/main.cpp src/backend.cpp src/backends/bitbang.cpp src/frontend.cpp src/frontends/uart.cpp libQ/src/cfifo.cpp libQ/src/avr/bootloader.c temp.S

bin/main.hex: bin/main.elf
	avr-objcopy -O ihex bin/main.elf $@

.PHONY: avrdump
avrdump: bin/main.elf
	{ avr-objdump -m avr5 -z -D bin/main.elf; nm -n bin/main.elf; } | less

.PHONY: avrinstall
avrinstall: bin/main.hex
	avrdude -cft232h -patmega328p -P/dev/ttyUSB0 -P ft0 -Uflash:w:bin/main.hex

bin/uart_shm: src/main.cpp src/backend.cpp include/backend.h src/backends/shm.cpp src/frontend.cpp include/frontend.h src/frontends/uart.cpp libQ/include/cfifo.h libQ/src/cfifo.cpp
	$(CXX) $(CFLAGS) 
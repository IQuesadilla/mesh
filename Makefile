CXX = g++
CFLAGS = -o $@ -c $< -std=c++17 -fPIC 
TARGET = libfilemesh.so
TESTS = meshfs_test1 meshfs_test2
LIBDIR = ./lib/
LIBS = -L$(LIBDIR) -lpthread

all: $(TARGET)

tests: $(TESTS)

meshfs_test1: tests/meshfs/test1.cpp meshfs.o
	$(CXX) -o ./tests/bin/$@ $^ -std=c++17

meshfs_test2: tests/meshfs/test2.cpp meshfs.o
	$(CXX) -o ./tests/bin/$@ $^ -std=c++17 -D_FILE_OFFSET_BITS=64 -lpthread -lfuse

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

netmesh.o: netmesh.cpp netmesh.h tinyxml2/libtinyxml2.a
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

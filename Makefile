CXX = g++
CFLAGS = -o $@ -c $< -std=c++17 -fPIC 
TARGET = libfilemesh.so
TESTS = interactive
LIBS = -L. -lmesh -lpthread

all: $(TARGET)

libfilemesh.so: filemesh.o netmesh.o
	$(CXX) -o $@ $^ -shared

filemesh.o: filemesh.cpp filemesh.h
	$(CXX) $(CFLAGS)

libnetmesh.so: netmesh.o
	$(CXX) -o $@ $< -shared

netmesh.o: netmesh.cpp netmesh.h tinyxml2/libtinyxml2.a
	$(CXX) $(CFLAGS)

interactive: interactive.cpp libmesh.so install
	$(CXX) -o $@ $< $(LIBS)

install: $(TARGET)
	cp $(TARGET) /usr/local/lib 

clean:
	-rm *.o
	-rm $(TARGET)
	-rm $(TESTS)

CXX = g++
CFLAGS = -o $@ -c $< -std=c++17
TARGET = libmesh.so
TESTS = interactive

all: libmesh.so

libmesh.so: mesh.o
	$(CXX) -o $(TARGET) $< -shared

mesh.o: mesh.cpp mesh.h libtinyxml2.a
	$(CXX) $(CFLAGS) -fPIC 

interactive: interactive.cpp libmesh.so
	$(CXX) $(CFLAGS) -lmesh

clean:
	-rm *.o
	-rm $(TARGET)
	-rm $(TESTS)

CXX = g++
CFLAGS = -fPIC -o $@ -c $< -std=c++17
TARGET = libmesh.so

all: mesh.o
	$(CXX) -o $(TARGET) $< -shared

mesh.o: mesh.cpp mesh.h
	$(CXX) $(CFLAGS)

clean:
	-rm *.o
	-rm $(TARGET)

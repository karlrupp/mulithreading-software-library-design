CC=gcc
CFLAGS=-I.
CXX=g++ # GCC 4.8 and higher recommended for C++11 support
CXXFLAGS=-I. --std=c++11   # adjust C++11 flag as needed

DEPS = mylib.h
OBJ = mylib.o

.PHONY: all
all: with_cpp11threads with_openmp with_pthread

%.o: %.c $(DEPS)
	$(CC) -c -o $@ $< $(CFLAGS)


with_cpp11threads: with_cpp11threads.cpp $(OBJ)
	$(CXX) -o $@ $^ $(CXXFLAGS) -pthread

with_openmp: with_openmp.c $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -fopenmp
    #adjust OpenMP flag as needed

with_pthread: with_pthread.c $(OBJ)
	$(CC) -o $@ $^ $(CFLAGS) -pthread

clean:
	rm *.o with_cpp11threads with_openmp with_pthread

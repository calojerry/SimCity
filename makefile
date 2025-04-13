# Setting the compiler
CC = g++

# Compiler flags (warnings enabled, C++17 standard)
CFLAGS = -Wall -std=c++17

# Target to build the executable
all: main

main: main.o simulation.o
	$(CC) $(CFLAGS) -o main main.o simulation.o

main.o: main.cpp simulation.h
	$(CC) $(CFLAGS) -c main.cpp

simulation.o: simulation.cpp simulation.h
	$(CC) $(CFLAGS) -c simulation.cpp

run: main
	./main

clean:
	rm -f *.o main

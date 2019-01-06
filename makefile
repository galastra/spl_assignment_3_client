CFLAGS:=-g -Wall -Weffc++ -std=c++11 -c -Iinclude
LDFLAGS:=-L/usr/lib -lboost_system -pthread

all: BGSclient

BGSclient: bin/Task.o bin/ConnectionHandler.o bin/main.o
	g++ -o bin/BGSclient bin/Task.o bin/ConnectionHandler.o bin/main.o $(LDFLAGS)

bin/Task.o: src/Task.cpp
	g++ $(CFLAGS) -o bin/Task.o src/Task.cpp

bin/ConnectionHandler.o: src/ConnectionHandler.cpp
	g++ $(CFLAGS) -o bin/ConnectionHandler.o src/ConnectionHandler.cpp

bin/main.o: src/main.cpp
	g++ $(CFLAGS) -o bin/main.o src/main.cpp

.PHONY: clean
clean:
	rm -f bin/*

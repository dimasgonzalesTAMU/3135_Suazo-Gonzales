# makefile
all: dataserver client

semaphore.o: semaphore.h semaphore.cpp
	g++ -std=c++11 -w -c -g semaphore.cpp -pthread

reqchannel.o: reqchannel.h reqchannel.cpp
	g++ -std=c++11 -w -c -g reqchannel.cpp

threadManager.o: threadManager.h threadManager.cpp reqchannel.o semaphore.o
	g++ -std=c++11 -c -g threadManager.cpp reqchannel.o semaphore.o -pthread

dataserver: dataserver.cpp reqchannel.o
	g++ -std=c++11 -w -g -o dataserver dataserver.cpp reqchannel.o -lpthread

client: client.cpp threadManager.o reqchannel.o
	g++ -std=c++11 -w -g -o client client.cpp threadManager.o reqchannel.o semaphore.o -pthread

clean:
	rm *o fifo* dataserver client

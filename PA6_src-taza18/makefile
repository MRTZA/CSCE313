# makefile

all: dataserver client

reqchannel.o: reqchannel.h reqchannel.cpp
	g++ -g -w -Wall -O1 -std=c++11 -c reqchannel.cpp

BoundedBuffer.o: BoundedBuffer.h BoundedBuffer.cpp
	g++ -g -w -Wall -O1 -std=c++11 -c BoundedBuffer.cpp

Histogram.o: Histogram.h Histogram.cpp
	g++ -g -w -Wall -O1 -std=c++11 -c Histogram.cpp

FIFORequestChannel.o: FIFORequestChannel.h FIFORequestChannel.cpp
	g++ -g -w -Wall -O1 -std=c++11 -c FIFORequestChannel.cpp

MQRequestChannel.o: MQRequestChannel.h MQRequestChannel.cpp
	g++ -g -w -Wall -O1 -std=c++11 -c MQRequestChannel.cpp

SHMRequestChannel.o: SHMRequestChannel.h SHMRequestChannel.cpp
	g++ -g -w -Wall -O1 -std=c++11 -c SHMRequestChannel.cpp

dataserver: dataserver.cpp reqchannel.o SHMRequestChannel.o FIFORequestChannel.o MQRequestChannel.o
	g++ -g -w -Wall -O1 -std=c++11 -o dataserver dataserver.cpp reqchannel.o SHMRequestChannel.o FIFORequestChannel.o MQRequestChannel.o -lpthread

client: client.cpp reqchannel.o BoundedBuffer.o Histogram.o SHMRequestChannel.o FIFORequestChannel.o MQRequestChannel.o
	g++ -g -w -Wall -O1 -std=c++11 -o client client.cpp reqchannel.o BoundedBuffer.o Histogram.o SHMRequestChannel.o FIFORequestChannel.o MQRequestChannel.o -lpthread

clean:
	rm -rf *.o fifo* dataserver client

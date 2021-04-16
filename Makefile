CC=gcc
CFLAGS=-Wall -g -std=c11 -pedantic
LDLIBS=-lrt -pthread  #linux
FUNCTIONS=./functions



ALL =   client server

all : $(ALL)


client : client.c memory.h
server : server.c 

clean:
	rm -rf *~
cleanall:
	rm -rf *~ $(ALL) *.o

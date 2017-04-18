CC=gcc
CFLAGS=-I. -g

all: server testClient

server: server.c dict.c clientmgr.c networking.c
	$(CC) -o simplekvpcache server.c dict.c clientmgr.c networking.c $(CFLAGS)

testClient: testclient.c
	$(CC) -o testclient testclient.c $(CFLAGS)
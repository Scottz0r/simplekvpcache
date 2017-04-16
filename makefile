CC=gcc
CFLAGS=-I.

all: server

server: server.c dict.c clientmgr.c
	$(CC) -o simplekvpcache server.c dict.c clientmgr.c $(CFLAGS)

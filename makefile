CC=gcc
CFLAGS=-I. -g

all: server

server: server.c dict.c clientmgr.c networking.c
	$(CC) -o simplekvpcache server.c dict.c clientmgr.c networking.c $(CFLAGS)

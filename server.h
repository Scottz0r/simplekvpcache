#ifndef SERVER_H_
#define SERVER_H_

#include "dict.h"
#include <stdlib.h>

#define C_OK 0
#define C_ERR -1

#define DEFAULT_PORT 1337

typedef struct server {
    int port;
    int fd;
} server;

int connectServer(server* serv);
void listenClients(server* serv, dict* d);
void signalCallbackHandler(int signalNum);

#endif

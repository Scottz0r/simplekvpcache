#include "server.h"
#include "clientmgr.h"

#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

int main(){
    server serv;
    dict* kvpdict;

    signal(SIGINT, signalCallbackHandler);
    printf("Starting csockets Server.\n");

    serv.port = DEFAULT_PORT;
    kvpdict = dictCreate();

    connectServer(&serv);
    listenClients(&serv, kvpdict);

    return 0;
}

void signalCallbackHandler(int signalNum){
    printf("Signal %d. Exiting...\n", signalNum);
    exit(signalNum);
}

/*
 * Get a handle for the socket specified by the server arg.
 */
int connectServer(server* serv){
    struct sockaddr_in servAddr;
    int socketfd, bindRes;
    socketfd = socket(AF_INET, SOCK_STREAM, 0);

    if(socketfd < 0){
        return C_ERR;
    }

    serv->fd = socketfd;

    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(serv->port);
    servAddr.sin_addr.s_addr = INADDR_ANY;

    bindRes = bind(socketfd, (struct sockaddr*) &servAddr, sizeof(servAddr));
    if(bindRes < 0){
        return C_ERR;
    }

    return C_OK;
}

/*
 * Listen for clients to connect to the server, then handle the request.
 */
void listenClients(server* serv, dict* d){
    int newSocketFd;
    socklen_t clilen;
    struct sockaddr_in cliAddr;

    listen(serv->fd, 5);
    clilen = sizeof(cliAddr);

    while(1){
        newSocketFd = accept(serv->fd, (struct sockaddr*) &cliAddr, &clilen);
        if(newSocketFd < 0){
            printf("Error accepting socket.\n");
        }
        client cli;
        cli.fd = newSocketFd;
        clientmgrHandleClient(&cli, d);
        close(newSocketFd);
    }
}

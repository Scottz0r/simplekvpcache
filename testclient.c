#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

#define PORT 1337

#define OP_WRITE 1
#define OP_READ 2
#define OP_DELETE 3

#define RESPONSE_OKAY 1
#define RESPONSE_DATA 2
#define RESPONSE_ERR 3

typedef struct header_request {
    uint16_t operation;
    uint16_t keyLength;
    uint16_t messageSize;
} header_request;

typedef struct header_response {
    uint16_t responseCode;
    uint16_t messageSize;
} header_response;

void sendData(int);
void getData(int);

int main(){
    int sockfd, portno, n;
    struct sockaddr_in serv_addr;
    struct hostent *server;

    portno = PORT;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        printf("Error connecting to server.\n");
        return 1;
    }

    server = gethostbyname("localhost");
    if (server == NULL){
        printf("Host not found.\n");
        return 1;
    }

    memset((char*) &serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char*)server->h_addr, (char*)&serv_addr.sin_addr.s_addr, server->h_length);
    serv_addr.sin_port = htons(portno);

    int connres = connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if (connres < 0){
        printf("Error connecting to server.\n");
        return 1;
    }

    sendData(sockfd);
    close(sockfd);

    //Second request.

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0){
        printf("Error connecting to server.\n");
        return 1;
    }

    connres = connect(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    if(connres < 0){
        printf("Error connecting to server (2).\n");
        return 1;
    }

    getData(sockfd);

    close(sockfd);
}

void sendData(int sockfd){
    header_request reqHeader;
    header_response resHeader;
    char *key, *content, *headBuffer, *bodyBuffer;
    int n;
    size_t keySize, contentSize;
    uint16_t tmpResponseCode, tmpMessageSize;

    key = "test";
    content = "{ \"test\": true }";

    reqHeader.operation = htons(OP_WRITE);
    reqHeader.keyLength = htons(strlen(key));
    reqHeader.messageSize = htons(strlen(content));

    //Write header
    headBuffer = malloc(sizeof(char) * 6);
    memcpy(headBuffer, &(reqHeader.operation), 2);
    memcpy(headBuffer + 2, &(reqHeader.keyLength), 2);
    memcpy(headBuffer + 4, &(reqHeader.messageSize), 2);
    n = write(sockfd, headBuffer, 6);
    free(headBuffer);

    //Write body
    keySize = strlen(key);
    contentSize = strlen(content);
    bodyBuffer = malloc(sizeof(char) * (keySize + contentSize));
    memcpy(bodyBuffer, key, keySize);
    memcpy(bodyBuffer + keySize, content, contentSize);
    n = write(sockfd, bodyBuffer, keySize + contentSize);
    free(bodyBuffer);

    //Receive header
    headBuffer = malloc(sizeof(char) * 4);
    n = read(sockfd, headBuffer, 4);
    memcpy(&tmpResponseCode, headBuffer, 2);
    memcpy(&tmpMessageSize, headBuffer + 2, 2);

    resHeader.responseCode = ntohs(tmpResponseCode);
    resHeader.messageSize = ntohs(tmpMessageSize);

    printf("Response code received: %i\n", resHeader.responseCode);
    printf("Message size received: %i\n", resHeader.messageSize);

    //Not receiving content.
}

void getData(int sockfd){
    int n;
    header_request reqHeader;
    header_response resHeader;
    char *key, *headBuffer, *bodyBuffer;
    size_t keySize;
    uint16_t tmpResponseCode, tmpMessageSize;

    key = "test";
    keySize = strlen(key);

    reqHeader.operation = htons(OP_READ);
    reqHeader.keyLength = htons(keySize);
    reqHeader.messageSize = htons(0);

    headBuffer = malloc(sizeof(char) * 6);
    memcpy(headBuffer, &(reqHeader.operation), 2);
    memcpy(headBuffer + 2, &(reqHeader.keyLength), 2);
    memcpy(headBuffer + 4, &(reqHeader.messageSize), 2);
    n = write(sockfd, headBuffer, 6);
    free(headBuffer);

    bodyBuffer = malloc(sizeof(char) * keySize);
    memcpy(bodyBuffer, key, keySize);
    n = write(sockfd, bodyBuffer, keySize);
    free(bodyBuffer);

    //Receive header
    headBuffer = malloc(sizeof(char) * 4);
    n = read(sockfd, headBuffer, 4);
    memcpy(&tmpResponseCode, headBuffer, 2);
    memcpy(&tmpMessageSize, headBuffer + 2, 2);

    resHeader.responseCode = ntohs(tmpResponseCode);
    resHeader.messageSize = ntohs(tmpMessageSize);

    printf("Response code received: %i\n", resHeader.responseCode);
    printf("Message size received: %i\n", resHeader.messageSize);

    //Making sure to null terminate the bytes received from the server.
    bodyBuffer = malloc(resHeader.messageSize + 1);
    memset(bodyBuffer, 0, resHeader.messageSize + 1);
    n = read(sockfd, bodyBuffer, resHeader.messageSize);

    printf("Message is: %s\n", bodyBuffer);

    free(bodyBuffer);
}

#ifndef CLIENTMGR_H_
#define CLIENTMGR_H_

#include "dict.h"
#include <stdlib.h>
#include <stdint.h>

#define REPLY_CHUNK_BYTES 1024
#define BUFFER_SIZE 1024

#define OP_WRITE 1
#define OP_READ 2
#define OP_DELETE 3

#define RESPONSE_OKAY 1
#define RESPONSE_DATA 2
#define RESPONSE_ERR 3

typedef struct client {
    int fd;
    int buffpos;
    char buff[REPLY_CHUNK_BYTES];

} client;

typedef struct message {
    uint16_t operation;
    uint16_t keylen;
    uint16_t msgSize;
    char* key;
    char* body;
} message;

typedef struct inputBuffer {
    char buffer[BUFFER_SIZE];
    int bufferPos;
    int bufferLen;
} inputBuffer;

void clientmgrHandleClient(client* cli, dict* d);

#endif

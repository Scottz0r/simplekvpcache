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

typedef struct header_in {
    uint16_t operation;
    uint16_t keyLength;
    uint16_t messageSize;
} header_in;

typedef struct header_out {
    uint16_t responseCode;
    uint16_t messageSize;
} header_out;

typedef struct strObject {
    char* value;
    size_t size;
} strObject;

typedef struct client {
    int fd;
    header_out headerOut;
    int buffpos;
    int buffsize;
    char buff[REPLY_CHUNK_BYTES];

} client;

int clientmgrHandleClient(client* cli, dict* d);

#endif

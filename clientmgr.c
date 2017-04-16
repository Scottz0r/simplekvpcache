/*
 * Holds functions that reads and writes to requests sent from a client.
 */
#include "clientmgr.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netinet/in.h>

static message* _readIncomming(client* cli);
static void _decodeHeader(inputBuffer* buffer, message* msg);
static void _readKey(inputBuffer* buffer, message* msg);
static void _readContent(inputBuffer* buffer, message* msg, const client* cli);
static void _writeClientOkay(client* cli);
static void _writeClientData(client* cli, const char* data);

void clientmgrHandleClient(client* cli, dict* d){
    message* msg;
    msg = _readIncomming(cli);

    printf("The key is: %s\n", msg->key);
    printf("The body is: %s\n", msg->body);

    switch(msg->operation){
    case OP_READ:
    {
        const char* data;
        data = dictSearch(d, msg->key);
        _writeClientData(cli, data);
        break;
    }
    case OP_WRITE:
        dictInsert(d, msg->key, msg->body);
        _writeClientOkay(cli);
        break;
    case OP_DELETE:
        dictDelete(d, msg->key);
        _writeClientOkay(cli);
        break;
    }
}

/*
 * Read the incoming message from the client's handle. Returns
 * a message struct populated with the entire request, piecing any
 * message chunks together.
 *
 * Implemented as a state machine:
 * 0. Initial state = Read buffer from socket handle.
 * 1. Read the header from the request.
 * 2. Read the key from the request.
 * 3. Read the content/body from the request.
 */
static message* _readIncomming(client* cli){
    int n, state;
    inputBuffer buffer;
    message* msg;

    msg = malloc(sizeof(message));

    buffer.bufferLen = BUFFER_SIZE;
    buffer.bufferPos = 0;

    state = 0;
    while(state < 4){
        switch(state){
        case 0:
            memset(buffer.buffer, 0, buffer.bufferLen);
            n = read(cli->fd, buffer.buffer, buffer.bufferLen);
            if(n < 0){
                printf("Error reading client bytes.");
                state = 4;
            }
            state = 1;
            break;
        case 1:
            _decodeHeader(&buffer, msg);
            state = 2;
            break;
        case 2:
            _readKey(&buffer, msg);
            state = 3;
            break;
        case 3:
            _readContent(&buffer, msg, cli);
            state = 4;
            break;
        }
    }

    return msg;
}

/*
 * Read the key and content lengths from the buffer.
 */
static void _decodeHeader(inputBuffer* buffer, message* msg){
    uint16_t tmpOperation, tmpKeyLen, tmpMsgSize;
    memcpy(&tmpOperation, buffer->buffer, 2);
    memcpy(&tmpKeyLen, buffer->buffer + 2, 2);
    memcpy(&tmpMsgSize, buffer->buffer + 4, 2);
    buffer->bufferPos = 6;

    msg->operation = ntohs(tmpOperation);
    msg->keylen = ntohs(tmpKeyLen);
    msg->msgSize = ntohs(tmpMsgSize);
}

/*
 * Allocate a string for the key and read from the initial buffer.
 */
static void _readKey(inputBuffer* buffer, message* msg){
    char* key = malloc(sizeof(char) * msg->keylen);
    memcpy(key, buffer->buffer + buffer->bufferPos, msg->keylen);
    msg->key = key;
    buffer->bufferPos += msg->keylen;
}

/*
 * Read the body from the request, either flushing the initial buffer or
 * reading in additional buffers from the client's handle until the entire
 * body of the message, as specified in the header, is read into memory.
 */
static void _readContent(inputBuffer* buffer, message* msg, const client* cli){
    int n;
    size_t bodyRead = 0;
    char* content = malloc(sizeof(char) * msg->msgSize);

    //If the message is contained in the initial buffer.
    if(msg->msgSize < buffer->bufferLen - buffer->bufferPos){
        memcpy(content, buffer->buffer + buffer->bufferPos, msg->msgSize);
        bodyRead = msg->msgSize;
    } else {
        //flush the remaining of the buffer.
        memcpy(content, buffer->buffer + buffer->bufferPos, buffer->bufferLen - buffer->bufferPos);
        bodyRead = buffer->bufferLen - buffer->bufferPos;
    }

    //Read more buffers from the network.
    while(bodyRead < msg->msgSize){
        memset(buffer->buffer, 0, buffer->bufferLen);
        n = read(cli->fd, buffer->buffer, buffer->bufferLen);
        if(n < 0){
            printf("Error reading all content.");
            return;
        }

        if(bodyRead + buffer->bufferLen < msg->msgSize){
            //Need to read more after this.
            memcpy(content + bodyRead, buffer->buffer, buffer->bufferLen);
            bodyRead += buffer->bufferLen;
        } else {
            //Need to flush
            memcpy(content + bodyRead, buffer->buffer, msg->msgSize - bodyRead);
            bodyRead += msg->msgSize - bodyRead;
        }
    }

    msg->body = content;
}

/*
 * Write the Okay response to the client to indicate a non-data retrieval op
 * was successful.
 */
static void _writeClientOkay(client* cli){
    int n;
    char buffer[1];
    buffer[0] = RESPONSE_OKAY;
    n = write(cli->fd, buffer, sizeof(buffer));
}

static void _writeClientData(client* cli, const char* data){
    int n;
    size_t bufferPos, dataLen;
    uint16_t opCode, contentSize;

    dataLen = strlen(data);

    char buffer[REPLY_CHUNK_BYTES];
    memset(buffer, 0, sizeof(buffer));
    bufferPos = 0;

    opCode = htons(RESPONSE_DATA);
    contentSize = htons(dataLen);

    memcpy(buffer, &opCode, sizeof(opCode));
    bufferPos += sizeof(opCode);

    memcpy(buffer + bufferPos, &contentSize, sizeof(contentSize));
    bufferPos += sizeof(contentSize);

    memcpy(buffer + bufferPos, data, dataLen);
    bufferPos += sizeof(data);

    n = write(cli->fd, buffer, sizeof(buffer));
}

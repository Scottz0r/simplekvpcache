#include "networking.h"
#include "server.h"

#include <string.h>
#include <stdint.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

/*
 * Write the header and buffer in the client struct to the file handle specified
 * in the client struct.
 */
int networkWriteToClient(client* cli){
    int n;
    size_t headerBuffPos;
    uint16_t ntResponseCode, ntMessageSize;
    char headerBuffer[sizeof(header_out)];

    //Prepare and write the header.
    ntResponseCode = htons(cli->headerOut.responseCode);
    ntMessageSize = htons(cli->headerOut.messageSize);

    memcpy(headerBuffer, &ntResponseCode, sizeof(ntResponseCode));
    headerBuffPos = sizeof(ntResponseCode);
    memcpy(headerBuffer + headerBuffPos, &ntMessageSize, sizeof(ntMessageSize));
    headerBuffPos += sizeof(ntMessageSize);

    n = write(cli->fd, headerBuffer, headerBuffPos);

    if(n <= 0){
        return C_ERR;
    }

    //Write the content in the client's buffer (if there is any contenet).
    if(cli->buffpos > 0){
        n = write(cli->fd, cli->buff, cli->buffpos);

        if(n <= 0){
            return C_ERR;
        }
    }

    return C_OK;
}

/*
 * Header of a request is defined as follows:
 * Pos  Bytes   Description
 * ---  -----   -----------
 * 1    2       Operation
 * 2    2       Key length in bytes
 * 3    2       Content length in bytes
 */
int networkReadHeader(const client* cli, header_in* hdr){
    int n;
    unsigned int bufferPos;
    size_t bufferLength;
    uint16_t tmpOperation, tmpKeyLen, tmpMsgSize;

    bufferLength = 6;

    char buffer[bufferLength];
    n = read(cli->fd, buffer, bufferLength);

    if(n <= 0){
        return C_ERR;
    }

    memcpy(&tmpOperation, buffer, sizeof(tmpOperation));
    bufferPos += sizeof(tmpOperation);

    memcpy(&tmpKeyLen, buffer + 2, sizeof(tmpKeyLen));
    bufferPos += sizeof(tmpKeyLen);

    memcpy(&tmpMsgSize, buffer + 4, sizeof(tmpMsgSize));
    bufferPos += sizeof(tmpMsgSize);

    hdr->operation = ntohs(tmpOperation);
    hdr->keyLength = ntohs(tmpKeyLen);
    hdr->messageSize = ntohs(tmpMsgSize);

    return C_OK;
}

/*
 * Read a string from the handle, allocating memory for the new string.
 * The value in dataPointer must be freed after use!
 */
int networkReadString(const client* cli, int dataLength, strObject* dataPointer){
    int n;
    char* buffer;

    buffer = malloc(sizeof(char) * dataLength);
    dataPointer->value = buffer;
    dataPointer->size = dataLength;

    n = read(cli->fd, buffer, dataLength);
    if(n <= 0){
        return C_ERR;
    }

    return C_OK;
}

/*
 * Holds functions that reads and writes to requests sent from a client.
 */
#include "clientmgr.h"
#include "networking.h"
#include "server.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

static int _writeOperation(client* cli, dict* d, header_in* inputHeader);
static int _readOperation(client* cli, dict* d, header_in* inputHeader);
static void _writeClientErr(client* cli);
static void _writeClientOkay(client* cli);

int clientmgrHandleClient(client* cli, dict* d){
    int headerRes, opRes;
    header_in inputHeader;

    cli->buffpos = 0;
    cli->buffsize = REPLY_CHUNK_BYTES;
    memset(cli->buff, 0, cli->buffsize);

    //Read the header from the client.
    headerRes = networkReadHeader(cli, &inputHeader);
    if(headerRes < 0){
        _writeClientErr(cli);
        return C_ERR;
    }

    switch(inputHeader.operation){
    case OP_WRITE:
        opRes = _writeOperation(cli, d, &inputHeader);
        break;
    case OP_READ:
        opRes = _readOperation(cli, d, &inputHeader);
        break;
    case OP_DELETE:
        opRes = -1;
        break;
    }

    return opRes || C_OK;
}

static int _writeOperation(client* cli, dict* d, header_in* inputHeader){
    strObject key, content;
    int keyRes, bodyRes, success;

    //Read the key from the client.
    keyRes = networkReadString(cli, inputHeader->keyLength, &key);
    if(keyRes < 0){
        _writeClientErr(cli);
        success = C_ERR;
    } else {
        bodyRes = networkReadString(cli, inputHeader->messageSize, &content);
        if(keyRes < 0){
            _writeClientErr(cli);
            success = C_ERR;
        } else {
            dictInsert(d, key.value, content.value);
            _writeClientOkay(cli);
            success = C_OK;
        }
    }

    free(key.value);
    free(content.value);
    return success;
}

static int _readOperation(client* cli, dict* d, header_in* inputHeader){
    strObject key;
    int keyRes, writeRes, success, messageSize;
    const char* content;

    keyRes = networkReadString(cli, inputHeader->keyLength, &key);
    if(keyRes < 0){
        _writeClientErr(cli);
        success = C_ERR;
    } else {
        content = dictSearch(d, key.value);
        messageSize = strlen(content);

        memcpy(cli->buff + cli->buffpos, content, messageSize);
        cli->buffpos += messageSize;

        cli->headerOut.messageSize = messageSize;
        cli->headerOut.responseCode = RESPONSE_DATA;

        writeRes = networkWriteToClient(cli);
        success = writeRes || C_OK;
    }

    free(key.value);
    return success;
}

static void _writeClientErr(client* cli){
    cli->headerOut.responseCode = RESPONSE_ERR;
    cli->headerOut.messageSize = 0;
    networkWriteToClient(cli);
}

/*
 * Write the Okay response to the client to indicate a non-data retrieval op
 * was successful.
 */
static void _writeClientOkay(client* cli){
    cli->headerOut.responseCode = RESPONSE_OKAY;
    cli->headerOut.messageSize = 0;
    networkWriteToClient(cli);
}

#ifndef NETWORKING_H_
#define NETWORKING_H_

#include "clientmgr.h"

int networkWriteToClient(client* cli);
int networkReadHeader(const client* cli, header_in* hdr);
int networkReadString(const client* cli, int dataLength, strObject* dataPointer);

#endif

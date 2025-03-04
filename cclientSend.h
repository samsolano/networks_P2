#ifndef CCLIENTSEND_H
#define CCLIENTSEND_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <strings.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdint.h>
#include <errno.h>

#include "pdu.h"



int readFromStdin(uint8_t * buffer);
void processStdin(int socketNumber);
void splitMessage(uint8_t * sendBuf, int sendLen, int socketNum);
void processMessage(int socketNum);
void packageMessage(uint8_t flag, int socketNum);
void sendMessage(uint8_t * sendBuf, int sendLen, int socketNum);
void packageBroadcast(uint8_t flag, int socketNum);
void packageHandleList(uint8_t flag, int socketNum);


#endif
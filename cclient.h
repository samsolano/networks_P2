#ifndef CCLIENT_H
#define CCLIENT_H

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

#include "networks.h"
#include "safeUtil.h"
#include "pollLib.h"
#include "pdu.h"

#define DEBUG_FLAG 1

void checkArgs(int argc, char * argv[]); 
void clientControl(int socketNumber);  
void processMsgFromServer(int socketNumber);

#endif
#include <string.h> 
#include <stdint.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>


int sendPDU(int clientSocket, uint8_t * dataBuffer, int lengthOfData);
int recvPDU(int socketNumber, uint8_t * dataBuffer, int bufferSize);
void addNewSocket(int socketNumber);
void processClient(int socketNumber);
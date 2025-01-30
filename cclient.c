/******************************************************************************
* myClient.c
*
* Writen by Prof. Smith, updated Jan 2023
* Use at your own risk.  
*
*****************************************************************************/

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
#include "pdu.h"
#include "pollLib.h"

#define MAXBUF 1024
#define DEBUG_FLAG 1

// void sendToServer(int socketNum);
int readFromStdin(uint8_t * buffer);
void checkArgs(int argc, char * argv[]);
void clientControl(int socketNumber);
void processStdin(int socketNumber);
void processMsgFromServer(int socketNumber);
void splitMessage(uint8_t * sendBuf, int sendLen);
void processMessage();
void sendMessage();

char type[3] = {0};
char handle[101] = {0};
char message[1295] = {0};
int messageLength = 0;
char * destinationClients[9]; //array of strings

int main(int argc, char * argv[])
{
	int socketNum = 0;         //socket descriptor
	
	checkArgs(argc, argv);

	/* set up the TCP Client socket  */
	socketNum = tcpClientSetup(argv[2], argv[3], DEBUG_FLAG);

	
	clientControl(socketNum);
	
	close(socketNum);
	return 0;
}

void clientControl(int socketNumber) {

	setupPollSet();
	addToPollSet(socketNumber);
	addToPollSet(STDIN_FILENO);

	int socketReturned = 0;
	while (1) {
		printf("$: ");
		fflush(stdout);
		socketReturned = pollCall(-1);

		if ( socketReturned == STDIN_FILENO ) {
			processStdin(socketNumber); 
		}
		else if ( socketReturned == socketNumber ) {
			processMsgFromServer(socketNumber);
		}
	}
}

void processMsgFromServer(int socketNumber) {

    uint8_t dataBuffer[MAXBUF];
	int messageLen = 0;
	messageLen = recvPDU(socketNumber, dataBuffer, MAXBUF);
	
	if (messageLen > 0) 
	{
		printf("Message received from server on socket: %d, length: %d Data: %s\n", socketNumber, messageLen, dataBuffer);
		return;
	} 
	else if ( messageLen < 0 ) 
	{
		if (errno != ECONNRESET)
        {
            perror("recv call");
            exit(-1);
        } 
	}

	close(socketNumber);

	removeFromPollSet(socketNumber);
	printf("\nServer terminated\n");
	exit(0);
}

void processStdin(int socketNum) {
	uint8_t sendBuf[MAXBUF];   //data buffer
	int sendLen = 0;        //amount of data to send
	int sent = 0;            //actual amount of data sent/* get the data and send it   */
	
	sendLen = readFromStdin(sendBuf);
	// printf("read: %s string len: %d (including null)\n", sendBuf, sendLen);

	splitMessage(sendBuf, sendLen);
	processMessage();
	
	sent =  sendPDU(socketNum, sendBuf, sendLen);
	if (sent < 0)
	{
		perror("send call");
		exit(-1);
	}

	// printf("Amount of data sent is: %d\n", sent);
}


void sendMessage(int flag, uint8_t numOfDestinations) {

	// 1 byte specifying the number of destination handles contained in this message.
	// This number should be between 2 and 9.
	// o For each destination handle in the message
	//  1 byte containing the length of the handle of the destination client you want to talk with1
	//  Handle name of the destination client (no nulls or padding allowed)

	



} 

void processMessage() {

	if(!strcmp(type, "%%m")) {
		sendMessage();
	}
}

void splitMessage(uint8_t * sendBuf, int sendLen) {

	char copy[sendLen];
	strncpy(copy, (char *)sendBuf, sendLen); 

    char *token = strtok((char *)copy, " ");  // Cast to char*
	uint8_t stateCounter = 0;

    while (token != NULL) {

		if(stateCounter == 0) {

			strcpy(type, (char *)token);
			stateCounter++;
		}
		else if (stateCounter == 1) {
			strcpy(handle, (char *)token);
			stateCounter++;
		}
		else {
			int handleLength = strlen(handle);
			messageLength = sendLen - 4 - handleLength;
			strncpy(message, (char *)(sendBuf + 4 + handleLength), sendLen - 4 - handleLength);
			break;
		}
        token = strtok(NULL, " ");
    }

	// printf("type: %s, handle: %s, message: %s\n", type, handle, message);

} 
//see if whats typed is valid

int readFromStdin(uint8_t * buffer)
{
	char aChar = 0;
	int inputLen = 0;        
	
	// Important you don't input more characters than you have space 
	buffer[0] = '\0';
	// printf("Enter data: ");
	while (inputLen < (MAXBUF - 1) && aChar != '\n')
	{
		aChar = getchar();
		if (aChar != '\n')
		{
			buffer[inputLen] = aChar;
			inputLen++;
		}
	}
	
	// Null terminate the string
	buffer[inputLen] = '\0';
	inputLen++;
	
	return inputLen;
}

void checkArgs(int argc, char * argv[])
{
	if (argc != 4)
	{
		printf("usage: %s handle server-name server-port \n", argv[0]);
		exit(1);
	}
}
//todo: handle cases where cclient args are not correct types



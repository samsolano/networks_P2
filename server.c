/******************************************************************************
* myServer.c
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
#include "handleTable.h"

#define MAXBUF 1024
#define DEBUG_FLAG 1

void recvFromClient(int clientSocket);
int checkArgs(int argc, char *argv[]);
void serverControl(int socketNumber);
void dissectClientMessage(uint8_t * dataBuffer, int socketNumber, int messageLen);
void verifyHandle(uint8_t * dataBuffer, int socketNumber);
void processClientMessage(uint8_t * dataBuffer, int socketNumber, int flag, int messageLen);
void sendHandleList(uint8_t * dataBuffer, int socketNumber);
void sendMessageToClient(uint8_t * dataBuffer, int socketNumber, int numOfDestinations, int messageLen);
void handleError(char * handleName, int socketNumber);
void clearHandles();

char destClient1Server[101] = {0};
char destClient2Server[101] = {0};
char destClient3Server[101] = {0};
char destClient4Server[101] = {0};
char destClient5Server[101] = {0};
char destClient6Server[101] = {0};
char destClient7Server[101] = {0};
char destClient8Server[101] = {0};
char destClient9Server[101] = {0};

char * destHandlesServer[9] = { 
    destClient1Server, destClient2Server, destClient3Server, 
    destClient4Server, destClient5Server, destClient6Server, 
    destClient7Server, destClient8Server, destClient9Server
};

int nfds = 1;
int fds_max = POLL_SET_SIZE;

int main(int argc, char *argv[])
{

	int mainServerSocket = 0;   //socket descriptor for the server socket
	int clientSocket = 0;   //socket descriptor for the client socket
	int portNumber = 0;
	
	portNumber = checkArgs(argc, argv);	
	
	//create the server socket
	mainServerSocket = tcpServerSetup(portNumber);

	serverControl(mainServerSocket);

	
	/* close the sockets */
	close(clientSocket);
	close(mainServerSocket);

	
	return 0;
}

void serverControl(int socketNumber) {

	setupPollSet();
	addToPollSet(socketNumber);


	int socketReturned = 0;

	while (1) {

		socketReturned = pollCall(-1);

		if ( socketReturned == socketNumber ) {
			addNewSocket(socketReturned);
		} else {
			processClient(socketReturned);
		}
	}

}


void processClient(int socketNumber) {

    uint8_t dataBuffer[MAXBUF];
	int messageLen = 0;
	messageLen = recvPDU(socketNumber, dataBuffer, MAXBUF);


	if ( messageLen < 0 ) {
		if (errno != ECONNRESET)
        {
            perror("recv call");
            exit(-1);
        } 
	} else if (messageLen > 0) {
		dissectClientMessage(dataBuffer, socketNumber, messageLen);
		clearHandles();
		// printf("length: %d Data: %s\n", messageLen, dataBuffer);
		memset(dataBuffer, 0, MAXBUF);
		// sendPDU(socketNumber, dataBuffer, messageLen);
		return;
	} 

	close(socketNumber);
	removeFromPollSet(socketNumber);
	removeHandle(lookupSocket(socketNumber));
	// printf("Connection closed by other side\n");

}


void dissectClientMessage(uint8_t * dataBuffer, int socketNumber, int messageLen) {

	uint8_t flag = dataBuffer[0];

	if(flag == 1) { 
		verifyHandle(dataBuffer, socketNumber);
	}

	else if (flag == 4) {
		// printf("Broadcast\n");
		//processClientMessage(dataBuffer, socketNumber, flag, messageLen);
		sendMessageToClient(dataBuffer, socketNumber, 0, messageLen);

	} else if (flag == 5) {
		// printf("message\n");
		processClientMessage(dataBuffer, socketNumber, flag, messageLen);

	} else if (flag == 6) {
		// printf("multicast\n");
		processClientMessage(dataBuffer, socketNumber, flag, messageLen);

	} else if (flag == 10) {

		sendHandleList(dataBuffer, socketNumber);

	} else {
		perror("flag error\n");
	}
}

void sendHandleList(uint8_t * dataBuffer, int socketNumber) {

	// Flag 11
	uint8_t packet[5]; // 1 byte for flag, 4 bytes for number
	packet[0] = 11;
	int numOfHandles = htonl(getEntries());
	memcpy(packet + 1, &numOfHandles, 4);
	sendPDU(socketNumber, packet, 5);

	// Flag 12
	uint8_t packet12[102];
	int handleLen = 0;
	packet12[0] = 12;

	for (int i = getEntries() -1; i > -1; i--) {

		handleLen = strlen(handleTable[i].name);
		packet12[1] = handleLen;
		memcpy(packet12 + 2, handleTable[i].name, handleLen);
		sendPDU(socketNumber, packet12, handleLen + 2);
	}


	// Flag 13
	uint8_t thirteenthFlagValueToSendToTheClientSoTheyKnowTheListOfHandlesIsOver = 13;
	sendPDU(socketNumber, &thirteenthFlagValueToSendToTheClientSoTheyKnowTheListOfHandlesIsOver, 1);

}

void processClientMessage(uint8_t * dataBuffer, int socketNumber, int flag, int messageLen) {

	int8_t numOfDestinations = 0;


	int byteIndex = 1; 									//starts at self handle length	

	byteIndex += dataBuffer[byteIndex] + 1;			//moves past self handle and its length byte

	numOfDestinations = dataBuffer[byteIndex++];
	// printf("\nnum of dests: %d\n", numOfDestinations);

	for(int i = 0; i < numOfDestinations; i++) {

		uint8_t lengthOfHandle = dataBuffer[byteIndex++];
		memcpy(destHandlesServer[i], dataBuffer + byteIndex, lengthOfHandle);
		// printf("handle len: %d, handle: %s\n", lengthOfHandle, destHandlesServer[i]);
		byteIndex += lengthOfHandle; 
	}


	sendMessageToClient(dataBuffer, socketNumber, numOfDestinations, messageLen);
}

void sendMessageToClient(uint8_t * dataBuffer, int socketNumber, int numOfDestinations, int messageLen) {

	if(numOfDestinations > 0) {

		// printf("handle: ");
		for (int i = 0; i < numOfDestinations; i++) {


			int tempSocket = lookupHandle(destHandlesServer[i], strlen(destHandlesServer[i]));

			//printf("sendMessage: length of handle: %d to socket %d\n", strlen(destHandlesServer[i]), tempSocket);

			if(tempSocket == -1) {

				handleError(destHandlesServer[i], socketNumber);
				continue;
			}
			// printf("\n in send for loop: %s \n", destHandlesServer[i]);

			sendPDU(tempSocket, dataBuffer, messageLen);
		}
		// printf("\n");

	}
	else {
		// printf("\nbroadcast in server\n");

		for (int i = 0; i < getEntries(); i++) {

			int destSocket = handleTable[i].socket;

			if(destSocket != socketNumber) {

				sendPDU(destSocket, dataBuffer, messageLen);
			}
			
		}

	}
}


void handleError(char * handleName, int socketNumber) {

	//this will send error to socket with handle name saying this handle doesnt exist

	uint8_t packet[102];
	int handleLength = strlen(handleName);

	packet[0] = 7;
	packet[1] = handleLength;
	memcpy(packet + 2, handleName, handleLength);

	// printf("error with: %s, len: %d\n", handleName, handleLength);

	sendPDU(socketNumber, packet, handleLength + 2);

}


void verifyHandle(uint8_t * dataBuffer, int socketNumber) {

	uint8_t handle[100];
	uint8_t handleLength = dataBuffer[1];
	uint8_t flag = 3;


	memcpy(handle, dataBuffer + 2, handleLength);	

	if (lookupHandle((char *)handle, handleLength) == -1) {
		addHandle((char *)handle, handleLength, socketNumber);
		flag = 2;
	}
	else {
		removeFromPollSet(socketNumber);
	}

	sendPDU(socketNumber, &flag, 1);
}

void addNewSocket(int socketNumber) {

    // struct sockaddr_in6 clientAddress;   
	// int clientAddressSize = sizeof(clientAddress);
	int client_socket = 0;

    client_socket = tcpAccept(socketNumber, 1);

	if (client_socket < 0)
	{
		perror("accept call");
		exit(-1);
	}

	// printf("\nsocket added %d\n", socketNumber);

    addToPollSet(client_socket);
}

int checkArgs(int argc, char *argv[])
{
	// Checks args and returns port number
	int portNumber = 0;

	if (argc > 2)
	{
		fprintf(stderr, "Usage %s [optional port number]\n", argv[0]);
		exit(-1);
	}
	
	if (argc == 2)
	{
		portNumber = atoi(argv[1]);
	}
	
	return portNumber;
}

void clearHandles() {
	
	for (int i = 0; i < 9; i++) {
		memset(destHandlesServer[i], 0, 101);
	}
}

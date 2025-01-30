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
int nfds = 1;
int fds_max = POLL_SET_SIZE;

int main(int argc, char *argv[])
{

	int mainServerSocket = 0;   //socket descriptor for the server socket
	int clientSocket = 0;   //socket descriptor for the client socket
	int portNumber = 0;



	// createTable();
	// addHandle("sammy", 32);
	// addHandle("carlitos", 14);
	// addHandle("rayito", 2);
	// addHandle("eduito", 98);

	// char * naaame = NULL;

	// if((naaame = lookupSocket(31)) == NULL) {
	// 	printf("\nname not found\n");
	// }
	// printf("\nhandle name is %s\n", naaame);

	// printTable();

	// removeHandle("sammy");
	// printf("\n");

	// printTable();

	// removeHandle("rayito");
	// printf("\n");

	// printTable();


	// return 0;





	
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

		if (socketReturned == 0) {
			continue; //closed connection
		}
		if (socketReturned < 0) {
			break; //error
		}

		if ( socketReturned == socketNumber ) {
			addNewSocket(socketReturned);
		} else {
			processClient(socketReturned);
		}
	}

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

    addToPollSet(client_socket);
}

void processClient(int socketNumber) {

    uint8_t dataBuffer[MAXBUF];
	int messageLen = 0;
	messageLen = recvPDU(socketNumber, dataBuffer, MAXBUF);

	
	if (messageLen > 0) 
	{
		printf("Message received on socket: %d, length: %d Data: %s\n", socketNumber, messageLen, dataBuffer);
		// sendPDU(socketNumber, dataBuffer, messageLen);
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
	printf("Connection closed by other side\n");

}

void recvFromClient(int clientSocket)
{
	uint8_t dataBuffer[MAXBUF];
	int messageLen = 0;
	
	//now get the data from the client_socket
	if ((messageLen = recvPDU(clientSocket, dataBuffer, MAXBUF)) < 0)
	{
		perror("recv call");
		exit(-1);
	}

	if (messageLen > 0)
	{
		printf("Message received, length: %d Data: %s\n", messageLen, dataBuffer);
	}
	else
	{
		printf("Connection closed by other side\n");
	}
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

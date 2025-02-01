#include "cclientSend.h"
#include "cclient.h"
#include "cclientGlobals.h"


int main(int argc, char * argv[])
{


	int socketNum = 0;         //socket descriptor
	
	checkArgs(argc, argv);

	/* set up the TCP Client socket  */
	socketNum = tcpClientSetup(argv[2], argv[3], DEBUG_FLAG);


	checkHandle(socketNum);
	
	clientControl(socketNum);
	
	close(socketNum);
	return 0;
}

/* Send flag 1 and block waiting for either flag 2 or 3 */
void checkHandle(int socketNum) {

	uint8_t firstPacket[102];
	uint8_t handleLength = strlen(selfHandle);

	firstPacket[0] = 1;
	firstPacket[1] = handleLength;
	memcpy(firstPacket + 2, selfHandle, handleLength);

	setupPollSet();
	addToPollSet(socketNum);

	sendMessage(firstPacket, handleLength + 2, socketNum);

	while (1) {

		int socketReturned = pollCall(-1);
		
		if (socketReturned == socketNum) {

			uint8_t dataBuffer[MAXBUF];
			recvPDU(socketNum, dataBuffer, MAXBUF);

			if (dataBuffer[0] != 2) {
				fprintf(stderr, "Handle already in use: %s\n", selfHandle);
				exit(-1);
			}
			// printf("New handle has Successfully connected\n");
			addToPollSet(STDIN_FILENO);
			break;
		}

		// int c;
		// while ((c = getchar()) != '\n' && c != EOF);				// for flushing STDIN_FILENO
	}
}

void clientControl(int socketNumber) {

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

		memset(type, 0, sizeof(type)); 				//reset values after handling a message
		memset(message, 0, sizeof(message));
		messageLength = 0;
	}
}

void processMsgFromServer(int socketNumber) {

    uint8_t dataBuffer[MAXBUF];
	int messageLen = 0;
	messageLen = recvPDU(socketNumber, dataBuffer, MAXBUF);


	if (messageLen > 0) {
		dissectMessage(dataBuffer, messageLen, socketNumber);
		// printf("Message received from server on socket: %d, length: %d Data: %s\n", socketNumber, messageLen, dataBuffer);
		return;
	} 
	else if ( messageLen < 0 ) {
		if (errno != ECONNRESET) {
            perror("recv call");
            exit(-1);
        } 
	}

	close(socketNumber);
	removeFromPollSet(socketNumber);
	printf("\nServer terminated\n");
	exit(0);
}








void dissectMessage(uint8_t * dataBuffer, int messageLen, int socketNumber) {

	char copyCommand[messageLen];
	strncpy(copyCommand, (char *)dataBuffer, messageLen); 
	uint8_t flag = 0;
	int currentLength = 0;


	uint8_t stateCounter = 0;           //state 0 is for getting flag, 1 is for getting sender handle, 2 is for skipping through dest handles, 3 is for receiving message  
	             
	if (stateCounter++ == 0) {					//get flag

		flag = dataBuffer[0];
		currentLength++;

		if(flag == 7) { 				// handle error
			handleError(dataBuffer);
			return;
		}
		if(flag == 11) {				// listing handles
			handleList(dataBuffer, socketNumber);
			return;
		}
	}
	if (stateCounter++ == 1) {				// get self handle length, and print out self handle length

		printf("\n");
		for(int i = 0; i < dataBuffer[1]; i++) {
			printf("%c", dataBuffer[i + 2]);
		}
		printf(": ");
		currentLength += dataBuffer[1] + 1;			//plus one for self handle length byte
		// printf("curr length: %d\n", currentLength);

		if (flag == 4 ) {
			stateCounter = 3;
		}

	}
	if (stateCounter == 2) {

		stateCounter++;
		int destNum = dataBuffer[currentLength++]; //gets number of destination handles

		for(int i = 0; i < destNum; i++) {

			int currentDestHandleLength = dataBuffer[currentLength]; 		//gets length of current destination handle
			currentLength += currentDestHandleLength + 1; 			//move to after destination handle, either to message or next destination handle length, plus one for byte for length
		}
	}
	if(stateCounter == 3) {												//get message

		memcpy(message, dataBuffer + currentLength, messageLen - currentLength);
		printf("%s\n", message);

	}
}



void handleList(uint8_t * dataBuffer, int socketNumber) {

	int handleNum = 0;
	memcpy(&handleNum, dataBuffer + 1, 4);
	handleNum = ntohl(handleNum);
	printf("Number of clients: %d\n", handleNum);


	while (1) {

		int socketReturned = pollCall(-1);
		
		if (socketReturned == socketNumber) {

			uint8_t dataBuffer[MAXBUF];
			recvPDU(socketNumber, dataBuffer, MAXBUF);

			if(dataBuffer[0] == 12) {

				char handle[101];
				uint8_t length = dataBuffer[1];
				memcpy(handle, dataBuffer + 2, length);
				handle[length] = 0;
				printf("   %s\n", handle);
			}
			if(dataBuffer[0] == 13) { break; }
		}

	}

}

void handleError(uint8_t * dataBuffer) {
	// Format: chat-header then 1 byte handle length then handle of the destination 
	// client (the one not found on the server) as specified in the flag = 5 or flag = 6 message.

	int length = dataBuffer[1];

	printf("\nClient with handle ");
	for(int i = 0; i < length; i++) {
		printf("%c", dataBuffer[i + 2]);
	}
	printf(" does not exist.\n");

}





void checkArgs(int argc, char * argv[])
{
	if (argc != 4)
	{
		printf("usage: %s handle server-name server-port \n", argv[0]);
		exit(1);
	}

	if (strlen(argv[1]) > 100) {
		printf("Invalid handle, handle longer than 100 characters: %s\n", argv[1]);
		exit(-1);
	}


	strcpy(selfHandle, argv[1]);

	

}





// if text message is larger than 1400
// handle larger than 100
// no message is new line
// if i just send zion

// aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa

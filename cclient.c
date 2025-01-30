#include "cclientSend.h"
#include "cclient.h"
#include "cclientGlobals.h"


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

		memset(type, 0, sizeof(type)); 				//reset values after handling a message
		memset(message, 0, sizeof(message));
		messageLength = 0;
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







void checkArgs(int argc, char * argv[])
{
	if (argc != 4)
	{
		printf("usage: %s handle server-name server-port \n", argv[0]);
		exit(1);
	}
	strcpy(selfHandle, argv[1]);

}
//todo: handle cases where cclient args are not correct types





// ./cclient sam localhost 52169  
// %m ryan yo blud

#include "cclientSend.h"
#include "cclientGlobals.h"

char destClient1[101] = {0};
char destClient2[101] = {0};
char destClient3[101] = {0};
char destClient4[101] = {0};
char destClient5[101] = {0};
char destClient6[101] = {0};
char destClient7[101] = {0};
char destClient8[101] = {0};
char destClient9[101] = {0};

char * destHandles[9] = { 
    destClient1, destClient2, destClient3, 
    destClient4, destClient5, destClient6, 
    destClient7, destClient8, destClient9
};

uint8_t numOfDestinations = 0;

void processStdin(int socketNum) {
	uint8_t sendBuf[MAXBUF];  
	int sendLen = 0;                
	
	sendLen = readFromStdin(sendBuf);
	// printf("read: %s string len: %d (including null)\n", sendBuf, sendLen);

	splitMessage(sendBuf, sendLen);
	processMessage(socketNum);

}

/* Gets different parts of input

	- Figures out type of message
	- Gets number of destinations
	- gets destination handles
	- gets message to be sent  */

void splitMessage(uint8_t * sendBuf, int sendLen) {

	char copyCommand[sendLen];
	strncpy(copyCommand, (char *)sendBuf, sendLen); 
    numOfDestinations = 0;

    char *token = strtok((char *)copyCommand, " ");  
	uint8_t stateCounter = 0;                           //state 0 = getting command, state 1 = get num of destinations, state 2 = getting name, state 3 = getting message

    while (token != NULL) {

		if(stateCounter == 0) {                 //handle command

			strcpy(type, (char *)token);

            if(!strcmp(type, "%m")) {
                stateCounter = 2;
                numOfDestinations = 1;
                messageLength = 3;
            } else if(!strcmp(type, "%c")) {
                stateCounter = 1;
                messageLength = 5;
            } else if(!strcmp(type, "%b")) {
                stateCounter = 3;
                messageLength = 3;
            } else {
                perror("Command not recognized");
                exit(-1);
            }
            token = strtok(NULL, " ");
	    }
        else if (stateCounter == 1) {               //handle number of handles
            if ((numOfDestinations = atoi(token)) == 0) {
                perror("invalid num of destinations");
                exit(-1);
            }
            stateCounter = 2;
            token = strtok(NULL, " ");
        }
        else if (stateCounter == 2) {               //handle handles

            int j = 0;
            for (int i = numOfDestinations; i > 0; i--) {
                if(strlen((char *)token) > 101) {
                    fprintf(stderr, "Error: %s is over 100 characters", (char *)token);
                    exit(-1);
                }
                messageLength += strlen((char *)token) + 1;
                strcpy(destHandles[j++], (char *)token);
                token = strtok(NULL, " ");
                if(token == NULL) {
                    if(j == numOfDestinations) {
                        //message is newline
                        return;
                    }
                    perror("Error: not enough handles");
                    exit(-1);
                }
            }
            stateCounter = 3;
        }
        else if (stateCounter == 3) {                   //handle message

            printf("\nchars in: %d, total length: %d\n", messageLength, sendLen - messageLength);

			strncpy(message, (char *)(sendBuf + messageLength), sendLen - messageLength);
			break;
        }
	}
} 
//handle if %m and no handle,

void processMessage(int socketNum) {

	if(!strcmp(type, "%m")) {
		packageMessage(5, numOfDestinations, socketNum);
	} else if(!strcmp(type, "%c")) {
		packageMessage(6, numOfDestinations, socketNum);
	} else if(!strcmp(type, "%b")) {
		// packageMessage(4, numOfDestinations, socketNum);
	} else if(!strcmp(type, "%l")) {
		// packageMessage(10, numOfDestinations, socketNum);
	}
	
}

void packageMessage(uint8_t flag, uint8_t destNum, int socketNum) {

	// 1 + possible 100 + 1 + possible 9 + possible 900 + possible 200 = maximum of 1211 bytes in message

	uint8_t messageWithHeader[1211];

	uint8_t lengthOfSelfHeader = strlen(selfHandle);
	uint16_t totalLength = 0;

	int tempMessageLength = messageLength;

	if(tempMessageLength > 200) {

		while(tempMessageLength > 200) {  //keep sending packets until message is finished
			tempMessageLength -= 200;
		}

	} else { 								//if message is less than 200 characters

		messageWithHeader[0] = flag;
		messageWithHeader[1] = lengthOfSelfHeader;
		memcpy(messageWithHeader + 2, selfHandle, lengthOfSelfHeader);
		messageWithHeader[lengthOfSelfHeader + 2] = destNum;

		int byteIndex = lengthOfSelfHeader + 3;

		for(uint8_t i = 0; i < destNum; i++) { 	//put in handle lengths and destination handles into message header

			int currentDestHandleLength = strlen(destHandles[i]);
			messageWithHeader[byteIndex++] = currentDestHandleLength;
			memcpy(&messageWithHeader[byteIndex], destHandles[i], currentDestHandleLength);
			byteIndex += currentDestHandleLength;
		}

		memcpy(messageWithHeader + byteIndex, message, messageLength);

		totalLength = byteIndex + messageLength;
	}

	sendMessage(messageWithHeader, totalLength, socketNum);
} 

void sendMessage(uint8_t * sendBuf, int sendLen, int socketNum) {

    //-------------------------- For Debugging the parts of the command --------------------------//
    printf("\ncommand: %s, dest num: %d, handles: ", type, numOfDestinations);
    for(int i = 0; i < numOfDestinations; i++) {
        printf("%d: %s, ",i + 1, destHandles[i]);
    }
    printf("message: %s\n", message);

    //-------------------------------------------------------------------------------------------//


    //-------------------------- For Debugging the bytes ----------------------------------------//
	// printf("\n");
	// for (int i = 0; i < sendLen; i++) {
	// 	printf("%02x ", sendBuf[i]);
	// }
	// printf("\n");
    //------------------------------------------------------------------------------------------//

	return;





	// int sent = 0;

	// sent =  sendPDU(socketNum, sendBuf, sendLen);
	// if (sent < 0)
	// {
	// 	perror("send call");
	// 	exit(-1);
	// }

	// printf("Amount of data sent is: %d\n", sent);

}


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
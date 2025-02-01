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

int8_t numOfDestinations = 0;

void processStdin(int socketNum) {
	uint8_t sendBuf[MAXBUF];  
	int sendLen = 0;                
	
	sendLen = readFromStdin(sendBuf);
	// printf("read: %s string len: %d (including null)\n", sendBuf, sendLen);

	if(sendLen > 1400) {
		fprintf(stderr, "Error: cannot enter more than 1400 characters total\n");
		return;
	}

	splitMessage(sendBuf, sendLen, socketNum);
	processMessage(socketNum);

}

/* Gets different parts of input

	- Figures out type of message
	- Gets number of destinations
	- gets destination handles
	- gets message to be sent  */

void splitMessage(uint8_t * sendBuf, int sendLen, int socketNum) {


	char copyCommand[sendLen];
	strncpy(copyCommand, (char *)sendBuf, sendLen); 
    numOfDestinations = 0;

    char *token = strtok((char *)copyCommand, " ");  
	uint8_t stateCounter = 0;                           //state 0 = getting command, state 1 = get num of destinations, state 2 = getting name, state 3 = getting message


	if(strlen(token) > 2) {
		fprintf(stderr, "Invalid Command format\n");
		type[0] = 0;
		return;
	}

    while (token != NULL) {

		if(stateCounter == 0) {                         //handle command

			strcpy(type, (char *)token);


            if(!strcmp(type, "%m") || !strcmp(type, "%M")) {
                stateCounter = 2;
                numOfDestinations = 1;
                messageLength = 3;
            } else if(!strcmp(type, "%c") || !strcmp(type, "%C")) {
                stateCounter = 1;
                messageLength = 5;
            } else if(!strcmp(type, "%b") || !strcmp(type, "%B")) {
                stateCounter = 3;
                messageLength = 3;
            } else if (!strcmp(type, "%l") || !strcmp(type, "%L")) {
                return;
            } else {
                fprintf(stderr, "Invalid Command\n");
				type[0] = 0;
				return;
            }
            token = strtok(NULL, " ");
	    }
        else if (stateCounter == 1) {               //handle number of handles
			numOfDestinations = atoi(token);
            if (numOfDestinations <= 0 || numOfDestinations > 9) {
				
                fprintf(stderr, "invalid num of destinations\n");
				type[0] = 0; // this is so in the next function called (processMessage()) the else block is reached and returns to polling again
				return;
            }
            stateCounter = 2;
            token = strtok(NULL, " ");
        }
        else if (stateCounter == 2) {               //handle handles

            int j = 0;
            for (int i = numOfDestinations; i > 0; i--) {
                if(strlen((char *)token) > 100) {
                    fprintf(stderr, "Error: %s is over 100 characters\n", (char *)token);
                    type[0] = 0;
					return;
                }
                messageLength += strlen((char *)token) + 1;
                strcpy(destHandles[j++], (char *)token);
                token = strtok(NULL, " ");
                if(token == NULL) {
                    if(j == numOfDestinations) {
                        //message is newline
                        return;
                    }
                    fprintf(stderr, "Error: not enough handles\n");
                    type[0] = 0;
					return;
                }
            }
            stateCounter = 3;
        }
        else if (stateCounter == 3) {                   //handle message

			strncpy(message, (char *)(sendBuf + messageLength), sendLen - messageLength);
            messageLength = sendLen - messageLength;
			break;
        }
	}
} 
//handle if %m and no handle, and different cases with %c (no message, too few handles)

void processMessage(int socketNum) {

	if(!strcmp(type, "%m") || !strcmp(type, "%M")) {

		packageMessage(5, socketNum);
	} else if(!strcmp(type, "%c") || !strcmp(type, "%C")) {

		packageMessage(6, socketNum);
	} else if(!strcmp(type, "%b") || !strcmp(type, "%B")) {

		packageBroadcast(4, socketNum);
	} else if(!strcmp(type, "%l") || !strcmp(type, "%L")) {

		packageHandleList(10, socketNum);
	} else {
		return;
	}
}

void packageHandleList(uint8_t flag, int socketNum) {

    uint8_t messageWithHeader = flag;

    sendMessage(&messageWithHeader, 1, socketNum);
}

void packageBroadcast(uint8_t flag, int socketNum) {

    //1 + 1 + possible 100 + possible 200 = 302

    uint8_t messageWithHeader[302];
	uint8_t lengthOfSelfHeader = strlen(selfHandle);

	int tempMessageLength = messageLength;

    if(tempMessageLength > 200) {


		messageWithHeader[0] = flag;
		messageWithHeader[1] = lengthOfSelfHeader;
		memcpy(messageWithHeader + 2, selfHandle, lengthOfSelfHeader);

		int byteIndex = 2 + lengthOfSelfHeader;
		int currentPlaceInMessage = 0;
		uint8_t zero = 0;

		while(tempMessageLength > 200) {  //keep sending packets until message is finished

			memcpy(messageWithHeader + byteIndex, message + currentPlaceInMessage, 199); 
			currentPlaceInMessage += 199;
			memcpy(messageWithHeader + byteIndex + 199, &zero, 1);
			tempMessageLength -= 199;
			sendMessage(messageWithHeader, byteIndex + 200, socketNum);
		}

		memcpy(messageWithHeader + byteIndex, message + currentPlaceInMessage, tempMessageLength); 
		memcpy(messageWithHeader + byteIndex + tempMessageLength, &zero, 1);
		sendMessage(messageWithHeader, byteIndex + tempMessageLength, socketNum);

		return;

	} else { 
        
        messageWithHeader[0] = flag;
		messageWithHeader[1] = lengthOfSelfHeader;
		memcpy(messageWithHeader + 2, selfHandle, lengthOfSelfHeader);
		memcpy(messageWithHeader + lengthOfSelfHeader + 2, message, messageLength);
    }

    // printf("\nmessage Length: %d, total: %d\n", messageLength, messageLength + lengthOfSelfHeader + 2);

    sendMessage(messageWithHeader, messageLength + lengthOfSelfHeader + 2, socketNum);
}



/* This function handles both message %m and multicast %c and the function is split into two parts
	whether or not the message is larger than 200 characters*/

void packageMessage(uint8_t flag, int socketNum) {

	// 1 + possible 100 + 1 + possible 9 + possible 900 + possible 200 = maximum of 1211 bytes in message

	uint8_t messageWithHeader[1211];

	uint8_t lengthOfSelfHeader = strlen(selfHandle);
	uint16_t totalLength = 0;

	int tempMessageLength = messageLength;

	if(tempMessageLength > 200) {


		messageWithHeader[0] = flag;
		messageWithHeader[1] = lengthOfSelfHeader;
		memcpy(messageWithHeader + 2, selfHandle, lengthOfSelfHeader);
		messageWithHeader[lengthOfSelfHeader + 2] = numOfDestinations;

		int byteIndex = lengthOfSelfHeader + 3;

		for(uint8_t i = 0; i < numOfDestinations; i++) { 	//put in handle lengths and destination handles into message header

			int currentDestHandleLength = strlen(destHandles[i]);
			messageWithHeader[byteIndex++] = currentDestHandleLength;
			memcpy(&messageWithHeader[byteIndex], destHandles[i], currentDestHandleLength);
			byteIndex += currentDestHandleLength;
		}

		totalLength = byteIndex + messageLength;

		int currentPlaceInMessage = 0;
		uint8_t zero = 0;
		
		

		while(tempMessageLength > 200) {  //keep sending packets until message is finished

			memcpy(messageWithHeader + byteIndex, message + currentPlaceInMessage, 199); 
			currentPlaceInMessage += 199;
			memcpy(messageWithHeader + byteIndex + 199, &zero, 1);
			tempMessageLength -= 199;
			sendMessage(messageWithHeader, byteIndex + 200, socketNum);
		}

		memcpy(messageWithHeader + byteIndex, message + currentPlaceInMessage, tempMessageLength); 
		memcpy(messageWithHeader + byteIndex + tempMessageLength, &zero, 1);
		sendMessage(messageWithHeader, byteIndex + tempMessageLength, socketNum);

		return;

	} else { 								//if message is less than 200 characters

		messageWithHeader[0] = flag;
		messageWithHeader[1] = lengthOfSelfHeader;
		memcpy(messageWithHeader + 2, selfHandle, lengthOfSelfHeader);
		messageWithHeader[lengthOfSelfHeader + 2] = numOfDestinations;

		int byteIndex = lengthOfSelfHeader + 3;

		for(uint8_t i = 0; i < numOfDestinations; i++) { 	//put in handle lengths and destination handles into message header

			int currentDestHandleLength = strlen(destHandles[i]);
			messageWithHeader[byteIndex++] = currentDestHandleLength;
			memcpy(&messageWithHeader[byteIndex], destHandles[i], currentDestHandleLength);
			byteIndex += currentDestHandleLength;
		}
        // printf("\nmessageLength: %d, byteIndex: %d\n", messageLength, byteIndex);
		memcpy(messageWithHeader + byteIndex, message, messageLength); 

		totalLength = byteIndex + messageLength;
	}

	sendMessage(messageWithHeader, totalLength, socketNum);
} 

void sendMessage(uint8_t * sendBuf, int sendLen, int socketNum) {

	int sent = 0;

	sent =  sendPDU(socketNum, sendBuf, sendLen);
	if (sent < 0)
	{
		perror("send call");
		exit(-1);
	}

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


    //-------------------------- For Debugging the parts of the command --------------------------//

    // printf("\nWords: \n");
    // printf("command: %s, dest num: %d, handles
	// ", type, numOfDestinations);
    // for(int i = 0; i < numOfDestinations; i++) {
    //     printf("%d: %s, ",i + 1, destHandles[i]);
    // }
    // printf("message: %s\n", message);

    //-------------------------------------------------------------------------------------------//


    //-------------------------- For Debugging the bytes ----------------------------------------//
    // printf("\nBytes: \n");
	// for (int i = 0; i < sendLen; i++) {
	// 	printf("%02x ", sendBuf[i]);
	// }
	// printf("\n\n");
    //------------------------------------------------------------------------------------------//

	// return;




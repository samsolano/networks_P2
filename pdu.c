#include "pdu.h"
#include "pollLib.h"
#include "networks.h"
#include "safeUtil.h"
#include <poll.h>

#define MAXBUF 1024

int sendPDU(int clientSocket, uint8_t * dataBuffer, int lengthOfData) {

    uint8_t pdu[lengthOfData + 2];
    memset(pdu, 0, sizeof(pdu));

    uint16_t data_length_network = htons(lengthOfData + 2);
    memcpy(pdu, &data_length_network, 2);
    memcpy(pdu + 2, dataBuffer, lengthOfData);

    if (send(clientSocket, pdu, lengthOfData + 2, 0) >= 0) {
        return lengthOfData + 2;
    }

    perror("issue with send");
    exit(-1);
}

int recvPDU(int socketNumber, uint8_t * dataBuffer, int bufferSize) {

    uint16_t pduLength[1];
    memset(pduLength, 0, 1);

    int bytes = safeRecv(socketNumber, (uint8_t *)pduLength, 2, MSG_WAITALL);

    if (bytes <= 0) {
        return bytes;
    }
    *pduLength = ntohs(*pduLength);

    if ( *pduLength > bufferSize ) {
        perror("Buffer Overflows");
        exit(-1);
    }

    int received = safeRecv(socketNumber, dataBuffer, *pduLength -2, MSG_WAITALL); 
    if ( received < 0 ) {
        perror("error receiving payload of pdu");
        exit(-1);
    } else if ( received == 0 ) {
        perror("connection closed");
        return 0;
    }

    return received;
}
#include "handleTable.h"




void createTable() {

    tableSize = START_SIZE;
    handleTable = (Entry *) sCalloc(tableSize, sizeof(Entry));
}

int getEntries() {
    return tableEntries;
}

int lookupHandle(char * name, int nameLength) {

    char nameCopy[101];
    memcpy(nameCopy, name, nameLength);
    nameCopy[nameLength] = '\0';
    // printf("%s entering lookupHandle()\n", nameCopy);

    for (int i = 0; i < tableEntries; i++) {
        if (strcmp(handleTable[i].name, nameCopy) == 0) {
            // printf("name: %s is in the table\n",nameCopy);
            return handleTable[i].socket;
        }
    }

    // printf("name: %s is not in the handle table\n",nameCopy);

    return -1;
}

char * lookupSocket(int socketNum) {

    for (int i = 0; i < tableEntries; i++) {
        if (handleTable[i].socket == socketNum) {
            return handleTable[i].name;
        }
    }

    return NULL;
}

void addHandle(char * name, int nameLength, int socketNum) {

    char nameCopy[101];
    memcpy(nameCopy, name, nameLength);
    nameCopy[nameLength] = '\0';

    if (tableEntries == tableSize) {
        handleTable = srealloc(handleTable, sizeof(Entry) * (tableSize + 5));
        tableSize += 5;
    }

    // printf("handle: %s has been added on socket: %d \n", nameCopy, socketNum);

    strncpy(handleTable[tableEntries].name, nameCopy, 100); 
    handleTable[tableEntries++].socket = socketNum;
}

//how to get namelength to this

void removeHandle(char * name) { 


    // char nameCopy[101];
    // memcpy(nameCopy, name, nameLength);
    // nameCopy[nameLength] = '\0';


    int index = -1;
    
    for (int i = 0; i < tableEntries; i++) {
       if (strcmp(handleTable[i].name, name) == 0) {
            index = i;
            break;
       }
    }

    if (index == -1) {
        printf("Handle does not exist\n");
        return;
    }

    for(int i = index; i < tableEntries - 1; i++) {         //move all handles down in array when one is removed
        handleTable[i] = handleTable[i + 1];
    }

    tableEntries--;
}

void printTable() {

    for (int i = 0; i < tableEntries; i++) {
        printf("%02d socket: %02d, name: %s\n", i + 1, handleTable[i].socket, handleTable[i].name);
    }
}
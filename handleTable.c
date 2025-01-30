#include "handleTable.h"

static Entry * handleTable;
static int tableEntries = 0;
static int tableSize = 0;

void createTable() {

    tableSize = START_SIZE;
    handleTable = (Entry *) sCalloc(tableSize, sizeof(Entry));
}

int lookupHandle(char * name) {

    for (int i = 0; i < tableEntries; i++) {
        if (strcmp(handleTable[i].name, name) == 0) {
            return handleTable[i].socket;
        }
    }

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

void addHandle(char * name, int socketNum) {

    if (tableEntries == tableSize) {
        handleTable = srealloc(handleTable, sizeof(Entry) * (tableSize + 5));
        tableSize += 5;
    }

    strncpy(handleTable[tableEntries].name, name, 100); //
    handleTable[tableEntries++].socket = socketNum;
}

void removeHandle(char * name) {

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

    for(int i = index; i < tableEntries - 1; i++) {
        handleTable[i] = handleTable[i + 1];
    }

    tableEntries--;
}

void printTable() {

    for (int i = 0; i < tableEntries; i++) {
        printf("%02d socket: %02d, name: %s\n", i + 1, handleTable[i].socket, handleTable[i].name);
    }
}
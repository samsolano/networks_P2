#ifndef HANDLETABLE_H
#define HANDLETABLE_H

#include "safeUtil.h"

#define START_SIZE 10

typedef struct {
    int socket;
    char name[101];
} Entry;

void   createTable();
int    getEntries();
int    lookupHandle(char * name, int nameLength);
char * lookupSocket(int socketNum);
void   addHandle(char * name, int nameLength, int socketNum);
void   removeHandle(char * name);
void   printTable();


Entry * handleTable;



#endif
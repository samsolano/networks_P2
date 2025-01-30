#include "safeUtil.h"

#define START_SIZE 10

typedef struct {
    int socket;
    char name[101];
} Entry;

void   createTable();
int    lookupHandle(char * name);
char * lookupSocket(int socketNum);
void   addHandle(char * name, int socketNum);
void   removeHandle(char * name);
void   printTable();
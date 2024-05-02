#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


typedef struct{
  unsigned char* block[16];
} Drive;

Drive* newDrive();
char* displayDrive(Drive*);
int isUsed(Drive* d, int pos);
unsigned char* dump(Drive*);
void overwriteDrive(Drive* d, unsigned char* data);
int findOpenCell(int bitmap);
int pathIsLegal(char* path);
char* parsePath(char* path);
int traversePath(Drive* d, char* path);
int findNodeBlock(Drive* d, char* path);
int nodeInDirectory(Drive* d, char node, int blockNum);
int addToBitmap(int bitmap);
int addNode(Drive* d, char* path);
void setFileData(Drive* d, char* data, int fileBlock);
void ls(Drive* d, char* path);
void importFile(Drive* d, char* rfsPath, char* tfsPath);
void exportFile(Drive* d, char* rfsPath, char* tfsPath);
void makeDirectory(Drive* d, char* path);
void removeNode(Drive* d, char* path);
int openTFSFile(Drive* d, char* rfsPath);
int createTFSFile(char* path);
int closeTFSFile(Drive* d, char* path);
char** parseCommand(char* command);
int countTokens(char** tokens);
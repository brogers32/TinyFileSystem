#include "drive.h"

unsigned char* dump(Drive* d)
{
  unsigned char* ret = malloc(256);
  for (int i=0; i<16; i++)
  {
    for (int j=0; j<16; j++)
    {
      ret[16*i+j] = d->block[i][j];
    }
  }
  return ret;
}

Drive* newDrive()
{
  Drive* d = malloc(sizeof(Drive));
  for (int i=0; i<16; i++)
  {
    d->block[i] = malloc(16);
  }
  d->block[0][0] = 1; //0000 0001
  d->block[0][1] = 0; //0000 0000
  return d;
}

int isUsed(Drive* d, int pos)
{
  unsigned short bitmap = 0;
  if (pos >= 0 && pos <= 16) {
    memcpy(&bitmap, d->block[0],2);
  }
  unsigned short mask = 1;
  if (pos >= 0 && pos <= 16) {
    mask = mask << pos;
  }
  else {
    mask = 0;
  }
  return !((mask&bitmap)==0);
  
}
char* displayDrive(Drive* d)
{
  const int RETSIZE = 1024;
  char* temp = malloc(RETSIZE);
  char* ret = malloc(RETSIZE);
  ret[0] = '\0';
  strncat(ret,"   ",1024);
  for(int i=0; i<16; i++)
  {
    sprintf(temp,"  %x",i);
    strncat(ret,temp,RETSIZE);
  }
  strncat(ret,"\n",RETSIZE);

  for(int row=0; row<16; row++)
  {
    sprintf(temp,"%x: ",row);
    strncat(ret,temp,RETSIZE);    
    for(int col=0; col< 16; col++)
    {
      sprintf(temp,"%3.2x",d->block[row][col]);
      strncat(ret,temp,RETSIZE);      
    }
    strncat(ret,"\n",RETSIZE);
  }
  return ret;
}

void overwriteDrive(Drive* d, unsigned char* data) {
  for (int i=0; i<16; ++i) {
    for (int j=0; j<16; ++j) {
      d->block[i][j] = data[i*16+j];
    }
  }
}

int pathIsLegal(char* path) {
  for (int i=0; i<strlen(path); ++i) {
    if (i % 2 == 0) {
      if (path[i] != '/') {
        return 0;
      }
    }
    else {
      if (!isalpha(path[i])) {
        return 0;
      }
      if (islower(path[i]) && (i != strlen(path) - 1)) {
        return 0;
      }
    }
  }
  return 1;
}

char* parsePath(char* path) {
  char* copyPath = malloc(strlen(path) + 1);
  strncpy(copyPath, path, strlen(path));
  int nodeCount = strlen(copyPath) / 2;
  char* fsNodes = malloc(nodeCount + 1);
  fsNodes[nodeCount] = '\0';
  char* fsNode = strtok(copyPath, "/");
  for (int i=0; fsNode != NULL; i++) {
    fsNodes[i] = *fsNode;
    fsNode = strtok(NULL, "/");
  }
  return fsNodes;
}

int traversePath(Drive* d, char* path) {
  char* fsNodes = parsePath(path);
  if (strlen(fsNodes) < 2) {
    return 0;
  }
  int blockNum = 0;
  for (int i=0; i<strlen(fsNodes)-1; ++i) {
    int dirPos = -1;
    for (int j=3; j<11; ++j) {
      if (d->block[blockNum][j] == fsNodes[i]) {
        dirPos = j - 3;
      }
    }
    if (dirPos == -1) {
      return -1;
    }
    int cell = d->block[blockNum][dirPos/2+12];
    blockNum = dirPos % 2 == 1 ? cell / 16 : cell % 16;
  }
  return blockNum;
}

int findNodeBlock(Drive* d, char* path) {
  char* fsNodes = parsePath(path);
  char fsNode = fsNodes[strlen(fsNodes)-1];
  int fileParentDir = traversePath(d, path);
  int filePos = -1;
  for (int i=3; i<11; ++i) {
    if (d->block[fileParentDir][i] == fsNode) {
      filePos = i - 3;
      break;
    }
  }
  if (filePos == -1) {
    return -1;
  }
  int fileBlock;
  if (filePos % 2 == 0) {
    fileBlock = d->block[fileParentDir][filePos/2+12] % 16;
  }
  else {
    fileBlock = d->block[fileParentDir][filePos/2+12] / 16;
  }
  return fileBlock;
}

int findOpenCell(int bitmap) {
  if (bitmap == 255) {
    return -1;
  }
  int pos = 1;
  int count = 0;
  while ((bitmap&pos) != 0) {
    pos <<= 1;
    count++;
  }
  return count + 3;
}

int nodeInDirectory(Drive* d, char node, int blockNum) {
  for (int i=3; i<11; ++i) {
    if (d->block[blockNum][i] == node) {
      return i;
    }
  }
  return 0;
}

int addToBitmap(int bitmap) {
  return bitmap + pow(2, (findOpenCell(bitmap) - 3));
}

int addNode(Drive* d, char* path) {
  char* fsNodes = parsePath(path);
  int freeblock = -1;
  for (int i=1; i<16; ++i) {
    if (isUsed(d, i) == 0) {
      freeblock = i;
      break;
    }
  }
  if (freeblock == -1) {
    return -1;
  }
  char newNode = fsNodes[strlen(fsNodes) - 1];
  int blockPositon = traversePath(d, path);
  if (blockPositon == -1) {
    return -1;
  }
  int openCell = findOpenCell(d->block[blockPositon][2]);
  if (openCell == -1) {
    return -1;
  }
  if (nodeInDirectory(d, newNode, blockPositon)) {
    return -1;
  }
  d->block[blockPositon][2] = addToBitmap(d->block[blockPositon][2]);
  d->block[blockPositon][openCell] = newNode;
  int correspondenceCell = (openCell - 3) / 2 + 12;
  d->block[blockPositon][correspondenceCell] += (int)(freeblock * pow(16, (openCell-3)%2 == 1));
  int bitmapPos = d->block[0][0] < 255 ? 0 : 1;
  d->block[0][bitmapPos] = addToBitmap(d->block[0][bitmapPos]);
  if (!islower(newNode)) {
    d->block[freeblock][1] = blockPositon;
  }
  return freeblock;
}

void setFileData(Drive* d, char* data, int fileBlock) {
  int fileSize = strlen(data);
  d->block[fileBlock][0] = fileSize;
  if (fileSize > 15) {
    int i = 0;
    for (int j=1; j<16 && i<(fileSize-1)/16+1; ++j) {
      if (!isUsed(d, j)) {
        d->block[fileBlock][i+1] = j;
        int bitmapPos = d->block[0][0] < 255 ? 0 : 1;
        d->block[0][bitmapPos] = addToBitmap(d->block[0][bitmapPos]);
        ++i;
      }
    }
    for (int j=0; j<fileSize; ++j) {
      int curBlock = d->block[fileBlock][j/16+1];
      d->block[curBlock][j%16] = data[j];
    }
  }
  else {
    for (int i=0; i<fileSize; ++i) {
      d->block[fileBlock][i+1] = data[i];
    }
  }
}

void ls(Drive* d, char* path) {
  if (!pathIsLegal(path)) {
    char errorMessage[] = "invalid path\n";
    write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
    return;
  }
  char* copyPath = malloc(strlen(path) + 1);
  copyPath[strlen(path)] = '\0';
  strncpy(copyPath, path, strlen(path));
  int pathLen = strlen(path);
  if (pathLen % 2 == 0) {
    strncat(copyPath, "/x", 2);
  }
  else {
    strncat(copyPath, "x", 1);
  }
  int blockNum = traversePath(d, copyPath);
  if ((blockNum == -1) || (!pathIsLegal(copyPath))) {
    char errorMessage[] = "invalid path\n";
    write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
    return;
  }
  int firstCellPos = 3;
  for (int bitmap = d->block[blockNum][2]; bitmap > 0; bitmap >>= 1) {
    char* message = malloc(3);
    message[0] = d->block[blockNum][firstCellPos++];
    message[1] = '\n';
    message[2] ='\0';
    write(STDOUT_FILENO, message, 2);
  }
}

void importFile(Drive* d, char* rfsPath, char* tfsPath) {
  if (!pathIsLegal(tfsPath)) {
    char errorMessage[] = "invalid tiny file system path\n";
    write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
    return;
  }
  char* fsNodes = parsePath(tfsPath);
  if (strlen(fsNodes) == 0) {
    char errorMessage[] = "invalid tiny file system path\n";
    write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
    return;
  }
  char newNode = fsNodes[(strlen(fsNodes) - 1)];
  if (!islower(newNode)) {
    char errorMessage[] = "file name must be lowercase\n";
    write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
    return;
  }
  int fd = open(rfsPath, O_RDONLY);
  if (fd == -1) {
    char errorMessage[] = "invalid real file system path\n";
    write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
    return;
  }
  struct stat inode;
  int err = stat(rfsPath, &inode);
  if (err == -1) {
    char errorMessage[] = "error occurred while reading file\n";
    write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
    return;
  }
  int fileSize = inode.st_size;
  int numAvailableBlocks = 0;
  for (int i=1; i<16; ++i) {
    if (!isUsed(d, i)) {
      numAvailableBlocks++;
    }
  }
  int blocksNeeded = (fileSize-1)/16+1;
  if ((numAvailableBlocks < blocksNeeded) && (fileSize > 15)) {
    char errorMessage[] = "file too large to store in tiny file system\n";
    write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
    return;
  }
  int success = addNode(d, tfsPath);
  if (success == -1) {
    char errorMessage[] = "file could not be added\n";
    write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
    return;
  }
  char* buf = malloc(fileSize + 1);
  read(fd, buf, fileSize);
  buf[fileSize] = '\0';
  setFileData(d, buf, success);
}

void exportFile(Drive* d, char* rfsPath, char* tfsPath) {
  int fd = open(rfsPath, O_RDWR | O_CREAT, 0777);
  if (fd == -1) {
    char errorMessage[] = "cannot create new file";
    write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
    return;
  }
  int fileBlock = findNodeBlock(d, tfsPath);
  if (fileBlock == -1) {
    char errorMessage[] = "invalid tiny file system path";
    write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
  }
  int fileSize = d->block[fileBlock][0];
  char* buf = malloc(fileSize + 1);
  buf[fileSize] = '\0';
  if (fileSize > 15) {
    for (int i=0; i<fileSize; ++i) {
      int curBlock = d->block[fileBlock][i / 16 + 1];
      buf[i] = d->block[curBlock][i%16];
    }
  }
  else {
    for (int i=0; i<fileSize; ++i) {
      buf[i] = d->block[fileBlock][i+1];
    }
  }
  write(fd, buf, fileSize);
}

void makeDirectory(Drive* d, char* path) {
  if (!pathIsLegal(path)) {
    char errorMessage[] = "invalid tiny file system path\n";
    write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
    return;
  }
  char* fsNodes = parsePath(path);
  if (strlen(fsNodes) == 0) {
    char errorMessage[] = "invalid tiny file system path\n";
    write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
    return;
  }
  char newNode = fsNodes[(strlen(fsNodes) - 1)];
  if (islower(newNode)) {
    char errorMessage[] = "directory name must be uppercase\n";
    write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
    return;
  }
  int success = addNode(d, path);
  if (success == -1) {
    char errorMessage[] = "directory could not be added\n";
    write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
  }
}

void removeNode(Drive* d, char* path) {
  if (!pathIsLegal(path)) {
    char errorMessage[] = "invalid tiny file system path\n";
    write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
    return;
  }
  char* fsNodes = parsePath(path);
  char fsNode = fsNodes[strlen(fsNodes) - 1];
  int parentBlock = traversePath(d, path);
  if (parentBlock == -1) {
    char errorMessage[] = "invalid tiny file system path";
    write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
  }
  int nodeBlock = findNodeBlock(d, path);
  if (nodeBlock == -1) {
    char errorMessage[] = "invalid tiny file system path";
    write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
  }
  if (isupper(fsNode)) {
    if (d->block[nodeBlock][2] != 0) {
      char errorMessage[] = "directory not empty\n";
      write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
      return;
    }
    d->block[nodeBlock][1] = '\0';
  }
  else {
    if (d->block[nodeBlock][0] <= 15) {
      for (int i=0; i<=d->block[nodeBlock][0]; ++i) {
        d->block[nodeBlock][i] ='\0';
      }
    }
    else {
      for (int i=0; i<d->block[nodeBlock][0]; ++i) {
        int curBlock = d->block[nodeBlock][i/16+1];
        d->block[curBlock][i%16] = '\0';
      }
      for (int i=0; i<(d->block[nodeBlock][0]-1)/16+1; ++i) {
        d->block[0][d->block[nodeBlock][i+1]/8] -= pow(2, d->block[nodeBlock][i+1]%8);
        d->block[nodeBlock][i+1] = '\0';
      }
      d->block[nodeBlock][0] = '\0';
    }
  }
  int filePos = nodeInDirectory(d, fsNode, parentBlock);
  d->block[parentBlock][filePos] = '\0';
  if ((filePos) % 2 == 0) {
    d->block[parentBlock][(filePos-3)/2+12] = d->block[parentBlock][(filePos-3)/2+12] - (d->block[parentBlock][(filePos-3)/2+12] / 16 * 16);
  }
  else {
    d->block[parentBlock][(filePos-3)/2+12] = d->block[parentBlock][(filePos-3)/2+12] - (d->block[parentBlock][(filePos-3)/2+12] % 16);
  }
  d->block[parentBlock][2] -= pow(2, filePos-3);
  d->block[0][nodeBlock/8] -= pow(2, nodeBlock%8);
}

int openTFSFile(Drive* d, char* path) {
  int fd = open(path, O_RDWR);
  if (fd == -1) {
    return -1;
  }
  struct stat inode;
  int err = stat(path, &inode);
  if (err == -1) {
    return -1;
  }
  if (inode.st_size != 256) {
    return -1;
  }
  unsigned char* data = malloc(256);
  read(fd, data, 256);
  overwriteDrive(d, data);
  free(data);
  return fd;
}

int createTFSFile(char* path) {
  Drive* d = newDrive();
  int fd = open(path, O_RDWR | O_CREAT, 0777);
  if (fd == -1) {
    return -1;
  }
  write(fd, dump(d), 256);
  return fd;
}

int closeTFSFile(Drive* d, char* path) {
  int fd = open(path, O_WRONLY);
  if (fd == -1) {
    return -1;
  }
  write(fd, dump(d), 256);
  return fd;
}

char** parseCommand(char* command) {
  char* copy = malloc(strlen(command) + 1);
  strncpy(copy, command, strlen(command));
  copy[strlen(command)] = '\0';

  char** tokens = malloc(strlen(command) / 2 + 1);
  char* token = strtok(copy, " ");
  int count = 0;
  while (token != NULL) {
    tokens[count] = token;
    token = strtok(NULL, " ");
    count++;
  }
  tokens[count] = NULL;
  return tokens;
}

int countTokens(char** tokens) {
  int count = 0;
  while (tokens[count] != NULL) {
    count++;
  }
  return count;
}
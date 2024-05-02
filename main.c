#include <stdio.h>
#include "drive.h"
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

int main(int argc, char** argv) {
  Drive* d = newDrive();
  char* currentDriveFile;

  int exit = 0;
  int fd;
  if (argc == 1) {
    fd = openTFSFile(d, "temp.bin");
    if (fd == -1) {
      fd = createTFSFile("temp.bin");
    }
    currentDriveFile = malloc(9);
    currentDriveFile[8] = '\0';
    strncpy(currentDriveFile, "temp.bin", 8);
  }
  else if (argc == 2) {
    fd = openTFSFile(d, argv[1]);
    if (fd == -1) {
      char errorMessage[] = "there was an error opening the file\n";
      write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
      exit = 1;
    }
    currentDriveFile = malloc(strlen(argv[1]) + 1);
    currentDriveFile[strlen(argv[1])] = '\0';
    strncpy(currentDriveFile, argv[1], strlen(argv[1]));
  }
  else {
    char errorMessage[] = "command not recognized\n";
    write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
    exit = 1;
  }
  while (!exit) {
    write(STDOUT_FILENO, "TFS:  ", 6);
    char* userInput = malloc(500);
    int bytes = read(STDIN_FILENO, userInput, 500);
    if (strncmp(userInput, "\n", 1) == 0){
      continue;
    }
    userInput[bytes-1] = '\0';
    char** tokens = parseCommand(userInput);
    if (strncmp(tokens[0], "exit", 4) == 0) {
      exit = 1;
    }
    else if (strncmp(tokens[0], "import", 6) == 0) {
      if (countTokens(tokens) != 3) {
        char errorMessage[] = "command cannot be processed\n";
        write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
        continue;
      }
      importFile(d, tokens[1], tokens[2]);
    }
    else if (strncmp(tokens[0], "export", 6) == 0) {
      if (countTokens(tokens) != 3) {
        char errorMessage[] = "command cannot be processed\n";
        write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
        continue;
      }
      exportFile(d, tokens[2], tokens[1]);
    }
    else if (strncmp(tokens[0], "ls", 2) == 0) {
      if (countTokens(tokens) != 2) {
        char errorMessage[] = "command cannot be processed\n";
        write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
        continue;
      }
      ls(d, tokens[1]);
    }
    else if (strncmp(tokens[0], "display", 7) == 0) {
      if (countTokens(tokens) != 1) {
        char errorMessage[] = "command cannot be processed\n";
        write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
        continue;
      }
      write(STDOUT_FILENO, displayDrive(d), strlen(displayDrive(d)));
    }
    else if (strncmp(tokens[0], "open", 4) == 0) {
      if (countTokens(tokens) != 2) {
        char errorMessage[] = "command cannot be processed\n";
        write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
        continue;
      }
      closeTFSFile(d, currentDriveFile);
      int fd = openTFSFile(d, tokens[1]);
      if (fd == -1) {
        char errorMessage[] = "TFS file could not be opened\n";
        write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
        continue;
      }
      currentDriveFile = malloc(strlen(tokens[1]) + 1);
      currentDriveFile[strlen(tokens[1])] = '\0';
      strncpy(currentDriveFile, tokens[1], strlen(tokens[1]));
    }
    else if (strncmp(tokens[0], "create", 6) == 0) {
      if (countTokens(tokens) != 2) {
        char errorMessage[] = "command cannot be processed\n";
        write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
        continue;
      }
      if (access(tokens[1], F_OK) == 0) {
        char errorMessage[] = "TFS file already exists\n";
        write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
        continue;
      }
      int fd = createTFSFile(tokens[1]);
      if (fd == -1) {
        char errorMessage[] = "TFS file could not be created\n";
        write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
        continue;
      }
    }
    else if (strncmp(tokens[0], "mkdir", 5) == 0) {
      if (countTokens(tokens) != 2) {
        char errorMessage[] = "command cannot be processed\n";
        write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
        continue;
      }
      makeDirectory(d, tokens[1]);
    }
    else if (strncmp(tokens[0], "remove", 6) == 0) {
      if (countTokens(tokens) != 2) {
        char errorMessage[] = "command cannot be processed\n";
        write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
        continue;
      }
      removeNode(d, tokens[1]);
    }
    else {
      char errorMessage[] = "command not recognized\n";
      write(STDOUT_FILENO, errorMessage, strlen(errorMessage));
    }
  }
  closeTFSFile(d, currentDriveFile);
  write(STDOUT_FILENO, "closing Tiny File System\n", 25);
  return 0;
}
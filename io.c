#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include "io.h"

int WriteBufferToFile(char * strFileName, char * buff, size_t bufferSize){
  FILE * fd;
  fd = fopen(strFileName, "w");
  int bytesWriten = fwrite(buff,1,bufferSize,fd);
  fclose(fd);
  return bytesWriten;
}

int ReadFileToBuffer(char * strFileName, char * buff, size_t bufferSize){
  FILE * fd;
  fd = fopen(strFileName, "r");
  int bytesRead = fread(buff,1,bufferSize,fd);
  fclose(fd);
  return bytesRead;
}

long GetFileSizeBytes(char * strFileName){
  struct stat st;
  stat(strFileName, &st);
  return st.st_size;
}

extern int FileExists(char * strFileName){
  if(access(strFileName,F_OK) != -1){
    return 1;
  }
  return 0;
}

int GetIpInfo(char * strConfigFile, struct IpInfo * ipI){
  char * buffer = malloc(GET_IP_INFO_BUFFER_SIZE);
  memset(buffer,0,GET_IP_INFO_BUFFER_SIZE);
  int bytesRead = ReadFileToBuffer(strConfigFile, buffer, GET_IP_INFO_BUFFER_SIZE);

  if(bytesRead > 0){
    char * addr = strtok(buffer,":");
    char * port = strtok(NULL,":");

    struct IpInfo ipInf;
    ipI->ipAddr = malloc(50);// DANGER!
    memset(ipI->ipAddr, 0, 50);
    strcpy(ipI->ipAddr, addr);
    ipI->portNumber = atoi(port);

    free(buffer);
    return 1;
  }

  free(buffer);
  return 0;
}

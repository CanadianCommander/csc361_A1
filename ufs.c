#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include "io.h"
#include "network.h"


void NetworkLoop(int socket);

int main(int argv, char ** argc){

  if(argv < 2){
    printf("Usage ./ufs <directoryName>\n");
    exit(1);
  }

  struct IpInfo ipInfo;
  int res = GetIpInfo("server.conf", &ipInfo);
  if(res == 0){
    printf("ERROR: could not find server.conf!");
    exit(1);
  }

  int sock = CreateServerSocketTCP(ipInfo.ipAddr, ipInfo.portNumber);
  if (sock == -1){
    printf("ERROR: failed to create socket!!!");
    exit(1);
  }

  if(chdir(argc[1]) == -1){
    printf("ERROR: could not find directory: %s \n", argc[1]);
    exit(1);
  }
  else{
    printf("Server Running on dir: %s\n", argc[1]);
  }

  NetworkLoop(sock);


  CloseSocket(sock);
  free(ipInfo.ipAddr);
  return 0;
}


void NetworkLoop(int socket){
  char message[NETWORK_BUFFER_SIZE];
  char ipAddr[50];// dat magic number tho
  int  port;

  while(1){
    memset(message,0, NETWORK_BUFFER_SIZE);
    memset(ipAddr,0,50);
    RecvTCP(socket,message,NETWORK_BUFFER_SIZE);

    if(message[0] == FTP_GET){
      if(FileExists(message+1)){
        // FOUND
        char found = FTP_FOUND;
        if(SendTCP(socket, &found,1)){

          printf("Starting file transmision\n");
          // SEND FILE
          long fSize = GetFileSizeBytes(message+1);
          char * fileBuffer = malloc(fSize);
          memset(fileBuffer, 0 , fSize);
          ReadFileToBuffer(message + 1, fileBuffer,fSize);
          if(SendFile(socket, fileBuffer, fSize, ipAddr, &port)){
            printf("file transmision complete\n");
          }
          else{
            printf("ERROR: could not send file\n");
          }
        }
      }
      else{
        // NOT FOUND
        char notFound = FTP_NOT_FOUND;
        SendTCP(socket, &notFound,1);
      }
    }
    else if(message[0] == FTP_THANKS){
      printf("Client Says Bye!\n");
    }
    else{
      printf("ERROR: unknown control sequence %x\n", message[0]);
    }
    fflush(stdout);

  }

}

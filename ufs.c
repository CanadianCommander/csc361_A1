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

  int sock = CreateServerSocket(ipInfo.ipAddr, ipInfo.portNumber);
  if (sock == -1){
    printf("ERROR: failed to create socket!!!");
    exit(1);
  }

  if(chdir(argc[1]) == -1){
    printf("ERROR: could not find directory: %s \n", argc[1]);
    exit(1);
  }
  else{
    printf("Server Running on dir: %s", argc[1]);
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
    SafeRecv(socket,message,NETWORK_BUFFER_SIZE,ipAddr,&port);

    if(message[0] == FTP_GET){
      if(FileExists(message+1)){
        // FOUND
      }
      else{
        // NOT FOUND
      }
    }
    else if(message[0] == FTP_THANKS){

    }
    else{
      printf("ERROR: unknown control sequence %x\n", message[0]);
    }
    fflush(stdout);

  }

}

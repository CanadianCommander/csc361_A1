#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "io.h"
#include "network.h"


void NetworkLoop(int socket,char * ipAddr, int port, char * targetFile);

int main(int argv, char ** argc){

  if(argv < 2){
    printf("usage ./ufc <fileName> \n");
    exit(1);
  }

  struct IpInfo ipInfo;
  int res = GetIpInfo("client.conf", &ipInfo);
  if(res == 0){
    printf("ERROR: could not find server.conf!");
    exit(1);
  }

  int sock = CreateClientSocket();
  if (sock == -1){
    printf("ERROR: failed to create socket!!!");
    exit(1);
  }

  NetworkLoop(sock,ipInfo.ipAddr,ipInfo.portNumber, argc[1]);


  CloseSocket(sock);
  free(ipInfo.ipAddr);
  return 0;
}


void NetworkLoop(int socket,char * ipAddr, int port, char * targetFile){
  char message[NETWORK_BUFFER_SIZE];
  memset(message,0, NETWORK_BUFFER_SIZE);
  message[0] = FTP_GET;
  strcpy(1 + message,targetFile);

  if(!SafeSend(socket,message,NETWORK_BUFFER_SIZE,ipAddr,&port)){
    printf("ERROR: could not send message to server!\n");
    return;
  }


}
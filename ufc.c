#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h>
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
    printf("ERROR: could not find client.conf!");
    exit(1);
  }

  int sock = CreateClientSocketTCP(ipInfo.ipAddr,ipInfo.portNumber);
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
  struct timeval begin_time, end_time;

  gettimeofday(&begin_time,NULL);
  message[0] = FTP_POST;
  strcpy(1 + message,targetFile);
  if(!SendTCP(socket,message,NETWORK_BUFFER_SIZE)){
    printf("ERROR: could not send POST message to server!\n");
    return;
  }

  //send zip file
  TransmitFile(socket,targetFile);

  // request the unziped file
  memset(message,0, NETWORK_BUFFER_SIZE);
  message[0] = FTP_GET;
  if(!SendTCP(socket,message,NETWORK_BUFFER_SIZE)){
    printf("ERROR: could not send GET message to server!\n");
    return;
  }

  //recieve file packets
  char * fileData;
  int fileLen = RecvFile(socket,&fileData,NULL,NULL);
  WriteBufferToFile(strcat(targetFile, ".dl"),fileData,fileLen);

  // say good bye
  memset(message,0, NETWORK_BUFFER_SIZE);
  message[0] = FTP_THANKS;
  if(!SendTCP(socket,message,NETWORK_BUFFER_SIZE)){
    printf("ERROR: could not send THANKS message to server!\n");
    return;
  }

  gettimeofday(&end_time,NULL);
  printf("transfer time: %lf ms\n",(end_time.tv_sec - begin_time.tv_sec)*1000.0 + (end_time.tv_usec - begin_time.tv_usec)/1000.0);
}

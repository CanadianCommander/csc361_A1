#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include "network.h"

int CreateServerSocket(char * ipAddr, int port){
  struct sockaddr_in sa;
  int mySocket = socket(AF_INET,SOCK_DGRAM, 0);

  memset(&sa, 0, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_port = port;
  inet_pton(AF_INET, ipAddr, &sa.sin_addr);

  bind(mySocket, (struct sockaddr *)&sa, sizeof(sa));

  return mySocket;
}


int CreateClientSocket(){
  int mySocket = socket(AF_INET,SOCK_DGRAM, 0);
  return mySocket;
}

void CloseSocket(int sock){
  close(sock);
}

int Recv(int sock,void * buffer,long bufferSize, char * ip_out, int * port_out){
  struct sockaddr_in sa;
  memset(&sa, 0 , sizeof(sa));

  uint sockSize = sizeof(sa);
  int bytesReceived = recvfrom(sock, buffer, bufferSize,0,(struct sockaddr *)&sa,&sockSize);

  if(ip_out != 0 && port_out != 0){
    inet_ntop(AF_INET, &sa.sin_addr,ip_out, 50); // DANGER!
    *port_out = sa.sin_port;
  }
  return bytesReceived;
}

int Send(int sock, void * buffer, long bufferSize, char * ipAddr, int * portNum){
  struct sockaddr_in sa;
  memset(&sa, 0, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_port = *portNum;
  inet_pton(AF_INET, ipAddr, &sa.sin_addr);

  return sendto(sock,buffer,bufferSize,0,(struct sockaddr *)&sa,sizeof(sa));
}


int SafeSend(int sock, char * buffer, long bufferSize, char * ipAddr, int * portNum){
  int i = 0;
  for(i = 0; i < NETWORK_RETRY_COUNT; i++){
    // send packet
    Send(sock, buffer, bufferSize, ipAddr, portNum);

    // wait for ack
    char ack[NETWORK_ACK_SIZE];
    memset(ack, 0, NETWORK_ACK_SIZE);

    int recv = Recv(sock,ack,NETWORK_ACK_SIZE, 0,0);
    if(recv < 1){
      printf("ERROR: bad ack On safe send!");
      return 0;
    }
    else if (strcmp(ack,"ACK") == 0){
      return 1;
    }
    else{
      printf("ERROR: got message when waiting for ack but is not ack!\n %s", ack);
      return 0;
    }
  }

  if (i == NETWORK_RETRY_COUNT){
    printf("ERROR: Safe send retry count exceeded!");
    return 0;
  }
  else{
    return 1;
  }
}


int SafeRecv(int sock, void * buffer, long bufferSize, char * ip_out, int * port_out){
  Recv(sock,buffer,bufferSize,ip_out,port_out);

  //send ack
  Send(sock,NETWORK_ACK,NETWORK_ACK_SIZE,ip_out,port_out);
  return 1;
}

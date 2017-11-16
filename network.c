#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include "network.h"

short GetChunkLength(char * buff){
  if(buff[0] != FTP_FILE_PACKAGE){
    printf("ERROR: buffer is not a file packet! it is: %x\n", buff[0]);
    return -1;
  }

  return ntohs(*(short*)(buff + 1));
}

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

int CreateServerSocketTCP(char * ipAddr, int port){
  struct sockaddr_in sa;
  int mySocket = socket(AF_INET,SOCK_STREAM, 0);

  memset(&sa, 0, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  inet_pton(AF_INET, ipAddr, &sa.sin_addr);

  bind(mySocket, (struct sockaddr *)&sa, sizeof(sa));

  listen(mySocket, 1);

  uint addrLen;
  return accept(mySocket, (struct sockaddr *)&sa, &addrLen);
}

int CreateClientSocket(){
  int mySocket = socket(AF_INET,SOCK_DGRAM, 0);
  return mySocket;
}

int CreateClientSocketTCP(char * ipAdder, int port){
  int newSock = socket(AF_INET,SOCK_STREAM, 0);
  struct sockaddr_in sa;

  memset(&sa, 0, sizeof(sa));
  sa.sin_family = AF_INET;
  sa.sin_port = htons(port);
  inet_pton(AF_INET, ipAdder, &sa.sin_addr);

  connect(newSock, (struct sockaddr *)&sa, sizeof(sa));
  return newSock;
}

void CloseSocket(int sock){
  close(sock);
}

int RecvTCP(int sock, void * buffer, long bufferSize){
  int bytesReceived = recv(sock, buffer, bufferSize,0);

  return bytesReceived;
}

int SendTCP(int sock, void * buffer, long bufferSize){
  char messageBuffer[NETWORK_BUFFER_SIZE];
  memset(&messageBuffer,0,NETWORK_BUFFER_SIZE);
  memcpy(&messageBuffer,buffer,bufferSize);

  return send(sock,&messageBuffer,NETWORK_BUFFER_SIZE,0);
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
      printf("ERROR: bad ack On safe send!\n");
      return 0;
    }
    else if (ack[0] == NETWORK_ACK){
      return 1;
    }
    else{
      printf("ERROR: got message when waiting for ack but is not ack!\n %s", ack);
      return 0;
    }
  }

  if (i == NETWORK_RETRY_COUNT){
    printf("ERROR: Safe send retry count exceeded!\n");
    return 0;
  }
  else{
    return 1;
  }
}


int SafeRecv(int sock, void * buffer, long bufferSize, char * ip_out, int * port_out){
  Recv(sock,buffer,bufferSize,ip_out,port_out);

  //send ack
  char ack = NETWORK_ACK;
  Send(sock,&ack,NETWORK_ACK_SIZE,ip_out,port_out);
  return 1;
}


extern int SendFile(int sock, void * buffer, long bufferSize, char * ipAddr, int * portNum){
  char * ptrBuff = buffer;
  int bytesLeft = bufferSize;
  char messageBuffer[NETWORK_BUFFER_SIZE];

  while(bytesLeft > 0){
    ushort writeSize = bytesLeft > (NETWORK_BUFFER_SIZE -3) ? (NETWORK_BUFFER_SIZE -3) : bytesLeft;
    bytesLeft -= writeSize;

    memset(messageBuffer,0,NETWORK_BUFFER_SIZE);
    messageBuffer[0] = FTP_FILE_PACKAGE;
    ushort wSize = htons(writeSize);
    memcpy(messageBuffer + 1, &wSize, 2);
    memcpy(messageBuffer + 3, ptrBuff, writeSize);
    if(!SendTCP(sock,messageBuffer, writeSize + 3)){
      printf("ERROR: Transmision Error when sending file\n");
      return 0;
    }
    ptrBuff += writeSize;
  }

  char ftp_end = FTP_FILE_END;
  if(!SendTCP(sock,&ftp_end,1)){
    printf("ERROR: could not send: FTP_FILE_END");
    return 0;
  }

  return 1;
}


extern int RecvFile(int sock, char ** ptrBuff,char * ipAddr,int * portNum){
  char packetBuffer[NETWORK_BUFFER_SIZE];
  memset(packetBuffer, 0, NETWORK_BUFFER_SIZE);

  //allocate return buffer
  char ** fileBuffer = malloc(sizeof(void *)*NETWORK_FILE_BUFFER_SIZE);
  int fbIndex = 0;

  printf("recieving file....\n");
  while(packetBuffer[0] != FTP_FILE_END){
    memset(packetBuffer, 0, NETWORK_BUFFER_SIZE);
    RecvTCP(sock,packetBuffer, NETWORK_BUFFER_SIZE);
    
    char * buff = malloc(NETWORK_BUFFER_SIZE);
    memcpy(buff, packetBuffer, NETWORK_BUFFER_SIZE);
    fileBuffer[fbIndex++] = buff;
  }
  printf(".complete\n");

  *ptrBuff = malloc((fbIndex-1)*NETWORK_BUFFER_SIZE);
  memset(*ptrBuff,0,(fbIndex-1)*NETWORK_BUFFER_SIZE);
  int pbIndex = 0;
  // reconstruct file
  for(int i =0; i < fbIndex-1; i++){
    int chunkLength = GetChunkLength(fileBuffer[i]);
    if (chunkLength == -1){
      return 0;
    }

    memcpy(*ptrBuff + pbIndex, (fileBuffer[i]) + 3, chunkLength);
    pbIndex += chunkLength;
  }

  return pbIndex;
}

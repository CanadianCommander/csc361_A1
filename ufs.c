#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <zip.h>
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
  char * fileData;
  int fileDataLen = 0;

  while(1){
    memset(message,0, NETWORK_BUFFER_SIZE);
    memset(ipAddr,0,50);
    RecvTCP(socket,message,NETWORK_BUFFER_SIZE);

    if(message[0] == FTP_POST){
      // recv file

      fileDataLen = RecvFile(socket,&fileData,NULL,NULL);

      //decompress the file. turns out libzip cannot decompress
      // in memory buffers :(
      WriteBufferToFile("tmp.foo.bar",fileData,fileDataLen);
      struct zip * pZip = zip_open("tmp.foo.bar",0,NULL);
      struct zip_file * zFile = zip_fopen_index(pZip,0,0);
      struct zip_stat stat;
      zip_stat_index(pZip,0,0,&stat);

      free(fileData);
      fileData = malloc(stat.size);
      memset(fileData,0,stat.size);
      fileDataLen = stat.size;
      zip_fread(zFile,fileData,stat.size);

      zip_fclose(zFile);
      zip_close(pZip);

      system("rm tmp.foo.bar");
    }
    else if(message[0] == FTP_GET){
      // send file back to client
      if(fileDataLen > 0){
        SendFile(socket, fileData, fileDataLen, NULL, NULL);
      }
      else{
        printf("ERROR: no file data to send!\n");
      }
    }
    else if(message[0] == FTP_THANKS){
      printf("Client Says Bye!\n");
      break;
    }
    else{
      printf("ERROR: unknown control sequence %x\n", message[0]);
    }
    fflush(stdout);

  }

}

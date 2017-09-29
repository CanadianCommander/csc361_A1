#ifndef NETWORK_H
#define NETWORK_H

#include <netinet/in.h>

#define NETWORK_BUFFER_SIZE 1024
#define NETWORK_FILE_BUFFER_SIZE 512000 // 4MB, min file buffer (on 64 bit sys)
#define NETWORK_ACK_SIZE 1

// proto spec
// control ---------------
#define FTP_GET 0x00
#define FTP_FOUND 0x01
#define FTP_NOT_FOUND 0x02
#define FTP_FILE_END 0x03
#define FTP_THANKS 0x04
#define NETWORK_ACK 0x05
// ---------------------------
#define FTP_FILE_PACKAGE 0x10

extern short GetChunkLength(char * buff);

extern int CreateServerSocket(char * ipAddr, int port);

extern int CreateClientSocket();

extern void CloseSocket(int sock);

extern int Recv(int sock,void * buffer,long bufferSize, char * ip_out, int * port_out);

extern int Send(int sock, void * buffer, long bufferSize, char * ipAddr, int * portNum);


#define NETWORK_RETRY_COUNT 10
extern int SafeSend(int sock, char * buffer, long bufferSize, char * ipAddr, int * portNum);

extern int SafeRecv(int sock, void * buffer, long bufferSize, char * ip_out, int * port_out);

extern int SendFile(int sock, void * buffer, long bufferSize, char * ipAddr, int * portNum);

extern int RecvFile(int sock, char ** ptrBuff,char * ipAddr, int * portNum);

#endif

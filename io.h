#ifndef IO_H
#define IO_H

extern int WriteBufferToFile(char * strFileName, char * buff, size_t bufferSize);

extern int ReadFileToBuffer(char * strFileName, char * buff, size_t bufferSize);

extern long GetFileSizeBytes(char * strFileName);

extern int FileExists(char * strFileName);

struct IpInfo{
  char * ipAddr;
  int portNumber;
};

#define GET_IP_INFO_BUFFER_SIZE 100
extern int GetIpInfo(char * strConfigFile, struct IpInfo * ipI);

#endif

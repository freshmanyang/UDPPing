/************************************************************************
* File:  SocketHelper.h
*
* Purpose:
*   This include file is for the SocketHelper module
*
* Notes:
*   Code should always exit using Unix convention:  exit(EXIT_SUCCESS) or exit(EXIT_FAILURE)
*
* Last update: 4/30/2019
*************************************************************************/
#ifndef	__SocketHelper_h
#define	__SocketHelper_h

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>


int sendMsg(int sock, void *SendBufPtr, int msgSize, struct sockaddr *dstAddrPtr, socklen_t dstAddrLen);
int RxMsg(int sock, void *RxBufPtr, int msgSize, struct sockaddr *srcAddrPtr, socklen_t *srcAddrLenPtr);

// Create, bind, and listen a new TCP server socket
int SetupTCPServerSocket(const char *service);
// Accept a new TCP connection on a server socket
int AcceptTCPConnection(int servSock);

// Handle new TCP client
uint32_t HandleTCPClient(int clntSocket, int chunkSize);

// Create and connect a new TCP socket
int SetupTCPClientSocket(const char *server, const char *service);
int SetupTCPServerSocket(const char *service);

// Create and connect a new UDP socket
//int SetupUDPClientSocket(const char *server, const char *service);
int SetupUDPClientSocket(const char *server, const char *servPort,struct sockaddr *clntAddrPtr, socklen_t *clntAddrLenPtr);
int SetupUDPServerSocket(const char *service);

int SetSocketOption( int sock, int option, void *optionData, int sizeData);
int GetSocketOption(int sock, int option);


#endif



/*********************************************************
*
* Module Name: Socket Helper routines  
*
* File Name:  SocketHelper.c
*
* Summary:  This holds helpful routines for sockets apps. 
*
* Notes:  see 'man 7 socket' for info on options
*
* Revisions:
*
*  $A1: added support for get/set IP_TOS 
*  
* Last update: 10/6/2019
*
*********************************************************/
#include "common.h"
#include "utils.h"
#include "AddressHelper.h"
#include "SocketHelper.h"


//#define TRACE 1

/*******************************************
*
*   The following are for either client or server programms
*
****************************************************/

/***********************************************************
* Function: int RxMsg(int sock, void *RxBufPtr, int msgSize, (struct sockaddr *)srcAddrPtr, (socklen_t *)srcAddrLenPtr)
*
* Explanation:  This receives a msg over the socket.  
*
* inputs:   
*   int sock : socket descriptor
*   void *RxBufPtr : ptr to callers buffer data is transferred to
*   int  msgSize : max size of the requested msg
*   (struct sockaddr *)srcAddrPtr:   the clients (remote hosts) socket address
*   int *srcAddrLenPtr  :  client's socket address size 
*
* outputs:
*      returns EXIT_FAILURE or number of bytes received
*      
*    The caller's RxBuffer is filled in (pointed to by RxBufPtr) -up to msgSize bytes.
*    The caller's client address and address len is filled in (pointed to by clntAddrPtr and clntAddrLenPtr )
*
***************************************************************/
int RxMsg(int sock, void *RxBufPtr, int msgSize, struct sockaddr *srcAddrPtr, socklen_t *srcAddrLenPtr)
{

int rc = EXIT_SUCCESS; 

#ifdef TRACE 
  printf("RxMsg: Sock:%d To Rx up to  %d bytes \n",sock, msgSize);
#endif

//       ssize_t recvfrom(int sockfd, void *buf, size_t len, int flags,
//                        struct sockaddr *src_addr, socklen_t *addrlen);


 rc  = (ssize_t) recvfrom(sock, RxBufPtr,msgSize, 0,srcAddrPtr, srcAddrLenPtr);
 if (rc < 0)
 {
  printf("RxMsg:  recvfrom failed,  msgSize:%d,  errno:%d \n", msgSize, errno);
  rc = EXIT_FAILURE;
 } else {
#ifdef TRACE 
  printf("RxMsg: Rxed msgSize:%d \n", msgSize);
#endif
  }

  return rc;
}


/***********************************************************
* Function: int sendMsg(int sock, void *SendBufPtr, int msgSize, (struct sockaddr *)dstAddrPtr, int dstAddrLen)
*
* Explanation:  This receives a msg over the socket.  
*
* inputs:   
*   int sock : socket descriptor
*   void *SendBufPtr : ptr to callers buffer data is transferred to
*   int  msgSize : max size of the requested msg
*   (struct sockaddr *)dstAddrPtr: dst (remote hosts) socket address
*   int *dstAddrLenPtr  :  dst socket address size 
*
* outputs:
*      returns EXIT_FAILURE or EXIT_SUCCESS 
*      
***************************************************/
int sendMsg(int sock, void *SendBufPtr, int msgSize, struct sockaddr *dstAddrPtr, socklen_t dstAddrLen)
{

int rc = EXIT_SUCCESS; 

#ifdef TRACE 
  printf("sendMsg: Entry:  sock:%d,  msgSize:%d \n", sock,msgSize);
#endif
  rc = (ssize_t)sendto(sock, SendBufPtr, (size_t)msgSize, 0, (const struct sockaddr *)dstAddrPtr, dstAddrLen);
  if (rc < 0) {
    printf("sendMsg:  sendto failed, rc:%d  msgSize:%d,  errno:%d \n", rc, msgSize, errno);
    rc = EXIT_FAILURE;
  }
  else {
    if (rc != msgSize) {
      printf("sendMsg:  sent unexpected number of bytes:%d  msgSize:%d,   errno:%d \n", 
            rc, msgSize, errno);
      rc = EXIT_FAILURE;
    }
    else {
#ifdef TRACE 
      printf("sendMsg: Succeeded to send %d bytes \n",rc);
#endif
      rc = EXIT_SUCCESS;
    }
  }

  return rc;
}



/***********************************************************
* Function: int SetSocketOptions( int sock, int option, void *optionData, int sizeData)
*
* Explanation:  This sets socket options 
*
* inputs:   
*      int sock
*      int option
*      void *optionData
*
* outputs: returns a ERROR or NOERROR
*
* Details:
*    For IP_QOS See 
*       -Original and now antiquated ToS bits: RFC 1349
*       -We use the IPTOS_CLASS values from RFC 2474 which
*          introduced diffserv code points.
*            See :  https://tools.ietf.org/html/rfc2474
*            See : https://tools.ietf.org/html/rfc1812
*                  Section 5.3.3 
*          The defines are in /usr/include/netinet/ip.h
*
*  These are the Class of Service bits: 
*        Values goes from low to high pri
*    IPTOS_CLASS_CS0  : for best effort / default (0x00)
*    IPTOS_CLASS_CS1  : for medium priority traffic (0x20,binary 00100000, decimal 32)
*    IPTOS_CLASS_CS2  : for high priority data  (0x40, 64)
*    IPTOS_CLASS_CS3  : for voice/video signaling (0x60, 01100000, 96)
*    IPTOS_CLASS_CS4  : for video                  (0x80, 128)
*    IPTOS_CLASS_CS5  : for voice                  (0xa0, 10100000, 160)
*  
**************************************************/
int SetSocketOption( int sock, int option, void *optionData, int sizeData)
{

int rc = NOERROR;
int optionValue  = 0;
//Used as a generic input -value should not matter
static int enable = 1;
static unsigned char ttl = 32; //Used only for multicast

#ifdef TRACE 
  printf("SetSockOption: Sock:%d  option:%d  dataLen:%d \n",
      sock,option,sizeData);
#endif

  switch (option) {

    case SO_BINDTODEVICE:
      rc =  setsockopt(sock,SOL_SOCKET,SO_BINDTODEVICE, optionData,sizeData);
      if (rc < 0)
      {
        printf("SetSocketOption:  failed, SO_BINDTODEVICE IF:%s  errno:%d \n",(char *) optionData, errno);
        rc = EXIT_FAILURE;
      }
      break;

    case SO_BROADCAST:
      rc = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, optionData, sizeData);
      if (rc < 0)
      {
        printf("SetSocketOption:  failed, SO_BROADCAST  errno:%d \n", errno);
        rc = EXIT_FAILURE;
      }
#ifdef TRACE 
      else {
         printf("SetSockOption: SUCCEEDED to set BROADCAST Option \n");
      }
#endif
      break;

    case IP_MULTICAST_IF:
//Can also add a route 
// route add -net 224.0.0.0 netmask 240.0.0.0 dev eth0
      rc= setsockopt(sock,IPPROTO_IP,IP_MULTICAST_IF, optionData, sizeData);
      if (rc < 0)
      {
        printf("SetSocketOptions:  failed, IP_MULTICAST_IF   errno:%d \n", errno);
        rc = EXIT_FAILURE;
      }
      setsockopt(sock,IPPROTO_IP,IP_MULTICAST_TTL,&ttl,sizeof(ttl));

      break;

    case IP_TOS:
      rc = setsockopt(sock, IPPROTO_IP,IP_TOS, optionData,sizeData);
      if (rc < 0)
      {
        printf("SetSocketOptions:  failed, IP_TOS   errno:%d \n", errno);
        rc = EXIT_FAILURE;
      }
      break;

    case SO_REUSEADDR:
      rc = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, optionData, sizeData);
      if (rc < 0)
      {
        printf("SetSocketOptions:  failed, SO_REUSEADDR  errno:%d \n", errno);
        rc = EXIT_FAILURE;
      }
#ifdef TRACE 
      else {
         printf("SetSockOption: SUCCEEDED to set REUSEADDR  Option \n");
      }
#endif
      break;

    case SO_RCVBUF:
      rc = setsockopt(sock, SOL_SOCKET, SO_RCVBUF, optionData, sizeData);
      if (rc < 0)
      {
        printf("SetSocketOptions:  failed SO_RCVBUF  errno:%d \n", errno);
        rc = EXIT_FAILURE;
      }
      break;

    case SO_SNDBUF:
      rc = setsockopt(sock, SOL_SOCKET, SO_SNDBUF, optionData, sizeData);
      if (rc < 0)
      {
        printf("SetSocketOptions:  failed SO_SNDBUF  errno:%d \n", errno);
        rc = EXIT_FAILURE;
      }
      break;

    default:
      printf("SetSocketOptions:  failed  Unknown option :%d  \n", option);
      rc = EXIT_FAILURE;
      break;
  }

  if (rc == EXIT_FAILURE)
    perror("SetSocketOptions:  failed:  \n");

  return rc;
}


/***********************************************************
* Function: int GetSocketOption( int sock, int option)
*
* Explanation:  This sets socket options 
*
* inputs:   
*      int sock
*      int option
*
* Adding:
*    case IP_TOS:
*    case IP_MTU;
*
* outputs: returns a EXIT_FAILURE or a  valid option value
*
**************************************************/
int GetSocketOption(int sock, int option)
{

int optionValue  = 0;
int rc = EXIT_SUCCESS;
int len = sizeof(optionValue);

  switch (option) {

    case IP_TOS:
      //if (getsockopt(sockfd, IPPROTO_IP, IP_TOS,&option, &optlen) < 0)
      //  err_sys("IP_TOS getsockopt error");
      rc = getsockopt( sock, IPPROTO_IP,IP_TOS, (void *) &optionValue,(socklen_t *) &len );
      if (rc < 0) {
        printf("GetSocketOption:  failed IP_TOS  errno:%d \n", errno);
        rc = EXIT_FAILURE;
      }

      break;

    //case IP_MTU;
    //  break;

    case SO_RCVBUF:
      rc = getsockopt( sock, SOL_SOCKET, SO_RCVBUF, (void *) &optionValue,(socklen_t *) &len );
      if (rc < 0) {
        printf("GetSocketOption:  failed SO_RCVBUF  errno:%d \n", errno);
        rc = EXIT_FAILURE;
      }
      break;

    case SO_SNDBUF:
      rc = getsockopt( sock, SOL_SOCKET, SO_SNDBUF, (void *) &optionValue,(socklen_t *) &len );
      if (rc < 0) {
        printf("GetSocketOption:  failed SO_SNDBUF  errno:%d \n", errno);
        rc = EXIT_FAILURE;
      }
      break;

    default:
      rc = EXIT_FAILURE;
      printf("GetSocketOption:  failed  Unknown option :%d  \n", option);
      break;
  }

  if (rc != EXIT_FAILURE)
    rc = optionValue;

  return rc;
}



/*******************************************
*
*   The following are for client side programms
*
****************************************************/


/***********************************************************
* Function: int SetupUDPClientSocket(const char *server, const char *servPort,(struct sockaddr *)&clntAddrPtr, &clntAddrLenPtr)
*
* Explanation:  This sets up the UDP Socket by a client 
*
* inputs:   
*     const char *host:  ptr to a string holding the name
*     const char *service  : ptr to a string holding the text name of the service
*                    or the string repr. of the actual port number.
*      (struct sockaddr *)&clntAddrPtr: callers buffer to hold the client address selected 
*       &clntAddrLenPtr : ptr to callers buffer to hold the client address size
*
* outputs: returns a EXIT_FAILURE (-1) or valid sock descriptor
*
* notes: 
*
*
**************************************************/
int SetupUDPClientSocket(const char *server, const char *servPort,struct sockaddr *clntAddrPtr, socklen_t *clntAddrLenPtr)
{

int rc = EXIT_SUCCESS;
int sock = -1;
struct addrinfo *selectedAddr = NULL;  //the selected address;
int  numberAddrChoices = 0;
struct addrinfo *addr;
 

// Tell the system what kind(s) of address info we want
  struct addrinfo addrCriteria;                   // Criteria for address match

  memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
//  addrCriteria.ai_family = AF_UNSPEC;             // Any address family
  addrCriteria.ai_family = AF_UNSPEC;             // Any address family
  // For the following fields, a zero value means "don't care"
  addrCriteria.ai_socktype = SOCK_DGRAM;          // Only datagram sockets
  addrCriteria.ai_protocol = IPPROTO_UDP;         // Only UDP protocol

  // Get address(es)
  struct addrinfo *addrList; // List of server addresses
  rc = getaddrinfo(server, servPort, &addrCriteria, &addrList);
  if (rc != 0)
    DieWithUserMessage("getaddrinfo() failed", gai_strerror(rc));
  else
    rc = EXIT_SUCCESS;

  // Display returned addresses
  numberAddrChoices =  NumberOfAddresses(addrList);
#ifdef TRACE 
  if (numberAddrChoices > 0)
    printf("SetupUDPClientSocket: Found %d possible addresses, first family:%d size:%d \n", 
           numberAddrChoices,addrList->ai_family,addrList->ai_addrlen);
  else 
    printf("SetupUDPClientSocket: WARNING Found %d possible addresses \n", numberAddrChoices);

  for (addr = addrList; addr != NULL; addr = addr->ai_next) {
    PrintSocketAddress(addr->ai_addr, stdout);
    fputc('\n', stdout);
  }
#endif

  // select the address... Create a datagram/UDP socket
  // method:  first on the list
  selectedAddr = addrList;

  //Set callers client address info
  memcpy((void *)clntAddrPtr, (void *)selectedAddr->ai_addr,(size_t) selectedAddr->ai_addrlen);
  *clntAddrLenPtr = selectedAddr->ai_addrlen;

  // Socket descriptor for client
  sock = socket(selectedAddr->ai_family, selectedAddr->ai_socktype, selectedAddr->ai_protocol); 

  if (sock < 0) {
    rc = EXIT_FAILURE;
#ifdef TRACE 
    printf("SetupUDPClientSocket: exit with ERROR,   errno:%d \n", errno);
#endif
  }
  else{ 
    rc = sock;
#ifdef TRACE 
    printf("SetupUDPClientSocket: exit with success,returning sock descriptor:%d   \n", sock);
#endif
  }

  freeaddrinfo(addrList); // Free addrinfo allocated in getaddrinfo()

  return rc;

}



/***********************************************************
* Function: int SetupTCPClientSocket(const char *host, const char *service) 
*
* Explanation:  This attempts to initiate the TCP Cx must be called by the main program
*               to set the Version. 
* inputs:   
*     const char *host:  ptr to a string holding the name
*     const char *service  : ptr to a string holding the text name of the service
*                    or the string repr. of the actual port number.
*
* outputs: returns a EXIT_FAILURE (-1) or valid sock descriptor
*
* notes: 
*   First step, the input params are used to create
*     one or more valid server IP addres/port.
*
* For now....we limit to IPv4 addressing.
*
**************************************************/
int SetupTCPClientSocket(const char *server, const char *servPort) 
{

int rc = EXIT_SUCCESS;
int bitmap = 0;
int  numberAddrChoices = 0;
int sock = -1;
struct addrinfo *addr;

//  bitmap |= AI_CANONNAME;

// Tell the system what kind(s) of address info we want
  struct addrinfo addrCriteria;                   // Criteria for address match
  memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
//  addrCriteria.ai_flags = bitmap;             //bitmap: AI_PASSIVE, AI_CANONNAME, AI_NUMERICHOST,AI_ADDRCONFIG,AI_V4MAPPED 
  addrCriteria.ai_family = AF_UNSPEC;             // AF_INET, AF_INET6, AF_UNSPEC 
  addrCriteria.ai_socktype = SOCK_STREAM;         // SOCK_STREAM, SOCK_DGRAM, or 0 for all
  addrCriteria.ai_protocol = IPPROTO_TCP;         //IPPROTO_TCP, IPROTO_DGRAM, or 0 for all

#ifdef TRACE 
  printf("SetupTCPClientSocket: entered, Host:%s, service: %s  \n",
               server, servPort);
#endif

  // Get address(es)
  struct addrinfo *addrList; // List of server addresses
  rc = getaddrinfo(server, servPort, &addrCriteria, &addrList);
  if (rc != 0)
    DieWithUserMessage("getaddrinfo() failed", gai_strerror(rc));
  else
    rc = EXIT_SUCCESS;

  // Display returned addresses
  numberAddrChoices =  NumberOfAddresses(addrList);
#ifdef TRACE 
  printf("SetupTCPClientSocket: Found %d possible addresses \n", numberAddrChoices);
#endif
  for (addr = addrList; addr != NULL; addr = addr->ai_next) {
//  for (struct addrinfo *addr = addrList; addr != NULL; addr = addr->ai_next) {
    PrintSocketAddress(addr->ai_addr, stdout);
    fputc('\n', stdout);
  }

  // select the address... Create a datagram/UDP socket
  // method:  first to work starting with first  on the list

  int counter=0;
  for (addr = addrList; addr != NULL; addr = addr->ai_next) {
//  for (struct addrinfo *addr = addrList; addr != NULL; addr = addr->ai_next) {
    // Create a reliable, stream socket using TCP
    counter++;
#ifdef TRACE 
    printf("SetupTCPClientSocket:loop(%d): try:family:%d socktype:%d, protocol:%d \n ", 
     counter,(int) addr->ai_family, (int) addr->ai_socktype, (int) addr->ai_protocol);
#endif
    sock = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (sock < 0)
      continue;  // Socket creation failed; try next address

    // Establish the connection to the echo server
    if (connect(sock, addr->ai_addr, addr->ai_addrlen) == 0)
      break;     // Socket connection succeeded; break and return socket

    close(sock); // Socket connection failed; try next address
    sock = -1;
  }

  freeaddrinfo(addrList); // Free addrinfo allocated in getaddrinfo()
  return sock;
}


/*******************************************
*
*   The following are for server side programms
*
****************************************************/

static const int MAXPENDING = 5; // Maximum outstanding connection requests

/***********************************************************
* Function: int SetupTCPServerSocket(const char *service) {
*
* Explanation:  This setups the server side
* inputs:   
*     const char *service  : ptr to a string holding the text name of the service
*                    or the string repr. of the actual port number.
*
* outputs: returns a EXIT_FAILURE (-1) or valid sock descriptor
*
* notes: 
*   First step, the input params are used to create
*     one or more valid server IP addres/port.
*
* For now....we limit to IPv4 addressing.
*
**************************************************/
int SetupTCPServerSocket(const char *service) 
{
int rc = EXIT_SUCCESS;
struct addrinfo *selectedAddr = NULL;  //the selected address;
int servSock = -1;
int  numberAddrChoices = 0;
struct addrinfo *addr;

  // Construct the server address structure
  struct addrinfo addrCriteria;                   // Criteria for address match
  memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
  addrCriteria.ai_family = AF_UNSPEC;             // Any address family
  addrCriteria.ai_flags = AI_PASSIVE;             // Accept on any address/port
  addrCriteria.ai_socktype = SOCK_STREAM;         // Only stream sockets
  addrCriteria.ai_protocol = IPPROTO_TCP;         // Only TCP protocol

  struct addrinfo *addrList; // List of server addresses
  rc= getaddrinfo(NULL, service, &addrCriteria, &addrList);
  if (rc != 0)
    DieWithUserMessage("getaddrinfo() failed", gai_strerror(rc));
  else
    rc = EXIT_SUCCESS;

  // Display returned addresses
  numberAddrChoices =  NumberOfAddresses(addrList);
#ifdef TRACE 
  printf("SetupTCPServerSocket: Found %d possible addresses \n", numberAddrChoices);
#endif
  // Display returned addresses
  for (addr = addrList; addr != NULL; addr = addr->ai_next) {
//  for (struct addrinfo *addr = addrList; addr != NULL; addr = addr->ai_next) {
    PrintSocketAddress(addr->ai_addr, stdout);
    fputc('\n', stdout);
  }



  // select the address... method is to use the first that works starting with first on list
  for (addr = addrList; addr != NULL; addr = addr->ai_next) {
//  for (struct addrinfo *addr = addrList; addr != NULL; addr = addr->ai_next) {
    // Create a TCP socket
    servSock = socket(addr->ai_family, addr->ai_socktype,
        addr->ai_protocol);

    if (servSock < 0)
      continue;       // Socket creation failed; try next address

    //set socket option to reuse the address
    int enable = 1;
    if (setsockopt(servSock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
       DieWithUserMessage("setsockopt(SO_REUSEADDR)", "failed");

    // Bind to the local address and set socket to listen
    if ((bind(servSock, addr->ai_addr, addr->ai_addrlen) == 0) &&
        (listen(servSock, MAXPENDING) == 0)) {
      // Print local address of socket
      struct sockaddr_storage localAddr;
      socklen_t addrSize = sizeof(localAddr);

      if (getsockname(servSock, (struct sockaddr *) &localAddr, &addrSize) < 0)
        DieWithSystemMessage("getsockname() failed");

      fputs("Binding to ", stdout);
      PrintSocketAddress((struct sockaddr *) &localAddr, stdout);
      fputc('\n', stdout);

      break;       // Bind and listen successful
    }

    close(servSock);  // Close and try again
    servSock = -1;
  }

  // Free address list allocated by getaddrinfo()
  freeaddrinfo(addrList);

  return servSock;
}

/***********************************************************
* Function: int AcceptTCPConnection(int servSock) 
*
* Explanation:  This setups the server side
* inputs:   
*     const char *service  : ptr to a string holding the text name of the service
*                    or the string repr. of the actual port number.
*
* outputs: returns a EXIT_FAILURE (-1) or valid sock descriptor
*
* notes: 
*   First step, the input params are used to create
*     one or more valid server IP addres/port.
*
* For now....we limit to IPv4 addressing.
***************************************************/
int AcceptTCPConnection(int servSock) 
{
  struct sockaddr_storage clntAddr; // Client address
  // Set length of client address structure (in-out parameter)
  socklen_t clntAddrLen = sizeof(clntAddr);

  // Wait for a client to connect
  int clntSock = accept(servSock, (struct sockaddr *) &clntAddr, &clntAddrLen);
  if (clntSock < 0)
    DieWithSystemMessage("accept() failed");

  return clntSock;
}

/***********************************************************
* Function: uint32_t HandleTCPClient(int clntSocket, int chunkSize) 
*
* Explanation:  This handles the TCP client.  
*
* inputs:   
*     int clntSocket: client socket desc
*      int chunkSize  :  chunkSize of inner loop
*
* outputs:
*   returns number bytes sent 
*
***************************************************************/
uint32_t HandleTCPClient(int clntSocket, int chunkSize) 
{
char buffer[MAX_TMP_BUFFER]; // Buffer for echo string
uint32_t transferSize=0;
uint32_t totalSent=0;
int bytesToSend = 0;
uint32_t numBytes = 0;
ssize_t numBytesSent = 0;

  // Receive message from client
  ssize_t numBytesRcvd = recv(clntSocket, buffer, MAX_TMP_BUFFER, 0);
  if (numBytesRcvd < 0)
    DieWithSystemMessage("recv() failed");

#ifdef TRACE 
  printf("HandleTCPClient: entered, chunkSize:%d  transferSize:%d  \n",
            chunkSize,transferSize);
#endif

  buffer[numBytesRcvd] = '\0';  
  numBytes = atoi(buffer);
  transferSize = numBytes;
  char *writeBuff = (char *)malloc(numBytes*sizeof(char));
  // Send received string and receive again until end of stream
  // printf("Req: %d B\n", numBytes);
  while (numBytes > 0) 
  { 
    // 0 indicates end of stream
    bytesToSend = (numBytes - chunkSize) > 0 ? chunkSize : numBytes;
    numBytesSent = send(clntSocket, writeBuff, bytesToSend, 0);
    if (numBytesSent < 0)
      DieWithSystemMessage("send() failed");
    else if (numBytesSent != bytesToSend)
      DieWithUserMessage("send()", "sent unexpected number of bytes");

    totalSent+=numBytesSent;
    // printf("Sent: %d bytes\n", (int)numBytesSent);
    numBytes -= numBytesSent;
    // printf("Left: %d bytes\n", numBytes);
  }
  fflush(stdout);
  free(writeBuff);

#ifdef TRACE 
  printf("HandleTCPClient: exit....transferSize:%d, totalSent:%d  \n",
            transferSize,totalSent);
#endif

  close(clntSocket); // Close client socket
  return totalSent;
}




/***********************************************************
* Function: int SetupUDPServerSocket(const char *service) 
*
* Explanation:  This sets up a UDP server socket 
*
* inputs:   
*     const char *service  : ptr to a string holding the text name of the service
*                    or the string repr. of the actual port number.
*
* outputs: returns a EXIT_FAILURE (-1) or valid sock descriptor
*
* notes: 
*
**************************************************/
int SetupUDPServerSocket(const char *service) 
{

int rc = EXIT_SUCCESS;
struct addrinfo *selectedAddr = NULL;  //the selected address;
int sock = -1;
int  numberAddrChoices = 0;
struct addrinfo *addr;

  // Construct the server address structure
  struct addrinfo addrCriteria;                   // Criteria for address
  memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
//  addrCriteria.ai_family = AF_UNSPEC;             // Any address family
  addrCriteria.ai_family = AF_INET6;             // Any address family
  addrCriteria.ai_flags = AI_PASSIVE;             // Accept on any address/port
  addrCriteria.ai_socktype = SOCK_DGRAM;          // Only datagram socket
  addrCriteria.ai_protocol = IPPROTO_UDP;         // Only UDP socket

  struct addrinfo *addrList; // List of server addresses
  rc = getaddrinfo(NULL, service, &addrCriteria, &addrList);
  if (rc != 0)
    DieWithUserMessage("getaddrinfo() failed", gai_strerror(rc));
  else 
    rc = EXIT_SUCCESS;

  // Display returned addresses
  numberAddrChoices =  NumberOfAddresses(addrList);
#ifdef TRACE 
  printf("SetupUDPServerSocket: Found %d possible addresses \n", numberAddrChoices);
  // Display returned addresses
  for (addr = addrList; addr != NULL; addr = addr->ai_next) {
    PrintSocketAddress(addr->ai_addr, stdout);
    fputc('\n', stdout);
  }
#endif

  // select the address... Create a datagram/UDP socket
  selectedAddr = addrList;

  // Create socket for incoming connections
  sock = socket(selectedAddr->ai_family, selectedAddr->ai_socktype,
      selectedAddr->ai_protocol);


  if (sock < 0)
    rc = EXIT_FAILURE;
  else  {

    //set socket option to reuse the address
    //int enable = 1;
    //if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0)
    //   DieWithUserMessage("setsockopt(SO_REUSEADDR)", "failed");

    // Bind to the local address , returns -1 on error else 0
    rc = bind(sock, selectedAddr->ai_addr, selectedAddr->ai_addrlen);
    if (rc == 0) {
      PrintSocketAddress(selectedAddr->ai_addr, stdout);
      fputc('\n', stdout);
      rc = EXIT_SUCCESS;
    }
    else {
      rc = EXIT_FAILURE;
      printf("SetupUDPServerSocket: Error on bind  rc:%d \n", rc);
      perror("SetupUDPServerSocket  failed:  \n");
    }
  }


  if (rc == EXIT_SUCCESS) {
    rc = sock;
#ifdef TRACE 
    printf("SetupUDPServerSocket: exit with success,returning sock descriptor:%d   \n", sock);
#endif
  }
  else {
#ifdef TRACE 
    printf("SetupUDPServerSocket: exit with ERROR,   errno:%d \n", errno);
#endif
  }

  // Free address list allocated by getaddrinfo()
  freeaddrinfo(addrList);
  return rc;
}


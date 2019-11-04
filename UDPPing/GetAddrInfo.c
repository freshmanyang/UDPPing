/*********************************************************
*
* Module Name: GetAddrInfo 
*
* File Name:  GetAddrInfo.c 
*
* Params:
*       host name:  dotted decimal v4 or v6 format or dns name
*       service:    port number
*
* Summary:  This is a simple illustration of how to 
*           use the getaddrinfo sockets call.
*
* Notes: 
*  addrCriteria.ai_socktype = SOCK_STREAM;         // Only stream sockets
*  addrCriteria.ai_protocol = IPPROTO_TCP;         // Only TCP protocol
*    appears redundant -  it was done as it is possible for their to be more > 1 STREAM
*             protocols.
*
* Invocation example:
*
*   the hints socktype and protocol were set to 0....
* GetAddrInfo ada8.computing.clemson.edu 23
* GetAddrInfo: numberIPs:3 for name:ada8.computing.clemson.edu 
* 130.127.48.229-23 socktype:1 protocol:6 
* 130.127.48.229-23 socktype:2 protocol:17 
* 130.127.48.229-23 socktype:3 protocol:0 
*   Note: socktype 1,2,3: SOCK_STREAM, SOCK_DGRAM, SOCK_RAW
*     SOCK_* defined in socket_type.h located in 
#        /usr/include/x86_64-linux-gnu/bits
*      socket.h defines the PF_* values
*        protocol 6, 17, 0 : PF_NETROM,PF_PACKET  , Unspecified
*                       PF_PACKET is similar to RAW
*
* Last update: 9/22/2019
*
*********************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include "./commonCode/common.h"
#include "./commonCode/AddressHelper.h"

#define TRACEME 1

int main(int argc, char *argv[]) {
  int rc = NOERROR;
  int numberIPs = 0;

  if (argc != 3) // Test for correct number of arguments
    DieWithUserMessage("Parameter(s)", "<Address/Name> <Port/Service>");

  char *addrString = argv[1];   // Server address/name
  char *portString = argv[2];   // Server port/service

  // Tell the system what kind(s) of address info we want
  struct addrinfo addrCriteria;                   // Criteria for address match
  memset(&addrCriteria, 0, sizeof(addrCriteria)); // Zero out structure
  addrCriteria.ai_family = AF_UNSPEC;             // Any address family
  addrCriteria.ai_socktype = 0;         // do not care 
  addrCriteria.ai_protocol = 0;        // do not care
  //addrCriteria.ai_socktype = SOCK_STREAM;         // Only stream sockets
  //addrCriteria.ai_protocol = IPPROTO_TCP;         // Only TCP protocol

  // Get address(es) associated with the specified name/service
  struct addrinfo *addrList; // Holder for list of addresses returned
  // Modify servAddr contents to reference linked list of addresses
  rc = getaddrinfo(addrString, portString, &addrCriteria, &addrList);
  if (rc != NOERROR) {
    printf("getaddrinfo() failed (rc:%d), error:%s\n", rc, gai_strerror(rc));
    numberIPs= 0;
    rc = ERROR;
    exit(EXIT_FAILURE);
  } else 
  {
    numberIPs= NumberOfAddresses(addrList);

//#ifdef TRACEME 
    printf("%s: numberIPs:%d for name:%s \n",argv[0], numberIPs, addrString);
//#endif

  // Display returned addresses
   struct addrinfo *addr;
   for (addr = addrList; addr != NULL; addr = addr->ai_next) {
    PrintSocketAddress(addr->ai_addr, stdout);
    printf(" socktype:%d protocol:%d ",  addr->ai_socktype,addr->ai_protocol);
    fputc('\n', stdout);
   }

   freeaddrinfo(addrList); // Free addrinfo allocated in getaddrinfo()

   exit(EXIT_SUCCESS);
  }
}

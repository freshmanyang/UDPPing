/************************************************************************
* File:  AddressHelper.h
*
* Purpose:
*   This is the include file for the AddressHelper module
*
* Notes:
*   Code should always exit using Unix convention:  exit(EXIT_SUCCESS) or exit(EXIT_FAILURE)
*
************************************************************************/
#ifndef	__AddressHelper_h
#define	__AddressHelper_h

#include <stdio.h>
#include <string.h>
#include <stdbool.h>
//#include <arpa/inet.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>



int NumberOfAddresses(struct addrinfo *addrList);
// Print socket address
void PrintSocketAddress(const struct sockaddr *address, FILE *stream);
// Test socket address equality
bool SockAddrsEqual(const struct sockaddr *addr1, const struct sockaddr *addr2);

#ifndef LINUX
#define INADDR_NONE 0xffffffff
#endif


#endif



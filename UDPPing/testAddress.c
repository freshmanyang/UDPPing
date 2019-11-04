/*********************************************************
*
* Module Name: testAddress
*
* File Name:  testAddress.c 
*
* Simple test illustration of routines:
*    inet_pton and inet_ntop
*
*       inet_pton - convert IPv4 and IPv6 addresses from text to binary form
*       inet_ntop - convert IPv4 and IPv6 addresses from binary to text form
*
*
* Params:
*   address family :  i4|i6| <num> :  the string i6 causes the domain to be IPV6. 
*                      Or if for some reason wanted a different domain, enter a number.
*   name            
*
* Summary:  This is a simple illustration of how to 
*           use the getaddrinfo sockets call.
*
* Invocation example:
*    testAddress i6 fe80::bf01:daec:9018:3c7b
*    testAddress: 3 params: i6 fe80::bf01:daec:9018:3c7b 
*     fe80::bf01:daec:9018:3c7b
*
* Last update: 9/21/2019
*
*********************************************************/
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TRACEME 1
int main(int argc, char *argv[])
{
  unsigned char buf[sizeof(struct in6_addr)];
  int domain, s;
  char str[INET6_ADDRSTRLEN];

  if (argc != 3) {
    fprintf(stderr, "Usage: %s {i4|i6|<num>} string\n", argv[0]);
    exit(EXIT_FAILURE);
  }

#ifdef TRACEME
  printf("%s: %d params: %s %s \n",argv[0],argc, argv[1], argv[2]); 
#endif
  domain = (strcmp(argv[1], "i4") == 0) ? AF_INET :
           (strcmp(argv[1], "i6") == 0) ? AF_INET6 : atoi(argv[1]);

  s = inet_pton(domain, argv[2], buf);
  if (s <= 0) {
     if (s == 0)
        fprintf(stderr, "Not in presentation format");
     else
        perror("inet_pton");

     exit(EXIT_FAILURE);
  }

  domain = (strcmp(argv[1], "i4") == 0) ? AF_INET :
           (strcmp(argv[1], "i6") == 0) ? AF_INET6 : atoi(argv[1]);

  s = inet_pton(domain, argv[2], buf);
  if (s <= 0) {
    if (s == 0)
      fprintf(stderr, "Not in presentation format");
    else
      perror("inet_pton");
    exit(EXIT_FAILURE);
  }

  if (inet_ntop(domain, buf, str, INET6_ADDRSTRLEN) == NULL) {
    perror("inet_ntop");
    exit(EXIT_FAILURE);
  }

  printf("%s\n", str);

  exit(EXIT_SUCCESS);
}



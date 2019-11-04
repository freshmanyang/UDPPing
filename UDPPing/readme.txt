
********************************************************
* UDPPing:  
*    A program that is similar to ping - it used UDP
*    rather than ICMP.  Also, it does not support any
*    of the options of ping.
*  
* NOTE: This code requires the symbolic link to commonCode to 
*       exist!!
*
*  Contact:  Dr Jim Martin (jmarty@clemson.edu)
*
*
*  last update: 9/20/2019
*
********************************************************

Make.defines :  included by the Makefile

The Makefile builds 3 programs:
   (Issue make 'print-PROGS' to see the Makefile variable PROGS )
GetAddrInfo -  the simple front end example program to getaddrinfo
UDPPingClient - the client
UDPPingServer - the server


Source code:

GetAddrInfo.c : main program for GetAddrInfo


UDPPingClient.c : client main program
UDPPingServer.c :  server main program

The following are symbolically linked to the client and server 
source files resp (just to reduce typing)
c.c
s.c

(Note:  this is how i linked c.c to UDPPingClient.c
       ln -s UDPPingClient.c  c.c 
)

The client and server use some routines that are located in commonCode.
Notice that commonCode is symbolically linked to ../commonCode.
This is so other programs in the perfTool directory can access
the commonCode in a similar manner.


NOTE: We use only a fraction of the code in commonCode.  
      



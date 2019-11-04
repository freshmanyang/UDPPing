********************************************************
*  readme.txt for perfTool  V8
*
*  Last update: 6/20/2019
*
*  Refer to resultsOutput.txt for a summary of output files
*
*  To run on localhost

Readme.txt :   ./commonCode
*                  sudo ./perfClient 192.168.2.255 5000 
*                            20000000 1000 1 0 0 1 0 0 ALL 1 

********************************************************

Networking support code:
AddressHelper.c
clientRx.c
netHelper.c
serialization.c
serverTx.c
session.c
SocketHelper.c
TCPHelper.c

Helper code that gets information about the node
sysInfoHelper.c
exampleGetIF.c

This includes routines that learn info about the node
Also other misc helper routines
utils.c

gpsdHelper.c
gpsdStubs.c


testTimeHelper.c
timeHelper.c
delayHelper.c

messages.c



4/16/2019

  Added ln -s ../../UnixTextBookCode/stevensBooks/unpv12e/ stevens-unpv12e

  cd ./stevens-unpv12e/sock     ;  edits main.c 

  TOS code in sockopt.c
  Also, use main.c  as the example to use when incorporating getopt  for config params



 3/26/2019

This directory holds "C" code that is used in programs in netlabrepo.
The approach is for each program to have a updateCommon.sh that copies
 a local ./common dir with any code from commonCode.

WARNING:  Be careful!!  Any changes to this common code should be tested 
   in all programs and then put back into commonCode.

   The idea is to NOT maintain a dozen different utils.c (and other files .... )

Notes and TODOs :

-Started out cp TCPHelper.* t SocketsHelper*.  Need to update TxRxPipes 
     Makefile to use this new filename. 
   I just rm'ed TCPHelper.* - 
-Need to modify TxRxPipes code to call a script to populate its common dir correctly.
-Each program must maintain their own version.h - removed it from this commonCode




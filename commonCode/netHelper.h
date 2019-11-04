/************************************************************************
* File:  netHelper.h 
*
* Purpose:
*   This include file is for the netHelper module
*
* Notes:
*   Code should always exit using Unix convention:  exit(EXIT_SUCCESS) or exit(EXIT_FAILURE)
*
* Last update:  8/1/2019
*
*************************************************************************/
#ifndef	__netHelper_h
#define	__netHelper_h

#include "common.h"
#include <sys/types.h>
#include <ifaddrs.h>
#include <sys/ioctl.h>

#ifdef LINUX
#include <linux/wireless.h>
#else
#define IFNAMSIZ 32
#endif

struct IFInfoStruct {
  double Timestamp;
  char *IFName;
  bool isBroadcastAddr;
  bool isWireless; 
  double lastRxByteCount;
  double lastTxByteCount;
  union {
    struct sockaddr *ifu_broadaddr;
    struct sockaddr *ifu_dstaddr;
  } ifa_ifu;
  uint32_t   ifa_flags;   
  struct sockaddr *ifa_addr;    
  struct sockaddr *ifa_netmask; 
  void *ifa_data;
}; 

#define MAX_NUMBER_IFS  16

bool isWireless(const char* ifname, char* protocol);

int queryBufferBytes(int socket_descriptor, unsigned long int option);

int getIFnames(char *arrayOfIFNames[]);
int getIFAddr(char *IFNampePtr, struct sockaddr  *sockaddrPtr);
int getIFInfo(struct IFInfoStruct *arrayOfIFInfoStructs[]);

#endif



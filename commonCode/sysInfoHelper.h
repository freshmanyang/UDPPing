/************************************************************************
* File: sysInfoHelper.h
*
* Purpose:
*   Set of helper routines related to gathering system information
*
* Notes:
*
* Last update: 3/22/2019
*
************************************************************************/
#ifndef	__sysInfoHelper_h
#define	__sysInfoHelper_h

#include "common.h"
#include <sys/utsname.h>
#include <sys/sysinfo.h>

//Possible platform types
#define UNIX_PLATFORM   	0
#define LINUX_PLATFORM          1
#define COHDA_V2V_PLATFORM      2
#define ARADA_V2V_PLATFORM      3
#define SMARTWAYS_V2V_PLATFORM  4

//Possible infoTypes
#define SYSTEM_INFO             0
#define HEARTBEAT               1
#define GPSandTIME              2
#define NETWORK_INFO            3
#define NETWORK_HEARTBEAT       4
#define WIRELESS_INFO           5
#define WIRELESS_HEARTBEAT      6

#define SECURITY_INFO           10
#define CONTROL_INFO            11

//Possible outputModes          
#define OUTPUT_MODE_STDOUT      0
#define OUTPUT_MODE_FILE        1
#define OUTPUT_MODE_FIFO        2
#define OUTPUT_MODE_NETWORK     3


int execCommand(char *cmdPtr, char *returnString, int maxSize);
int initSysInfoModule();
int resetSysInfoModule();
int getSysInfoORG(int platformType, int infoType, char *bufPtr, uint32_t maxSize);
int getSysInfo(int platformType, int infoType, char *bufPtr, uint32_t maxSize);
int getRunTimeInfo(int platformType, int infoType, char *bufPtr, uint32_t maxSize);
void getKernelVersion(char *ret);
void getArch(char *ret);
char *getEndianness(void);

#endif



/*********************************************************
* Module Name:  statsHelper 
*
* File Name:  statsHelper.c
*
* Summary:
*
*   This module contains a "C" interface to routines that
*     are used to obtain system stats.
*
*  Last update: 10/8/2019
*
*********************************************************/
#include "./common.h"
#include "statsHelper.h"

GPSStats *GPSStatsPtr = NULL;
GPSStatsLong * GPSStatsLongPtr = NULL;
WirelessStats *wirelessStatsPtr = NULL;
WirelessStatsLong *wirelessStatsLongPtr = NULL;
NetStats *netStatsPtr = NULL;
NetStatsLong *netStatsLongPtr = NULL;
SystemStats *systemStatsPtr = NULL;
SystemStatsLong *systemStatsLongPtr = NULL;
SystemNodeInfo *systemNodeInfoPtr = NULL;
StatsStruct myStats;
StatsStructLong myStatsLong;



//Uncomment to turn on printf debug statements
//#define  TRACEME 0

/***********************************************************
* Function: int initStats()
*
* Explanation: This inits the stats data structures
*
*
* inputs: 
*       
* outputs: returns NO_ERROR or ERROR
*    
* notes: 
*
***********************************************************/
int initStats()
{
  int rc = NOERROR;

  GPSStatsPtr = (GPSStats *) malloc(sizeof(GPSStats));
  GPSStatsLongPtr = (GPSStatsLong *) malloc (sizeof(GPSStatsLong));
  wirelessStatsPtr = (WirelessStats *) malloc (sizeof(WirelessStats));
  wirelessStatsLongPtr = (WirelessStatsLong *) malloc (sizeof(WirelessStatsLong));
  netStatsPtr = (NetStats *) malloc (sizeof(NetStats));
  netStatsLongPtr = (NetStatsLong *) malloc (sizeof(NetStatsLong));
  systemStatsPtr = (SystemStats *) malloc (sizeof(SystemStats));
  systemStatsLongPtr = (SystemStatsLong *) malloc (sizeof(SystemStatsLong));
  systemNodeInfoPtr = (SystemNodeInfo *) malloc (sizeof(SystemNodeInfo));

  myStats.nodeInfoPtr = systemNodeInfoPtr;
  myStats.GPSStatsPtr = GPSStatsPtr;
  myStats.wirelessStatsPtr = wirelessStatsPtr;
  myStats.netStatsPtr  = netStatsPtr;
  myStats.sysStatsPtr = systemStatsPtr;


  return rc;
}


/***********************************************************
* Function: int closeStats()
*
* Explanation: This closes the stats module. All memory
*  that was malloced is freed.
*
*
* inputs: 
*       
* outputs: returns NO_ERROR or ERROR
*    
* notes: 
*
***********************************************************/
int closeStats()
{
  int rc = NOERROR;

  if (GPSStatsPtr!=NULL){
    free(GPSStatsPtr);
    GPSStatsPtr=NULL;
  }

  if (GPSStatsLongPtr!=NULL){
    free(GPSStatsLongPtr);
    GPSStatsLongPtr=NULL;
  }

  if (wirelessStatsPtr!=NULL){
    free(wirelessStatsPtr);
    wirelessStatsPtr=NULL;
  }

  if (wirelessStatsLongPtr!=NULL){
    free(wirelessStatsLongPtr);
    wirelessStatsLongPtr=NULL;
  }

  if (netStatsPtr!=NULL){
    free(netStatsPtr);
    netStatsPtr=NULL;
  }

  if (netStatsLongPtr!=NULL){
    free(netStatsLongPtr);
    netStatsLongPtr=NULL;
  }

  if (systemStatsPtr!=NULL){
    free(systemStatsPtr);
    systemStatsPtr=NULL;
  }
  if (systemStatsLongPtr!=NULL){
    free(systemStatsLongPtr);
    systemStatsLongPtr=NULL;
  }

  if (systemNodeInfoPtr!=NULL){
    free(systemNodeInfoPtr);
    systemNodeInfoPtr=NULL;
  }


  myStats.nodeInfoPtr = NULL;
  myStats.GPSStatsPtr = NULL;
  myStats.wirelessStatsPtr = NULL;
  myStats.netStatsPtr  = NULL;
  myStats.sysStatsPtr = NULL;

  return rc;

}


/***********************************************************
* Function: GPSStats * getGPSStats()
*
* Explanation: This returns a GSPStats struct filled with
*   the latest GPS information.
*
* inputs: 
*       
* outputs: returns a struct ptr or NULL on error
*    
* notes: 
*
***********************************************************/
GPSStats * getGPSStats()
{
  int rc = NOERROR;
  GPSStats *dataPtr = GPSStatsPtr;
  size_t n = 0;
  FILE *statsfp = NULL;
  char localLine[MAX_LINE_SIZE];
  char *bufPtr = localLine;


  statsfp = fopen("/etc/TGIF/GPSLog.out","r");
  if (!statsfp){
    perror("fopen GPSStats");
    exit(EXIT_FAILURE);
  }

  rc = getline(&bufPtr, &n, statsfp);
  fclose(statsfp);

#ifdef TRACEME
  printf("GPSStats: %s \n",bufPtr);
#endif

//  strncpy(bufPtr, clock_src, maxSize);
//stripStr(bufPtr, "\n");
//  rc = getSubStringIndex(bufPtr, "\n");

  int    status;
  int    numberSats;
  int      mode;       //type of fix: 0,1,2,3  0 means no fix or error
  double sampleTime;
  double altError;

  rc = sscanf(bufPtr, "%lf %d %d %d %lf %lf %lf %lf %lf %lf %lf %lf",
  &GPSStatsPtr->timestamp,
  &GPSStatsPtr->mode,
  &status, &numberSats,&sampleTime,
  &GPSStatsPtr->velocity,
  &GPSStatsPtr->latitude,
  &GPSStatsPtr->longitude,
  &GPSStatsPtr->elevation,
  &GPSStatsPtr->latError,
  &GPSStatsPtr->longError,
  &altError);

#ifdef TRACEME
  printf("GPSStats: %f mode:%d   \n",
    GPSStatsPtr->timestamp,GPSStatsPtr->mode);

  printf("GPSStats: velocit:%f lat/long/alt: %f %f %f  lat/long error: %f %f\n",
      GPSStatsPtr->velocity,
      GPSStatsPtr->latitude, 
      GPSStatsPtr->longitude, 
      GPSStatsPtr->elevation,
      GPSStatsPtr->latError, 
      GPSStatsPtr->longError);
#endif

  return dataPtr;
}

GPSStatsLong * getGPSStatsLong()
{
  int rc = NOERROR;
  GPSStatsLong *dataPtr = GPSStatsLongPtr;

  return dataPtr;
}

WirelessStats *getWirelessStats()
{
  int rc = NOERROR;
  WirelessStats *dataPtr = wirelessStatsPtr;

  return dataPtr;
}

WirelessStatsLong *getWirelessStatsLong()
{
  int rc = NOERROR;
  WirelessStatsLong *dataPtr = wirelessStatsLongPtr;

  return dataPtr;
}

NetStats *getNetStats()
{
  int rc = NOERROR;
  NetStats *dataPtr = netStatsPtr;

  return dataPtr;
}

NetStatsLong *getNetStatsLong()
{
  int rc = NOERROR;
  NetStatsLong *dataPtr = netStatsLongPtr;

  return dataPtr;
}

/***********************************************************
* Function: SystemStats *getSystemStats()
*
* Explanation: This returns a reference to a SystemStats struct
*   filled with the latest information.
*
* inputs: 
*       
* outputs: returns a struct ptr or NULL on error
*    
* notes: 
*
***********************************************************/
SystemStats *getSystemStats()
{
  int rc = NOERROR;
  SystemStats *dataPtr = systemStatsPtr;
  size_t n = 0;
  FILE *statsfp = NULL;
  char localLine[MAX_LINE_SIZE];
  char *bufPtr = localLine;


  statsfp = fopen("/etc/TGIF/SYSSTATSLog.out","r");
  if (!statsfp){
    perror("fopen SYSStats");
    exit(EXIT_FAILURE);
  }

  rc = getline(&bufPtr, &n, statsfp);
  fclose(statsfp);

#if 0
typedef struct {
  double   timestamp;
  uint32_t load1;      //1 minute load avg
  uint32_t load2;      //5 minute load avg
  uint32_t load3;      //15 minute load avg
  uint8_t  chronyStatus;  //None, not synced, synced
  uint32_t chronyTimeError; 
  int8_t   GPSstatus;  //none, synced local device, synced remote device
} SystemStats;
#endif

  rc = sscanf(bufPtr, "%lf %d %d %d ", 
         &dataPtr->timestamp,
         &dataPtr->load1, &dataPtr->load2, &dataPtr->load3);

#ifdef TRACEME
  printf("SYSStats: rc:%d,  %s \n",rc, bufPtr);
#endif

  return dataPtr;
}

SystemStatsLong *getSystemStatsLong()
{
  int rc = NOERROR;
  SystemStatsLong *dataPtr = systemStatsLongPtr;

  return dataPtr;
}

SystemNodeInfo *getSysNodeInfo()
{
  int rc = NOERROR;
  SystemNodeInfo *dataPtr = systemNodeInfoPtr;

  return dataPtr;
}

/***********************************************************
* Function: StatsStruct *getStats()
*
* Explanation: This returns a reference to the
*   StatsStruct. The ownership of this memory
*   is NOT passed to the caller.
*
* inputs: 
*       
* outputs: returns a struct ptr or NULL on error
*    
* notes: 
*
***********************************************************/
StatsStruct *getStats()
{
  int rc = NOERROR;

  myStats.nodeInfoPtr = systemNodeInfoPtr;
  myStats.GPSStatsPtr = GPSStatsPtr;
  myStats.wirelessStatsPtr = wirelessStatsPtr;
  myStats.netStatsPtr  = netStatsPtr;
  myStats.sysStatsPtr = systemStatsPtr;

  return &myStats;
}





/************************************************************************
* File:  statsHelper.h
*
* Purpose:
*   This provides functions to obtain system stats.
* 
*     GPSStatsMsg;
*     WirelessStatsMsg:
*     NetStatsMsg:
*     SystemStatsMsg:
*
* NOTE:
*
* Last update:  10/2/2019
*
************************************************************************/
#ifndef	__statsHelper_h
#define	__statsHelper_h

#include "./gpsdHelper.h"
#include "./messages.h"



typedef struct {
  double   timestamp;
  int      mode;       //type of fix: 0,1,2,3  0 means no fix or error
  double latitude;
  double latError; 
  double longitude;
  double longError;
  double elevation;
  double velocity;
} GPSStats;



typedef  struct{
  double   timestamp;
  int mode;
  double latitude;
  double longitude;
  double altitude;
  double speed;
  double climb;
  double track;
  double latAcc;
  double longAcc;
  double altAcc;
  double speedAcc;
  double climbAcc;
  double trackAcc;
} GPSStatsLong;


typedef struct {
  double   timestamp;
  uint32_t SINR;
  uint32_t RSSI;
  uint32_t Channel;
  uint32_t Power;
} WirelessStats;

typedef struct {
  double   timestamp;
  char  IFName[32];     //interface name
  uint32_t typeDevice; // 802.11, LTE-A,  
  uint32_t deviceMode;  //current mode of device (e.g., 802.11n, ...)
  uint32_t Channel;  // channel id
//The following can be instantaneous or averaged over the timeInterval
//If timeInterval, the RxMeasure are in bps the node must clear stats each interval 
//   And measures like SINR,RSSI are avg over the interval
// if timeInternet is 0, RxMeasure is bytes rx'ed, SINR is instantaneous...
  uint32_t RxMeasure;
  uint32_t TxMeasure;
  uint32_t Power;    // Tx power or received power
  uint32_t TxMCS;      //modulation and coding -averaged or last used
  uint32_t RxMCS;      //modulation and coding -averaged or last used
  uint32_t SINR;      //
  uint32_t RSSI;     //
  uint32_t utilization;  //
  uint32_t channelErrors;  // means diff things depending on the rat
  uint32_t timeIntervalSecs;
  uint32_t timeIntervalNSecs;
} WirelessStatsLong;


typedef struct {
  double   timestamp;
  char  IFName[32];     //interface name
  uint32_t typeDevice; // 802.11, LTE-A,  
  uint32_t deviceMode;  //current mode of device (OFF, 1Gbps, ....)
  uint32_t RxThroughput;
  uint32_t TxThroughput;
  void *next;
} NetStats;

typedef struct {
  double   timestamp;
  char  IFName[32];     //interface name
  uint32_t typeDevice; // 802.11, LTE-A,  
  uint32_t deviceMode;  //current mode of device (OFF, 1Gbps, ....)
  uint32_t RxBytes;
  uint32_t TxBytes;
  uint32_t RxThroughput;
  uint32_t TxThroughput;
  uint32_t timeIntervalSecs;
  uint32_t timeIntervalNSecs;
  uint32_t bytesBuffered;
  void *next;
} NetStatsLong;


typedef struct {
  double   timestamp;
  uint32_t load1;      //1 minute load avg
  uint32_t load2;      //5 minute load avg
  uint32_t load3;      //15 minute load avg
  uint8_t  chronyStatus;  //None, not synced, synced
  uint32_t chronyTimeError; 
  int8_t   GPSstatus;  //none, synced local device, synced remote device
} SystemStats;


typedef struct {
  double   timestamp;
                        //If this is to be echoed, the server
  uint32_t load1;      //1 minute load avg
  uint32_t load2;      //5 minute load avg
  uint32_t load3;      //15 minute load avg
  uint32_t memAvail;    //Amount of mem availabl minute load avg
  uint32_t memFree;    //Amount of mem free
  uint32_t memUsed;    //Amount of mem used
  uint32_t numberUsers;
  uint32_t totalNetworkTxRate;
  uint32_t totalNetworkRxRate;
  uint32_t internalLatencyTest; //results of clock accuracy test
  uint8_t  chronyStatus;  //None, not synced, synced
  uint32_t chronyTimeError; 
  int8_t   GPSstatus;  //none, synced local device, synced remote device
} SystemStatsLong;



typedef struct {
  double   timestamp;
  uint8_t  TypeArch;
  uint8_t  numberProcessors;
  uint8_t  amountMemory;
  uint8_t  OS_code;
  char *OSDistro;
  uint8_t  currentClockType;
  uint8_t  TGIFNodeType;
  char  HostName[32];    
} SystemNodeInfo;



typedef struct {
  double   timestamp;
  SystemNodeInfo *nodeInfoPtr;
  GPSStats *GPSStatsPtr;
  WirelessStats *wirelessStatsPtr;
  NetStats *netStatsPtr;
  SystemStats *sysStatsPtr;
} StatsStruct;

typedef struct {
  double   timestamp;
  SystemNodeInfo *nodeInfoPtr;
  GPSStatsLong *GPSStatsPtr;
  WirelessStatsLong *wirelessStatsPtr;
  NetStatsLong *netStatsPtr;
  SystemStatsLong *sysStatsPtr;
} StatsStructLong;

int initStats();
int closeStats();
GPSStats * getGPSStats();
GPSStatsLong * getGPSStatsLong();
WirelessStats *getWirelessStats();
WirelessStatsLong *getWirelessStatsLong();
NetStats *getNetStats();
NetStatsLong *getNetStatsLong();
SystemStats *getSystemStats();
SystemStatsLong *getSystemStatsLong();
SystemNodeInfo *getSysNodeInfo();
StatsStruct *getStats();


#endif



/************************************************************************
* File: timeHelper.h
*
* Purpose:
*   Set of helper routines related to Linux timekeeping
*
* Notes:
*       We define a clock type and a clock source for a particular clock.
*       Similarly, for delays/sleep functions, we define the sleep type
*       and the clock source for that particular method of delay/sleep.
*       This module defines helper functions for obtaining wall clock
*       times and also timestamps.  Another module, delayHelper.c does
*       the same for delay/sleep functions.
*
*       Issue a 'man 7 time' for an intro to Unix time terminology. 
*       This module defines  a wall clock time to a time from a clock
*       that represents an absolute real time time value. 
*       This module defines a timestamp to be a time from  a clock
*       that is relative to a prior time whose value has no significance
*       on its own.  
*
*       Timestamps are used to measure delays or to assess the time
*       required to perform a function.  A wall clock time is meant
*       to be a time that in the best case is exactly equal to a 
*       what human's believe to be true time (UTC time).  
*
* Last update: 6/5/2019
*
************************************************************************/
#ifndef	__timeHelper_h
#define	__timeHelper_h
#include "common.h"
#include <sys/time.h>
#ifdef LINUX
#include <linux/rtc.h>
#endif
#include <time.h>

struct TGIF_timespec {
  uint64_t ts_sec;
  uint64_t ts_nsec;
};


/*  set to 1 if want all timestamps relative to time of first pkt sent/arrived */
//Not supported until we need it 
//#define RELATIVE_TIME_FLAG 1

//Note <sys/time.h> should have:
//These are available POSIX clocks
//We refer to these as clock wsources
//CLOCK_REALTIME, CLOCK_MONOTONIC, CLOCK_MONOTONIC_RAW, CLOCK_PROCESS_CPUTIME_ID

//The following are sources for obtaining time-  they might be affected
//by the choice of clock
//the time() call returns time_t seconds

//These are timestampType choices
#define  CLOCK_ERRONEOUS  -1
#define  CLOCK_UNKNOWN  254

//CLOCK_REALTIME, CLOCK_MONOTONIC, CLOCK_MONOTONIC_RAW, CLOCK_PROCESS_CPUTIME_ID
//These would be clock types.  It defines the function called....
#define  CLOCK_GETTIME          31
#define  CLOCK_RDTSC 		32
#define  CLOCK_TIME   		33	 
#define CLOCK_gettimeofday 	34
#define CLOCK_RTC 		35
#define CLOCK_GPS 		36
#define CLOCK_NTPServer 	37

#define  TIME_RDTSC 		32
#define  TIME_TIME    		33	 //man 2 time 
#define TIME_gettimeofday 	34
#define TIME_RTC 		35
#define TIME_NTPServer 		36
#define TIME_GPS 		37
#define  _ gettime		38

//These are the clockid (clock types) param values we expose to users 
#define clockGettimeofday  	0
#define clockREALTIME 		1
#define clockMONOTONIC 		2 
#define clockMONOTONIC_RAW  	3
#define clockCPUTIME 		4
#define clockRDTSC    		5
#define clockcTime         	6
#define clockRTC         	7
#define clockNTPServer         	8
#define clockLocalGPS         	9
#define clockRemoteGPS         	10

#define timeGETTIMEOFDAY 	0
#define timeGETTIME         	1
#define timeRDTSC    		2
#define timecTIME    		3	 
#define timeRTC 		4
#define timeNTPServer 		5
#define timeLocalGPS 		6
#define timeRemoteGPS 		7

/*On older linux systems, might not have CLOCK_BOOTTIME*/

#ifndef CLOCK_BOOTTIME 
#define  CLOCK_BOOTTIME -1
#endif

//values of clockMode
#define CLOCK_SYSTEM_DEFAULTS  0
#define CLOCK_TGIF_DEFAULTS    1

#define CLOCK_GPSD             4
#define CLOCK_LOCAL_GPSD       5
#define CLOCK_REMOTE_GPSD      6

#define CLOCK_NETWORK_TIME_CLIENT     16
#define CLOCK_CHRONY_CLIENT           17
#define CLOCK_NTP_CLIENT              18

#define CLOCK_NETWORK_TIME_SERVER     32
#define CLOCK_CHRONY_SERVER           33
#define CLOCK_NTP_SERVER              34

/***********************************************
* Set 0 helper functions for the module
***********************************************/
void initClockModule();

typedef long int clocktype_t;   //specifies clock type
//clock_t already defined  which specifies clock source type

//state:

int setDefaultWallClockType(clocktype_t wallClockType);
int setDefaultWallClockSource(clock_t wallClockSource);
int setDefaultTimestampClockType(clocktype_t timestampType);
int setDefaultTimestampClockSource(clock_t timestampClockSource);
clocktype_t getDefaultWallClockType();
clocktype_t getDefaultTimestampClockType();
clock_t getDefaultWallClockSource();
clock_t getDefaultTimestampClockSource();

int getClockSource(char *bufPtr, int maxSize);

/***********************************************
* Set 1 functions:  getting system info
*      and general helper functions
***********************************************/

clock_t convertClockParam(uint32_t clockParam);
void initClockModule();
int getClockInfo(char *bufPtr, int maxSize);
int getCurrentClockSource(char *myBuf, int bufSize);
int getAvailableClockSources(char *bufPtr, int maxSize);
clock_t  clockSourceStringToClockType(char *myBuf, int bufSize);
int  clockSourceClockTypeToString(clock_t clockID, char *myBuf, int bufSize);


/********************************
* Set 2: Routines to get curTime or a timestamp
***********************************/
//Most generic...
int    getTime(clock_t clockSource, struct timespec *ts);
double getTimeD(clock_t clockType);

double getCurTime(struct timespec *ts);
double getCurTimeD();
int    getCurTimeTS(struct timespec *ts);  //Recommended!!

     //to get a timestamp- uses the clock specified in delayClockType variable
     //which is CLOCK_MONOTONIC_RAW by default
double getTimestamp(struct timespec *ts); 
double getTimestampD(); 
int    getTimestampTS(struct timespec *ts);  //Recommended

//returns wall clock in human readable format and double format
double getWallClockTimeString(char *bufPtr, int maxSize);

double wallClockTime();
double timestamp();  //returns seconds.microseconds wall clock time (gettimeofday)
int getTimevalByRef(struct timeval *callersTime);
#ifdef LINUX
int getRTC(struct rtc_time *);
#endif

/********************************
* Set 3:  Helper Routines- convert or manipulate timestamps
***********************************/
bool isTS1GTTS2(struct timespec *TS1, struct timespec *TS2);
bool isZeroTime(struct timespec *myTS);
bool isLessThanOne(struct timespec *myTS);
int convertTimespecToString (char *callersTimeString, int maxSize, struct timespec *myTS);
double convertTS2D(struct timespec *t);
double convertTGIFTS2D(struct TGIF_timespec *t);
int convertD2TS(double *timeDouble,  struct timespec *timeTSpec);
uint64_t  diffTSpecs(struct timespec *start_time, struct timespec *end_time); //finds time diff and returuns in nanoseconds
uint64_t  getNanoSeconds(struct timespec *t);  //converts a ts to nanoseconds

//The following operate on timeval's
double convertTimeval(struct timeval *t);
uint32_t getTimeSpan(struct timeval *start_time, struct timeval *end_time);
uint64_t getTimeSpanTS(struct timespec *start_time, struct timespec *end_time);
uint32_t getMicroseconds(struct timeval *t);

/********************************
* Set 5:Experimental or not used (yet) 
***********************************/

uint64_t rdtsc(void);
uint64_t tsc();
uint64_t glibc_nsec(uint64_t tsc, uint64_t freq);
//uint64_t clockCycleCount();
double testGetTime(clock_t clockType);

#endif



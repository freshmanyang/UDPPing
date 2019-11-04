/*********************************************************
* Module Name:  routines that provide time related function
*
* File Name:     timeHelper.c 
*
*
* Warning:  There is a clear difference between
*           a wall clock time and a time stamp
*           This module tries to stay consistent by
*           using curTime and timestamp but there are
*           areas in the code where this is confused. Sorry....
*
*           So make sure the routine is listed in its header
*           is tagged as  WALLCLOCK or TIMESTAMP
*           Any functiont that returns a time whree
*           the caller specifies the clock source can
*           be either WALLCLOCK or TIMESTAMP depending on
*           the caller's choice. 
*
*           The three tags:
*     TAG WALLCLOCK 
*     TAG TIMESTAMP
*     TAG WALLCLOCK TIMESTAMP
*
*
*  Summary: picking my first and second choice to use for wallclock and TS:
*
*     wallclock time:
*              double getCurTimeD()    
*              int getCurTimeTS(struct timespec *ts) 
*              double getCurTime(struct timespec *ts) 
*              double getWallClockTimeString(char *bufPtr, int maxSize)
*              double wallClockTime()
*
*     timestamps:
*          double getTimestamp(struct timespec *ts)  
*          double getTimestampD()
*          int getTimestampTS(struct timespec *ts)   //Recommended
*
*
*  On DARWIN 
*     uint64_t  clock_gettime_nsec_np(clockSource);
*
*  Last update: 4/17/2019
* 
*********************************************************/
#include <poll.h>
#include <sys/select.h>
#include "common.h"
#include "utils.h"
#include "timeHelper.h"
#include "delayHelper.h"

//#define TRACEME 0
#define TRACE_ERRORS 0

//State set by init
bool isTimeModuleInitialized = false;
double startingTime = -1;

//State that specifies
// the default wall clock time  and timestamp 
// settings.   
// The Type specifies the type of clock used : CLOCK_*
// The TimeSource specifies a clock that can be used by a clock
// In some cases, the type of clock and the choice of clock are the same: 
//    Examples:  RDTSC or RTC
// But in more likely cases, the type of clock is CLOCK_GETTIME
//       OR CLOCK_GPS or CLOCK_NTPServer. 
//   Worst case would be CLOCK_RTC, CLOCK_
// which allows us to specify a range of clock sources (e.g., CLOCK_MONOTONIC).

clocktype_t wallClockType = CLOCK_GETTIME;
clock_t wallClockSource = CLOCK_REALTIME;
clocktype_t timestampClockType = CLOCK_GETTIME;
clock_t timestampClockSource = CLOCK_MONOTONIC;
clocktype_t  defaultWallClockType = CLOCK_GETTIME;
clock_t  defaultWallClockSource = CLOCK_MONOTONIC;
clocktype_t  defaultTimestampClockType = CLOCK_GETTIME;
clock_t  defaultTimestampClockSource = CLOCK_MONOTONIC;

int setDefaultWallClockType(clocktype_t wallClockType);
int setDefaultWallClockSource(clock_t wallClockSource);
int setDefaultTimestampClockType(clocktype_t timestampClockType);
int setDefaultTimestampClockSource(clock_t timestampClockSource);
clocktype_t getDefaultWallClockType();
clock_t getDefaultWallClockSource();
clocktype_t getDefaultTimestampClockType();
clock_t getDefaultTimestampClockSource();

//int getClockSource(char *bufPtr, int maxSize);

bool isThereCLOCK_REALTIME = true;
bool isThereCLOCK_MONOTONIC = true;
bool isThereCLOCK_MONOTONIC_RAW = false;
bool isThereCLOCK_BOOTTIME = false;

char clockSrcFile[] = "/sys/devices/system/clocksource/clocksource0/current_clocksource";
char availClockSrcsFile[] = "/sys/devices/system/clocksource/clocksource0/available_clocksource";
#ifdef LINUX
char timerListFile[] = "/proc/timer_list";
#else
char timerListFile[] = "UNKNOWN";
#endif

/****************************************
* routine: void initClockModule()
*
* explanation: inits globals
*  
* inputs:  none
*
* outputs: none
*
* Details/notes:
*  INIT
****************************************/
void initClockModule()
{
  int rc = NOERROR;

  //todo:  find out if these are really true....
  wallClockSource =        CLOCK_REALTIME;
  wallClockType =          CLOCK_REALTIME;
  timestampClockType =     CLOCK_GETTIME;
  timestampClockSource =   CLOCK_MONOTONIC;
  defaultWallClockType =   CLOCK_GETTIME;
  defaultWallClockSource = CLOCK_REALTIME;
  defaultTimestampClockType = CLOCK_GETTIME;
  defaultTimestampClockSource = CLOCK_MONOTONIC;

  startingTime  = getCurTimeD();
  isThereCLOCK_REALTIME = true;
  isThereCLOCK_MONOTONIC = true;
  isThereCLOCK_MONOTONIC_RAW = false;
  isThereCLOCK_BOOTTIME = false;
  isTimeModuleInitialized = true;
  initDelayModule();
  rc = set_delayClockTypeandSource (DELAY_CLOCK_SLEEP,CLOCK_MONOTONIC_RAW);
  if (rc == ERROR) {
    rc = set_delayClockTypeandSource (DELAY_CLOCK_SLEEP,CLOCK_MONOTONIC);
    if (rc == ERROR)
      rc = set_delayClockTypeandSource (DELAY_NANOSLEEP1,CLOCK_MONOTONIC_RAW);
      if (rc == ERROR)
        rc = set_delayClockTypeandSource (DELAY_NANOSLEEP1,CLOCK_MONOTONIC);
        if (rc == ERROR)
          rc = set_delayClockTypeandSource (DELAY_USLEEP,CLOCK_REALTIME);
  }
#ifdef TRACEME
  printf("initClockModule: defaultDelayType:%d defaultDelayClockSource:%d \n", 
        getDefaultDelayType(),getDefaultDelayClockSource());
#endif

  if (rc == ERROR) {
    printf("initClockModule: FAILED ?? defaultDelayType:%ld defaultDelayClockSource:%ld \n", 
        getDefaultDelayType(),getDefaultDelayClockSource());
    exit(EXIT_FAILURE);
  }
}


/***********************************************
* SET 1 routines to get wall clock time 
*     TAG WALLCLOCK
*
*     These are any routines that use either
*        CLOCK_REALTIME
*        gettimeofday
*        Or the configured wallClockType 
*
*              double getCurTimeD()    
*              int getCurTimeTS(struct timespec *ts) 
*              double getCurTime(struct timespec *ts) 
*              double getWallClockTimeString(char *bufPtr, int maxSize)
*              double wallClockTime()
*
*              These are based on gettimeofday
*              timestamp()
*              int getTimevalByRef(struct timeval *callersTime)
*
*
***********************************************/

/***********************************************************
* Function: double getCurTimeD() 
*
* Explanation:  This returns the wall time using 
*               CLOCK_REALTIME as the clock source.
*               Should do the same as timestamp() or
*                wallClockTime(), but possibly with
*               more precision.
*
* inputs: none
*
* outputs:
*    returns the wall time as a double representing 
*     the wall clock time  in seconds  with nanosecond precision
*
* notes: 
*     TAG WALLCLOCK
*
***********************************************************/
double getCurTimeD() 
{
  double timestamp = -1.0;
  struct timespec ts;
  int rc = NOERROR;

  //likely to use CLOCK_REALTIME 
  //rc = clock_gettime(wallClockType, &ts);
  //No reason to NOT use the REALTIME clock
  rc = clock_gettime(CLOCK_REALTIME, &ts);

  if (rc==NOERROR) { 
      timestamp = ( (double)ts.tv_sec +  (double) (((double)ts.tv_nsec)/1000000000) );
  } else {
    printf("getCurTimeD:  HARD error on clock_gettime (%ld,%d),  errno:%d \n",
              wallClockType,CLOCK_REALTIME, errno);
    perror("getCurTimeD:  HARD error on clock_gettime\n");
  }

  return(timestamp);

}

/***********************************************************
* Function: double getWallClockTimeString(char *bufPtr, int maxSize)
*
* Explanation: returns to callers bufPtr wall time in human readable format
* 
* inputs: 
*   char *bufPtr: callers buffer to place text wall clock time
*   int maxSize : max size of wall clock info returned
*
* outputs:
*    returns ERROR or size of string 
*
* notes: 
*     TAG WALLCLOCK
*
***********************************************************/
double getWallClockTimeString(char *bufPtr, int maxSize)
{
  int rc = NOERROR;
  time_t seconds = 0;
  struct timespec ts = {0, 0};
  char localLine[MAX_LINE_SIZE];

  rc =  getTime(CLOCK_REALTIME,&ts);
  if (rc == SUCCESS) 
  { 
    seconds = (time_t)ts.tv_sec;
    asctime_r(localtime(&seconds), bufPtr);
    rc = getSubStringIndex(bufPtr,"\n");
    if (rc != strlen(bufPtr)) {
      rc = ERROR;
      printf("getWallTime: ERROR on size  rc:%d and strlen:%lu  ??  \n",rc, strlen(bufPtr));
    }
  } else {
    rc = ERROR;
    printf("getWallTime:  Error on getTime \n");
  }

  if (rc == ERROR)
    printf("getWallTime:  return ERROR \n");
#ifdef TRACEME
  else
    printf("getWallTime:  succeeded, return substring size: %d \n",rc);
#endif

  return rc;
}


/***********************************************************
* Function: double getCurTime(struct timespec *ts) 
*
* Explanation:  This returns the wall time using 
*               CLOCK_REALTIME as the clock source.
*
* inputs: 
*       struct timespec *ts : callers timespec that is to be filed in.
*
* outputs:
*    returns the wall time in seconds  with nanosecond precision
*
* notes: 
*     TAG WALLCLOCK 
*
***********************************************************/
double getCurTime(struct timespec *ts) 
{
  double timestamp = -1.0;
  int rc = NOERROR;

  //likely to use CLOCK_REALTIME 
  rc = clock_gettime(CLOCK_REALTIME, ts);

  if (rc==NOERROR) { 
      timestamp = ( (double)ts->tv_sec +  (double) (((double)ts->tv_nsec)/1000000000) );
  } else {
    printf("getCurTime:  HARD error on clock_gettime,  errno:%d \n", errno);
    perror("getCurTimeD:  HARD error on clock_gettime\n");
  }

  return(timestamp);

}

/***********************************************************
* Function: double wallClockTime()
*
* Explanation:  This  returns the current wall clock time
*               in  double format representings secs.useconds
*               since the beginning of the time (as far as Unix is aware)
*
*               This is based on gettimeofday which is potentially inaccurate or not precise.
*               THEREFORE:  USE  on of the getCurTime functions available in this module.
* inputs: 
*
* outputs:
*        returns the timestamp in  double format representings secs.useconds
*               since the beginning of the time (as far as Unix is aware)
*
*
*     TAG WALLCLOCK
***********************************************************/
double wallClockTime()
{
  return(getCurTimeD()); 
}
/***********************************************************
* Function: int getCurTimeTS(struct timespec *ts) 
*
* Explanation:  This returns the wall time using 
*               CLOCK_REALTIME as the clock source.
*
* inputs: 
*       struct timespec *ts : callers timespec that is to be filed in.
*
* outputs:
*    returns an ERROR or NOERROR
*
* notes: 
*
*     TAG WALLCLOCK 
***********************************************************/
int getCurTimeTS(struct timespec *ts) 
{
  double timestamp = -1.0;
  int rc = NOERROR;

  rc = clock_gettime(CLOCK_REALTIME, ts);

  if (rc==NOERROR) { 
      timestamp = ( (double)ts->tv_sec +  (double) (((double)ts->tv_nsec)/1000000000) );
  } else {
    printf("getCurTime:  HARD error on clock_gettime,  errno:%d \n", errno);
  }
  return rc;
}

/***********************************************************
* Function: double timestamp() 
*
* Explanation:  This  returns the current wall clock time
*               in  double format representings secs.useconds
*               since the beginning of the time (as far as Unix is aware)
*
*               This is based on gettimeofday which is potentially inaccurate or not precise.
*               THEREFORE:  USE  on of the getCurTime functions available in this module.
* inputs: 
*
* outputs:
*        returns the timestamp in  double format representings secs.useconds
*               since the beginning of the time (as far as Unix is aware)
*
*     TAG WALLCLOCK
*
***********************************************************/
double timestamp() 
{
 double rc = DOUBLE_NOERROR;
  struct timeval tv;
  if (gettimeofday(&tv, NULL) < 0) { 
     printf("utils:timestampt: gettimeofday failed, errno: %d \n",errno); 
     rc=DOUBLE_ERROR;
  } else {
    rc = (double)tv.tv_sec + ((double)tv.tv_usec / 1000000);
  }

  return rc;
}

/***********************************************************
* Function: int getTimevalByRef(struct timeval *callersTime);
*
* Explanation:  This returns the current time
*               using the standard timeval format.
*
* inputs: 
*  struct timeval *callersTime - the caller's ptr to hold the time
*
* outputs:
*        returns NOERROR or ERROR.
*
* notes: 
*     TAG WALLCLOCK
*
***********************************************************/
int getTimevalByRef(struct timeval *callersTime)
{
int rc = NOERROR;

  if (gettimeofday(callersTime, NULL) < 0) { 
     rc = ERROR;
     printf("getTimeValByRef: gettimeofday failed, errno: %d \n",errno); 
  } 
  return rc;
}




/***********************************************
* SET 2 routines to get a time stamp
*     TAG TIMESTAMP
*
*     These are any routines that use either
*     clocks:  CLOCK_MONOTONIC or CLOCK_MONOTONIC_RAW
*
*          double getTimestamp(struct timespec *ts)  
*          double getTimestampD()
*          int getTimestampTS(struct timespec *ts)   //Recommended
***********************************************/

/***********************************************************
* Function:  double getTimestamp(struct timespec *ts) 
*
* Explanation:  This returns  a timestamp using 
*             the setting of the variable defaultTimestampClockSource
*
* inputs: 
*      struct timespec *ts : callers timespec that will be filled in
*
* outputs:
*        returns a timestamp in seconds  with nanosecond precision
*        a return of -1 indicates an  ERROR.
*
* notes: 
*
*     TAG TIMESTAMP
***********************************************************/
double getTimestamp(struct timespec *ts) 
{
  double timestamp = -1.0;
  int rc = NOERROR;

  //likely Use clock_gettime with clock CLOCK_MONOTONIC
  if (defaultTimestampClockSource != CLOCK_MONOTONIC) {
    printf("getTimestamp():  error, clocksource not CLOCK_MONOTONIC  (%d,%d)\n",
         defaultTimestampClockType,defaultTimestampClockSource);
   defaultTimestampClockSource = CLOCK_MONOTONIC;
  }
   
  rc = clock_gettime(defaultTimestampClockSource, ts);
  if (rc==NOERROR) { 
      timestamp = (double)ts->tv_sec + ((double)ts->tv_nsec)/1000000000;
  } else {
    printf("getTimestamp():  HARD error on clock_gettime defaultTimestamptType/Source: %lu:%lu\n", 
         defaultTimestampClockType,defaultTimestampClockSource);
        
    perror("getTimestamp():  HARD error on clock_gettime\n");
    rc = ERROR;
  }

  if (rc == ERROR)
    timestamp = DOUBLE_ERROR;

  return(timestamp);

}


/***********************************************************
* Function:  double getTimestampD() 
*
* Explanation:  This returns  a timestamp using 
*               the MONOTONIC_RAW clock source
*               Same as getTimestamp but without the ts param               
*
* inputs: 
*
* outputs:
*     returns a double representing a timestamp in seconds  with nanosecond precision
*
* notes: 
*
*     TAG TIMESTAMP
***********************************************************/
double getTimestampD() 
{
  double timestamp = -1.0;
  struct timespec ts;
  int rc = NOERROR;

  //timestampType is a variable maintained by this module
  if (defaultTimestampClockSource != CLOCK_MONOTONIC) {
    printf("getTimestampD():  error, clocksource not CLOCK_MONOTONIC  (%d,%d)\n",
         defaultTimestampClockType,defaultTimestampClockSource);
   defaultTimestampClockSource = CLOCK_MONOTONIC;
  }
  rc = clock_gettime(defaultTimestampClockSource, &ts);
  if (rc==NOERROR) { 
      timestamp = (double)ts.tv_sec + ((double)ts.tv_nsec)/1000000000;
  } else {
    printf("getTimestamp():  HARD error on clock_gettime defaultTimestamptType/Source: %lu:%lu\n", 
           defaultTimestampClockType,defaultTimestampClockSource);
    perror("getTimestampD():  HARD error on clock_gettime\n");
    rc = ERROR;
  }

  if (rc == ERROR)
    timestamp = DOUBLE_ERROR;

  return(timestamp);

}


/***********************************************************
* Function:  double getTimestamp(struct timespec *ts) 
*
* Explanation:  This returns  a timestamp using 
*             the setting of the variable defaultTimestampClockSource
*            This should be set by init code to CLOCK_MONOTONIC  CLOCK_MONOTONIC_RAW clock source
*
* inputs: 
*      struct timespec *ts : callers timespec that will be filled in
*
* outputs:
*        returns a ERROR or NOERROR
* notes: 
*
*     TAG TIMESTAMP
***********************************************************/
int getTimestampTS(struct timespec *ts) 
{
  double timestamp = -1.0;
  int rc = NOERROR;

  //likely Use clock_gettime with clock CLOCK_MONOTONIC_RAW:
  if (defaultTimestampClockSource != CLOCK_MONOTONIC) {
    printf("getTimestampTS():  error, clocksource not CLOCK_MONOTONIC  (%d,%d)\n",
         defaultTimestampClockType,defaultTimestampClockSource);
   defaultTimestampClockSource = CLOCK_MONOTONIC;
  }
  rc = clock_gettime( defaultTimestampClockSource, ts);
  if (rc==NOERROR) { 
      timestamp = (double)ts->tv_sec + ((double)ts->tv_nsec)/1000000000;
  } else {
    printf("getTimestamp():  HARD error on clock_gettime  errno:%d \n", errno);
  }
  return rc;
}

/***********************************************
* SET 3 routines that can be told the clock source
*     TAG WALLCLOCK AND TIMESTAMP
*        int getTime(clock_t clockSource, struct timespec *ts)
*        double getTimeD(clock_t clockSource)
***********************************************/

/***********************************************************
* Function: int getTime(clock_t clockSource, struct timespec *ts)
*
* Explanation:  maps directly to clock_gettime()
*
* inputs: 
*   clock_t clockSource : desired clockSrc
*   struct timespec *ts:  callers timespec to be filled in
*
* outputs:
*    returns ERROR or NOERROR;
*
* notes: 
*
*     TAG WALLCLOCK AND TIMESTAMP
***********************************************************/
int getTime(clock_t clockSource, struct timespec *ts)
{
  int rc = NOERROR;
  rc = clock_gettime(clockSource,  ts);
  if (rc == -1) { 
    rc = ERROR;
    printf("getTime:   HARD error on clock_gettime (requested clksource:%d)  errno:%d \n", 
                (int)clockSource, errno);
  } 
  return rc;
 }

/***********************************************************
* Function: double getTimeD(clock_t clockSource)
*
* Explanation:  returns clock_gettime() as a double
*
* inputs: 
*   clock_t clockSource : desired clockSrc
*
* outputs:
*    returns -1.0  on error or the time as a double (secs.nsecs)
*
* notes: 
*
*     TAG WALLCLOCK AND TIMESTAMP
***********************************************************/
double getTimeD(clock_t clockSource)
{
  double timestamp = -1.0;
  int rc = NOERROR;
  struct timespec ts;

//DARWIN 
//  uint64_t  clock_gettime_nsec_np(clockSource);
  rc = clock_gettime(clockSource,  &ts);
  if (rc==NOERROR) { 
      timestamp = ( (double)ts.tv_sec +  (double) (((double)ts.tv_nsec)/1000000000) );
  } else {
    printf("getTimeD:  HARD error on clock_gettime (req clockid:%ld),  errno:%d \n",clockSource, errno);
   perror("getTimeD:  HARD error on clock_gettime\n");
  }

  return(timestamp);
}


/***********************************************
* SET 4 functions:  getting system info
*        such as system time/clock information/config/capabilities 
*
* functions:
*          clock_t convertClockParam(uint32_t clockParam)
*          int getClockInfo(char *bufPtr, int maxSize)
*          int getAvailableClockSources(char *myBuf, int bufSize)
*          int getCurrentClockSource(char *myBuf, int bufSize)
*          clock_t  clockSourceStringToClockType(char *myBuf, int bufSize);
*          int  clockSourceClockTypeToString(clock_t clockID, char *myBuf, int bufSize);
*
***********************************************/

/****************************************
* routine: clock_t convertClockParam(uint32_t clockParam)
*
* explanation: converts program param form of clock ID to standard system form
*
* inputs: 
*  uint32_t:clockParam:  program param form of clock id
* 
*
* outputs: returns a clock_t NOTE:  Some values are 
*           not standard. 
*            CLOCK_ERRONEOUS is returned on error.
*
* Details/notes:
*
****************************************/
clock_t convertClockParam(uint32_t clockParam)
{
  clock_t clockID = CLOCK_ERRONEOUS;

  //convert program clock choice param to a valid system clock_t
  switch (clockParam)
  {
     case clockGettimeofday:
           clockID = CLOCK_gettimeofday;
           break;

     case clockREALTIME:
           clockID = CLOCK_REALTIME;
           break;

     case clockMONOTONIC:
           clockID = CLOCK_MONOTONIC;
           break;

     case clockMONOTONIC_RAW:
           clockID = CLOCK_MONOTONIC_RAW;
           break;

     case clockCPUTIME:
           clockID = CLOCK_PROCESS_CPUTIME_ID;
           break;

     case clockRDTSC:
           clockID = CLOCK_RDTSC;
           break;

     case clockcTime:
           clockID = CLOCK_TIME;
           break;

     case clockRTC:
           clockID = CLOCK_RTC;
           break;

     case clockNTPServer:
           clockID = CLOCK_NTPServer;
           break;

     default:
           clockID = CLOCK_ERRONEOUS;
           printf ("convertClockParam:  Invalid clock param:  %d, return CLOCK_ERRONEOUS  %ld \n",
              clockParam,clockID);
           break;

  }

  return clockID;
}




/****************************************
* routine: int getAvailableClockSources(char *bufPtr, int maxSize)
*
* explanation: Places into the callers buffer
*    a string indicating current clock and available clocks
*
*  
* inputs: 
*  char *bufPtr : caller's buffer ptr 
*  int maxSize : max size of caller's buffer
*
* outputs: returns the clock_t reflecting
*        the current clock
*         else ERROR
**************************************/
int getAvailableClockSources(char *bufPtr, int maxSize)
{
  int rc = NOERROR;
  size_t n = 0;
  char *clock_src = bufPtr;
  FILE *clock_src_fp = NULL;


  clock_src_fp = fopen("/sys/devices/system/clocksource/clocksource0/available_clocksource", "r");
  if (!clock_src_fp){
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  rc = getline(&clock_src, &n, clock_src_fp);
  fclose(clock_src_fp);

  strncpy(bufPtr, clock_src, maxSize);
  //stripStr(bufPtr, "\n");
  rc = getSubStringIndex(bufPtr, "\n");
 
  return rc;
}


/****************************************
* routine: clock_t getClockSource(char *bufPtr, int maxSize)
*
* explanation: Places into the callers buffer
*    a string indicating current clock and available clocks
*
*  
* inputs: 
*  char *bufPtr : caller's buffer ptr 
*  int maxSize : max size of caller's buffer
*
* outputs: returns the clock_t reflecting
*        the current clock
*         else ERROR
* TODO: This should be call getCurrentSystemClockSource
*       And, this should be used in the init to 
*       set the default clock source
*clock_t getClockSource(char *bufPtr, int maxSize);
*  
**************************************/
int getClockSource(char *bufPtr, int maxSize)
{
  int rc = NOERROR;
  size_t n = 0;
  char *clock_src = bufPtr;
  FILE *clock_src_fp = NULL;


  clock_src_fp = fopen("/sys/devices/system/clocksource/clocksource0/current_clocksource", "r");
  if (!clock_src_fp){
    perror("fopen");
    exit(EXIT_FAILURE);
  }

  rc = getline(&clock_src, &n, clock_src_fp);
  fclose(clock_src_fp);

  strncpy(bufPtr, clock_src, maxSize);
  //stripStr(bufPtr, "\n");
  rc = getSubStringIndex(bufPtr, "\n");
  //printf("getClockSource: rc:%d, source:%s \n",rc,bufPtr);
 
  return rc;
}


/****************************************
* routine: int getCurrentClockSource(char *myBuf)
*
* explanation: returns a text string containing
*  the name of the current clock source.
*
*  Presumably this is what is used by clock_gettime()
*
* inputs: 
*    char *myBuf : caller supplied buffer
*
* outputs: returns  NOERROR or  ERROR.
*
* Details 
*
********************************************************************/
int getCurrentClockSource(char *myBuf, int bufSize)
{
  int rc = NOERROR;
  FILE *fd = NULL;
  int itemsRead;
  struct timespec ts; 
  double res=0;
  clock_t mostAccurateClock=CLOCK_UNKNOWN;
  double mostAccurateRes=100.100; //set high


   //Find if the 4 clocks of interest are supported...and check the resolution
   rc = clock_getres(CLOCK_REALTIME, &ts); 
   if (rc == NOERROR) {
      res = convertTS2D(&ts);
      if (res < mostAccurateRes) {
        mostAccurateRes = res;
        mostAccurateClock = CLOCK_REALTIME;
      }
#ifdef TRACEME 
      printf("getCurrentClockSource: resolution CLOCK_REALTIME:\t%2.12f \n", res);
#endif
   }
   else
     isThereCLOCK_REALTIME = false;


   rc = clock_getres(CLOCK_MONOTONIC, &ts); 
   if (rc == NOERROR) {
      res = convertTS2D(&ts);
      if (res < mostAccurateRes) {
        mostAccurateRes = res;
        mostAccurateClock = CLOCK_MONOTONIC;
      }
#ifdef TRACEME 
      printf("getCurrentClockSource: resolution CLOCK_MONOTONIC:\t%2.12f \n", res);
#endif
   }
   else
     isThereCLOCK_MONOTONIC = false;

   rc = clock_getres(CLOCK_MONOTONIC_RAW, &ts); 
   if (rc == NOERROR) {
      isThereCLOCK_MONOTONIC_RAW = true;
      res = convertTS2D(&ts);
      if (res < mostAccurateRes) {
        mostAccurateRes = res;
        mostAccurateClock = CLOCK_MONOTONIC_RAW;
      }
#ifdef TRACEME 
      printf("getCurrentClockSource: resolution CLOCK_MONOTONIC_RAW:\t%2.12f \n", res);
#endif
   }
   else
     isThereCLOCK_MONOTONIC_RAW = false;

   isThereCLOCK_BOOTTIME = false;
#ifdef LINUX
   rc = clock_getres(CLOCK_BOOTTIME, &ts); 
   if (rc == NOERROR) {
      isThereCLOCK_BOOTTIME = true;
      res = convertTS2D(&ts);
      if (res < mostAccurateRes) {
        mostAccurateRes = res;
        mostAccurateClock = CLOCK_BOOTTIME;
      } 
#ifdef TRACEME 
      printf("getCurrentClockSource: resolution CLOCK_BOOTTIME:\t%2.12f \n", res);
#endif
   }
   else
     isThereCLOCK_BOOTTIME = false;
#endif

#ifdef TRACEME 
    printf("getCurrentClockSource: most accurate clock: %ld,  res:%2.12f\n", mostAccurateClock, mostAccurateRes);
    printf("getCurrentClockSource: the system supports these clocks :  \n");
    if (isThereCLOCK_REALTIME)
      printf("  CLOCK_REALTIME (%d) ", CLOCK_REALTIME);
    if (isThereCLOCK_MONOTONIC)
      printf("  CLOCK_MONOTONIC (%d) ", CLOCK_MONOTONIC);
    if (isThereCLOCK_MONOTONIC_RAW)
      printf("  CLOCK_MONOTONIC_RAW (%d) ", CLOCK_MONOTONIC_RAW);
    if (isThereCLOCK_BOOTTIME)
#ifdef LINUX
      printf("  CLOCK_BOOTTIME (%d) \n", CLOCK_BOOTTIME);
#else
      printf("  CLOCK_BOOTTIME NO-not on this platform  \n");
#endif
#endif 

  fd =  fopen(clockSrcFile, "r");
  if (!fd) {
    rc = ERROR;
#ifdef TRACE_ERRORS
    printf("getCurrentClockSource:  ERROR on fopen  %d \n",errno); 
#endif
  } else {

    // on error, the fread does not return -1, might be >=0 
    itemsRead = fread(myBuf, sizeof(char), bufSize, fd);
    myBuf[itemsRead]='\0';  //add string terminator
    //ferror returns non zero if fread failed....
    if ( ferror(fd) !=0 ) {
      rc = ERROR;
#ifdef TRACE_ERRORS
      printf("getCurrentClockSource:  ERROR on fread -   errno : %d \n", errno);
      perror("getCurrentClockSource: Error ?? ");
#endif
    } 

#ifdef TRACEME 
    if (rc == NOERROR) {
      printf("getCurrentClockSource: itemsRead:%d, currentClockSource::%s \n", 
          itemsRead, myBuf);
    }
#endif 
  }

  if (!isThereCLOCK_REALTIME) {
     
     //wallClockType = ?? 
     rc = ERROR;
     printf("getCurrentClockSource: HARD ERROR:  no CLOCK_REALTIME ?? \n");
     exit(EXIT_FAILURE);
  }
//uint32_t wallClockType = CLOCK_REALTIME;
//clock_t wallClockTimeSource = CLOCK_REALTIME;
//uint32_t timestampType = CLOCK_REALTIME;
//clock_t timestampTimeSource = CLOCK_REALTIME;
  if (!isThereCLOCK_MONOTONIC_RAW) {
     if (isThereCLOCK_MONOTONIC) {
       defaultTimestampClockType = CLOCK_GETTIME;
       defaultTimestampClockSource = CLOCK_MONOTONIC;
     }
     else{   
       if (isThereCLOCK_REALTIME) {
       defaultTimestampClockType = CLOCK_gettimeofday;
       defaultTimestampClockSource = CLOCK_REALTIME;
       }
       else {
         rc = ERROR;
         printf("getCurrentClockSource: HARD ERROR:  no CLOCK_REALTIME ?? \n");
         exit(EXIT_FAILURE);
       }
     }
  }
  return rc;
}

/********************************************************
* routine: clock_t   clockSourceStringToClockType(char *myBuf, int bufSize)
*
* explanation :  converts the clock type in string form to a valid clock_t
*
*   If a match is NOT found and the string is not empty,
*   a generic CLOCK_UNKNOWN type is returned.
*
*   Any error results in CLOCK_ERRONEOUS to be returned (value ERROR)
*
* inputs: 
*    char *myBuf : caller supplied buffer holding the string with the clock type
*    int bufSize : size of the string
*
* outputs: returns a valid system clock_t, or CLOCK_UNKNOWN or CLOCK_ERRONEOUS on error.
*
* Details/notes:
*
*************************************************************/
clock_t  clockSourceStringToClockType(char *myBuf, int bufSize)
{
  int rc = NOERROR;
  clock_t clockID =  CLOCK_UNKNOWN;
  if (myBuf == NULL) {
    printf("clockSourceStringToClockType:   HARD error, bad ptr param ! \n");
    rc = ERROR;
    clockID =  CLOCK_ERRONEOUS;
  } else {

    if (strcasecmp(myBuf, "CLOCK_REALTIME"))
      clockID =  CLOCK_REALTIME;
    else if (strcasecmp(myBuf, "CLOCK_MONOTONIC"))
      clockID =  CLOCK_MONOTONIC;
    else if (strcasecmp(myBuf, "CLOCK_MONOTONIC_RAW")) 
      clockID =  CLOCK_MONOTONIC_RAW;
    else if (strcasecmp(myBuf, "CLOCK_PROCESS_CPUTIME_ID"))
      clockID =  CLOCK_PROCESS_CPUTIME_ID;
    else if (strcasecmp(myBuf, "CLOCK_RDTSC")) 
      clockID =  CLOCK_RDTSC;
#ifdef LINUX
    else if (strcasecmp(myBuf, "CLOCK_BOOTTIME")) 
      clockID =  CLOCK_BOOTTIME;
#endif
    else if (strcasecmp(myBuf, "CLOCK_TIME")) 
      clockID =  CLOCK_TIME;
    else if (strcasecmp(myBuf, "CLOCK_gettimeofday"))
      clockID =  CLOCK_gettimeofday;
    else{ 
      clockID =  CLOCK_UNKNOWN;
      rc = ERROR;
    }
  }

#ifdef TRACEME 
  if (rc == NOERROR) 
    printf("clockSourceStringToClockType:  SUCCEEDED, %s is value  %ld \n",myBuf,clockID);
  else 
    printf("clockSourceStringToClockType:  FAILED,  %s does not match a known value \n",myBuf);
#endif 

 return clockID;

}

/********************************************************
* routine: int  clockSourceClockTypeToString(clock_t clockID, char *myBuf, int bufSize);
*
* explanation :  converts the clock_t to string form
*
* inputs: 
*    clock_t clockID : caller's ID
*    char *myBuf : caller supplied buffer to hold the  string form of the clock type
*    int bufSize : size of the string
*
* outputs: returns an ERROR or NOERROR
*
* Details/notes:
*
*************************************************************/
int  clockSourceClockTypeToString(clock_t clockID, char *myBuf, int bufSize)
{
  int rc = NOERROR;

#ifdef TRACEME 
  if (rc == NOERROR) 
    printf("clockSourceClockTypeToString:  SUCCEEDED, returning %ld \n",clockID);
  else 
    printf("clockSourceClockTypeToString:  FAILED !!\n");

#endif 

  if ((myBuf == NULL) || (bufSize < 32)) {
    printf("clockSourceClockTypeToString:   HARD error, bad ptr param or bufSize (%d) ! \n",bufSize);
    rc = ERROR;
  } else {

    switch(clockID) {

     case CLOCK_RDTSC:
       strcpy(myBuf,"CLOCK_RDTSC");
       break;

     case CLOCK_REALTIME:
       strcpy(myBuf,"CLOCK_REALTIME");
       break;

     case CLOCK_MONOTONIC:
       strcpy(myBuf,"CLOCK_MONOTONIC");
       break;

     case CLOCK_MONOTONIC_RAW:
       strcpy(myBuf,"CLOCK_MONOTONIC_RAW");
       break;

#ifdef LINUX
     case CLOCK_BOOTTIME:
       strcpy(myBuf,"CLOCK_BOOTTIME");
       break;
#endif

     case CLOCK_TIME:
       strcpy(myBuf,"CLOCK_TIME");
       break;

     case CLOCK_gettimeofday:
       strcpy(myBuf,"CLOCK_gettimeofday");
       break;

      default:
        printf("clockSourceClockTypeToString:  bad clockID :  %ld \n",clockID);
        rc = ERROR;
        break;
    }
  }

  return rc;

}

/****************************************
* routine: int getClockInfo(char *bufPtr, int maxSize)
*
* explanation: Places into the callers buffer
*    a string indicating current clock and available clocks
*
*  
* inputs: 
*  char *bufPtr : caller's buffer ptr 
*  int maxSize : max size of caller's buffer
*
* outputs: returns the clock_t reflecting
*        the current clock
*         else ERROR
*
* Details/notes:
*
****************************************/
int getClockInfo(char *bufPtr, int maxSize)
{
  int rc = NOERROR;
  char myBuf1[MAX_LINE_SIZE];
  char myBuf2[MAX_LINE_SIZE];
  char *myBuf=myBuf1;
  int bufSize  = maxSize;
  clock_t clockType = CLOCK_ERRONEOUS;
  clock_t currentClockType = CLOCK_ERRONEOUS;

  rc = getCurrentClockSource(myBuf1,bufSize);
  if (rc == NOERROR)  
  {
    rc =  getAvailableClockSources(myBuf2,  bufSize);
    if (rc == NOERROR) 
    {
      printf ("getClockInfo:cur:%s, avail:%s \n", myBuf1,myBuf2);
    }
  }

  //Convert the current clock source to its value
  rc = getCurrentClockSource(myBuf,bufSize);
  bufSize =   (int) strlen((const char *) myBuf);
  // >0 implies success.  
  if (bufSize <= 0)  {
    printf ("getClockInfo:  ERROR on strlen before calling clockSourceStringToType, string: %s  bufSize:%d\n",
              myBuf, bufSize);
    clockType = CLOCK_ERRONEOUS;
  } else 
  {
    clockType  = clockSourceStringToClockType(myBuf, bufSize);
    if (rc != CLOCK_ERRONEOUS) 
      printf ("getClockInfo:  clockSourceStringToClockType: %s maps to a clock_t of %ld \n",myBuf, clockType);
    else
      printf ("getClockInfo:  ERROR on clockSourceStringToClockType, %s  returns clock_t %ld \n",myBuf, clockType);
  }
  if (rc == NOERROR) {
    rc = (int) currentClockType;
  }
  return rc;
}

/***********************************************************
* Function: int setDefaultWallClockType(clocktype_t wallClockType)
*
* Explanation: sets default wall clock type
*
* inputs: 
*     clocktype_t wallClockType
*
* outputs:  Returns NOERROR 
*
* notes: 
*
***********************************************************/
int setDefaultWallClockType(clocktype_t wallClockType)
{
  int rc = NOERROR;

  defaultWallClockType = wallClockType;

  return rc;
}

/***********************************************************
* Function: int setDefaultWallClockSource(clock_t wallClockSource)
*
* Explanation: sets default wall clock source 
*
* inputs: 
*    clock_t wallClockSOurce   
*
* outputs:  Returns NOERROR 
*
* notes: 
*
***********************************************************/
int setDefaultWallClockSource(clock_t wallClockSource)
{
  int rc = NOERROR;

  defaultWallClockSource = wallClockSource;

  return rc;
}

/***********************************************************
* Function: int setDefaultTimestampClockType(clocktype_t timestampType)
*
* Explanation: sets default timestamp clock source 
*
* inputs: 
*    clock_t timestampClockSource
*
* outputs:  Returns NOERROR 
*
* notes: 
*
***********************************************************/
int setDefaultTimestampClockType(clocktype_t timestampType)
{
  int rc = NOERROR;
  defaultTimestampClockType = timestampType;
  return rc;
}

/***********************************************************
* Function: int setDefaultTimestampClockSource(clock_t timestampClockSource)
*
* Explanation: sets default timestamp clock source 
*
* inputs: 
*    clock_t timestampClockSource
*
* outputs:  Returns NOERROR 
*
* notes: 
*
***********************************************************/
int setDefaultTimestampClockSource(clock_t timestampClockSource)
{
  int rc = NOERROR;
  defaultTimestampClockSource = timestampClockSource;
  return rc;
}

/***********************************************************
* Function: clocktype_t getDefaultWallClockType()
*
* Explanation:Returns default wall clock type
*
* inputs: 
*
* outputs:  Returns default wall clock source 
*
* notes: 
*
***********************************************************/
clocktype_t getDefaultWallClockType()
{
  return defaultWallClockType;
}


/***********************************************************
* Function: clock_t getDefaultWallClockSource()
*
* Explanation:Returns default wall clock source
*
* inputs: 
*
* outputs:
* Returns default wall clock source 
*
* notes: 
*
***********************************************************/
clock_t getDefaultWallClockSource()
{
  return defaultWallClockSource;
}

/***********************************************************
* Function: clocktype_t getDefaultTimestampClockType()
*
* Explanation:Returns default timestamp clock type
*
* inputs: 
*
* outputs:
* Returns default timestamp clock type
*
* notes: 
*
***********************************************************/
clocktype_t getDefaultTimestampClockType()
{
  return defaultTimestampClockType;
}

/***********************************************************
* Function: clock_t getDefaultTimestampClockSource();
*
* Explanation:Returns default timestamp clock source
*
* inputs: 
*
* outputs:
* Returns default timestamp clock source
*
* notes: 
*
***********************************************************/
clock_t getDefaultTimestampClockSource()
{
  return defaultWallClockSource;
}





#ifdef LINUX
/****************************************
* routine: int getRTC(struct rtc_time *myRTCTime)
*
* explanation: obtains the wall clock time
*            sourced from the RTC 
*
* inputs: 
*
* outputs: returns to standard out NOERROR or  ERROR.
*
* Details/notes:
*
    struct rtc_time 
       int tm_sec;
       int tm_min;
       int tm_hour;
       int tm_mday;
       int tm_mon;
       int tm_year;
       int tm_wday;     
       int tm_yday;     
       int tm_isdst;   
*
****************************************/
int getRTC(struct rtc_time *myRTCTime)
{
  int rc = NOERROR;
  int dd = ERROR;  //device descriptor
  long int RTCData=0;

//could not get this to work
  //Issue 'man 2 open' for details on this system call
  dd = open("/dev/rtc0",O_RDONLY); 
  if (dd == ERROR) {
    rc = ERROR;
    printf("getRTC: ERROR open dev rtc??  errno:%d \n",errno); 
    perror("getRTC: ERROR open dev rtc: ");
    exit(EXIT_FAILURE);
  } 
#if 0
  else  {
    rc = read(dd,&RTCData, sizeof(long int));
    if (rc == ERROR) { 
      perror("getRTC: ERROR read ");
      exit(EXIT_FAILURE);
    } else {
      printf("getRTC: succeded to read, %lx   \n", RTCData);
    }
    rc = close(dd);
    if (rc == ERROR) { 
      perror("getRTC: ERROR close ");
      exit(EXIT_FAILURE);
    } else {
      printf("getRTC: succeded to close \n");
    }
  }
#endif
  //     RTC_RD_TIME
  rc =  ioctl(dd, RTC_RD_TIME, (void *) myRTCTime);
  if (rc == ERROR) { 
    printf("getRTC: ERROR ioctl   errno:%d \n",errno); 
    perror("getRTC: ERROR ioctl ");
    exit(EXIT_FAILURE);
  } else { 
   rc = NOERROR;
   printf("getRTC: ioctl succeeded, hr:min:sec: %d:%d:%d  \n",
        myRTCTime->tm_hour, myRTCTime->tm_min, myRTCTime->tm_sec);
  }
  return rc;

}
#endif


/********************************
* SET 5:  Helper Routines- convert or manipulate timestamps
*
*   bool isTS1GTTS2(struct timespec *TS1, struct timespec *TS2) 
*   double convertTS2D(struct timespec *t);
*   int convertD2TS(double *timeDouble,  struct timespec *timeTSpec);
*   uint64_t  diffTSpecs(struct timespec *start_time, struct timespec *end_time); //finds time diff and returuns in nanoseconds
*   uint64_t  getNanoSeconds(struct timespec *t);  //converts a ts to nanoseconds
*
*   The following operate on timeval's
*   double convertTimeval(struct timeval *t);
*   uint32_t getTimeSpan(struct timeval *start_time, struct timeval *end_time);
*   uint64_t getTimeSpanTS(struct timespec *start_time, struct timespec *end_time);
*   uint32_t getMicroseconds(struct timeval *t);
*
***********************************/

/*************************************************************
* Function: bool isTS1GTTS2(struct timespec *TS1, struct timespec *TS2) 
* 
* Summary: returns true if the first timestamp is greater than the second  TS
*
* Inputs: 
*     struct timespec *TS1: first timestamp
*     struct timespec *TS2: second timestapm
*
* outputs:  
*   returns  true if TS1>TS2.  returns false if TS1<=TS2.
************************************************/
bool isTS1GTTS2(struct timespec *TS1, struct timespec *TS2) 
{
  bool rc = false;
  if (TS1->tv_sec > TS2->tv_sec ||					\
     ((TS1->tv_sec == TS2->tv_sec) && (TS1->tv_nsec > TS2->tv_nsec))) {
	rc = true;
  }
  return rc;
}

/*************************************************************
* Function: bool isZeroTime(struct timespec *myTS);
* 
* Summary: returns true if the timespec is zero
*
* Inputs:   
*   struct timespect *myTS:   caller's timestamp
*
* outputs: returns true or false
*   A true is returned for a bad ptr
*
*************************************************************/
bool isZeroTime(struct timespec *myTS)
{
  bool rc = false;

  if (myTS == NULL) {
    printf("isZeroTime: HARD ERROR:  bad myTS param ??\n");
      rc = true;
      //exit(EXIT_FAILURE);
  } else {
    if ((myTS->tv_sec == 0) && (myTS->tv_nsec == 0) )
      rc = true;
  }
  return rc;
}

/*************************************************************
* Function: bool isLessThanOne(struct timespect *myTS);
* 
* Summary: returns true if the timespec is <1 
*          If nsecs > 1000000000, the sec field is adjusted
*
* Inputs:   
*   struct timespect *myTS:   caller's timestamp
*
* outputs: returns true or false
*   A true is returned for a bad ptr
*
*************************************************************/
bool isLessThanOne(struct timespec *myTS)
{
  bool rc = false;

  if (myTS == NULL) {
    printf("isZeroTime: HARD ERROR:  bad myTS param ??\n");
      rc = true;
      //exit(EXIT_FAILURE);
  } else {
    if (myTS->tv_sec == 0)
      rc = true;
    else {
      if (myTS->tv_nsec > 1000000000) {
       printf("isZeroTime: HARD ERROR: nsec > 1 sec %ld\n",myTS->tv_nsec);
          ;
      }
    }
  }
  return rc;
}

/*************************************************************
* Function: int convertTimespecToString(char *callersTimeString, struct timespec *myTS)
* 
* Summary: converts the caller's timestamp (a timespec) to
*          a human readable string which is placed in the callersTimeString buffer.
*
* Inputs:   
*   char *callersTimeString: reference to caller's buffer to hold the string
*   int maxSize:  size of caller's buffer
*   struct timespect *myTS:   caller's timestamp
*
* outputs: returns ERROR or number of characters in the string
*   One error is if the caller's buffer is  not large enough
*
*************************************************************/
int convertTimespecToString(char *callersTimeString,int maxSize, struct timespec *myTS)
{
  int rc = NOERROR;
  int dataSize = MAX_TMP_BUFFER;
  long int testTime;
  char *timeString=NULL;
  time_t timeInSeconds = 0;
  int sizeOfString=0;

  double callersTime = convertTS2D(myTS);
#ifdef TRACEME
  printf("convertTimespecToString: callers Time: %9.9f maxSize:%d \n",callersTime,maxSize);
#endif

  if (myTS == NULL)
   rc = ERROR;
  else {
   timeInSeconds = myTS->tv_sec;
   timeString  = malloc((size_t)MAX_LINE_SIZE); 
   bzero((void *)timeString, (size_t) MAX_LINE_SIZE);
   timeString =  ctime((const time_t *)&timeInSeconds);
   sizeOfString = strlen(timeString);
   if (timeString != NULL) {
#ifdef TRACEME
      printf("convertTimespecToString: size:%d %s \n",sizeOfString, timeString);
#endif
     if (sizeOfString <= maxSize) {
       // char *strcpy(char *dest, const char *src);
       // char *strncpy(char *dest, const char *src, size_t n);
       //strncpy(callersTimeString, timeString, (size_t) maxSize);
       strcpy(callersTimeString, timeString);
     } else 
       rc = ERROR;

   } else {
     rc = ERROR;
   }
  }

  if (rc == NOERROR) {
   rc = sizeOfString;
  }

#ifdef TRACEME
  if (rc == ERROR) {
    printf("convertTimespecToString: ERROR ??  size:%d %s \n",sizeOfString, timeString);
  } else {
    printf("convertTimespecToString: SUCCESS:  return size:%d %s \n",rc,  timeString);
  }
#endif

  return rc;
}

/***********************************************************
* Function:  double convertTGIFTS2D(struct TGIF_timespec *t) 
*
* Explanation:  This converts the timestamp from 
*             a timespec into a double
*
* inputs: 
*  struct TGIF_timespec *callersTime - the caller's ptr to the timestamp
*
* outputs:
*        returns the timestamp as a double in units of seconds with nsec precision
*
* notes: 
*
***********************************************************/
double convertTGIFTS2D(struct TGIF_timespec *t) {
  return ( t->ts_sec + ( (double) t->ts_nsec)/1000000000 );
}


/***********************************************************
* Function:  double convertTS2D(struct timespec *t) 
*
* Explanation:  This converts the timestamp from 
*             a timespec into a double
*
* inputs: 
*  struct timespec *callersTime - the caller's ptr to the timestamp
*
* outputs:
*        returns the timestamp as a double in units of seconds with nsec precision
*
* notes: 
*
***********************************************************/
double convertTS2D(struct timespec *t) {
  return ( t->tv_sec + ( (double) t->tv_nsec)/1000000000 );
}


/***********************************************************
* Function: int convertD2TS(double *timeDouble,  struct timespec *ts) 
*
* Explanation:  This converts the timestamp from a double to a timespec
*
* inputs: 
*    double *timeDouble: ptr to callers time in double format
*    struct timespec *timeTSpec : ptr to callers timespec that 
*             is to be filled in
*
*  struct timespec *callersTime - the caller's ptr to the timestamp
*
* outputs:
*        returns ERROR or NOERROR
*
* notes: 
*
***********************************************************/
int convertD2TS(double *timeDouble,  struct timespec *ts) 
{
  uint64_t delaySecs = 0;
  uint64_t delayNsecs = 0;
  int rc = NOERROR;

  if (*timeDouble >= 1.0)
    delaySecs =  (uint64_t)floor(*timeDouble);

  delayNsecs = (uint64_t)( 1000000000 * (*timeDouble - (double) delaySecs)); 
  
  ts->tv_sec = delaySecs;
  ts->tv_nsec = delayNsecs;


  return rc;

}


/***********************************************************
* Function: uint64_t diffTSpecs(struct timespec *ts1, struct timespec *ts2) 
*
* Explanation: finds the time difference between the two timespec params
*
* inputs: 
*  struct timespec *ts1 - the caller's first TS
*  struct timespec *ts2 - the caller's second TS
*
* outputs:
*        returns the time difference in units of nanoseconds
*
* notes: 
*  This should be faster than if implemented with doubles
*  (TODO:  on 64bit mchines, set diff asa register variable)
*
***********************************************************/
uint64_t diffTSpecs(struct timespec *ts1, struct timespec *ts2)
{
  uint64_t diff = 0;

  diff = (uint64_t)(ts2->tv_sec - ts1->tv_sec)*(1*1000*1000*1000);
  diff += (uint64_t)(ts2->tv_nsec - ts2->tv_nsec);

  return diff;
}

/***********************************************************
* Function: uint64_t  getNanoSeconds(struct timespec *t) 
*
* Explanation:  This converts the timestpec to a time in nanosecions
*
* inputs: 
*  struct timespec *callersTime - the caller's ptr to hold the time
*
* outputs:
*  returns the timestamp in units of nanoseconds
*
* notes: 
*
***********************************************************/
uint64_t  getNanoSeconds(struct timespec *ts) 
{
  uint64_t TimeStamp = 0;
  TimeStamp = ( (uint64_t) ts->tv_sec) * 1000000000;
  TimeStamp +=  (uint64_t) (ts->tv_nsec);
  return (TimeStamp);
}


/***********************************************************
* Function: double convertTimeval(struct timeval *t) {
*
* Explanation:  This converts the timestamp from a timeval into a double
*
* inputs: 
*  struct timeval *callersTime - the caller's ptr to the timestamp
*
* outputs:
*        returns the timestamp as a double in units of seconds with usec precision
*
* notes: 
*
***********************************************************/
double convertTimeval(struct timeval *t) {
  return ( t->tv_sec + ( (double) t->tv_usec)/1000000 );
}

/***********************************************************
* Function: uint32_t getTimeSpan(struct timeval *start_time, struct timeval *end_time) {
*
* Explanation:  Returns the dfference in time between
*               two timeval times. 
* inputs: 
*         struct timeval *start_time
*         struct timeval *end_time) 
*
* outputs:
*          A 32 bit unsigned int that represents the time difference in units of
*               microseconds.
*
* notes: 
*
***********************************************************/
uint32_t getTimeSpan(struct timeval *start_time, struct timeval *end_time) {
  uint32_t usec2 = getMicroseconds(end_time);
  uint32_t usec1 = getMicroseconds(start_time);
  return (usec2 - usec1);
}
/***********************************************************
* Function: uint64_t getTimeSpan(struct timeval *start_time, struct timeval *end_time) {
*
* Explanation:  Returns the dfference in time between
*               two timespec times. 
* inputs: 
*         struct timespec *start_time
*         struct timespec *end_time) 
*
* outputs:
*          A 64 bit unsigned int that represents the time difference in units of
*               nanoseconds.
*           Or if we see a negative number might occur, we return ERROR
*
* notes: 
*
***********************************************************/
uint64_t getTimeSpanTS(struct timespec *start_time, struct timespec *end_time)
{ 
  uint64_t rc = NOERROR;
  uint64_t nsecs =0; 

  uint64_t nsec2 = getNanoSeconds(end_time);
  uint64_t nsec1 = getNanoSeconds(start_time);
  if (nsec1 > nsec2) 
    rc = ERROR;
  else 
    rc = nsec2 - nsec1;

  return rc;
}

/***********************************************************
* Function: uint32_t getMicroseconds(struct timeval *t) 
*
* Explanation:  This converts the timestamp from a timeval into microseconds.
*
* inputs: 
*  struct timespec *callersTime - the caller's ptr to hold the time
*
* outputs:
*        returns the timestamp in units of microseconds.
*
* notes: 
*
***********************************************************/
uint32_t getMicroseconds(struct timeval *t) 
{
  return (t->tv_sec) * 1000000 + (t->tv_usec);
}




/******************************************************************
*
* SET 10: functions that are experimental or not used (yet) 
*
*
*          uint64_t rdtsc(void)
*          uint64_t tsc() 
*          uint64_t glibc_nsec(uint64_t tsc, uint64_t freq) 
*          uint64_t clockCycleCount()
*          
*          double gettimeofday_benchmark(uint32_t numberSamples)
*          double testGetTime(clock_t clockType) 
*
*
*****************************************************************/



/*************************************************************
* Function: uint64_t rdtsc(void)
* 
* Summary: reads the TSC counter directly 
*
* Inputs:  
*
* outputs:returns the counter value - it is in units of 'CPU cycles'
*
* Notes:
*   To convert to nanoseconds:
*    ns = CPUCycles * (ns_per_sec / CPU freq)
*
*   Intel CPUs has issues with TSC
*      1.  Variant TSC -  TSC inc impacted by CPU freq changes (on old CPUs)
*      2.  constant TSC - Corrects above but TSC can be stopped in deep C-state (??)
*      3   invariant TSC -  -on latest CPUs
*
*   Try this to see which TSC is support:
*         cat  /proc/cpuinfo | grep -E "constant_tsc|nonstop_tsc"
* 
*      X86_FEATURE_TSC :  TSC is available in CPU 
*      X86_FEATURE_CONSTANT_TSC :  CPU has constant TSC (over cores)
*      X86_FEATURE_NONSTOP_TSC  - 
*          The last two are the best - referred to as invariant TSC
*      X86_FEATURE_TSC_RELIABLE - also good
*       
*
* Example delay calculation
*    start = rdtsc();
*          put code here 
*
*       end = rdtsc();
*        cycle = end - start;
*   latency = cycle_2_ns(cycle)
*
*  TSC_Value = (ART_Value * CPUID.15H:EBX[31:0] )/ CPUID.15H:EAX[31:0] + K
*
*   lscpu | grep MHz
*   watch -n1 "lscpu | grep MHz | awk '{print $1}'";
*
*
**********************************************************/
uint64_t rdtsc(void)
{
    uint32_t low =0;
    uint32_t high =0;
//    asm volatile("rdtsc":"=a"(low),"=d"(high));
    return ((uint64_t)high << 32) | low;
}

uint64_t tsc() 
{
    uint32_t low =0;
    uint32_t high =0;
//  __asm__ __volatile__("rdtsc" : "=a" (low), "=d" (high));
  uint64_t r=((uint64_t) high<<32) | low;
  return r;
}


/* From sysdeps/unix/clock_gettime.c:
   "And the nanoseconds.  This computation should be stable until
    we get machines with about 16GHz frequency."
*/
uint64_t glibc_nsec(uint64_t tsc, uint64_t freq) 
{
  return ((tsc % freq) * UINT64_C (1000000000)) / freq;
}

#if 0
uint64_t clockCycleCount()
{
   uint64_t c;
    __asm {
                cpuid       // serialize processor
                rdtsc       // read time stamp counter
                mov dword ptr [c + 0], eax
                mov dword ptr [c + 4], edx
            }
            return c;
}
#endif


/*************************************************************
* Function: double testGetTime(clock_t clockType, struct timespec *ts) 
* 
* Summary: gets the time using the specified time source
* 
*  NOTE: This is experimental. It is used to allow us
*        to further develop our time related functions. 
*
* Inputs:  
*   myClockSource:  can be standard get_clock () ...
*                  Additional should start at MAX_CLOCKS+ x 
*          CLOCK_RDTSC -  real time clock accessed directly
*          CLOCK_REALTIME - will get clock using TSC through
*                   gettimeofday
*          CLOCK_MONOTONIC - used for timeing/delay measurements
*          CLOCK_MONOTONIC_RAW - used for timeing/delay measurements
*                            not subject to time sync catchups
*                           
*         
*
* outputs:  timestamp in double format, an -1.0 on error;
*           timestamp in units of seconds. Precision depends on clock choice
*
* Notes:
*  clock_t is /usr/include/linux/time.h.
*  #s...0 -  11  with MAX_CLOCKS set 16
*
*************************************************************/
double testGetTime(clock_t clockType) 
{

  struct timeval myTime;
  struct timespec curTime;
  int rc = 0;
  double rcTimestamp=-1.0;
  uint64_t tmpX;
  uint64_t freq = 2496000000;

  switch(clockType) {

    case CLOCK_RDTSC:
        tmpX = rdtsc();
        if (tmpX == 0) {
#ifdef TRACE_ERRORS
          printf("testGetTime: Error on rdtsc  errno:%d (clockType:%ld) \n",
           errno,clockType);
#endif
          rcTimestamp = -1.0;
        }
        else{
          rcTimestamp = (double) (((double) glibc_nsec(tmpX, freq)) / 1000000000); 
        }

    break;

    case CLOCK_REALTIME:
      //Use clock_gettime
      tmpX =clock_gettime(clockType, &curTime);
      if (tmpX==0) { 
        rcTimestamp = (double)curTime.tv_sec + ((double)curTime.tv_nsec)/1000000000;
      }
      else{
#ifdef TRACE_ERRORS
        printf("testGetTime: Error on gettimeofday, errno:%d (clockType:%ld) \n",
           errno,clockType);
#endif
        rcTimestamp = -1.0;
      }

    break;

    case CLOCK_MONOTONIC:
      //Use clock_gettime
      tmpX =clock_gettime(clockType, &curTime);
      if (tmpX==0) { 
        rcTimestamp = (double)curTime.tv_sec + ((double)curTime.tv_nsec)/1000000000;
      }
      else{
#ifdef TRACE_ERRORS
        printf("testGetTime: Error on gettimeofday, errno:%d (clockType:%ld) \n",
           errno,clockType);
#endif
        rcTimestamp = -1.0;
      }
    break;

    case CLOCK_MONOTONIC_RAW:
      //Use clock_gettime
      rc =clock_gettime(clockType, &curTime);
      if (rc==0) { 
        return ((double)curTime.tv_sec + ((double)curTime.tv_nsec)/1000000000);
      }
      else{
#ifdef TRACE_ERRORS
          printf("testGetTime: Error on gettimeofday, errno:%d (clockType:%ld) \n",
           errno,clockType);
#endif
        rcTimestamp = -1.0;
      }
    break;

#ifdef LINUX
    case CLOCK_BOOTTIME:
      //Use clock_gettime
      rc =clock_gettime(clockType, &curTime);
      if (rc==0) { 
        return ((double)curTime.tv_sec + ((double)curTime.tv_nsec)/1000000000);
      }
      else{
#ifdef TRACE_ERRORS
          printf("testGetTime: Error on gettimeofday, errno:%d (clockType:%ld) \n",
           errno,clockType);
#endif
        rcTimestamp = -1.0;
      }
    break;

#endif

    default:
      printf("testGetTime: Error on clockType: %ld \n",clockType);
      rcTimestamp = -1.0;
  }

    return rcTimestamp;
}





/*********************************************************
* Module Name:  sysInfoHelper  
*       routines that provide functions related to obtaining info about the system
*       Uses primarily uname system call.  
*       We could use sys_info but this would not compile on MACOS
*
* NOTE:  Must call initSysInfo routine!! 
*
* State mainted by the module...see the below 
*  The two main worker routines:
*    int getSysInfo(int platformType, int infoType, char *bufPtr, uint32_t maxSize)
*    int getRunTimeInfo(int platformType, int infoType, char *bufPtr, uint32_t maxSize)
*  Return the information to the caller's buffer -
*  The information is also retained locally in the buffers 
*   char *systemInfoBufPtr=NULL;
*   char *systemStatusBufPtr=NULL;
*
*
* File Name:     sysInfoHelper.c 
*
* Last update: 10/5/2019
* 
*********************************************************/
#include "common.h"
#include "sysInfoHelper.h"
#include "timeHelper.h"
#include "utils.h"

//#define TRACEME 0
#define TRACE_ERRORS 0

#define BRIEF_MODE 0

/***********************************************
* state - the following globals are set by the
*          init routine
*
************************************************/
double startTime = -1.0;
bool sysInfoInitFlag =  false;
bool isBigEndianFlag = false;

int  maxPipeSize = -1;
int  maxBufferSize = -1;
int  bufferSize= -1;

//If not NULL, a malloc has allocated memory.
//To resuse, if not NULL free and then malloc
//fget bufs are MAX_LINE_SIZE
//popen pipes usually involve PIPE_BUF  - a ssystem defined variable
//MAX_BUFFER is for largest buffer

char *systemInfoBufPtr=NULL;
char *systemStatusBufPtr=NULL;

double curWallClockTime;
char wallClockTimeLine[MAX_LINE_SIZE];
char *wallClockTimePtr = wallClockTimeLine;
char currentClockSourceLine[MAX_LINE_SIZE];
char availableClockSourcesLine[MAX_LINE_SIZE];

struct utsname uname_info;
//Will hold the results from uname
// The struct utsname has the following fields
// All arrays are strings
//   char sysname[];    
//   Operating system name (e.g., "Linux") */
//   char nodename[];    Name within "some implementation-defined net
//  char release[];    Operating system release (e.g., "2.6.28") 
//  char version[];    Operating system version 
//  char machine[];     Hardware identifier 
//  #ifdef _GNU_SOURCE
//  char domainname[]; 
//  #endif

//Need to confirm size of the tv and ts fields
//struct timeval {
//  time_t      tv_sec;    
// suseconds_t tv_usec;  
// };
//struct timespec {
// time_t   tv_sec;    
// long     tv_nsec;  
// };


uint32_t sizeDouble;
uint32_t sizeInt;
uint32_t sizeLongInt;
uint32_t sizeLongLongInt;
uint32_t sizeVoidPtr;
uint32_t sizeClock_t;
uint32_t sizeTime_t;
uint32_t sizeSuseconds_t;
uint32_t sizeSize_t;
uint32_t sizeTS;
uint32_t sizeTV;

double startWallClockTime;
double curWallClockTime;
char   curWallClockTimeString[MAX_LINE_SIZE];
char *curWallClockTimePtr = curWallClockTimeString;

/****************************************
* routine: int resetSysInfoModule()
*
* explanation: prepares for the program
*    to be terminated. Places the module in
*    an uninit state.
*
* Inputs:
*
* Outputs: returns ERROR or NOERROR
*
* Details/notes:
*
*****************************************/
int resetSysInfoModule()
{
  int rc = NOERROR;

  startTime = -1.0;

  maxPipeSize = -1;
  maxBufferSize = -1;
  bufferSize= -1;

  bzero( (void *)curWallClockTimePtr, MAX_LINE_SIZE);

  bzero((void *)&uname_info, sizeof(struct utsname));

  bzero((void *)wallClockTimePtr, MAX_LINE_SIZE);
  bzero((void *)currentClockSourceLine, MAX_LINE_SIZE);
  bzero((void *)availableClockSourcesLine, MAX_LINE_SIZE);

  if (systemInfoBufPtr!=NULL)
   free(systemInfoBufPtr);
  if (systemStatusBufPtr!=NULL)
   free(systemStatusBufPtr);

  systemInfoBufPtr=NULL;
  systemStatusBufPtr=NULL;

  sizeDouble = 0;
 sizeInt = 0;
 sizeLongInt = 0;
 sizeLongLongInt = 0;
 sizeVoidPtr = 0;
 sizeClock_t = 0;
 sizeTime_t = 0;
 sizeSize_t = 0;
 sizeSuseconds_t=0;
 sizeTS = 0;
 sizeTV = 0;

  sysInfoInitFlag = false;

  return rc;
}

/****************************************
* routine: int initSysInfoModule()
*
* explanation: inits sysInfo module state data.
*     if sysInfoInitFlag is true
*     then this means the init
*     is being called after the initial init.
*     Non null ptrs must be freed before malloc'ed.
*
*  Output:  ERROR or NOERROR
*
* Details/notes:
*
* Might want to treat as a CR
*
****************************************/
int initSysInfoModule()
{
  void *voidPtr=NULL;
  int rc = NOERROR;

  maxPipeSize = PIPE_BUF;
  maxBufferSize = MAX_BUFFER;
  bufferSize= MAX_LINE_SIZE;

  bzero((void *)wallClockTimePtr, bufferSize);
  bzero((void *)currentClockSourceLine, bufferSize);
  bzero((void *)availableClockSourcesLine,bufferSize);

  if (systemInfoBufPtr!=NULL)
   free(systemInfoBufPtr);
  if (systemStatusBufPtr!=NULL)
   free(systemStatusBufPtr);


  systemInfoBufPtr = calloc(maxBufferSize, sizeof(char));
  systemStatusBufPtr = calloc(maxBufferSize,  sizeof(char));

   bzero((void *)&uname_info, sizeof(struct utsname));

   curWallClockTime = getWallClockTimeString(curWallClockTimePtr,bufferSize);
   startWallClockTime= curWallClockTime;
   startTime = getCurTimeD();
   systemInfoBufPtr = calloc(maxBufferSize, sizeof(char));
   bzero( (void *)curWallClockTimePtr, bufferSize);

  sizeDouble=sizeof(double);
  sizeInt = sizeof(int);
  sizeLongInt = sizeof(long int);
  sizeLongLongInt = sizeof(long long int);
  sizeVoidPtr = sizeof(voidPtr);
  sizeClock_t = sizeof(clock_t);
  sizeTime_t = sizeof(time_t);
  sizeSuseconds_t= sizeof(sizeSuseconds_t);

  sizeSize_t = sizeof(size_t);
  sizeTS = sizeof (struct timespec);
  sizeTV = sizeof (struct timeval);


  //call uname to fill in the utsname structure
  bzero((void *)&uname_info, sizeof(struct utsname));
  if (uname(&uname_info) != 0){
   perror("uname");
   exit(EXIT_FAILURE);
  }

  isBigEndianFlag = is_bigendian();

  sysInfoInitFlag = true;
#ifdef TRACEME
  printf("initSysInfo(%s):  PIPE_BUF:%d  \n",wallClockTimeLine,PIPE_BUF);
#endif
  return rc;
}



/***********************************************************
* Function: int getSystemInfoORG(int platformType, int infoType, char *bufPtr, uint32_t maxSize)
*
* Explanation:This fills in the caller's buffer with
*     a line that summarizes the system - details depend on platformType and infoType.
*
* inputs:  
*   platform:  0:Unix System 1:LinuxSystem 2:VehicularNode 3:Edge Node
*   infoType :  0:static system info; 1: Heartbeat
*   outputMode :  0:stdout 1:file 2:namedPipe 3:IPServer Name:port 4)MQTT broker 5)HTTPRestName
*   param1 :  Depends on outuptMode:  filename, pipename, server:port, mqtt
*
* outputs: returns ERROR or the number of characters placed in the bufPtr
*    Creates a copy of the  line of sys info in the global systemInfoBufPtr
*    And fills in the callers buffer with the line.  
*
* notes: 
*   Platform and inputType params not yet supported
*
**************************************************/
int getSysInfoORG(int platformType, int infoType, char *bufPtr, uint32_t maxSize)
{ 
  int  rc = NOERROR;
  char *cmdLine = NULL;
  uint32_t stringSizeCMD=0; //size of each cmd
  uint32_t stringSize=0; //maintains  the size in the sysInfo Line 
  char spaces[4] =" ";
  char *fgetLineBufPtr=NULL;
  char *cmdOutBufPtr = NULL;
  char *cmdInBufPtr=NULL;

  if (sysInfoInitFlag == false) {
   printf("getSysInfo: Failed -  not initialized !!! \n");
   rc = ERROR;
   exit(EXIT_FAILURE);
  } 
  //We know the two main buffers have been malloced  
  //char *systemInfoBufPtr=NULL;
  //char *systemStatusBufPtr=NULL;
  fgetLineBufPtr = calloc(MAX_LINE_SIZE, sizeof(char));
  cmdInBufPtr = calloc(MAX_LINE_SIZE,  sizeof(char));
  cmdOutBufPtr = calloc(maxBufferSize, sizeof(char));


#ifdef BRIEF_MODE 
  strcat(fgetLineBufPtr, uname_info.sysname);
  strcat(fgetLineBufPtr, spaces);
  strcat(fgetLineBufPtr, uname_info.machine);
  strcat(fgetLineBufPtr, spaces);
#else

  // char *strcat(char *dest, const char *src);
  //Now build the uname info string 
  strcat(fgetLineBufPtr, uname_info.sysname);
  strcat(fgetLineBufPtr, spaces);
  strcat(fgetLineBufPtr, uname_info.release);
  strcat(fgetLineBufPtr, spaces);
  strcat(fgetLineBufPtr, uname_info.version);
  strcat(fgetLineBufPtr, spaces);
  strcat(fgetLineBufPtr, uname_info.machine);
  strcat(fgetLineBufPtr, spaces);
#endif 

  strcpy(systemInfoBufPtr, fgetLineBufPtr);

#ifdef TRACEME1
 stringSize=strlen(systemInfoBufPtr);
 printf("getSysInfo: stringSize:%d, systemInfoBufPtr:%s \n",
          stringSize, systemInfoBufPtr);
#endif

  if (isBigEndianFlag) 
     sprintf(fgetLineBufPtr," bigEndian ");
  else  
     sprintf(fgetLineBufPtr," littleEndian ");

  strcat(systemInfoBufPtr, fgetLineBufPtr);

#ifdef BRIEF_MODE 
  sprintf(fgetLineBufPtr," sizeInt:%dbits ",(((uint32_t)sizeInt)*8));
  strcat(systemInfoBufPtr, fgetLineBufPtr);
  sprintf(fgetLineBufPtr," sizeLongInt:%dbits ",(((uint32_t)sizeLongInt)*8));
  strcat(systemInfoBufPtr, fgetLineBufPtr);
  sprintf(fgetLineBufPtr," sizeLongLongInt:%dbits ",(((uint32_t)sizeLongLongInt)*8));
  strcat(systemInfoBufPtr, fgetLineBufPtr);
  sprintf(fgetLineBufPtr," sizeVoidPtr:%dbits ",( ((uint32_t)sizeVoidPtr)*8));
  strcat(systemInfoBufPtr, fgetLineBufPtr);
  sprintf(fgetLineBufPtr," sizeClock_t:%dbits ",(((uint32_t)sizeClock_t)*8));
  strcat(systemInfoBufPtr, fgetLineBufPtr);
  sprintf(fgetLineBufPtr," sizeTime_t(tv/ts_sec):%dbits ",(((uint32_t)sizeTime_t)*8));
  strcat(systemInfoBufPtr, fgetLineBufPtr);
  sprintf(fgetLineBufPtr," sizeSuseconds_t(tv_usec):%dbits ",(((uint32_t)sizeSuseconds_t)*8));
  strcat(systemInfoBufPtr, fgetLineBufPtr);
  sprintf(fgetLineBufPtr," sizeLongInt:%dbits(ts_nsec) ",(((uint32_t)sizeLongInt)*8));
  strcat(systemInfoBufPtr, fgetLineBufPtr);
  sprintf(fgetLineBufPtr," sizeTimespec: %dbytes ",((uint32_t)sizeTS));
  strcat(systemInfoBufPtr, fgetLineBufPtr);
  sprintf(fgetLineBufPtr," sizeTimeval: %dbytes ",((uint32_t)sizeTV));
  strcat(systemInfoBufPtr, fgetLineBufPtr);
#else
  sprintf(fgetLineBufPtr," sizeInt:%dbits ",(((uint32_t)sizeInt)*8));
  strcat(systemInfoBufPtr, fgetLineBufPtr);
  sprintf(fgetLineBufPtr," sizeLongInt:%dbits ",(((uint32_t)sizeLongInt)*8));
  strcat(systemInfoBufPtr, fgetLineBufPtr);
  sprintf(fgetLineBufPtr," sizeLongLongInt:%dbits ",(((uint32_t)sizeLongLongInt)*8));
  strcat(systemInfoBufPtr, fgetLineBufPtr);
  sprintf(fgetLineBufPtr," sizeVoidPtr:%dbits ",( ((uint32_t)sizeVoidPtr)*8));
  strcat(systemInfoBufPtr, fgetLineBufPtr);
  sprintf(fgetLineBufPtr," sizeDouble:%dbytes ",((uint32_t)sizeDouble));
  strcat(systemInfoBufPtr, fgetLineBufPtr);
  sprintf(fgetLineBufPtr," sizeClock_t:%dbits ",(((uint32_t)sizeClock_t)*8));
  strcat(systemInfoBufPtr, fgetLineBufPtr);
  sprintf(fgetLineBufPtr," sizeSize_t: %dbytes ",((uint32_t)sizeSize_t));
  strcat(systemInfoBufPtr, fgetLineBufPtr);
  sprintf(fgetLineBufPtr," sizeTimespec: %dbytes ",((uint32_t)sizeTS));
  strcat(systemInfoBufPtr, fgetLineBufPtr);
  sprintf(fgetLineBufPtr," sizeTimeval: %dbytes ",((uint32_t)sizeTV));
  strcat(systemInfoBufPtr, fgetLineBufPtr);


#endif


#ifdef TRACEME
 stringSize=strlen(systemInfoBufPtr);
 printf("getSysInfo:  stringSize:%d, systemInfoBufPtr:%s \n",
        stringSize,systemInfoBufPtr);
#endif

//These are the  first and second bash commands we will run
// sprintf(cmdInBufPtr,"%s", "ip addr list | awk -F': ' '/^[0-9]/ && $2 != \"lo\" {print $2}' | xargs");
//sprintf(cmdInBufPtr,"%s", "iw dev | wc -l");

//Before FIRST  call to execCommand
  bzero((void *)cmdOutBufPtr, sizeof(maxBufferSize));
  sprintf(cmdInBufPtr,"%s", "ps aux | grep \"/usr/sbin/gpsd\"  | wc -l");
  cmdLine = cmdInBufPtr;
  stringSizeCMD=strlen(cmdLine);
#ifdef TRACEME
 printf("getSysInfo: third cmd size:%d cmdLine:%s \n",stringSizeCMD,cmdLine);
#endif
  rc = execCommand(cmdLine, cmdOutBufPtr, maxBufferSize);
  if (rc == ERROR) {
   printf("getSysInfo: third execCommand failed \n");
   exit(EXIT_FAILURE);
  } 
  int tmpX=0;
  tmpX = atoi(cmdOutBufPtr);
  if (tmpX > 2) {
    strcat(systemInfoBufPtr, "GPSD ");
  } else {
    strcat(systemInfoBufPtr, "NO GPSD" );
  } 
  stringSize=strlen(systemInfoBufPtr);
#ifdef TRACEME
  printf("getSysInfo:  stringSize:%d, cmdOutBufPtr: %s\n",
         stringSize,  cmdOutBufPtr);
  printf("tmpX: %d \n",tmpX);
#endif

//FIRST call to execCommand
  bzero((void *)cmdOutBufPtr, sizeof(maxBufferSize));
  sprintf(cmdInBufPtr,"%s", "iw dev | wc -l");
  cmdLine = cmdInBufPtr;
  stringSizeCMD=strlen(cmdLine);
#ifdef TRACEME
 printf("getSysInfo: First cmd size:%d cmdLine:%s \n",stringSizeCMD,cmdLine);
#endif
  rc = execCommand(cmdLine, cmdOutBufPtr, maxBufferSize);
  if (rc == ERROR) {
   printf("getSysInfo: execCommand failed \n");
   exit(EXIT_FAILURE);
  } 

  stringSize=strlen(cmdOutBufPtr);
  if (stringSize > 0) 
  {
    if (*cmdOutBufPtr == '0')
      strcat(systemInfoBufPtr, " NO WIRELESS ");
    else 
      strcat(systemInfoBufPtr, " WIRELESS ");

  } else
    printf("getSysInfo: ERROR: stringSize %d  \n",stringSize);

  stringSize=strlen(systemInfoBufPtr);

#ifdef TRACEME
  printf("getSysInfo:  stringSize:%d, totalLine: \n%s\n",
         stringSize, systemInfoBufPtr);
#endif



//SECOND call to execCommand
  bzero((void *)cmdOutBufPtr, sizeof(maxBufferSize));
  sprintf(cmdInBufPtr,"%s", "ip addr list | awk -F': ' '/^[0-9]/ && $2 != \"lo\" {print $2}' | xargs");
  cmdLine = cmdInBufPtr;
  stringSizeCMD=strlen(cmdLine);
#ifdef TRACEME
 printf("getSysInfo: second cmd size:%d cmdLine:%s \n",stringSizeCMD,cmdLine);
#endif
  rc = execCommand(cmdLine, cmdOutBufPtr, maxBufferSize);
  if (rc == ERROR) {
   printf("getSysInfo: execCommand failed \n");
   exit(EXIT_FAILURE);
  } 
  strcat(systemInfoBufPtr, cmdOutBufPtr);
  stringSize=strlen(systemInfoBufPtr);
#ifdef TRACEME
  printf("getSysInfo:  stringSize:%d, cmdOutBufPtr:%s\n",
         stringSize,  cmdOutBufPtr);
#endif




  //copy systemInfoBufPtr to the caller's buffer 
  if (stringSize > maxSize) {
     printf("getSysInfo: final copy: WARNING  stringSize:%d  exceeds maxSize:%d, cp partial \n",
         stringSize,maxSize);
    strncpy(bufPtr,systemInfoBufPtr,(size_t)maxSize);
    stringSize=strlen(bufPtr);
    //rc = ERROR;
    rc = SUCCESS;
  } else {
    strcpy(bufPtr,systemInfoBufPtr);
    stringSize=strlen(bufPtr);
    rc = SUCCESS;
  }

  if (rc == NOERROR)
    rc= stringSize;

#ifdef TRACEME
 printf("getSysInfo: returning rc:%d, final stringSize:%d \n", 
         rc,stringSize);
 printf("getSysInfo: transfered to bufPtr:\n%s \n", bufPtr);
#endif

  if  (fgetLineBufPtr != NULL)
   free(fgetLineBufPtr);
  if  (cmdOutBufPtr != NULL)
    free(cmdOutBufPtr);
  if  (cmdInBufPtr != NULL)
    free(cmdInBufPtr);

  return rc;
}

/***********************************************************
* Function: int getSystemInfo(int platformType, int infoType, char *bufPtr, uint32_t maxSize)
*
* Explanation:This fills in the caller's buffer with
*     a line that summarizes the system - details depend on platformType and infoType.
*
* inputs:  
*   platform:  0:Unix System 1:LinuxSystem 2:VehicularNode 3:Edge Node
*   infoType :  0:static system info; 1: Heartbeat
*   outputMode :  0:stdout 1:file 2:namedPipe 3:IPServer Name:port 4)MQTT broker 5)HTTPRestName
*   param1 :  Depends on outuptMode:  filename, pipename, server:port, mqtt
*
* outputs: returns ERROR or the number of characters placed in the bufPtr
*    Creates a copy of the  line of sys info in the global systemInfoBufPtr
*    And fills in the callers buffer with the line.  
*
* notes: 
*   Platform and inputType params not yet supported
*
**************************************************/
int getSysInfo(int platformType, int infoType, char *bufPtr, uint32_t maxSize)
{ 
  int  rc = NOERROR;
  char *cmdLine = NULL;
  uint32_t stringSizeCMD=0; //size of each cmd
  uint32_t stringSize=0; //maintains  the size in the sysInfo Line 
  char spaces[4] =" ";
  char *fgetLineBufPtr=NULL;
  char *cmdOutBufPtr = NULL;
  char *cmdInBufPtr=NULL;

  if (sysInfoInitFlag == false) {
   printf("getSysInfo: Failed -  not initialized !!! \n");
   rc = ERROR;
   exit(EXIT_FAILURE);
  } 
  //We know the two main buffers have been malloced  
  //char *systemInfoBufPtr=NULL;
  //char *systemStatusBufPtr=NULL;
  fgetLineBufPtr = calloc(MAX_LINE_SIZE, sizeof(char));
  cmdInBufPtr = calloc(MAX_LINE_SIZE,  sizeof(char));
  cmdOutBufPtr = calloc(maxBufferSize, sizeof(char));


//Before FIRST  call to execCommand
  bzero((void *)cmdOutBufPtr, sizeof(maxBufferSize));
//JJM
  sprintf(cmdInBufPtr,"%s", "/etc/TGIF/TGIFbin/getSystemInfo.sh");
//  sprintf(cmdInBufPtr,"%s", "getSystemInfo.sh");
  cmdLine = cmdInBufPtr;
  stringSizeCMD=strlen(cmdLine);
#ifdef TRACEME
 printf("getSysInfo: third cmd size:%d cmdLine:%s \n",stringSizeCMD,cmdLine);
#endif
  rc = execCommand(cmdLine, cmdOutBufPtr, maxBufferSize);
  if (rc == ERROR) {
   printf("getSysInfo: execCommand failed \n");
   exit(EXIT_FAILURE);
  } 
  stringSize=strlen(systemInfoBufPtr);
#ifdef TRACEME
  printf("getSysInfo:  stringSize:%d, cmdOutBufPtr: %s\n",
         stringSize,  cmdOutBufPtr);
#endif

  strcat(systemInfoBufPtr, cmdOutBufPtr);
  stringSize=strlen(systemInfoBufPtr);
#ifdef TRACEME
  printf("getSysInfo:  stringSize:%d, systemInfoBuf:%s\n",
         stringSize, systemInfoBufPtr);
#endif

  //copy systemInfoBufPtr to the caller's buffer 
  if (stringSize > maxSize) {
     printf("getSysInfo: final copy: WARNING  stringSize:%d  exceeds maxSize:%d, cp partial \n",
         stringSize,maxSize);
    strncpy(bufPtr,systemInfoBufPtr,(size_t)maxSize);
    stringSize=strlen(bufPtr);
    //rc = ERROR;
    rc = SUCCESS;
  } else {
    strcpy(bufPtr,systemInfoBufPtr);
    stringSize=strlen(bufPtr);
    rc = SUCCESS;
  }

  if (rc == NOERROR)
    rc= stringSize;

#ifdef TRACEME
 printf("getSysInfo: returning rc:%d, final stringSize:%d \n", 
         rc,stringSize);
 printf("getSysInfo: transfered to bufPtr:\n%s \n", bufPtr);
#endif

  if  (fgetLineBufPtr != NULL)
   free(fgetLineBufPtr);
  if  (cmdOutBufPtr != NULL)
    free(cmdOutBufPtr);
  if  (cmdInBufPtr != NULL)
    free(cmdInBufPtr);

  return rc;
}



/***********************************************************
* Function: int getRunTimeInfo(int platformType, int infoType, char *bufPtr, uint32_t maxSize)
*
* Explanation:This fills in the caller's bufPtr with a line summarizing
*             current system performance.
*
* inputs:  
*   platform:  0:Unix System 1:LinuxSystem 2:VehicularNode 3:Edge Node
*   infoType :  0:static system info; 1: Heartbeat
*   outputMode :  0:stdout 1:file 2:namedPipe 3:IPServer Name:port 4)MQTT broker 5)HTTPRestName
*   param1 :  Depends on outuptMode:  filename, pipename, server:port, mqtt
*
* outputs: returns ERROR or the number of characters placed in the bufPtr
*    Creates a copy of the  line of sys info in the global systemStatusBufPtr
*    And fills in the callers buffer with the line.  
*
* notes: 
*
**************************************************/
int getRunTimeInfo(int platformType, int infoType, char *bufPtr, uint32_t maxSize)
{ 
  int  rc = NOERROR;
  double TS1 = 0.0;
  struct timespec ts;
  clock_t clockSource = CLOCK_MONOTONIC;

  char spaces[4] =" ";
  char *fgetLineBufPtr=NULL;
  char *cmdOutBufPtr = NULL;
  char *cmdInBufPtr=NULL;
  char *cmdLine = NULL;
  int  stringSizeCMD=0;
  int  stringSize=0;
  int  stringSize1=0;

  if (sysInfoInitFlag == false) {
   printf("getRunTimeInfo: Failed -  not initialized !!! \n");
   rc = ERROR;
   exit(EXIT_FAILURE);
  } 

  fgetLineBufPtr = calloc(MAX_LINE_SIZE, sizeof(char));
  cmdInBufPtr = calloc(MAX_LINE_SIZE,  sizeof(char));
  cmdOutBufPtr = calloc(maxBufferSize, sizeof(char));
  

  //Get an accurate  Timestamp
  TS1 =  getTimeD(clockSource);
  maxBufferSize = MAX_BUFFER;
//JJM
  sprintf(cmdInBufPtr,"%s", "/etc/TGIF/TGIFbin/getSystemStatus.sh");
//  sprintf(cmdInBufPtr,"%s", "getSystemStatus.sh");
  cmdLine = cmdInBufPtr;
  stringSizeCMD=strlen(cmdLine);
#ifdef TRACEME
 printf("getRunTime: cmd size:%d cmdLine:%s \n",stringSizeCMD,cmdLine);
#endif

  rc = execCommand(cmdLine, cmdOutBufPtr, maxBufferSize);
  if (rc == ERROR) {
   printf("getRunTime: execCommand failed \n");
   exit(EXIT_FAILURE);
  } 

  stringSize=strlen(cmdOutBufPtr);
#ifdef TRACEME
  printf("getRunTime: stringSize %d, output:%s  \n",
      stringSize,cmdOutBufPtr);
#endif

  if (stringSize > maxSize) {
    printf("getRunTime: ERROR: stringSize %d > maxSize %d  \n",stringSize,maxSize);
    strncpy(bufPtr,cmdOutBufPtr,(size_t)maxSize);
    stringSize=strlen(bufPtr);
    //rc = ERROR;
    rc = NOERROR;
  } else {
    strcpy(bufPtr,cmdOutBufPtr);
    stringSize=strlen(bufPtr);
    rc = NOERROR;
  }
  strcpy(systemStatusBufPtr,bufPtr);

  if  (fgetLineBufPtr != NULL)
   free(fgetLineBufPtr);
  if  (cmdOutBufPtr != NULL)
    free(cmdOutBufPtr);
  if  (cmdInBufPtr != NULL)
    free(cmdInBufPtr);


  if (rc != ERROR)
    rc = stringSize;

  return rc;

}

/****************************************
* routine: int execCommand(char *cmdInBufPtr, char *returnPtr, int maxSize)
*
* explanation: runs the specific command 
*      with results placed in the callers returnPtr
*  
* inputs: 
*  char *cmdInBufPtr  : string holding a valide bash command (or pipeline)
*  char *returnPtr:  the results from stdout of the command are returned
*  int  maxSize:  max size of the caller's buffer
*
* outputs: returns ERROR or number of characters placed in the buffer.
*          If ERROR occurs because the callers returnPtr was not large
*          enough to hold all the output,  the output that could fit
*           will be in the caller's return buffer.
*
* Details/notes:
*
*   Uses popen to run the command and to get access to command output
*     FILE *popen(const char *command, const char *type);
*
******************************************/
int execCommand(char *cmdInBufPtr, char *returnPtr, int maxSize)
{
  int rc = SUCCESS;
  FILE *popenfp = NULL;
  FILE *outputfp = NULL;
  bool loopFlag = true;
  int  returnDataCount=0;
  char *tmpPtr=NULL;
  char *fgetLineBufPtr=NULL;
  char *cmdOutBufPtr = NULL;
  int stringSize=0;
  int tmpSize = maxBufferSize * sizeof(char);

  if (sysInfoInitFlag == false) {
   printf("execCommand:  Failed -  not initialized !!! \n");
   rc = ERROR;
   exit(EXIT_FAILURE);
  } 

  fgetLineBufPtr = calloc(MAX_LINE_SIZE, sizeof(char));
  cmdOutBufPtr = calloc(maxBufferSize, sizeof(char));

  if (fgetLineBufPtr == NULL) {
     printf("execCommand:  HARD Error, failed calloc fgetLineBufPtr size %d\n",
            (MAX_LINE_SIZE*sizeof(char)));
     return ERROR;
  }


  if (cmdOutBufPtr == NULL) {
     printf("execCommand:  HARD Error, failed calloc cmdOutBufPtr size %d\n",
           (maxBufferSize*sizeof(char)));
     return ERROR;
  }

  //Set outFile to defaultFile or another file (or add as a param)
  // If set, the output from the cmd (run via popen) will be
  // output to outFile. If not specified, output goes to stdout 
  char *outFile = NULL;
  //char defaultFile[] = "execResults.out"
  if (outFile == NULL) {
    outputfp = stdout;
#ifdef TRACEME
    printf("execCommand: output set to stdout \n");
#endif
  }
  else 
  {
    remove(outFile);
    outputfp = fopen(outFile, "w+");
    if (outputfp == NULL) {
      perror("execCommand: ERROR on fopen \n"); 
      rc = ERROR;
    } 
#ifdef TRACEME
    else
      printf("execCommand: successfully created  %s\n",outFile);
#endif
  }
#ifdef TRACEME
  printf("execCommand: cmd:%s \n",cmdInBufPtr);
#endif

  //popen captures the stream from executing the shell command
  //in a file stream format, allowing the user to capture
  //the output in a variable.
//  popenfp = popen("sudo ping -D -c 4 -f 8.8.8.8|tail -n 2", "r");
  popenfp = popen(cmdInBufPtr, "r");
  if (popenfp == NULL) {
    printf("execCommand:  error on popen errno:%d \n",errno);
    rc = ERROR;
    exit(EXIT_FAILURE);
  } else {
#ifdef TRACEME
    printf("execComand: popen succeeded, rc=%d  \n",rc);
#endif
    returnDataCount = 0;
    int loopCount=0;
    while(loopFlag)
    {
       loopCount++;
#ifdef TRACEME
       printf("execCommand(loopCount:%d):  begin while  \n",loopCount);
#endif
       tmpPtr = fgets(fgetLineBufPtr, MAX_LINE_SIZE, popenfp);
       if (tmpPtr == NULL) {
#ifdef TRACEME
         printf("execCommand: While:  NULL  \n");
#endif
         loopFlag=false;
         //break;
       } else {
         stringSize=strlen(fgetLineBufPtr);
         returnDataCount += strlen(fgetLineBufPtr);
#ifdef TRACEME
         printf("execCommand(loopCount:%d): in while, not NULL  \n",loopCount);
         printf("execCommand: read %d bytes(total:%d): Line:%s \n",
               stringSize,returnDataCount,fgetLineBufPtr);
#endif
         strcat(cmdOutBufPtr, fgetLineBufPtr);
       }
    }
#ifdef TRACEME
    printf("execCommand: end loop, read total %d bytes: cmdOutBufPtr:%s \n",returnDataCount,cmdOutBufPtr);
#endif
  }
  stringSize=strlen(cmdOutBufPtr);
  if (stringSize > maxSize) {
     printf("execCommand:  ERROR:  stringSize:%d exceeds maxSize:%d \n",
         stringSize, maxSize);
  }
  strncpy(returnPtr,cmdOutBufPtr,(size_t)maxSize);
  stringSize=strlen(returnPtr);

  if (rc == NOERROR)
    rc= stringSize;

#ifdef TRACEME
 printf("execCommand: returning rc:%d, returnDataCount:%d  final stringSize:%d \n", 
         rc,returnDataCount,stringSize);
 printf("execCommand: returnPtr contents: \n%s \n", returnPtr);
#endif

  if (popenfp != NULL)
    pclose(popenfp);
//  if (outputfp != NULL)
//    fclose(outputfp);

  free(fgetLineBufPtr);
  free(cmdOutBufPtr);

  return rc;
}




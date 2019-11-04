/************************************************************************
* File:  session.h
*
* Purpose:
*   This include file is for session includes/defines.
*
*  session *findActive(struct in_addr clientIP, uint16_t clientPort);
*  session *getActive(struct in_addr clientIP, uint16_t clientPort);
*  int removeActive(struct in_addr clientIP, uint16_t clientPort);
*  double updateSessionDuration(session *s);
*  int  printSessions(FiLE *fileID);
*  uint32_t freeAllSessions();
*
* Notes:
*
************************************************************************/
#ifndef	__session_h
#define	__session_h

#define MAX_SESSIONS 2048


//TODO:  byte counts should be uint64
typedef struct session {
  uint32_t errorCount;  //tracks number of any type of error that might occur
  bool isActive;        //true if on the active list, false if on archived list
  uint32_t sessionID;   //uniquely id's this session 
  uint32_t currentArrayIndex;  //increments each time we wrap the session array
  struct in_addr clientIP;
  uint16_t clientPort;
  uint16_t mode;
  struct timespec timeStarted;  
  struct timespec sessionEnd;  
  double timeStartedD;         //This should be a wall time 
  double firstArrivalTimeD;    //this should be based on a high precision clock
  uint32_t MBL1;
  uint32_t MBL2;
  uint32_t MBL3;
  uint32_t MBL4;
  uint32_t MBL5;
  uint32_t MBL6;
  uint32_t MBL7;
  uint32_t MBL8;
  uint32_t MBL9;
  uint32_t MBL10;
  uint32_t MBL11;
  uint32_t MBL12;

  double lastArrivalTimeD;
  double thisInterArrivalTime;
  double interArrivalTimeSum;
  double interArrivalTimeCount;

  //accurate based on timestamps
  double   timeFromFirstToLast;
  double   duration;
  double   bytesReceived;
  uint32_t bytesRxCountWrap;
  uint32_t messagesReceived;
  double   bytesSent;
  uint32_t bytesTxCountWrap;
  uint32_t messagesSent;
  uint32_t messagesLost;
  uint32_t lossEventCount;
  uint32_t lossEventSizeCount;
  uint32_t lastSequenceNum;
  uint32_t sequenceNumberWrap;
  uint32_t largestSeqRecv;
  uint32_t largestSeqSent;
  uint32_t ArrivalsBeforeAck;
  uint32_t outOfOrderArrival;
  uint32_t duplicateArrival;
  double   curJitter;   //Jitter based on sequential packet arrivals 
  double   jitterSum;
  uint32_t countPOWDelaySamples; //Counts number of positive delays
  uint32_t countNOWDelaySamples; //Counts number of negative delays
  double   curOWDelay;
  double   OWDelaySum;
  double   delayChange;  //difference between this and the previous delay
  double   delayChangeSum;
  struct session *prev;
  struct session *next;
} session;


//Not used ??
#if 0
typedef struct {
	int *sock;
	struct sockaddr_in *ServerAddress;
	struct gps_data_t gpsdata;
} ConInfo;
#endif


void initSessions();
int createSession(session  **sPtr);
int getNumberActiveSessions();
int getNumberSessions();

session *findActive(struct in_addr clientIP, uint16_t clientPort);
session *findSession(struct in_addr clientIP, unsigned short clientPort);
session *getActive(struct in_addr clientIP, uint16_t clientPort);

int removeActive(struct in_addr clientIP, uint16_t clientPort);
double updateSessionDuration(session *s);

int printSession(double curTime, FILE *fileFID, session *sPtr);
int printAllSessions(double curTime, FILE *fileFID);

int printActiveSessions(double curTime, FILE *fileFID); 

int printArchivedSessions(double curTime, FILE *fileFID); 
uint32_t freeAllSessions();

#endif


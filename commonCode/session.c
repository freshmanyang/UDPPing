/*********************************************************
* Module Name: session Manager 
*
* File Name:  session.c
*
* Summary:
*  This file contains code to manage a list of UDP IP sessions. 
*
*  The methods include:
*   int createSession(struct **sPtr);
*  session *findActive(struct in_addr clientIP, uint16_t clientPort);
*          NEW sessions are placed at the head of the list
*  session *getActive(struct in_addr clientIP, uint16_t clientPort);
*  void removeActive(struct in_addr clientIP, uint16_t clientPort);
*  void updateSessionDuration(session *s);
*  void printSessions();
*  void freeAllSessions();
*
*  The session mgr maintains two lists of sessions
*  Maintains two lists of sessions:
*    ACTIVE LIST:   session *firstActive : the active session list
*          NEW sessions are placed at the head of the list
*    ARCHIVED LIST session *firstSession : the archived session list 
*
*  Caller uses getActive to either get an existing session handle of 
*     for the client or to get a new session handle. 
*     Only the active list is used...
*
*
*  session *getActive(struct in_addr clientIP, uint16_t clientPort) {
*
*  Notes: 
*     -socket address/port fields are stored in network byte order.
*       When we display these fields in printfs, we convert to 
*        host byte order.
*
*  TODO:  fix the active / archive lists. A session should be moved 
*         to the archive list after a SESSION_TIMEOUT such as 15 minutes).
*         Further, we can have archive elements be smaller than active list
*         elements. 
*
*  Last update: 4/30/2019
*
******************************************************************/
#include "common.h"
#include "session.h"


#define BRIEF_OUTPUT 1 

//Uncomment to turn on printf debug statements
//#define  TRACEME 0

//Uncomment to trace ERRORs and some warnings
#define  TRACE_ERRORS 0


//There are two lists: ACTIVE AND ARCHIVED
//In both, Elements are enqueued at the head....
//   so the 'oldest' session is the last in the list

//This is the head and tail of the archived list. 
session *firstSession = NULL;
session *lastSession = NULL;

//This is the head of the ACTIVE list
//Any element on this list has not yet been placed on the ARCHIVED list
session *firstActive = NULL;

//Sums all elements on either lists
uint32_t sessionCount = 0;

//Number in active list
uint32_t activeSessionCount = 0;
//Number in archived list
uint32_t archivedSessionCount = 0;

static double wallTime = 0.0;
static struct timespec wallTimeTS;
static char wallTimeString[MAX_LINE_SIZE];

static uint32_t sessionNumber=0;

/***********************************************************
* Function: void initSessions() 
*
* Explanation:  This inits the sessionManager. If there 
*               are sessions in the list, they are freed.
*
* inputs: none
*
* outputs:
*
* notes: 
*
***********************************************************/
void initSessions() {

  //If there are sessions, we delete them
  if  (sessionCount > 0 )  {
//#ifdef TRACE_ERRORS
    printf("initSessions:  WARNING sessionCount NOT 0 :  %d  \n", sessionCount);
//#endif 
    while (firstSession != NULL) {
      sessionCount--;
      session *tofree = firstSession;
      firstSession = tofree->next;
      free(tofree);
    }
  }
  //init state 

  //ARCHIVED LIST
  firstSession = NULL;
  lastSession = NULL;
  archivedSessionCount = 0;

  //ACTIVE LIST
  firstActive = NULL;
  activeSessionCount = 0;

  //Total number in both lists
  sessionCount = 0;
}

/***********************************************************
* Function: int getNumberActiveSessions() 
*
* Explanation:  This returns the number of Active sessions in the list.
*
* inputs: none
*
* outputs:
*         returns the number of sessions.
*
* notes: 
*
***********************************************************/
int getNumberActiveSessions() {

  return activeSessionCount;
}


/***********************************************************
* Function: int getNumberSessions() 
*
* Explanation:  This returns the total  number of sessions in both lists
*
* inputs: none
*
* outputs:
*         returns the number of sessions.
*
* notes: 
*
***********************************************************/
int getNumberSessions() {

  return sessionCount;
}

/***********************************************************
* Function: session *findActive(struct in_addr clientIP, uint16_t clientPort) 
*
* Explanation:  This searches the active list of sessions for the client IP/port.
*               It returns the session handle if found, else a NULL;
*
* inputs: passed the IP and port of the client 
*
* outputs:
*    Returns an address of the session struct entry.
*    Or a NULL on error.
*
* notes: 
*   session *firstActive : the active session list
*   session *firstSession : the archived session list 
*
***********************************************************/
session *findActive(struct in_addr clientIP, uint16_t clientPort) {
  session *s = firstActive;
  while (s != NULL) {
    if (s->clientIP.s_addr == clientIP.s_addr && s->clientPort == clientPort){
      break;
    }
    s = s->next;
  }

#ifdef TRACEME
  if (s == NULL)
    printf("findActive: DID NOT FIND the session   \n");
  else {
    printf("findActive: FOUND session :    \n");
  }
#endif

  return s;
}

/***********************************************************
* Function: session *findSession(struct in_addr clientIP, unsigned short clientPort) 
*
* Explanation:  This searches the archived list of sessions for the client IP/port.
*               It returns the session handle if found, else a NULL;
*
* inputs: passed the IP and port of the client 
*
* outputs:
*    Returns an address of the session struct entry.
*    Or a NULL on error.
*
* notes: 
*   session *firstActive : the active session list
*   session *firstSession : the archived session list 
*
***********************************************************/
session *findSession(struct in_addr clientIP, unsigned short clientPort) 
{
  session *sPtr = firstSession;

  while (sPtr != NULL) {
    if (sPtr->clientIP.s_addr == clientIP.s_addr && sPtr->clientPort == clientPort) {
      break;
    }
    sPtr = sPtr->next;
  }

#ifdef TRACEME
  if (sPtr == NULL)
    printf("findSession: DID NOT FIND the session   \n");
  else {
    printf("findSession: FOUND session :    \n");
  }
#endif

  return sPtr;

}


/***********************************************************
* Function: int createSession(session **sPtr);
*
* Explanation:  This creates a new session structure, 
*               fills the struct ptr in the caller's ptr.
*               The caller owns the memory.
*
* inputs: 
*  struct **sPtr:  ptr to callers ptr to a session struct
*
* outputs:
*    The caller's struc is updated.
*    Returns ERROR or NOERROR
*    Fails if:
*           bad sPtr, sessionCount is MAX'ed out
*           or malloc error.
* notes: 
*
***********************************************************/
int createSession(session  **sPtr)
{
  int rc = NOERROR;
  session *s = NULL;

  if (sPtr == NULL)
    rc = ERROR;
  else {  

    if (sessionCount < MAX_SESSIONS) {
      // ...add new session
      s = malloc(1 * sizeof(session));
      memset(s, 0, sizeof(session)); 

      s->errorCount=0;  //tracks number of any type of error that might occur
      s->sessionID = 0;   //uniquely id's this session 
      s->currentArrayIndex=0;  //helps track stats logged as the array wraps
      s->isActive = true;
      s->clientPort = -1;
//    Should be a wall clock time 
      s->timeStartedD = getCurTime(&(s->timeStarted));  
      s->firstArrivalTimeD= -1;
      s->lastArrivalTimeD = -1;
      s->interArrivalTimeSum=0;
      s->interArrivalTimeCount=0;
      s->lastSequenceNum = 0;
      s->bytesReceived = 0;
      s->bytesRxCountWrap =0;
      s->messagesReceived = 0;
      s->messagesLost = 0;
      s->bytesSent = 0;
      s->bytesTxCountWrap =0;
      s->messagesSent = 0;
      s->sequenceNumberWrap = 0;
      s->largestSeqRecv = 0;
      s->largestSeqSent = 0;
      s->outOfOrderArrival = 0;
      s->countPOWDelaySamples = 0;
      s->countNOWDelaySamples = 0;
      s->curOWDelay = 0;
      s->OWDelaySum = 0;
      s->delayChange = 0;
      s->delayChangeSum = 0;
      s->ArrivalsBeforeAck = 0;
      s->duration = 0;
      s->next = NULL;
      s->prev = NULL;

      //set caller's ptr var with a ptr to a session
      *sPtr = s;
    } else {
      rc = ERROR;
    }
  }
  return rc;
}

/***********************************************************
* Function: session *getActive(struct in_addr clientIP, uint16_t clientPort) 
*
* Explanation:  This searches the active list of known sessions for the client IP/port.
*               It returns the session handle if found, 
*               If the client session does not exist,
*               a session is created and the session handle is returned.
*               The new session is placed at the head of the list
*                (pointed to by firstActive)
*
* inputs: passed the IP and port of the client 
*
* outputs:
*    Returns an address of the session struct entry.
*    Or a NULL on error.
*
* notes: 
*
***********************************************************/
session *getActive(struct in_addr clientIP, uint16_t clientPort) 
{
  int rc= NOERROR;
  session *s = NULL;

  // Try to find the client from the active list
  s  = findActive(clientIP, clientPort);
  if (s == NULL) {
    //If not on the active list, create a new session on the active list
    rc = createSession(&s);
    if (rc == NOERROR) {
      s->clientIP = clientIP;
      s->clientPort = clientPort;
      s->next = firstActive;
      s->prev = NULL;
      firstActive = s;
      sessionCount++;
      activeSessionCount++;
    } else {
      rc = ERROR;
      s=NULL;
      printf("session: getActive(): ERROR: No existing and failed \n");
    }
  }

#ifdef TRACE_ERRORS
  if (s == NULL)
    printf("session: getActive():  WARNING: Failed create a new session, current count:%d  \n",
             sessionCount);
#endif
  return s;
}

/***********************************************************
* Function: void removeActive(struct in_addr clientIP, uint16_t clientPort) 
*
* Explanation:  This removes the session from the active list
*               placing it on the archived list.
* TODO:  Have it compute the stats and change the archive list
*        to more of a stat summary result
*        Assume this list might get huge.
* TODO:  What if there are duplicates?  It is possible. Might
*        need to add a random number as a unique session ID.
*        Or just a uint32_t session number.
*
* inputs: passed the IP and port of the client 
*
* outputs: returns ERROR or NOERROR. An error occurs
*         if the active session was not found. 
*
* notes: 
*
***********************************************************/
int removeActive(struct in_addr clientIP, uint16_t clientPort) 
{
  int rc = NOERROR;
  session *s = findActive(clientIP, clientPort);
  if (s != NULL) {
#ifdef TRACEME 
    printf("removeActiveSession(sessioncount:%d, numberActiveSessions:%d, archived:%d): BEFORE  found session   \n",
                sessionCount, activeSessionCount, archivedSessionCount);
#endif
    activeSessionCount--;
    s->isActive = false;
    if (s->next != NULL)
      s->next->prev = s->prev;
    if (s->prev != NULL)
      s->prev->next = s->next;
    if (s == firstActive)
      firstActive = (s->prev != NULL) ? s->prev : s->next;

    // Add to end of the archive list:
    archivedSessionCount++;
    s->next = NULL;
    if (firstSession == NULL) {
      firstSession = s;
      lastSession = s;
      s->prev = NULL;
    } else {
      lastSession->next = s;
      lastSession = s;
    }

#ifdef TRACEME 
    printf("removeActiveSession(sessioncount:%d, numberActiveSessions:%d, archived:%d): AFTER  found session   \n",
                sessionCount, activeSessionCount, archivedSessionCount);
#endif
  }
  else {
   rc = ERROR;
  }
  return rc;
}

/***********************************************************
* Function:  double updateSessionDuration(session *s) {
*
* Explanation: this function updates both the 
*               ssession duration. 
*               This is based on the previously set 
*               first time and last time accessed times.
*
* inputs: 
*  session *s: ptr to session
*
* outputs : 
*    returns ERRORD or the accurate time of the session
*       in seconds.nanoseconds
*
* notes: 
*
***********************************************************/
double updateSessionDuration(session *s) 
{
 
  int rc = NOERROR;


  s->timeFromFirstToLast = s->lastArrivalTimeD - s->firstArrivalTimeD;
  s->duration= s->timeFromFirstToLast;

#ifdef TRACEME 
    printf("updateSessionDuration: lastArrivalTimeD:%f firstAddrivalTimeD:%f \n",
      s->lastArrivalTimeD, s->firstArrivalTimeD);
#endif

  return s->timeFromFirstToLast;
}

/***********************************************************
* Function: int printSession(double curTime, FILE *fileFID, session *toprint) 
*
* Explanation:  Prints status of one particular session 
*
* inputs: 
*  double curTime : caller passes current wall clock time
*  FILE *fileFID:  specifies output file descriptor
*  status *s : ptr to the session struct
*
* outputs : 
*    returns ERROR or NOERROR
*
***********************************************************/
int printSession(double curTime, FILE *fileFID, session *toprint) 
{
  double durSecs = 0.0;
  double bps = 0;
  double tmpVar = 0.0;
  double lossRate = 0.0;
  double lossEventRate = 0.0;
  int rc = NOERROR;
  double totalNumberSamples=0;
  double avgJitter=0.0;
  double avgOWDelay=0.0;
  double avgMBL=0.0;  //avgMBL based on single count-size of each loss event
  double avgMBLBINs=0.0; //avgMBL: weighted avg of bins
  double MBLMetric=0.0;
  double avgIAT=0.0;
  char activeChar[2];
  double  MMNumerator= 0.0;

  if (fileFID == NULL) {
    printf("printSession: HARD ERROR: bad fileFID \n");
    return ERROR;
  }
  if (toprint==NULL) {
    printf("printSession: HARD ERROR: bad sPtr  \n");
    return ERROR;
  }
  if (sessionCount == 0) {
    return NOERROR;
  }

  wallTime = getCurTime(&wallTimeTS);
  rc = convertTimespecToString (wallTimeString,MAX_LINE_SIZE,&wallTimeTS);
  if (rc == ERROR) {
    printf("printSession: HARD ERROR: return from convertTimespectToString \n");
    return ERROR;
  } else 
   rc = NOERROR;

  char *cAddr = inet_ntoa(toprint->clientIP);
  activeChar[0]=' ';
  activeChar[1]='\n';
  if (toprint->isActive)
    activeChar[0]='a';
  else
    activeChar[0]='s';

  durSecs = updateSessionDuration(toprint);
  if (durSecs > 0)
    bps = ( (double)toprint->bytesReceived * 8) / durSecs;

#ifdef TRACEME
    printf("printSession(%d): session:client IP %s: , durSecs:%f  \n",
        sessionCount, cAddr, durSecs);
#endif

      //tmpVar = ((double)toprint->messagesLost) + ( (double) toprint->messagesReceived);
      tmpVar = (double)toprint->largestSeqRecv;
      //note: this is the loss rate not a percent
      lossRate = 0.0;
      if (tmpVar > 0.0) 
        lossRate = ( (double) toprint->messagesLost) / tmpVar;

      lossEventRate=0.0;
      if (tmpVar > 0.0) 
        lossEventRate = toprint->lossEventSizeCount/tmpVar;

      totalNumberSamples = 
         (double)toprint->countPOWDelaySamples +
         (double)toprint->countNOWDelaySamples;

      if (totalNumberSamples > 0) {
        avgOWDelay= ( (double) toprint->OWDelaySum) / totalNumberSamples;
      }

      if (toprint->lossEventSizeCount>0)
        avgMBL= toprint->messagesLost/(double)toprint->lossEventSizeCount;

      if (toprint->interArrivalTimeCount>0) {

        avgJitter = toprint->jitterSum / 
                     (double)toprint->interArrivalTimeCount;
        avgIAT = toprint->interArrivalTimeSum / 
                     (double)toprint->interArrivalTimeCount;
      }

      MMNumerator= (toprint->MBL1 / toprint->messagesLost) + 
                   (toprint->MBL2 / toprint->messagesLost) + 
                   (toprint->MBL3 / toprint->messagesLost) + 
                   (toprint->MBL4 / toprint->messagesLost) + 
                   (toprint->MBL5 / toprint->messagesLost) + 
                   (toprint->MBL6 / toprint->messagesLost) + 
                   (toprint->MBL7 / toprint->messagesLost) + 
                   (toprint->MBL8 / toprint->messagesLost) + 
                   (toprint->MBL9 / toprint->messagesLost) + 
                   (toprint->MBL10 / toprint->messagesLost) + 
                   (toprint->MBL11 / toprint->messagesLost);

      MBLMetric=0.0;

  int tmpX = toprint->MBL1+ toprint->MBL2+ toprint->MBL3+ toprint->MBL4+ toprint->MBL5+ toprint->MBL6+ toprint->MBL7+ toprint->MBL8+ toprint->MBL9+ toprint->MBL10+ toprint->MBL11+ toprint->MBL12;

  fprintf(fileFID,"%12.9f %s:%d %4.9f %d %d %d %12.0f bps %2.4f %2.4f %2.2f %2.9f %2.9f %2.9f \n", 
          curTime, cAddr, toprint->clientPort, 
          durSecs,
          toprint->messagesReceived, toprint->messagesLost,
          toprint->largestSeqRecv,
          bps, lossRate,lossEventRate, avgMBL, avgOWDelay, avgJitter, avgIAT);

          fprintf(fileFID,"%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d \n",
          toprint->messagesLost,toprint->lossEventSizeCount,tmpX,
          toprint->MBL1,
          toprint->MBL2,
          toprint->MBL3,
          toprint->MBL4,
          toprint->MBL5,
          toprint->MBL6,
          toprint->MBL7,
          toprint->MBL8,
          toprint->MBL9,
          toprint->MBL10,
          toprint->MBL11,
          toprint->MBL12);

  return rc;
}

/***********************************************************
* Function: int printActiveSessions(double curTime, FILE *fileFID) 
*
* Explanation:  Prints summary stats of active sessions 
*
* inputs: 
*  double curTime : caller passes current wall clock time
*  FILE *fileFID:  specifies output file descriptor
*
* outputs : 
*    returns ERROR or >=0 representing the number of 
*    sessions whose stats were placed in the output file.
*    A 0 is not necessarily an error.
*
***********************************************************/
int printActiveSessions(double curTime, FILE *fileFID) 
{
  int tmpSessionCount=0;
  int rc = NOERROR;

  session *toprint = NULL;
  if (fileFID == NULL) {
    printf("printActiveSessions: HARD ERROR: bad fileFID \n");
    return(ERROR);
  }
  if (activeSessionCount == 0) {
    return 0;
  }

#ifdef TRACEME 
    printf("printActiveSessions: sessionCount:%d  activeSessionCount:%d archivedSessionCount:%d   \n",
       sessionCount, activeSessionCount, archivedSessionCount);
    printf("printActiveSessions: cAddr clientPort duration bytesRxed MsgsRxed   bps lossrate \n");
#endif


  if ( (activeSessionCount > 0) && (firstActive == NULL) ) {
    rc = ERROR;
    printf("printSessions: ERROR: activeSession NULL AND activeSessionCount:%i \n", activeSessionCount);
    fprintf(fileFID,"printActiveSessions: ERROR!!  sessionCount:%d  activeSessionCount:%d archivedSessionCount:%d   \n",
        sessionCount, activeSessionCount, archivedSessionCount);
  }  else  {
    // So no error....
    toprint = firstActive;

    while (toprint != NULL) {
      rc =  printSession(curTime,fileFID,toprint);
      if (rc == NOERROR)
        tmpSessionCount++;
      else{
        printf("printActiveSessions:  ERROR from printSessions, tmpSessionCount:%d \n",tmpSessionCount);
        break;
      }
      toprint = toprint->next;
    }

  }

  if (rc == NOERROR)
    rc = tmpSessionCount;
  
  return rc;

}

/***********************************************************
* Function: int printArchivedSessions(double curTime, FILE *fileFID) 
*
* Explanation:  Prints summary stats of archived sessions 
*
* inputs: 
*  double curTime : caller passes current wall clock time
*  FILE *fileFID:  specifies output file descriptor
*
* outputs : 
*    returns ERROR or >=0 representing the number of 
*    sessions whose stats were placed in the output file.
*    A 0 is not necessarily an error.
*
***********************************************************/
int printArchivedSessions(double curTime, FILE *fileFID) 
{

  int tmpSessionCount=0;
  session *toprint = NULL;
  int rc = NOERROR;

  if (fileFID == NULL) {
    printf("printSessions: HARD ERROR: bad fileFID \n");
    return(ERROR);
  }
  if (archivedSessionCount == 0) {
    return 0;
  }

#ifdef TRACEME 
    printf("printSessions: sessionCount:%d  activeSessionCount:%d archivedSessionCount:%d   \n",
       sessionCount, activeSessionCount, archivedSessionCount);
    printf("printSessions: cAddr clientPort duration bytesRxed MsgsRxed   bps lossrate \n");
#endif
  if ( (archivedSessionCount > 0) && (firstSession == NULL) ) {
    rc = ERROR;
    printf("printSessions: ERROR: firstSession NULL AND sessionCount:%i \n", sessionCount);
    fprintf(fileFID,"printArchivedSessions: ERROR!!  sessionCount:%d  activeSessionCount:%d archivedSessionCount:%d   \n",
        sessionCount, activeSessionCount, archivedSessionCount);
  }  else  {
    // So no error....
    toprint = firstSession;

    while (toprint != NULL) {
      rc =  printSession(curTime,fileFID,toprint);
      if (rc == NOERROR)
        tmpSessionCount++;
      else{
        printf("printArchivedSessions: HARD ERROR!! printSessions, tmpSessionCount:%d \n",tmpSessionCount);
        break;
      }
      toprint = toprint->next;
    }

  }

  if (rc == NOERROR)
    rc = tmpSessionCount;

  return rc;

}

/***********************************************************
* Function: int printAllSessions(double curTime, FILE *fileFID) 
*
* Explanation:  Prints summary stats of all sessions 
*
* inputs: 
*  double curTime : caller passes current wall clock time
*  FILE *fileFID:  specifies output file descriptor
*
* outputs : 
*    returns ERROR or >=0 representing the number of 
*    sessions whose stats were placed in the output file.
*    A 0 is not necessarily an error.
*
***********************************************************/
int printAllSessions(double curTime, FILE *fileFID) 
{
  int rc = NOERROR;

  rc = printActiveSessions(curTime, fileFID); 
  rc = printArchivedSessions(curTime, fileFID); 
  return rc;

}

/***********************************************************
* Function: void freeAllSessions() 
*
* Explanation:  This frees all sessions in archived list.
*               Used to free memory. 
*
* inputs: none
*
* outputs: returns number freed.
*
* notes: 
*
***********************************************************/
uint32_t  freeAllSessions() 
{
  uint32_t rc = 0;
  while (firstSession != NULL) {
    sessionCount--;
    session *tofree = firstSession;
    firstSession = tofree->next;
    rc++;
    free(tofree);
  }
  return rc;
}



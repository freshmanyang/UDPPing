/*********************************************************
* Module Name: delayHelper  
*
*  Holds routines that provide delay or sleep related function
*
* File Name:     delayHelper.c 
*
*
*  These delay calls do NOT make delay system calls...
*   They are in a tight loop checking the latest time
*   to see if the delaytime has expired.  
*       int delay_busyloop (double delayTime)
*       int delay_busyloop1 (unsigned long nanos ) 
*       int delay_busyloop2 (unsigned long usec) 
*       int busyWait (double delayTime)
*
*   The most accurate way to delay is for the caller to:
*            call busyWait with the corrected time to 
*           wait to meet the interval.  See testTime2.c.
*
* These  Delay Routines invoke system delay calls 
*       int myDelayTS(struct timespec *ts)
*       int myDelayN(uint64_t delayTime)
*       int myDelayD(double delayTime)
*   And each has a uint64 version
*         int clockNanoDelay(uint64_t  delayNano)
*         int nanoDelayTS(struct timespec *ts)
*         int nanoDelay(int64_t ns)
*
*   Based on less accurate timers
*         int microDelay(int32_t delayUseconds)
*
* 
*  Last update: 10/14/2019
*
*********************************************************/
 #define _GNU_SOURCE         /* See feature_test_macros(7) */
 #include <signal.h>
 #include <poll.h>

#include "common.h"
#include "timeHelper.h"
#include "delayHelper.h"
#include "utils.h"
//#include <sys/select.h>




void timeval_add_ulong (struct timeval *tv0, unsigned long value);
int timeval_greaterthan(struct timeval tv1, struct timeval tv0);

//State set by init
bool isDelayModuleInitialized = false;

//State that specifies for the system
// the default delay settings.   
// init will determine the best possible choice
//  and overwrite this
delay_t defaultDelayType = DELAY_NANOSLEEP1;
clock_t defaultDelayClockSource = CLOCK_MONOTONIC;



struct timespec startTS;
struct timespec endTS;
struct timeval startTV;
struct timeval endTV;
double start;
double end;

struct timespec req;
struct timespec rem;

int numberActiveDelays = 0;

//#define TRACEME 0
#define TRACE_ERRORS 0


/****************************************
* routine: int set_delayClockTypeandSource (clock_t clockType, clock_t clockSource)
*
* explanation: inits globals
*  
* inputs: 
*
* outputs: returns the clock_t reflecting
*        the current clock
*         else ERROR
*
* Details/notes:
*
****************************************/
int set_delayClockTypeandSource (clock_t clockType, clock_t clockSource)
{
  int rc = NOERROR;
  rc = setDefaultDelayType(clockType);
  if (rc == ERROR) {
    printf("set_delayClockTypeandSource :  ERROR on setDefaultDelayType to %ld (%ld) \n",
        clockType,clockSource);
  }
  rc = setDefaultDelayClockSource(clockSource);
  if (rc == ERROR) {
    printf("set_delayClockTypeandSource :  ERROR on setDefaultDelayType to %ld (%ld) \n",
        clockType,clockSource);
  }
  return rc;
}

/****************************************
* routine: void initDelayModule()
*
* explanation: inits globals
*  
* inputs: 
*
* outputs: returns the clock_t reflecting
*        the current clock
*         else ERROR
*
* Details/notes:
*
****************************************/
void initDelayModule()
{
  int rc = NOERROR;
  //TODO: make sure these get set to settings this machine supports
  rc = set_delayClockTypeandSource (CLOCK_GETTIME,CLOCK_MONOTONIC_RAW);
  if (rc == ERROR){
    rc = set_delayClockTypeandSource (DELAY_NANOSLEEP1,CLOCK_MONOTONIC);
    if (rc == ERROR)
      rc = set_delayClockTypeandSource (DELAY_USLEEP,CLOCK_REALTIME);
  }
  isDelayModuleInitialized = true;
  numberActiveDelays = 0;
}

/****************************************
* routine: delay_t getDefaultDelayType()
*
* explanation: returns the configured
*        delay type. 
*  
* inputs: 
*
* outputs: a valid delayType 
*
* Details/notes:
*
****************************************/
delay_t getDefaultDelayType()
{
 return defaultDelayType;
}

/****************************************
* routine: clock_t getDefaultDelayClockSource()
*
* explanation: returns the configured
*         clock source used for the default delay type
*  
* inputs: 
*
* outputs: a valid  clock_t
*
* Details/notes:
*
****************************************/
clock_t getDefaultDelayClockSource()
{
 return defaultDelayClockSource;
}

/****************************************
* routine: int setDefaultDelayType(delay_t delayType)
*
* explanation: returns the configured
*        delay type. 
*  
* inputs: 
*
* outputs: a valid delayType 
*
* Details/notes:
*
****************************************/
int setDefaultDelayType(delay_t delayType)
{
  int rc = NOERROR;
  //TODO  chk
  defaultDelayType = delayType;

  return rc;
}

/****************************************
* routine: int setDefaultDelayClockSource(clock_t delayClockSource)
*
* explanation: returns the configured
*         clock source used for the default delay type
*  
* inputs: 
*
* outputs: ERROR or NOERROR
*
* Details/notes:
*
****************************************/
int setDefaultDelayClockSource(clock_t delayClockSource)
{
  int rc = NOERROR;

  defaultDelayClockSource = delayClockSource;
  //TODO:  check 
  return rc;
}


/********************************
*   Delay Routines
*       int myDelayTS(struct timespec *ts)
*       int myDelayN(uint64_t delayTime)
*       int myDelayD(double delayTime)
*   And each has a uint64 version
*         int clockNanoDelay(uint64_t  delayNano)
*         int nanoDelayTS(struct timespec *ts)
*         int nanoDelay(int64_t ns)
*
*   Perhaps the most accurate....busy waits
* int delay_busyloop (double delayTime)
*     busy waits using clock_gettime(CLOCK_MONOTONIC, &t2);
*  int delay_busyloop1 (unsigned long nanos ) 
*     busy waits using clock_gettime(CLOCK_MONOTONIC, &t2);
*  int delay_busyloop (double delayTime)
*     rc = gettimeofday( &t2, NULL );

*   Based on less accurate timers
*         int microDelay(int32_t delayUseconds)
***********************************/

/*************************************************************
* Function: int myDelayTS(struct timespec *ts)
* 
* Summary: Delays the specified amount of time 
*
* Inputs: 
*  struct timespec *ts : callers desired delay time (timespec)
*
* outputs:  
*   returns  ERROR or a number >=0 (the number of times
*    te while loop iterates SUCCESS
*
*    NOT on MAC OS !!
*    rc =  clock_nanosleep(clockid, delayFlags, (const struct timespec *)&ts_req, &ts_rem);
*    EFAULT request or remain specified an invalid address.
*    EINTR  The sleep was interrupted by a signal handler; see signal(7).
*    EINVAL The value in the tv_nsec field was not in the range 0 to
*        999999999 or tv_sec was negative.
*    EINVAL clock_id was invalid.  (CLOCK_THREAD_CPUTIME_ID is not a
*        permitted value for clock_id.)
*
*************************************************************/
int myDelayTS(struct timespec *ts)
{

struct timespec req, rem;
int rc = NOERROR;
int loopCount = 0;
int delayFlags=0;

  req = *ts;


//   printf("myDelayTS: clock_nanosleep: req.tv_sec:%lu req.tv_nsec:%lu  \n", rem.tv_sec, rem.tv_nsec);

#ifdef LINUX
  rc = clock_nanosleep(CLOCK_MONOTONIC, delayFlags, (const struct timespec *)&req, &rem);
#else
  rc = nanosleep(&req, &rem);
#endif
  if (rc == ERROR) {
    printf("myDelayTS: clock_nanosleep: ERROR? errno:%d  \n",errno);
  }

  //It returns 1 on success, -1 on error...we can recover from a signal
  while (rc  == -1) 
  {
    loopCount++;
    if (EINTR == errno)
      memcpy(&req, &rem, sizeof(rem));
    else {
      rc = FAILURE;
      printf("myDelayTS: nanosleep return in error, rem.t_sec:%d t_nsec:%d,  errno: %d \n",(int) rem.tv_sec,(int) rem.tv_nsec,errno);
      // return -1;
    }
#ifdef LINUX
    rc = clock_nanosleep(CLOCK_MONOTONIC, delayFlags, (const struct timespec *)&req, &rem);
#else
  rc = nanosleep(&req, &rem);
#endif

  }

  if (rc != FAILURE)
    rc = loopCount;

  if (rc >0)
    printf("myDelayTS: WARNING  rc is %d \n",rc);

  return rc;
}


/*************************************************************
* Function: int myDelayN(uint64_t delayTime)
* 
* Summary: Delays the specified number of nanoseconds
*
* Inputs:   uint64_t delayTime:  the time to delay in ns
*
* outputs:  
*   returns  ERROR or NOERROR
*
*
*************************************************************/
int myDelayN(uint64_t delayTime)
{
  int rc = NOERROR;
  rc = nanoDelay(delayTime);
  return rc;
}


/*************************************************************
* Function: int myDelayD(double delayTime)
* 
* Summary: Delays the specified amount of time (secs.nsecs)
*
* Inputs:   double delayTime:  the time to delay represented 
*           as a double in seconds with nanosecond precision
*                (e.g,   1.000001526 )
* outputs:  
*   returns  ERROR or NOERROR
*
*
*************************************************************/
int myDelayD(double delayTime)
{
  int rc = NOERROR;

//convert double secs.nsecs  to int64_t 
  int64_t ns = (int64_t) (delayTime * 1000000000.0); 
#ifdef TRACEME 
  printf("myDelayD:  delayTime:%f secs,  %"PRIu64" ns \n",
        delayTime, ns);
#endif

  rc = nanoDelay(ns);
  return rc;
}

/*************************************************************
* Function: int clockNanoDelay(uint64_t  delayNano)
* 
* Summary: Delays the specified number of nanoseconds
*          Equivalent to  myDelayN(uint64_t delayTime)
*          except uses the system call 
* 
*                  NOTE: Not available on MACOS
*                  int clock_nanosleep(clockid_t clock_id, int flags,
*                          const struct timespec *request,
*                           struct timespec *remain);
*
* Inputs:   uint64_t delayNano:  the time to delay in ns
*
* outputs:  
*   returns  ERROR or NOERROR
*
*************************************************************/
int clockNanoDelay(uint64_t  delayNano)
{
  int rc = NOERROR;
  clockid_t clockid = defaultDelayClockSource;
  int count=0;
  uint64_t delaySecs = 0;
  uint64_t delayNsecs = 0;
  int delayFlags = 0;  //maps to relative delay to current clock value
  struct timespec ts_req, ts_rem;


   ts_req.tv_sec=0;
   ts_req.tv_nsec=0;

   while (delayNano >  1000000000) {
     count++;
     ts_req.tv_sec++;
     delayNano -= 1000000000;
   }

   //chk if delayNano<0
   if (delayNano < 0){
     ts_req.tv_sec--;
     delayNano += 1000000000;
   }

   ts_req.tv_nsec = delayNano;

   //finally, chk tv_sec not negative
   //Call it an error 
   if (ts_req.tv_sec < 0) 
   {
     rc = ERROR;
     printf("clockNanoDelay: ERROR: negative sec ??    delayNano:%"PRIu64"  ts_req.tv_sec:%ld  ts_req.tv_nsec:%ld  \n",delayNano, ts_req.tv_sec, ts_req.tv_nsec);
     exit(EXIT_FAILURE);
   }


#ifdef LINUX
  rc =  clock_nanosleep(clockid, delayFlags, (const struct timespec *)&ts_req, &ts_rem);
#else
  rc = nanoDelay(delayNano);
#endif

  if (rc == 0)
   rc = NOERROR;
  else {
   rc = ERROR;
   printf("delay: ERROR:  f rc : %d \n",rc);
       //EFAULT request or remain specified an invalid address.
       //EINTR  The sleep was interrupted by a signal handler; see signal(7).
       //EINVAL The value in the tv_nsec field was not in the range 0 to
       //       999999999 or tv_sec was negative.
       //EINVAL clock_id was invalid.  (CLOCK_THREAD_CPUTIME_ID is not a
       //      permitted value for clock_id.)
   exit(EXIT_FAILURE);
  }
  return rc;
}

/*************************************************************
* Function: int nanoDelayTS(struct timespec *ts)
* 
* Summary:  Delays the specified amount of time 
*
* Inputs: 
*  struct timespec *ts
*
* outputs:  
*   returns  ERROR or a number >=0 (the number of times
*    the while loop iterates SUCCESS
*
*
*************************************************************/
int nanoDelayTS(struct timespec *ts)
{

struct timespec req, rem;
int rc = NOERROR;
int loopCount = 0;

  req = *ts;


  while (nanosleep(&req, &rem) == -1) {
    loopCount++;
    if (EINTR == errno)
      memcpy(&req, &rem, sizeof(rem));
    else {
      rc = FAILURE;
      printf("nanoDelay: nanosleep return in error, rem.t_sec:%d t_nsec:%d,  errno: %d \n",(int) rem.tv_sec,(int) rem.tv_nsec,errno);
      // return -1;
    }
  }

  if (rc != FAILURE)
    rc = loopCount;

  if (rc >0)
    printf("nanoDelayTS: WARNING  rc is %d \n",rc);

  return rc;
}


/***********************************************************
* Function: int nanoDelay(int64_t ns)
*
* Explanation:  This delays the specified number of nanoseconds
*
* inputs: 
*    int64_t ns : number of ns to delay 
*
* outputs:
*   returns  ERROR or a number >=0 (the number of times
*    the while loop iterates SUCCESS
*
* notes: 
*
***********************************************************/
int nanoDelay(int64_t ns)
{
struct timespec req, rem;
int rc = NOERROR;
int loopCount = 0;

  req.tv_sec = 0;

  //This makes sure seconds is 0
  //But...what if the int rolls over?
  //Why do we need this?
  while (ns >= 1000000000L) {
        ns -= 1000000000L;
        req.tv_sec += 1;
  }

  req.tv_nsec = ns;

  while (nanosleep(&req, &rem) == -1) {
        loopCount++;
        if (EINTR == errno)
            memcpy(&req, &rem, sizeof(rem));
        else {
          rc = FAILURE;
          printf("nanoDelay: nanosleep return in error, rem.t_sec:%d t_nsec:%d,  errno: %d \n",(int) rem.tv_sec,(int) rem.tv_nsec,errno);
           exit(EXIT_FAILURE);
        }
  }

  if (rc != FAILURE)
    rc = loopCount;

  return  rc;
}



#ifdef LINUX
/***********************************************************
* Function: int delayPpoll(timespec *delayTS)
*
* Explanation:  This delays the specified time
*     by using a ppoll with NO fd's events listed.
*
* inputs: 
*    timespec *delayTS : specifies amount of time to delay
*
* outputs:
*        returns NOERROR or ERROR
*
* notes: 
*
**************************************************/
int delayPpoll(struct timespec *delayTS)
{
  int rc = NOERROR;
  struct pollfd   myPoll;
  sigset_t sigmask;

  myPoll.fd= -1;
  myPoll.events=0;
  myPoll.revents=0;
  //poll timeout granularity of 1 ms
  //int poll(struct pollfd *fds, nfds_t nfds, int timeout);

  //ppoll timeout granularity of 1 ns 
  //ready = ppoll(&fds, nfds, tmo_p, &sigmask);
  //if  sigmask is NULL then ppoll==poll execpt precision of timeout is nano 
  // ready = ppoll(&fds, nfds, tmo_p, &sigmask);
  rc = ppoll(&myPoll, 0, delayTS, NULL);

  if (rc == 0) {
   rc = NOERROR;
  } else {
    if (rc > 0) {
        printf("delayPpoll: ppoll return indicates fd events ??? :%d \n",rc);
    } else {
        perror("delayPpoll: ppoll return ERROR ");
    }
    rc = ERROR;
  }

  return rc;
}


/***********************************************************
* Function: int delayPselect(timespec *delayTS)
*
* Explanation:  This delays the specified time
*     by using a pSelect with  NO fd's lists 
*
* inputs: 
*    timespec *delayTS : specifies amount of time to delay
*
* outputs:
*        returns NOERROR or ERROR
*
* notes: 
*
**************************************************/
int delayPselect(struct timespec *delayTS)
{
  int rc = NOERROR;
  struct pollfd   myPoll;
  sigset_t sigmask;
  fd_set readfds;
  fd_set writefds;
  fd_set execptfds;

  FD_ZERO(&readfds);
  FD_ZERO(&writefds);
  FD_ZERO(&execptfds);

  //int pselect(nt nfds, fd_set *readfds, fd_set *writefds,
  //              fd_set execptfds, const struct timespec *timeout,
  //              const sigset_t *sigmask);
  //rc =  pselect(0, &readfds, NULL, NULL, delayTS, NULL);
  rc =  pselect(0, NULL, NULL, NULL, delayTS, NULL);

  if (rc == 0) {
   rc = NOERROR;
  } else {
    rc = ERROR;
    printf("delayPselect: pselect return ERROR %d \n",errno);
    perror("delayPselect: pselect return ERROR ");
  }

  return rc;

}

#endif

/***********************************************************
* Function: int microDelay(int32_t delayUseconds)
*
* Explanation:  This delays the specified number of microseconds
*
* inputs: 
*    int32_t  delayUseconds : number of microseconds to delay 
*
* outputs:
*        returns FAILURE or SUCCESS
*
* notes: 
*
**************************************************/
int microDelay(int32_t delayUseconds)
{
  int rc = NOERROR;

//  int64_t ns = (int64_t) (delayUseconds * 1000); 
//rc = nanoDelay(ns);

  rc =  usleep((useconds_t)delayUseconds);
  return rc;
}




/*************************************************************
* Function: int generalDelay(clock_t clockSource, double delay )
* 
* Summary: sleeps for the sec amount of time using the requested clock.
*
*   under ideal conditions is accurate to one microsecond. To get nanosecond
*   accuracy, replace sleep()/usleep() with something with higher resolution
*   like nanosleep() or ppoll().
*
* Inputs:  
*   int time (seconds)
*
* outputs:  timestamp in double format
*
* Note: this is not complete.... adapted from true sleep
*
*************************************************************/
int generalDelay(clock_t clockSource, double delay )
{
   struct timespec ts_start;
   struct timespec ts_end;
   int rc = NOERROR;

   clock_gettime(CLOCK_MONOTONIC, &ts_start);

   ts_end = ts_start;
//   ts_end.tv_sec += delay;

   long int delaySecs = 0;
   if (delay > 1.0)
      delaySecs =  (uint32_t)floor(delay);

//   long int delayUsecs = (uint32_t)( 1000000 *  (delay - (double) delaySecs));


   for(;;) {
     struct timespec ts_current;
     struct timespec ts_remaining;

     clock_gettime(CLOCK_MONOTONIC, &ts_current);

     ts_remaining.tv_sec = ts_end.tv_sec - ts_current.tv_sec;
     ts_remaining.tv_nsec = ts_end.tv_nsec - ts_current.tv_nsec;
     while (ts_remaining.tv_nsec > 1000000000) {
        ts_remaining.tv_sec++;
        ts_remaining.tv_nsec -= 1000000000;
     }

     while (ts_remaining.tv_nsec < 0) {
       ts_remaining.tv_sec--;
       ts_remaining.tv_nsec += 1000000000;
     }

     if (ts_remaining.tv_sec < 0) {
        break;
     }

     if (ts_remaining.tv_sec > 0) 
     {
       sleep(ts_remaining.tv_sec);
     } else {
       usleep(ts_remaining.tv_nsec / 1000);
     }
  }
  return rc;
}


void timespec_add_nanos (struct timespec *tv0, unsigned long nanos) {
    tv0->tv_nsec += nanos;
    if (tv0->tv_nsec >= BILLION) {
	tv0->tv_sec++;
	tv0->tv_nsec -= BILLION;
    }
}

void timespec_add_ulong (struct timespec *tv0, unsigned long value) {
    tv0->tv_nsec += value;
    if (tv0->tv_nsec >= BILLION) {
	tv0->tv_sec++;
	tv0->tv_nsec -= BILLION;
    }
}

// Kalman versions attempt to support delay request
// accuracy over a minimum guaranteed delay by
// prediciting the delay error. This is
// the basic recursive algorithm.
void kalman_update (kalman_state *state, double measurement) {
    //prediction update
    state->p = state->p + state->q;
    //measurement update
    state->k = state->p / (state->p + state->r);
    state->x = state->x + (state->k * (measurement - state->x));
    state->p = (1 - state->k) * state->p;
}

// Delay calls for systems with clock_gettime
// Working units are nanoseconds and structures are timespec
void timespec_add_double (struct timespec *tv0, double value) {
    tv0->tv_nsec += (unsigned long) value;
    if (tv0->tv_nsec >= BILLION) {
	tv0->tv_sec++;
	tv0->tv_nsec -= BILLION;
    }
}
// tv1 assumed greater than tv0
double timespec_diff (struct timespec tv1, struct timespec tv0) {
    double result;
    if (tv1.tv_nsec < tv0.tv_nsec) {
	tv1.tv_nsec += BILLION;
	tv1.tv_sec--;
    }
    result = (double) (((tv1.tv_sec - tv0.tv_sec) * BILLION) + (tv1.tv_nsec - tv0.tv_nsec));
    return result;
}
void timespec_add( struct timespec *tv0, struct timespec *tv1)
{
    tv0->tv_sec += tv1->tv_sec;
    tv0->tv_nsec += tv1->tv_nsec;
    if ( tv0->tv_nsec >= BILLION ) {
	tv0->tv_nsec -= BILLION;
	tv0->tv_sec++;
    }
}

/*************************************************************
* Function: int busyWait (double delayTime)
* 
* Summary: Delays (busywaits) until the specified time 
*          This assumes clock_gettime with clock type MONOTONIC 
*
* Inputs: 
*    double delayTime: caller's future time....
*
* outputs:  
*   returns  ERROR or  NOERROR;
*
************************************************/
int busyWait (double delayTime)
{
  int rc = NOERROR;
  struct timespec TS1, TS2;

  rc =  convertD2TS(&delayTime,&TS2);
  while (1) {
    rc = clock_gettime(CLOCK_MONOTONIC, &TS1);
    //returns true once 
    if (isTS1GTTS2(&TS1, &TS2)) 
      break;
  }
  return rc;
}


/*************************************************************
* Function: int timespec_greaterthan(struct timespec tv1, struct timespec tv0) {
* 
* Summary: Delays the specified amount of time 
*
* Inputs: 
*    double delayTime: callers desired delay time (seconds.nano) 
*
* outputs:  
*   returns  1 is tv1>tv0 
*
************************************************/
int timespec_greaterthan(struct timespec tv1, struct timespec tv0) {
    if (tv1.tv_sec > tv0.tv_sec ||					\
	((tv0.tv_sec == tv1.tv_sec) && (tv1.tv_nsec > tv0.tv_nsec))) {
	return 1;
    } else {
	return 0;
    }
}


/*************************************************************
* Function: int delay_busyloop (double delay)
* 
* Summary: Delays the specified amount of time 
*
* Inputs: 
*    double delayTime: callers desired delay time (seconds.nano) 
*
* outputs:  
*   returns  ERROR or  NOERROR;
*
************************************************/
int delay_busyloop (double delayTime)
{
  int rc = NOERROR;
  struct timespec t1, t2;

  if (delayTime==0.0) 
    return rc;

  rc =  convertD2TS(&delayTime,&t2);
  clock_gettime(CLOCK_MONOTONIC, &t1);
  timespec_add_ulong(&t1, (delayTime * 1000000000));
  while (1) {
    clock_gettime(CLOCK_MONOTONIC, &t2);
    if (timespec_greaterthan(t2, t1))
      break;
  }
  return rc;
}



int delay_busyloop1 (unsigned long nanos ) 
{
  int rc = NOERROR;
  struct timespec t1,t2;

  rc = clock_gettime(CLOCK_MONOTONIC, &t1);
  timespec_add_nanos(&t1, nanos);
  while (1) {
    clock_gettime(CLOCK_MONOTONIC, &t2);
    if (timespec_greaterthan(t2, t1))
     break;
  }
  return rc;
}


int delay_busyloop2 (unsigned long usec) 
{
  int rc = NOERROR;
  struct timeval t1, t2;

  rc = gettimeofday( &t1, NULL );
  timeval_add_ulong(&t1, usec);
  while (1) {
   rc = gettimeofday( &t2, NULL );
   if (timeval_greaterthan(t2, t1))
     break;
  }
  return rc;
}

// Kalman routines for systems with clock_gettime
// Request units is microseconds
// Adjust units is nanoseconds
void delay_kalman1 (unsigned long usec) {
    struct timespec t1, t2, finishtime, requested={0,0}, remaining;
    double nsec_adjusted, err;
    static kalman_state kalmanerr={
	0.00001, //q process noise covariance
	0.1, //r measurement noise covariance
	0.0, //x value, error predictio (units nanoseconds)
	1, //p estimation error covariance
	0.75 //k kalman gain
    };
    // Get the current clock
    clock_gettime(CLOCK_REALTIME, &t1);
    // Perform the kalman adjust per the predicted delay error
    nsec_adjusted = (usec * 1000.0) - kalmanerr.x;
    // Set a timespec to be used by the nanosleep
    // as well as for the finished time calculation
    timespec_add_double(&requested, nsec_adjusted);
    // Set the finish time in timespec format
    finishtime = t1;
    timespec_add(&finishtime, &requested);
    // Don't call nanosleep for values less than 10 microseconds
    // as the syscall is too expensive.  Let the busy loop
    // provide the delay for times under that.
    if (nsec_adjusted > 10000) {
	nanosleep(&requested, &remaining);
    }
    while (1) {
	clock_gettime(CLOCK_REALTIME, &t2);
	if (timespec_greaterthan(t2, finishtime))
	    break;
    }
    // Compute the delay error in units of nanoseconds
    // and cast to type double
    err = (double) (timespec_diff(t2, t1) - (usec * 1000));
    // printf("req: %ld adj: %f err: %.5f (ns)\n", usec, nsec_adjusted, kalmanerr.x);
    kalman_update(&kalmanerr, err);
}


// Sadly, these systems must use the not so efficient gettimeofday()
// and working units are microseconds, struct is timeval
void timeval_add_ulong (struct timeval *tv0, unsigned long value) {
    tv0->tv_usec += value;
    if (tv0->tv_usec >= MILLION) {
	tv0->tv_sec++;
	tv0->tv_usec -= MILLION;
    }
}

int timeval_greaterthan(struct timeval tv1, struct timeval tv0) {
    if(tv1.tv_sec > tv0.tv_sec || ((tv0.tv_sec == tv1.tv_sec) && (tv1.tv_usec > tv0.tv_usec))){
	return 1;
    } else {
	return 0;
    }
}

// tv1 assumed greater than tv0
double timeval_diff (struct timeval tv1, struct timeval tv0) {
    double result;
    if (tv1.tv_usec < tv0.tv_usec) {
	tv1.tv_usec += MILLION;
	tv1.tv_sec--;
    }
    result = (double) (((tv1.tv_sec - tv0.tv_sec) * MILLION) + (tv1.tv_usec - tv0.tv_usec));
    return result;
}



// Request units is microseconds
// Adjust units is microseconds
void delay_kalman2 (unsigned long usec) {
    struct timeval t1, t2, finishtime;
    long usec_adjusted;
    double err;
    static kalman_state kalmanerr={
	0.00001, //q process noise covariance
	0.1, //r measurement noise covariance
	0.0, //x value, error predictio (units nanoseconds)
	1, //p estimation error covariance
	0.25 //k kalman gain
    };
    // Get the current clock
    gettimeofday( &t1, NULL );
    // Perform the kalman adjust per the predicted delay error
    if (kalmanerr.x > 0) {
	usec_adjusted = usec - (long) floor(kalmanerr.x);
	if (usec_adjusted < 0)
	    usec_adjusted = 0;
    }
    else
	usec_adjusted = usec + (long) floor(kalmanerr.x);
    // Set the finishtime
    finishtime = t1;
    timeval_add_ulong(&finishtime, usec_adjusted);

    // Don't call nanosleep for values less than 10 microseconds
    // as the syscall is too expensive.  Let the busy loop
    // provide the delay for times under that.
    if (usec_adjusted > 10) {
	struct timespec requested={0,0}, remaining;
	timespec_add_ulong(&requested, (usec_adjusted * 1000));
	nanosleep(&requested, &remaining);
    }
    //busy loop....
    while (1) {
	gettimeofday(&t2, NULL );
	if (timeval_greaterthan(t2, finishtime))
	    break;
    }
    // Compute the delay error in units of microseconds
    // and cast to type double
    err = (double)(timeval_diff(t2, t1)  - usec);
    // printf("req: %ld adj: %ld err: %.5f (us)\n", usec, usec_adjusted, kalmanerr.x);
    kalman_update(&kalmanerr, err);
}




/************************************************************************
* File: delayHelper.h
*
* Purpose:
*   Set of helper routines related to delay or sleep
*
*   Available delay functions: 
*       int myDelayTS(struct timespec *ts)
*       int myDelayN(uint64_t delayTime)
*       int myDelayD(double delayTime)
*         int clockNanoDelay(uint64_t  delayNano)
*         int nanoDelayTS(struct timespec *ts)
*         int nanoDelay(int64_t ns)
*         int microDelay(int32_t delayUseconds)
*    void delay_busyloop (double delay){
*    void delay_busyloop1 (unsigned long usec) {
*    void delay_busyloop2 (unsigned long usec) {
*
* Notes:
*
* Last update: 4/17/2019
*
************************************************************************/
#ifndef	__delayHelper_h
#define	__delayHelper_h

#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include <sys/time.h>
#include <time.h>

typedef long int delay_t;   //specifies clock type

//The following define delay_t  
//This specifies how the delay is to be performed
// TODO: make an enum 

//Note that the defaultDelayClockType will be set to one of these
// A caller can change this with 
//delay_t defaultDelayType = DELAY_NANOSLEEP1;
//clock_t defaultDelayClockSource = CLOCK_MONOTONIC;
int setDefaultDelayType(delay_t delayType);
int setDefaultDelayClockSource(clock_t delayClockType);
int set_delayClockTypeandSource (clock_t clockType, clock_t clockSource);
delay_t getDefaultDelayType();
clock_t getDefaultDelayClockSource();
//Used to be in timeHelper...now should call setDefaultDelayType and ClockSource
int set_delayClock (clock_t clockSource);

int timespec_greaterthan(struct timespec tv1, struct timespec tv0);
void timespec_add( struct timespec *tv0, struct timespec *tv1);
double timespec_diff (struct timespec tv1, struct timespec tv0);
void timespec_add_double (struct timespec *tv0, double value);
void timespec_add_ulong (struct timespec *tv0, unsigned long value);

//We call this the delayType or delayID
//The following are possible delay types
//TODO:  Get rid of this...it's a clock source not a delay type
#define DELAY_GETTIMEOFDAY 0

        //case DELAY_USLEEP: 0
        //case DELAY_CLOCK_SLEEP: 1
        //case DELAY_NANOSLEEP1:  2
        //case DELAY_BUSY_WAIT1:  3
        //case DELAY_BUSY_WAIT2:  4
        //case DELAY_BUSY_WAIT3:  5
        //case DELAY_BUSY_WAIT4:  6
        //case busyWait           6
#define DELAY_USLEEP       0
#define DELAY_CLOCK_SLEEP  1
#define DELAY_NANOSLEEP1   2
#define DELAY_NANOSLEEP2   7 
#define DELAY_NANO_WITH_BUSY  32

#define DELAY_BUSY_WAIT1   3
#define DELAY_BUSY_WAIT2   4
#define DELAY_BUSY_WAIT3   5
#define DELAY_BUSY_WAIT4   6

//delayPselect
#define DELAY_PTHREAD_WAIT 8 
#define DELAY_PSELECT      9
#define DELAY_PPOLL        10 

#
#define DELAY_NANO_WITH_BUSY    32
#define DELAY_DELAY_KALMAN    33

void initDelayModule();

//These return the best choice for the system
uint32_t getDelayType();
uint32_t getDelayTimeSource();

int set_defaultDelayType (clock_t clockSource);
int set_defaultDelayClockSource (clock_t clockSource);


int myDelayTS(struct timespec *ts);
//identical to myDelayTS
int nanoDelayTS(struct timespec *ts);//param is delay time in a timespec

int myDelayD(double delayTime);    //delayTime as secs.nsecs
//identical to myDelayN
int nanoDelay(int64_t ns);

int myDelayN(uint64_t delayTime);  //delayTime in ns's
//OR ...
int clockNanoDelay(uint64_t  delayNano);

#ifdef LINUX
int delayPselect(struct timespec *delayTS);
int delayPpoll(struct timespec *delayTS);
#endif

int microDelay(int32_t delayUseconds);
int generalDelay(clock_t clockSource, double delay );

//This busyWaits until time delayTime (based on clock_gettime with CLOCK_MONOTONIC)
int busyWait (double delayTime);

//These were from an open source program (checkdelay.c and delay.c):
//void delay_loop(uint32_t delayType, unsigned long usecs );
int delay_busyloop (double delayTime);
int delay_busyloop1 (unsigned long nanos);
int delay_busyloop2 (unsigned long usec);



//This attempts to adjust to converge on an accurate delay
//But...seems to need the right type of randomness to 
// work properly
// A much simpler algorithm would be better, safer.
void delay_kalman1(unsigned long usecs);
void delay_kalman2(unsigned long usecs);

// Kalman filter states
typedef struct kalman_state {
    double q; //process noise covariance
    double r; //measurement noise covariance
    double x; //value
    double p; //estimation error covariance
    double k; //kalman gain
} kalman_state;
void delay_kalman(unsigned long usecs);

#ifdef __cplusplus
} /* end extern "C" */
#endif

#endif



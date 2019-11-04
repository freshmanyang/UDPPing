/************************************************************************
* File:  common.h
*
* Purpose:
*   This include file is for common includes/defines.
*   Assumes utils.c is compiled/built.
*
* Notes:
*
************************************************************************/
#ifndef	__common_h
#define	__common_h

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>  /*brings in C99 types of uint32_t uint64_t  ...*/
#include <stdbool.h>
#include <string.h>     
#include  <stdarg.h>		/* ANSI C header file */
#include <errno.h>
#include <sys/file.h>
#include <math.h>    /* floor, ... */
#include  <syslog.h>

#include <sys/types.h>
#include <limits.h>  /*brings in limits such as LONG_MIN LLONG_MAX ... */

#include <pthread.h>

#include <signal.h>

#include <unistd.h>     /* for close() */
#include <sys/stat.h>
#include <fcntl.h>
#include <netdb.h>
#include <sys/ioctl.h>

#include <sys/socket.h> /* for socket(), connect(), sendto(), and recvfrom() */
#include <netinet/in.h> /* for in_addr */
#include <arpa/inet.h> /* for inet_addr ... */

//Needed to printf uint64_t  with PRI64 
#include <inttypes.h>

#if 0
#ifdef LINUX
#include <endian.h>
#endif
#ifdef DARWIN
#include <libkern/OSByteOrder.h>
#endif

#endif
#include "portable_endian.h"
#include "timeHelper.h"
#include "delayHelper.h"
#include "utils.h"

#define QOS_MinL_MaxR       0x14

#define QOS_MIN_LATENCY    0x10
#define QOS_MAX_THROUGHPUT 0x08
#define QOS_MAX_RELIABILTY 0x04
#define QOS_MIN_COST       0x02

typedef double myTS_t;	/* Unix time in seconds with fractional part */
typedef uint64_t myTSns_t;	
typedef uint32_t mySize_t;

#define	min(a,b)	((a) < (b) ? (a) : (b))
#define	max(a,b)	((a) > (b) ? (a) : (b))

//Defines the timeout
#define TIMEOUT 2 

//Definition, FALSE is 0,  TRUE is anything other
#define TRUE 1
#define FALSE 0

#define VALID 1
#define NOTVALID 0

#define MAX_CHILDREN 100

//#define BILLION 1000000000LLU
#define BILLION 1000000000L
//#define BILLION 1000000000
#define MILLION 1000000


#define MAXMSGSIZE 50000

//Defines max size temp buffer that any object might create
#define MAX_TMP_BUFFER 1024

/*
  Consistent with C++, -1 for ERROR, and not error is >= 0

  For routines/functions :  ERROR or NOERROR (or SUCCESS)

  If issuing an exit ,   can specify a return of EXIT_SUCCESS 
            or EXIT FAILURE  

  Try to name any funcion that returns with a bool with is... 

*/
#define SUCCESS 0
#define NOERROR 0

#define ERROR   -1
#define FAILURE -1
#define FAILED -1
#define CHAR_ERROR 0xFF
#define DOUBLE_NOERROR 0.0
#define DOUBLE_ERROR  -1.0

#define MAX_FILENAME_SIZE 128
#define MAX_LINE_SIZE  1024
#define MAX_BUFFER   1000000
//#define MAX_LINE_SIZE  128
//#define MAX_BUFFER    1024


#endif



/************************************************************************
* File:  gpsdStubs.c
*
* Purpose:
*  This module contains stubs for the gps client library.
*  This should not be used on a machine if GPSD is installed.
*  This provides a way to test without having gpds installed or 
*  a working gps sensor.
*
* Notes:
*
* Last update: 7/11/2019
*
************************************************************************/

#ifdef NOGPS 

#include <sys/types.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

#include "gpsdStubs.h"

//#define TRACEME 1 

int gps_open(const char *myString, const char *myString1, struct gps_data_t *thisGPSData)
{
#ifdef TRACEME
  printf("gps_open: stub:  hostname: %s \n", myString);
#endif
  return 0;
}


int gps_close(struct gps_data_t *thisGPSData) 
{
  return 0;
}

int gps_send(struct gps_data_t *thisGPSData, const char * myString, ... )
{
  return 0;
}

int gps_read(struct gps_data_t *thisGPSData)
{
  return -1;
}

int gps_stream(struct gps_data_t *thisGPSData, unsigned int x,void *thisPtr)
{
  return 0;
}

const char  *gps_data(const struct gps_data_t *thisGPSData)
{
  const char *myString= NULL;
  return myString;
}

const char  *gps_errstr(const int x)
{
  const char *myString= NULL;
  return myString;
}


int gps_waiting(struct gps_data_t *thisGPSData, int thisTimeMSs)
{
  int rc = 0;
  double timeSeconds = thisTimeMSs* 1000.0;
 
  rc = usleep(timeSeconds);

  thisGPSData->status = STATUS_FIX;   //fake a fix 
#ifdef TRACEME
  printf("gpsdStubs: gps_waiting:  waited %f seconds, status:%d  rc: %d \n",timeSeconds, thisGPSData->status,rc);
#endif
  return 1;
}


#endif 


#ifndef _GPSD_HELPER_H_
#define _GPSD_HELPER_H_

#ifdef NOGPS
#include "gpsdStubs.h"
#else
#include <gps.h>
#endif

#define MAX_OPEN_ATTEMPTS 10

#define GPS_ERROR_NO_SOFTWARE               1 
#define GPS_ERROR_NO_SENSOR                 2
#define GPS_ERROR_CONNECT_FAILURE           3
#define GPS_ERROR_REMOTE_CONNECT_FAILURE    4
#define GPS_ERROR_FAILED_TO_LOCK_NO_SATS    5
#define GPS_ERROR_FAILED_TO_LOCK_POOR_RF    7
#define GPS_ERROR_INACCURACY_TOO_LARGE      8
#define GPS_ERROR_FAILED_TO_GET_LOCATION    10

#define GPS_READ_ERROR                      20
#define GPS_MISC_ERROR                      21


//values of locationMode
#define noGPS                     0 //No GPS location data or devices used 
#define USE_BEST_AVAILABLE        1
#define fixedGPS                  2 //static location from file fixedGPS.dat
#define localGPSD                 8 
#define remoteGPSD                9
#define localIP_GEO_LOCATION      16
#define network_GEO_LOCATION      17

#define RELATIVE_LOCATION         32

#define PRECISE_LOCATION_DEVICE    64
#define PRECISE_LOCATION_RTK       65

//Value in seconds that we will wait to get a successful location from a location service
#define MAX_GPS_LOCATION_WAIT_TIME 2 

//Value in microseconds we wait for any type of response from a gps device
#define MAX_GPS_POLL_DELAY_TIME 100   

void debugDumpOLD(struct gps_data_t *gpsdata);

int writeGPSFakeData(double curTime, double GPSSampleTime, double latitude, double longitude, double alt, char *outputFileName,uint32_t verbosity);
int writeGPSFile(double curTime, double GPSSampleTime, struct gps_data_t *gpsdata, char *outputFileName,uint32_t verbosity);
int writeGPSLine(double curTime,double GPSSampleTime, struct gps_data_t *gpsdata, FILE *outFD, uint32_t verbosity);
int openGPS(char* hostname, char* hostport,struct gps_data_t *g, int numberAttempts,int GPSSamplesFlag, char *GPSSamplesFileName);
int init_GPS(char* hostname, char* hostport,struct gps_data_t *g);
double get_location(struct gps_data_t *g, double maxWaitTime, uint32_t verbosity);

int close_GPS(struct gps_data_t *g);

int getGPSData(char *GPSDName, char *GPSDPort, struct gps_data_t *gpsdata);
int getGPSDataOLD(char *GPSDName, char *GPSDPort, struct gps_data_t *gpsdata);
bool isGPSDataValid(struct gps_data_t *gpsdata);

#endif


/************************************************************************
* File:  gpsdHelper.c 
*
* Purpose:
*  This module contains helper functions that are an abstraction 
*   above  a specific location service such as gpsd. 
*   For example, this abstraction would be used on a device such
*   as an LTE dongle that can provide a gps fix.
*
*   It currently assumes a single client in use at a time.
*
* External:
*    int init_GPS(char* hostname, char* hostport,struct gps_data_t *g)
*
* Internal 
* 

* Notes:
*   Check if getGPSData() is correct.
*   Remove getGPSDataOLD and dumpOlD
*
* last update: 9/21/2019
*
************************************************************************/
#include "common.h"
#ifdef NOGPS
#include "gpsdStubs.h"
#else
#include <gps.h>
#endif

#include "gpsdHelper.h"
#include "timeHelper.h"
#include <sys/file.h>

//#define TRACEME1 1 
//#define TRACEME 1 

char *logFile = "GPSSampleLog.out";
FILE *logFP = NULL;

char *lockFileName = "LogLock.out";
FILE *lockFP = NULL;
char *outFile = NULL;


static int numberOfClients = 0;
bool GPSServerInitialized = false;
bool isGPSDInstalled = false;
bool isGPSDInitialized  = false;
bool isGPSDRunning = false;
bool isGPSDRunningLocally = false;
bool isGPSDRunningRemotely = false;
bool isGPSDLocked = false;
bool isCurrentLocationValid = false;

int GPSSamplesFlag = 0;

double measuredGPSSampleTime =0;
double minMeasuredGPSSampleTime =0;
double maxMeasuredGPSSampleTime =0;
uint32_t numberGPSSamples=0;
uint32_t numberGPSFailures=0;
double GPSSampleTimeSum =0;
double GPSXErrorSum=0;
double GPSYErrorSum=0;



/***********************************************************
* Function: int init_GPS(char* hostname, char* hostport,struct gps_data_t *g)
*
* Explanation: inits the client to communicate with the designated 
*              location service. 
*   Note: this succeeds if gpsd is running with and without a source device.
*
* inputs:   
*   char* hostname: name of host running the location service
*   char* hostport : port number of socket to location service
*   struct gps_data_t *g : ptr to callers gps data struct.
*
* outputs: returns a ERROR or NOERROR
*
**************************************************/
int init_GPS(char* hostname, char* hostport,struct gps_data_t *g)
{
  int rc = NOERROR;
  time_t t = time(NULL);
  struct tm timeObj = *localtime(&t);
#ifdef TRACEME
  printf("init_GPS: hostname: %s \n", hostname);
#endif

  isGPSDInstalled = false;
  isGPSDInitialized  = false;
  isGPSDRunning = false;
  isGPSDRunningLocally = false;
  isGPSDRunningRemotely = false;
  isGPSDLocked = false;
  isCurrentLocationValid = false;
  GPSSamplesFlag = 0;


  rc= gps_open(hostname, hostport, g);
  if (rc == ERROR) {
      printf("init_GPS: openGPS FAILED!  %s \n", 
        hostname);
  } else { 
    //else no error
    ;
#ifdef TRACEME
       printf("init_GPS:succedded gps_open %s \n", 
           hostname);
#endif
  } 

   
  lockFP = fopen(lockFileName,"w");
  if (lockFP==NULL) {
    printf("initGPS: HARD ERROR: Failed to open lock file %s \n",lockFileName);
    rc = ERROR;
  } 

  if (rc == NOERROR) { 
    GPSServerInitialized = true;
    numberOfClients++;
#ifdef TRACEME
    printf("init_GPS: SUCCEEDED! \n");
#endif
  }

  return rc;
}


/***********************************************************
* Function: int openGPS(char* hostname, char* hostport,struct gps_data_t *g, int numberAttempts, bool logSamples)
*
* Explanation: This opens the GPS location service
*
* inputs:   
*   char* hostname: name of host running the location service
*   char* hostport : port number of socket to location service
*   struct gps_data_t *g : ptr to callers gps data struct.
*   int numberAttempts:  indicates how many times to try.
*   bool logSamples:  if true, all GPS samples logged
* outputs: returns a ERROR or NOERROR
*
**************************************************/
int openGPS(char* hostname, char* hostport,struct gps_data_t *g, int numberAttempts,int GPSSamplesFlagParam,char *GPSSamplesFileName)
{
  int rc = NOERROR;
  int outerLoop = numberAttempts;
  struct gps_data_t *gpsdata=g;

#ifdef TRACEME
  printf("openGPS: hostname: %s, number attempts:%d  \n", hostname,numberAttempts);
#endif
  if (!isGPSDInitialized)
    rc = init_GPS(hostname, hostport, gpsdata);

  GPSSamplesFlag=GPSSamplesFlagParam;

#ifdef TRACEME
    printf("openGPS: returned from init_GPS with rc of %d \n",rc);
#endif

  if (GPSSamplesFlag == 2)
  {
    if (logFP != NULL)
      fclose(logFP);
    logFP = fopen(GPSSamplesFileName, "w");
#ifdef TRACEME
    printf("openGPS: GPSSamples output file: %s  \n",GPSSamplesFileName);
#endif
  }
  else {
    ;
#ifdef TRACEME
    printf("openGPS: NOT logging GPS Samples \n");
#endif
  }


  if (rc == ERROR) {
    while (outerLoop--) {
      rc= gps_open(hostname, hostport, gpsdata);
      printf("openGPS: returned from init_GPS with %d\n",rc);
      if( rc != NOERROR)
      {
        //Error...
        if (outerLoop<=1) {
          printf("openGPS: Failed %d times , break with error \n",outerLoop);
          outerLoop=0;
          rc = ERROR;
          break;
        } else {
          //else try again
          printf("openGPS: Failed but try again %d \n",outerLoop);
          rc = ERROR;
          sleep(1);
          continue;
        }
      } else 
      {
        printf("openGPS: succeeded to init gps \n");
        rc = NOERROR;
        break;
      }
      sleep(1);
      }
  }
  return rc;
}


/***********************************************************
* Function: int get_location(struct gps_data_t *g, double maxWaitTime, uint32_t verbosity)
*
* Explanation: This fills the caller's gps_datae struct
*        with a location.  Else it returns ERROR.
*        The routine tracks the average time to obtain a location
*        and the number of times it fails.
*
*
* inputs:   
*   struct gps_data_t *g : ptr to callers gps data struct.
*   double  maxWaitTime: max time this routine waits for a gps location fix.
*            If a fix does not occur within this time, an error is returned.
*   uint32_t verbosity : 0,1,2:  level of info for the log entries
*
* outputs: returns a ERROR or NOERROR
*
**************************************************/
double get_location(struct gps_data_t *g, double maxWaitTime, uint32_t verbosity)
{
 int rc = NOERROR;
 int poll_delay=MAX_GPS_POLL_DELAY_TIME;  //in microseconds
 int maxNumberTOs=0;
 int curNumberTOs=0;
 int dataCount=0;
 double TS1=0;
 double TS2=0;
 double curTime=0;
 double startTime=0;
 bool loopFlag=true;

 curTime =  getCurTimeD();

 if (!GPSServerInitialized ) {
    rc = ERROR;
    printf("get_location: ERROR:  not initalized !! \n");
 } else 
 {

  //converts the defined timeout from seconds to the
  // number of wait poll timeouts 
  maxNumberTOs=maxWaitTime*1000000/poll_delay; 
#ifdef TRACEME
  printf("get_location: maxNumberTOs:%d,    \n",maxNumberTOs);
#endif
  rc = gps_stream(g, WATCH_ENABLE | WATCH_JSON ,NULL);

  if (rc == ERROR) {
    printf("get_location:  ERROR on gps_stream,  errno:%d \n",errno);
  } else 
  {
    dataCount =0;
    rc = NOERROR;
    //We want to time how long it takes to get each gps sample
    measuredGPSSampleTime = -1.0;
    TS1=getTimestampD(); 
    while(loopFlag)
    {
      //returns true if data is available, else false if it times out
      if (gps_waiting(g,poll_delay)) {
        //-1 error, 0 no data, >0 bytes read
        rc = gps_read(g);
#ifdef TRACEME
        printf("get_location: rc from gps_read :%d  \n",rc);
#endif

        if(rc == ERROR)
        { 
          printf("get_location: error from gps_read ??  \n");
          break;
        } else if (rc == 0) {
          //indicates no data yet....keep trying 
#ifdef TRACEME
          printf("get_location: read returned 0,  keep trying (rc:%d,dataCount:%d) \n",rc,dataCount);
#endif
          continue;
        } else 
        {
          if (g->online == 0) {
            printf("get_location: WARNING: online timestamp 0 ?? \n");
            rc = ERROR;
          } else 
          {
            rc = NOERROR;
            //else we got a sample!
            curNumberTOs=0;
            //dataCount should always == rc 
            dataCount=rc;
#ifdef TRACEME
            printf("get_location: gps_read returned %d bytes \n",dataCount);
#endif
            if (isGPSDataValid(g)) {
              //Returns true if we got a location
              //We might have received a valid msg that does not contain a 
              //location OR we might have received a msg with a location that
              //was not mode 2 or 3
              ;
#ifdef TRACEME
              printf("get_location:succedded gps_open  \n");
#endif
            }
            else { 
#ifdef TRACEME
              printf("get_location: WARNING isGPSDataValid test-just ignore  \n");
#endif
              //rc = ERROR;
              //break;
              continue;
            }
          }
          if (rc == NOERROR) 
          {
            //check for NaN  as a gps data
            if(g->fix.longitude != g->fix.longitude || g->fix.latitude != g->fix.latitude)
              continue;
            else  {
              ;
#ifdef TRACEME
              printf("get_location: Break as we found a fix (x,y,z), rc:%d \n",rc);
              printf("%f, %f, %f\n",g->fix.longitude,g->fix.latitude,g->fix.altitude);
#endif
              break;
            }
          }
        }
      } else { 
         //else no data yet
         curNumberTOs++;
#ifdef TRACEME1
         printf("get_location: gps_waiting says there is NO data, curNumberTOs:%d (max:%d) \n",curNumberTOs,maxNumberTOs);
#endif
         if(curNumberTOs > maxNumberTOs){
            rc = ERROR;
#ifdef TRACEME
            printf("get_location: TIMED OUT WAITING for a location, curNumberTOs:%d (max:%d) \n",curNumberTOs,maxNumberTOs);
#endif
            numberGPSFailures++;
            loopFlag=false;
            break;
         }
      }
      //sleep for a small amount of time 
      //usleep(0.05);
      //usleep(poll_delay);
    }  //end of while loop

    //If NOERROR, we got a GPS sample
    if (rc == NOERROR) 
    {
      TS2=getTimestampD(); 
      measuredGPSSampleTime = TS2-TS1;
      if (measuredGPSSampleTime < minMeasuredGPSSampleTime)
        minMeasuredGPSSampleTime = measuredGPSSampleTime;
      if (measuredGPSSampleTime > maxMeasuredGPSSampleTime)
        maxMeasuredGPSSampleTime = measuredGPSSampleTime;
      numberGPSSamples++;
      GPSSampleTimeSum+=measuredGPSSampleTime;
      GPSXErrorSum += g->fix.epx;
      GPSYErrorSum += g->fix.epy;
      if (GPSSamplesFlag == 2)
      {
        if (logFP != NULL){
         fprintf(logFP,"%12.9f %12.9f %f %d  %f %f %f %f %f %f %d %2.9f %2.9f %2.9f \n", 
              curTime, g->fix.time, g->fix.ept,
              g->satellites_used,
              g->fix.longitude,g->fix.latitude,g->fix.altitude,
              g->fix.epx, g->fix.epy, g->fix.epv,
              numberGPSSamples, 
              measuredGPSSampleTime,
              minMeasuredGPSSampleTime, maxMeasuredGPSSampleTime);
        }
      } else if (GPSSamplesFlag == 1)
      {
         printf("%12.9f %12.9f %f %d  %f %f %f %f %f %f %d %2.9f %2.9f %2.9f \n", 
              curTime, g->fix.time, g->fix.ept,
              g->satellites_used,
              g->fix.longitude,g->fix.latitude,g->fix.altitude,
              g->fix.epx, g->fix.epy, g->fix.epv,
              numberGPSSamples, 
              measuredGPSSampleTime,
              minMeasuredGPSSampleTime, maxMeasuredGPSSampleTime);
      }
    }
  }

  if (dataCount > 0)
    rc = NOERROR;

#ifdef TRACEME
  if (rc == ERROR) {
    printf("get_location: Exit with error:dataCount:%d, curNumberTOs:%d maxNumberTOs:%d\n",
      dataCount,curNumberTOs,maxNumberTOs);
  }
  else { 
    printf("get_location: Exit with NO error:dataCount:%d, curNumberTOs:%d max:%d\n",
      dataCount,curNumberTOs,maxNumberTOs);
  }
#endif
 }

 if (rc == NOERROR)
   return measuredGPSSampleTime;
 else{ 
   return -1.0;
 }
}

/***********************************************************
* Function: int close_GPS(struct gps_data_t *g)
*
* Explanation: This closes the client session wiht the gps service
*
* inputs:   
*   struct gps_data_t *g : ptr to callers gps data struct.
*
* outputs: returns a ERROR or NOERROR
*
**************************************************/
int close_GPS(struct gps_data_t *g)
{
  int rc = NOERROR;
  double avgGPSSampleTime=0.0;
  double curTime=0;
  double avgGPSXError = 0.0;
  double avgGPSYError = 0.0;

  curTime =  getCurTimeD();
  rc = gps_stream(g,WATCH_DISABLE,NULL);
  if(rc != ERROR ) 
  {
    rc =  gps_close(g);
  }

  if (rc == ERROR){
    printf("close_GPS: HARD ERROR  ??  \n ");
    exit(EXIT_FAILURE);
  } else {
    numberOfClients--;
    if (numberGPSSamples>0) {
     avgGPSSampleTime = GPSSampleTimeSum / (double)numberGPSSamples;
     avgGPSXError = GPSXErrorSum / (double)numberGPSSamples;
     avgGPSYError = GPSYErrorSum / (double)numberGPSSamples;
    }

//#ifdef TRACEME
    printf("close_GPS:curTime numberGPSSamples/Failures min/max/avgSampleTime avgX/Y Error\n");
    printf("%12.9f %d %d %2.9f %2.9f %2.9f %2.4f %2.4f \n",
          curTime,numberGPSSamples,numberGPSFailures, 
          minMeasuredGPSSampleTime, maxMeasuredGPSSampleTime,
          avgGPSSampleTime,avgGPSXError,avgGPSYError);   
//#endif

    if (GPSSamplesFlag == 2){
      if (logFP != NULL){
        fprintf(logFP,"%12.9f %d %d %2.9f %2.9f %2.9f %2.4f %2.4f \n",
          curTime,numberGPSSamples,numberGPSFailures, 
          minMeasuredGPSSampleTime, maxMeasuredGPSSampleTime,
          avgGPSSampleTime,avgGPSXError,avgGPSYError);   
  
        fclose(logFP);
      }
    } else if (GPSSamplesFlag == 1) 
    {
        printf("%12.9f %d %d %2.9f %2.9f %2.9f %2.4f %2.4f \n",
          curTime,numberGPSSamples,numberGPSFailures, 
          minMeasuredGPSSampleTime, maxMeasuredGPSSampleTime,
          avgGPSSampleTime,avgGPSXError,avgGPSYError);   
    }
  }

  if (lockFP != NULL)
    fclose(lockFP);

  return rc;
}

/***********************************************************
* Function: bool isGPSDataValid(struct gps_data_t *gpsdata)
*
* Explanation: determies if the gps data is valid 
*
* inputs:   
*   struct gps_data_t *g : ptr to callers gps data struct.
*
* outputs: returns a true or false 
*
**************************************************/
bool isGPSDataValid(struct gps_data_t *gpsdata)
{
  int rc = NOERROR;

  if (gpsdata==NULL) {
    printf("validateGPSData:  NOT VALID: gpsdata ptr null \n");
    rc=ERROR;
  }
  else { 
#ifdef TRACEME
    printf("validateGPSData: online:%f status:%d numberSats:%d  \n",
            gpsdata->online, gpsdata->status,gpsdata->satellites_used);
#endif
    if (gpsdata->status == 0) {
      //printf("validateGPSData:  WARNING status = 0, just continue  \n");
      rc = ERROR;
    } else {
       rc = NOERROR;
#ifdef TRACEME
       if (gpsdata->status == 1) {
         printf("validateGPSData:  Valid fix without DGPS \n");
       } else if (gpsdata->status == 2) {
         printf("validateGPSData:  Valid fix with DGPS \n");
       } else {
         printf("validateGPSData:  NOT A VALID FIX value ?? %d \n",
                gpsdata->status);
         rc=ERROR;
       }
#endif
    }
    if (rc == NOERROR) {
         //sometimes the  GPS doesnt have a fix, it sends you data anyways
         //the values for the fix are NaN. this is a clever way to check for NaN.
         if(gpsdata->fix.longitude!=gpsdata->fix.longitude || gpsdata->fix.altitude!=gpsdata->fix.altitude){
           printf("validateGPSData: long equal so strange nan gps return....return error as it is not a valid location  \n");
           rc = ERROR;
         }

#ifdef TRACEME
       if (rc == NOERROR) {
         printf("ValidateGPSData: mode:%d  x,y,z: (%lf, %lf, %lf);  \n",
              gpsdata->fix.mode,
              gpsdata->fix.latitude, 
              gpsdata->fix.longitude, gpsdata->fix.altitude);
         printf("   accuracy x,y,z  %lf, %lf, %lf);  \n",
              gpsdata->fix.epx, 
              gpsdata->fix.epy, gpsdata->fix.epv);

         printf("   timestamp: %f  time error:%f ",
              gpsdata->fix.time, gpsdata->fix.ept); 

         printf("   online:%f status:%d numberSats:%d  \n",
              gpsdata->online, 
              gpsdata->status,gpsdata->satellites_used);
       }
#endif
    }
  }
  if (rc == NOERROR) 
   return true;
  else 
   return false;
}

/***********************************************************
* Function: int writeGPSLine(double curTime, struct gps_data_t *gpsdata, FILE *outFD, uint32_t verbosity)
*
* Explanation:writes a summary of gps data to the output file.
*
* inputs:   
*   double curTime: 
*   struct gps_data_t *g : ptr to callers gps data struct.
*   FILE *outFD : target file descriptor
*   verbosity: 0,1, or 2 for min, medium, max amount of info to place in a log entry
*
* outputs: returns an  ERROR or NOERROR
*
**************************************************/
int writeGPSLine(double curTime,double GPSSampleTime, struct gps_data_t *gpsdata, FILE *outFD, uint32_t verbosity)
{
  int rc = NOERROR;
  double largestError=0;

  if (gpsdata->fix.epx >  gpsdata->fix.epy)
      largestError =  gpsdata->fix.epx;
  else 
      largestError =  gpsdata->fix.epy;

#ifdef TRACEME 
       printf("ValidateGPSData(%f):GPSSampleTime:%f  mode:%d,speed:%d  x,y,z: (%lf, %lf, %lf);  \n",
              curTime, GPSSampleTime, gpsdata->fix.mode,
              gpsdata->fix.speed,
              gpsdata->fix.latitude, gpsdata->fix.longitude, gpsdata->fix.altitude);
         printf("   accuracy x,y,z  %lf, %lf, %lf);  \n",
              gpsdata->fix.epx, gpsdata->fix.epy, gpsdata->fix.epv);
         printf("   timestamp: %f  time error:%f ",
              gpsdata->fix.time, gpsdata->fix.ept); 
         printf("   online:%f status:%d numberSats:%d  \n",
              gpsdata->online,  gpsdata->status,gpsdata->satellites_used);
#endif

  if (outFD == NULL){
     printf("writeGPSLine(%f): ERROR: Bad output File handle \n",curTime);
     rc = ERROR;
  }
  else {

    if (verbosity == 0) {
      fprintf(outFD,"%f %d %d %lf %lf %3.6f \n", 
          curTime, gpsdata->fix.mode, gpsdata->status, 
          gpsdata->fix.latitude, gpsdata->fix.longitude,largestError);
    } else if (verbosity == 1) {
       fprintf(outFD,"%f %d %d %d %3.9f %f %lf %lf %lf %lf %lf %lf\n",
              curTime, 
              gpsdata->fix.mode, gpsdata->status, gpsdata->satellites_used, 
              GPSSampleTime, 
              gpsdata->fix.speed,
              gpsdata->fix.latitude, gpsdata->fix.longitude, 
                  gpsdata->fix.altitude,
              gpsdata->fix.epx, gpsdata->fix.epy, gpsdata->fix.epv);
    } else if (verbosity == 2 ) {
      fprintf(outFD," %12.9f MODE:%d STATUS%d SATS:%d sampleTime:%3.9f %3.9f %3.9f %12.9f %3.6f GPS: %lf %lf %lf ERROR: %lf %lf %lf \n",
              curTime, gpsdata->fix.mode, gpsdata->status, gpsdata->satellites_used, 
              GPSSampleTime,  minMeasuredGPSSampleTime, maxMeasuredGPSSampleTime,
              gpsdata->fix.speed, gpsdata->fix.eps,
              gpsdata->fix.latitude, gpsdata->fix.longitude, gpsdata->fix.altitude,
              gpsdata->fix.epx, gpsdata->fix.epy, gpsdata->fix.epv);
double maxMeasuredGPSSampleTime =0;
    } else  {
     rc = ERROR;
     printf("writeGPSLine(%f): ERROR: Bad verbosity value (%d) ?? \n",curTime,verbosity);
    }
  }
  return rc;
}

/***********************************************************
* Function: int writeGPSFakeData(double curTime, 
*     double GPSSampleTime,double lat, double long, double alt,
*     char *outputFileName,uint32_t verbosity);
*
* Explanation:writes Fake data 
*      The file will need to be opened, flocked, written, unlocked, closed.
*
* inputs:   
*    double curTime, 
*    double GPSSampleTime:
*    double lat, double long, double alt,
*    char *outputFileName : output file 
*    uint32_t verbosity: 0, 1, 2 adds detail to output
*
* outputs: returns an  ERROR or NOERROR
*
**************************************************/
int writeGPSFakeData(double curTime, double GPSSampleTime, double latitude, double longitude, double alt, char *outputFileName,uint32_t verbosity)
{
  int rc = NOERROR;
  FILE *fp = NULL;
  char *localFN=NULL;
  struct gps_data_t fakeGpsData;
  struct gps_data_t *gpsdata = &fakeGpsData;
  double largestError = 0.0;

  //First, lets fill the fake gps data struct 
  gpsdata->fix.mode = 3;
  gpsdata->status  = 0;
  gpsdata->fix.latitude= latitude;
  gpsdata->fix.longitude= longitude;
  gpsdata->fix.altitude= alt;
  largestError = 0.0;
  gpsdata->satellites_used = 0;
  GPSSampleTime = 0.0;
  gpsdata->fix.speed = 0.0;
  gpsdata->fix.epx = 0.0;
  gpsdata->fix.epy = 0.0;
  gpsdata->fix.epv = 0.0;

  if (outputFileName == NULL)
   localFN=outFile;
  else
   localFN=outputFileName;


  fp = fopen(localFN,"w");

  if (fp==NULL) {
    printf("writeGPSFile: HARD ERROR: Failed to open file %s \n",localFN);
    rc = ERROR;
  } else 
  {

#ifdef TRACEME
    printf("writeGPSFile: opening LOCKED file %s \n",localFN);
#endif 
    rc= flock(fileno(fp), LOCK_EX);
//    rc = writeGPSLine(curTime,GPSSampleTime, gpsdata, fp, verbosity);
    if (verbosity == 0) {
      fprintf(fp,"%f %d %d %lf %lf %3.6f \n", 
          curTime, gpsdata->fix.mode, gpsdata->status, 
          gpsdata->fix.latitude, gpsdata->fix.longitude,largestError);
    } else if (verbosity == 1) {
       fprintf(fp,"%f %d %d %d %3.9f %f %lf %lf %lf %lf %lf %lf\n",
              curTime, 
              gpsdata->fix.mode, gpsdata->status, gpsdata->satellites_used, 
              GPSSampleTime, 
              gpsdata->fix.speed,
              gpsdata->fix.latitude, gpsdata->fix.longitude, 
                  gpsdata->fix.altitude,
              gpsdata->fix.epx, gpsdata->fix.epy, gpsdata->fix.epv);
    }

    rc= flock(fileno(fp), LOCK_UN);
  }

  if (fp != NULL)
    fclose(fp);

  return rc;
}

/***********************************************************
* Function: int writeGPSFile(struct gps_data_t *gpsdata, char *outputFileName,uint32_t verbosity)
*
* Explanation:writes a summary of gps data to the output file.
*      The file will need to be opened, flocked, written, unlocked, closed.
*
* inputs:   
*   struct gps_data_t *g : ptr to callers gps data struct.
*   char *outFile:  If null, then we use the default outFile. Else,
*              we use the file specified by this param.
*
*   uint32_t verbosity :  0,1,2 for low, medium, large amount of info each log entry
*
* outputs: returns an  ERROR or NOERROR
*
**************************************************/
int writeGPSFile(double curTime, double GPSSampleTime, struct gps_data_t *gpsdata, char *outputFileName,uint32_t verbosity)
{
  int rc = NOERROR;
  FILE *fp = NULL;
  char *localFN=NULL;

  if (outputFileName == NULL)
   localFN=outFile;
  else
   localFN=outputFileName;


  fp = fopen(localFN,"w");

  if (fp==NULL) {
    printf("writeGPSFile: HARD ERROR: Failed to open file %s \n",localFN);
    rc = ERROR;
  } else 
  {

#ifdef TRACEME
    printf("writeGPSFile: opening LOCKED file %s \n",localFN);
#endif 
    rc= flock(fileno(fp), LOCK_EX);
    rc = writeGPSLine(curTime,GPSSampleTime, gpsdata, fp, verbosity);
    rc= flock(fileno(fp), LOCK_UN);
  }

  if (fp != NULL)
    fclose(fp);

  return rc;
}


/****************************************
* routine: int getGPSData(char *GPSDName, char *GPSDPort, struct gps_data_t *gpsdata)
*
*  This function fills in the caller's gpsdata
*   struct with updated gps data. 
*
*
* inputs: none
*   char *GPSDName, : caller's GPSD network name
*   uint16_t GPSDPort: caller's GPSD port
*   struct gps_data_t *gpsdata:  reference to caller's gpsdata struct
*
* outputs: returns ERROR (-1)   NOERROR (0) 
*     0:  successful,
*   Anything > 0 is generally an error. Provides an error code.
*     SOme of these might have resulted in data inserted in gpsdata.
*       
*     1:No software installed to get a location
*     2:software installed -  no access to gps sensing data
*     3:local gps looks ok - failed to connect to the local server
*     4:local gps setup to remotely use a different location service - and this fails 
*     5:gps device can not lock to any satellites 
*     6: gps device locks to at least 1 satellite but can not sync with others to get a waypoint
*     7: poor RF conditions prevents access to gps 
*     8: GPS device read error 
*
* Details/notes:
*
*    Outer loop tries for 10 times before quitting. 
*
****************************************/
int getGPSData(char *GPSDName, char *GPSDPort, struct gps_data_t *gpsdata)
{
  int rc = NOERROR;
  int outerLoop = MAX_OPEN_ATTEMPTS;
  double curTime =  getCurTimeD();
  uint32_t verbosity = 1;
 double TS1=0;
 double TS2=0;
  double GPSSampleTime =0;

  //Check for these errors?
  //sofware??  isMachineConfiguredWithGPS
  //sensing hardware ok ?
  //local GPS fails to connect  
  //using remote GPS - fails to connect 
  //gps can not lock to any satellites 
  //gps locks to 1 or more sats but can not sync or get a fix
  // gps : poor rf conditions prevents sync/lock. 

#ifdef TRACEME 
printf("getGPSData: try %d attempts to open gpsd %s with port %s \n",
        outerLoop, GPSDName,GPSDPort);
#endif
  GPSSampleTime = -1.0;
  TS1=getTimestampD(); 
  while (outerLoop--) {

    rc = gps_open(GPSDName, GPSDPort, gpsdata);
    if( rc != NOERROR)
    {
      if (outerLoop<=1) {
        fprintf(stderr,"Could not connect to GPSd\n");
        outerLoop=0;
        rc = GPS_ERROR_CONNECT_FAILURE;
        break;
      } else {
       //else try again
       printf("getGPSData: Error on open attempt :%d, errno:%d \n",
                  outerLoop, errno); 
       perror("getGPSData: Error on open ?? ");
       sleep(1);
       continue;
      }
    } else 
    {

      //register for updates
      gps_stream(gpsdata, WATCH_ENABLE | WATCH_JSON, NULL);
    
      fprintf(stderr,"Waiting for gps lock.");
      //when status is >0, you have data.
      while(gpsdata->status==0)
      {
        //block for up to .5 seconds
        if (gps_waiting(gpsdata, 500)){

            if(gps_read(gpsdata)==-1){
                fprintf(stderr,"GPSd Error\n");
                gps_stream(gpsdata, WATCH_DISABLE, NULL);
                gps_close(gpsdata);
                rc = GPS_READ_ERROR;
                break;
            }
            else{
                //status>0 means you have data
                if(gpsdata->status>0){
                    //sometimes if your GPS doesnt have a fix, it sends you data anyways
                    //the values for the fix are NaN. this is a clever way to check for NaN.
                    if(gpsdata->fix.longitude!=gpsdata->fix.longitude || gpsdata->fix.altitude!=gpsdata->fix.altitude){
                        fprintf(stderr,"Could not get a GPS fix.\n");
                        gps_stream(gpsdata, WATCH_DISABLE, NULL);
                        gps_close(gpsdata);
                        rc = GPS_ERROR_FAILED_TO_GET_LOCATION;
                        break;
                    }
                    //otherwise you have a legitimate fix!
                    else
                        fprintf(stderr,"\n");
                }
                //if you don't have any data yet, keep waiting for it.
                else
                    fprintf(stderr,".");
            }
        }
        //apparently gps_stream disables itself after a few seconds.. 
        //in this case, gps_waiting returns false.
        //we want to re-register for updates and keep looping! we dont have a fix yet.
        else
            gps_stream(gpsdata, WATCH_ENABLE | WATCH_JSON, NULL);

        //just a sleep for good measure.
        sleep(1);
      }
    }
    sleep(1);
  }
  TS2=getTimestampD(); 
  GPSSampleTime = TS2-TS1;

  //cleanup
  if (rc == NOERROR) {
    rc = writeGPSLine(curTime,GPSSampleTime, gpsdata, NULL,verbosity);
    gps_stream(gpsdata, WATCH_DISABLE, NULL);
    gps_close(gpsdata);
  } else {

    if (rc != GPS_ERROR_CONNECT_FAILURE)
       gps_close(gpsdata);
  }
    return rc;
}

/****************************************
* routine: int getGPSDataOLD(struct gps_data_t *gpsdata){
*
*
* explanation: inits the gps and get a location
*
* inputs: none
*  struct gps_data_t *gpsdata): reference to caller's gpsdata structure
*
* outputs: returns ERROR or NOERROR.  NOERROR implies gpsdata is not 
*   hold a valid location.
*
* Details/notes:
*
****************************************/
int getGPSDataOLD(char *gpsServerName, char *GPSDPort, struct gps_data_t *gpsdata)
{
  int rc = NOERROR;

    //connect to GPSd
    if(gps_open(gpsServerName, GPSDPort, gpsdata)<0){
        printf("getGPSDataOLD: Could not connect to GPSd %s \n",gpsServerName);
        return(-1);
    }

    //register for updates
    gps_stream(gpsdata, WATCH_ENABLE | WATCH_JSON, NULL);
    
    fprintf(stderr,"Waiting for gps lock.");
    //when status is >0, you have data.
    while(gpsdata->status==0){
        //block for up to .5 seconds
        if (gps_waiting(gpsdata, 500)){
            if(gps_read(gpsdata)==-1){
                printf("getGPSDataOLD:  GPSd read Error\n");
                gps_stream(gpsdata, WATCH_DISABLE, NULL);
                gps_close(gpsdata);
                return(-1);
                break;
            }
            else{
                //status>0 means you have data
                printf("getGPSDataOLD: read rc: %d \n",rc);
                if(gpsdata->status>0){
                    //sometimes if your GPS doesnt have a fix, it sends you data anyways
                    //the values for the fix are NaN. this is a clever way to check for NaN.
                    if(gpsdata->fix.longitude!=gpsdata->fix.longitude || gpsdata->fix.altitude!=gpsdata->fix.altitude){
                        printf("getGPSDataOLD: Could not get a GPS fix.\n");
                        //gps_stream(gpsdata, WATCH_DISABLE, NULL);
                        //gps_close(gpsdata);
                        // return(-1);
                        gpsdata->status=0;
                        continue;
                    }
                    //otherwise you have a legitimate fix!
                    else
                        printf("getGPSDataOLD: GOT A FIX !! rc:%d\n",rc);
                }
                //if you don't have any data yet, keep waiting for it.
                else
                 printf("getGPSDataOLD:read returned a 0, just keep waiting\n");
            }
        }
        //apparently gps_stream disables itself after a few seconds.. in this case, gps_waiting returns false.
        //we want to re-register for updates and keep looping! we dont have a fix yet.
        else {
         rc = gps_stream(gpsdata, WATCH_ENABLE | WATCH_JSON, NULL);
         printf("getGPSDataOLD: JJM: redid  gps_stream, rc : %d \n",rc);
        }
        //just a sleep for good measure.
        sleep(1);
    }


  if (rc != ERROR) {
    //cleanup
    debugDumpOLD(gpsdata);
    gps_stream(gpsdata, WATCH_DISABLE, NULL);
    gps_close(gpsdata);
    rc = NOERROR;
  }

  return rc;
}


void debugDumpOLD(struct gps_data_t *gpsdata){
    fprintf(stderr,"Longitude: %lf\nLatitude: %lf\nAltitude: %lf\nAccuracy: %lf\n\n",
                gpsdata->fix.latitude, gpsdata->fix.longitude, gpsdata->fix.altitude,
                (gpsdata->fix.epx>gpsdata->fix.epy)?gpsdata->fix.epx:gpsdata->fix.epy);
}





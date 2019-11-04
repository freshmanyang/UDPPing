/*********************************************************
*
* Module Name: UDP Ping Server main program 
*
* File Name:  UDPPingServer 
*
* Summary:  This is the main UDP ping  server program
*
* Invocation:
*        ./UDPPingServer <service or port number > <MAX msg Size>   <traceLevel>
*           <serveric/port >  string holding service or port
*           <maxMsgSize> : optional param that allows the server to specify
*               the max allowed on a read. Otherwise the
*               define MAXMSGSIZE is used.
*               The max a UDP socket would ever return is defined by TCP/IP:
*               64Kbytes - (size of the IP header + UDP header)
*               Assuming smallest size headers this is 65535-(20+8) = 65507 bytes.
*               REMEMBER: Using the max size will lead to an IP datagram of  64Kbytes
*               but this will be broken up into many IP packets (fragments) as the
*               largest IP packet size that can fit in a typical ethernet type frame
*                    is limited by the link/phy layers maximum transmission unit (MTU). 
*                    An MTU of 1500 bytes is common for ethernet.  This maps to 
*                    a maximum message size of 1472 bytes to avoid fragmentation.
*
*      <traceLevel>       : optional....value 1 by default.  Sets the level of info displayed.
*                            Values/tracelevel
*                                   0   displays end of program stats and error msgs
*                                   1   Additionally displays RTT sample each iteration
*                                   2   displays debug info
*                             
* Design notes;
*    The server uses a single buffer for both the receive and the send
*    The server does not yet maintain per session statistics
*    
*
* Revisions:
*
*  Last update: 10/18/2019
*
*********************************************************/
#include "./commonCode/common.h"
#include "./commonCode/AddressHelper.h"
#include "./commonCode/SocketHelper.h"
#include "./commonCode/messages.h"
#include "version.h"

//#define TRACEME 1

//Routines found in this file
void CNTCHandler();
void exitProcessing(int errorStatus, double curTime);
double gettimestampD(uint32_t sec, uint32_t nsec);
bool runFlag = true;
uint32_t numberIterations = 0;
int sock = -1;
double totalBytesRxed = 0;
uint32_t numberMessages = 0;
uint32_t RxSeqNumber = 0;
uint32_t lastSeqNumber = 0;
uint32_t dropEstimate = 0;
uint32_t outOfOrderArrivals = 0;
double avgQuality = 0;
double avgRSSI = 0;

char *RxBufPtr = NULL;
int traceLevel = 1;
double startTime = -1;
double finishTime = -1;
double lastRxTime = -1;
TGIFHeartbeat *rxHeader;
TGIFACK *rxACK;
double servStartTime = -1;
double servFinishTime = -1;
double avgOwd = 0;

//  0:  normal ping mode
//  1:  ACKs a message that only contains the ACK number - so if the client sends 1472 bytes,
//         the server modifies the msgSize on the sendMsg to 4.
int mode = 0;

int main(int argc, char *argv[])
{

  int rc = NOERROR;
  char *service = NULL;   //sets port number
  double wallTime = -1.0; //wall clock time
  int sockOption = 1;
  int sockOptionSize = sizeof(sockOption);

  int maxMsgSize = -1;
  //Maintains the most recent ACK number that has been received
  //Used when we pull the RxSequence Number from a received MSG
  unsigned int *RxSeqNumberPtr = NULL;

  int bytesRxed = -1;

  wallTime = getCurTimeD();
  servStartTime = getCurTimeD();

  setVersion(VersionLevel);
  if (argc < 2)
  { // Test for correct number of arguments
    printf("%s(Version:%s) pid:%d:Usage: <port number>  <max msgSize>  <traceLevel> \n ",
           argv[0], getVersion(), getpid());
    rc = EXIT_FAILURE;
    exit(rc);
  }

  service = argv[1]; // First arg:  local port

  traceLevel = 1;
  maxMsgSize = MAXMSGSIZE;
  if (argc == 3)
    maxMsgSize = atoi(argv[2]);
  else if (argc == 4)
  {
    maxMsgSize = atoi(argv[2]);
    traceLevel = atoi(argv[3]);
  }

  signal(SIGINT, CNTCHandler);

  printf("%s(Version:%s) pid:%d Entered with %d arguements, maxMsgSize:%d, service:%s, traceLevel:%d\n",
         argv[0], getVersion(), getpid(), argc, maxMsgSize, service, traceLevel);

  RxBufPtr = (char *)malloc(sizeof(char) * maxMsgSize);
  if (RxBufPtr == NULL)
  {
    printf("%s(Version:%s) pid:%d  Malloc error ,  errno:%d \n",
           argv[0], getVersion(), getpid(), errno);
    return EXIT_FAILURE;
  }

  //This points to rx buffer so client can obtain the RxSeqNumber
  RxSeqNumberPtr = (unsigned int *)RxBufPtr;

  // Create socket for incoming connections
  sock = SetupUDPServerSocket(service);
  if (sock < 0)
  {
    printf("%s(Version:%s):  SetupUDPServiceSocket error.....errno:%d \n",
           argv[0], getVersion(), errno);
    return EXIT_FAILURE;
  }
  //Allow multiple sockets to use the same port
  //We might want have > 1 instances running all using the same port
  // one might use unicast another broadcast
  sockOption = 1;
  rc = SetSocketOption(sock, SO_REUSEADDR, &sockOption, sockOptionSize);
  if (rc == ERROR)
  {
    perror("perfServer: ERROR enabling REUSEADDR  ");
    exit(EXIT_FAILURE);
  }

  /* setup broadcast even if specifiied UNICAST.... */
  rc = SetSocketOption(sock, SO_BROADCAST, &sockOption, sockOptionSize);
  if (rc == ERROR)
  {
    printf("perfServer(%f) ERROR:  setsockopt error when enabling broadcast,  errno: %d  \n", wallTime, errno);
    perror("perfServer: ERROR enabling broadcast ");
    exit(EXIT_FAILURE);
  }

  rc = NOERROR;
  // Begin LOOP
  wallTime = getCurTimeD();
  //startTime=getTimestampD();
  for (;;)
  {
    struct sockaddr_storage clntAddr; // Client address
    // Set Length of client address structure (in-out parameter)
    socklen_t clntAddrLen = sizeof(clntAddr);

    if (runFlag == true)
    {
      numberIterations++;
      bytesRxed = RxMsg(sock, (void *)RxBufPtr, maxMsgSize, (struct sockaddr *)&clntAddr, &clntAddrLen);
      lastRxTime = getTimestampD();
      if (startTime == -1.0)
        startTime = lastRxTime;
      wallTime = getCurTimeD();
      if (bytesRxed == EXIT_FAILURE)
      {
        rc = ERROR;
        printf("UDPPingServer:  RxMsg failed  \n");
        close(sock);
        exit(EXIT_FAILURE);
      }
      else
      {

        if (numberIterations > 0)
        {
          totalBytesRxed += bytesRxed;
          numberMessages += 1;
          rc = NOERROR;
          rxHeader = (TGIFHeartbeat *)RxBufPtr;
          int32_t quality = ntohl(rxHeader->SignalQuality);
          int32_t RSSI = ntohl(rxHeader->RSSI);
          avgQuality += (quality - avgQuality) / numberMessages;
          avgRSSI += (RSSI - avgRSSI) / numberMessages;
          mode = rxHeader->code;
          RxSeqNumber = (unsigned int)ntohl((unsigned int)(rxHeader->sequenceNum));
          // struct timespec ts;
          // ts.tv_nsec = ntohl(rxHeader->ts_nsec);
          if (traceLevel == 2)
            printf("#TRACE nsec %ld\n", rxHeader->ts_nsec);
          if (RxSeqNumber <= lastSeqNumber)
            outOfOrderArrivals++;
          else if (RxSeqNumber == (lastSeqNumber + 1))
          {
            lastSeqNumber = RxSeqNumber;
          }
          else
          {
            dropEstimate += (RxSeqNumber - lastSeqNumber - 1);
          }
          if (traceLevel >= 1)
          {
            printf("%f,%d,%d,%d,%d,%d,%9.0f,%d",
                   wallTime, RxSeqNumber, lastSeqNumber,
                   numberMessages, outOfOrderArrivals, dropEstimate,
                   totalBytesRxed, numberIterations);
          }
#ifdef TRACEME
          PrintSocketAddress((struct sockaddr *)&clntAddr, stdout);
          fputc('\n', stdout);
#endif
          if (traceLevel > 1)
          {
            printf("UDPPingServer: RxSeqNumber:%d,  %d bytes, client: ", RxSeqNumber, bytesRxed);
            PrintSocketAddress((struct sockaddr *)&clntAddr, stdout);
            fputc('\n', stdout);
          }
          struct timespec *ts;
          ts = (struct timespec *)malloc(sizeof(struct timespec));
          clock_gettime(CLOCK_REALTIME, ts);
          double owd = gettimestampD(ts->tv_sec, ts->tv_nsec) - gettimestampD(ntohl(rxHeader->ts_sec), ntohl(rxHeader->ts_nsec));
          avgOwd += (owd - avgOwd) / RxSeqNumber;
          if (traceLevel == 2)
            printf("#TRACE owd : %f", owd);
          if (traceLevel >= 1)
            printf("%f\n", owd);
          if (mode == 0)
          {
            //while(1){;}
            rxHeader->ts_nsec = htonl(ts->tv_nsec);
            rxHeader->ts_sec = htonl(ts->tv_sec);
            rc = sendMsg(sock, (void *)RxBufPtr, bytesRxed, (struct sockaddr *)&clntAddr, clntAddrLen);
          }
          else if (mode == 1)
          {
            //while(1){;}
            rxACK = (TGIFACK *)malloc(sizeof(TGIFACK));
            rxACK->sequenceNum = htonl(RxSeqNumber);
            rxACK->ts_nsec = htonl(ts->tv_nsec);
            rxACK->ts_sec = htonl(ts->tv_sec);
            rc = sendMsg(sock, (void *)rxACK, sizeof(TGIFACK), (struct sockaddr *)&clntAddr, clntAddrLen);
          }
          else if (mode == 2)
          {
            if (traceLevel == 2)
              printf("#TRACE mode2 seq %ld:\n", rxHeader->sequenceNum);
          }
          if (rc == ERROR)
          {
            printf("UDPPingServer:  sendMsg failed \n");
            close(sock);
            exit(EXIT_FAILURE);
          }
        }
      }
    }
    else
    {
      printf("%s(Version:%s): Loop should break....numberIterations:%d  \n",
             argv[0], getVersion(), numberIterations);
      break;
    }
  }
  exitProcessing(rc, getCurTimeD());
  if (traceLevel > 1)
  {
    printf("%s(Version:%s): Exiting rc: %d, numberIterations:%d  \n",
           argv[0], getVersion(), rc, numberIterations);
  }
  exit(rc);
}

/***********************************************************
* Function: void CNTCHandler() 
*
* Explanation:  This is called when a SIGTERM is received
*               This occurs when the program terminates with a CNT-C input.
*
* inputs:   
*        none
* outputs:
*        none 
*
* notes: 
*
*************************************************/
void CNTCHandler()
{
  int rc = NOERROR;
  runFlag = false;
  if (traceLevel > 1)
  {
    printf("CNTCHandler: numberIterations:%d  \n", numberIterations);
  }

  time_t rawtime;
  struct tm *timeinfo;
  time(&rawtime);
  timeinfo = localtime(&rawtime);
  servFinishTime = getCurTimeD();
  double testDuration = servFinishTime - servStartTime;

  printf("\nCurrent time %s, Duration of the test %f secs, mode %d, number of samples %d, avg One way delay %f, estimate of number lost %d,throughput %f, avg Quality Level %f, avg RSSI %f \n",
   asctime(timeinfo), testDuration, mode, numberMessages, avgOwd, dropEstimate, totalBytesRxed / testDuration,avgQuality,avgRSSI);

  exitProcessing(rc, getCurTimeD());
  exit(0);
}

/***********************************************************
* Function: void exitProcessing(int errorStatus, double curTime) 
*
* Explanation:  This is called when the client is about to
*               terminate. It displays session statistics.
*
* inputs:   
*     int errorStatus : ERROR or NOERROR
*     double curTime:  the current wall clock time represented as a double 
*
* outputs:
*        none 
*
* notes: 
*
**************************************************/
void exitProcessing(int errorStatus, double curTime)
{
  double sessionDuration = 0.0;
  double avgRxRate = 0.0;
  double avgLossRate = 0.0;

  finishTime = lastRxTime;
  sessionDuration = finishTime - startTime;
  avgRxRate = totalBytesRxed * 8 / sessionDuration;
  avgLossRate = dropEstimate / (numberIterations);

  if (sock != -1)
  {
    close(sock);
  }

  if (RxBufPtr != NULL)
  {
    free(RxBufPtr);
  }

  if (errorStatus == ERROR)
    printf("UDPEchoServer: Exit in ERROR:  ");
  else
  {
    if (traceLevel == 0)
    {
      printf("%f %f %9.0f %1.4f %d %d %d %d \n",
             curTime, sessionDuration, avgRxRate, avgLossRate,
             numberMessages, RxSeqNumber, outOfOrderArrivals, dropEstimate);
    }
    if (traceLevel == 0)
    {
      printf("UDPEchoServer Results(%f): duration:%f avgRxRate:%9.0f bps avgLossRate:%1.4f \n",
             curTime, sessionDuration, avgRxRate, avgLossRate);
      printf("totalArrivals:%d, last RxSeqNumber:%d, outOfOrders:%d, drops:%d \n",
             numberMessages, RxSeqNumber, outOfOrderArrivals, dropEstimate);
    }
  }
}

double gettimestampD(uint32_t sec, uint32_t nsec)
{
  return (double)sec + (double)nsec / 1000000000;
}
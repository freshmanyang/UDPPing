/*********************************************************
*
* Module Name: network  Utility routines  
*
* File Name:  netHelper.c 
*
* Summary:  This holds helpful routines related to networking
*
* NOTE: See system call  netdevice 
* NOTE: To see socket ioctl's,  issue 'man 7 socket'
*          To see tty ioctls (like TIOCOUTQ) 'man 4 tty_ioctl'
*
* Last update: 8/1/2019
*
*********************************************************/
#include "common.h"
#include "utils.h"
#include "AddressHelper.h"
#include "SocketHelper.h"
#include "netHelper.h"

//#define TRACEME 0


/***********************************************************
* Function: int queryBufferBytes(int socket_descriptor, unsigned long int option)
*
* Explanation:  This issues an ioctl to learn either 
*               the number of bytes in the output buffer (TIOCOUTQ)
*               OR
*               the number of bytes in the input buffer (TIOCINQ)
*
* inputs:   
*      int sock
*      int option
*
* outputs: returns an ERROR or the number of bytes waiting in the queue.
*
**************************************************/
int queryBufferBytes(int socket_descriptor, unsigned long int option)
{
  int rc = NOERROR;
  uint32_t size;
  int optionValue  = 0;
  int len = sizeof(optionValue);

  if ( (socket_descriptor != -1) || (option != TIOCINQ) || (option != TIOCOUTQ) ){
    rc = ERROR;
  } else 
  {

    // Move this to ??
    //To communicate with the rtc
    //dd = open("/dev/rtc0",O_RDONLY); 
    //rc =  ioctl(dd, RTC_RD_TIME, (void *) myRTCTime);
    //rc = read(dd,&RTCData, sizeof(long int));


    //The length of data in Send Buffer which is not drained yet 
    //  (either not sent yet or sent but not acknowledged by receiver):
    //rc =ioctl( socket_descriptor, TIOCOUTQ, &size );  // alternative 1
    rc =ioctl( socket_descriptor, option, &size );  // alternative 1

  if (rc == ERROR) {
    printf("queryBufferBytes: Error on ioctl (TIOCOUTQ) : %d \n",errno);
  } else {
    rc = (int)size;
#ifdef TRACEME 
    printf("queryBufferBytes: IOCTL rc:%d  size:%d  \n",rc, size);
#endif
  }

  }
  return rc;
}

/***********************************************************
* Function: bool isWireless(const char* ifname, char* protocol) 
*
* Explanation:  This indicates if the IF name is wireless 
*
* inputs:   
*   const char* ifname: String ptr to the interface name of interest
*   char* protocol:
*
* outputs: returns true if the ifname is wireless.
*        Returns false if not wireless or if an error occurs.
* 
**************************************************/
bool isWireless(const char* ifname, char* protocol) 
{
  bool rc=false;
  int sock = -1;
  struct iwreq pwrq;
  memset(&pwrq, 0, sizeof(pwrq));

  if ((ifname == NULL) || (protocol == NULL)) {
    return rc;
  }
 
  strncpy(pwrq.ifr_name, ifname, IFNAMSIZ);

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    perror("isWireless:  ERROR socket call: ");
    rc = false;
  } else {
    if (ioctl(sock, SIOCGIWNAME, &pwrq) != -1) {
      if (protocol) 
        strncpy(protocol, pwrq.u.name, IFNAMSIZ);
      rc = true;
    }
  }
  close(sock);
  return rc;
}

/***********************************************************
* Function: int getIFnames(char *arrayOfIFNames[]) 
*       
* Explanation: The caller passes in an array that
*              is to be filled in by this routine
*              with the names of all network interfaces
*              that are 'up' and NOT lo (local IF).
*              The number of IF names inserted in the
*              array is returned to the caller.
*
* inputs:   
*      char *arrayIFNames[] : array of ptrs to IF name strings 
*        Note: we could pass this as char **arrayIFNames
*
* outputs: returns a ERROR OR a number >=0 representing
*          the number of IF's that are 'up'  
*          This routine mallocs the memory for the array
*          The ownership of the memory transfers to the caller. 
*          when the callers ptr is set to point to the arrayOfPtrs
*         
*
***************************************************/
int getIFnames(char *arrayOfIFNames[]) 
{
  char *IFName = NULL;
  struct ifaddrs *ifaddr, *ifa;
  bool loopFlag=true;
  int arrayIndex=0;
  int mallocSize =  (MAX_NUMBER_IFS * sizeof(void *));

  if (getifaddrs(&ifaddr) == -1) {
    perror("getIFnames: failed getifaddrs");
    return -1;
  }

    /* Walk through linked list, maintaining head pointer so we
       can free list later .
       Each IF name is placed in a string whose ptr is added
       to the arrayOfPtrs;
    */
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
      char protocol[IFNAMSIZ]  = {0};

      if (ifa->ifa_addr == NULL ||
          ifa->ifa_addr->sa_family != AF_PACKET) continue;

#ifdef TRACEME 
      printf("cmp: %s and %s \n", ifa->ifa_name, "lo");
#endif
      if ( strcmp(ifa->ifa_name, "lo") == 0)
        continue;

#ifdef TRACEME 
      if (isWireless(ifa->ifa_name, protocol)) {
        printf("interface %s is wireless: %s\n", ifa->ifa_name, protocol);
      }  else {
        printf("interface %s is not wireless\n", ifa->ifa_name);
      }
#endif 
      IFName = malloc (IFNAMSIZ*sizeof(char));
      if (IFName == NULL){
        perror("malloc IFName ");
        return -1;
      }
      strncpy(IFName, ifa->ifa_name, IFNAMSIZ);
      arrayOfIFNames[arrayIndex++] = IFName;
    }
  freeifaddrs(ifaddr);
  return arrayIndex;
}


/***********************************************************
* Function: int getIFAddr(char *IFNampePtr, struct in_addr addrPtr)
*       
* Explanation: The caller passes in a valid IF name.
*              The routine finds the IF info and
*              fills in the caller's in_addr.
*
* inputs:   
*       char *IFNampePtr
*        struct in_addr addrPtr)
*
* outputs: returns a ERROR OR a number >=0 representing
*          the number of IF's that are 'up'  
*          This routine mallocs the memory for the array
*          The ownership of the memory transfers to the caller. 
*          when the callers ptr is set to point to the arrayOfPtrs
*         
*
***************************************************/
int getIFAddr(char *IFNamePtr, struct sockaddr  *sockAddrPtr)
{
  int rc = ERROR;
  struct ifaddrs *ifaddr=NULL;
  struct ifaddrs  *ifa = NULL;
  char protocol[IFNAMSIZ]  = {0};

  if ((sockAddrPtr == NULL) || (getifaddrs(&ifaddr) == -1)) {
    perror("getIFAddr: bad sockAddrPtr or  failed getifaddrs");
    return ERROR;
  }

   /* Walk through linked list, maintaining head pointer so we
       can free list later .
       Each IF name is placed in a string whose ptr is added
       to the arrayOfPtrs;
   */
   rc = ERROR;
  for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) 
  {

      if (ifa->ifa_addr == NULL ||
          ifa->ifa_addr->sa_family != AF_PACKET) continue;

#ifdef TRACEME 
      printf("cmp: %s and %s \n", ifa->ifa_name, "lo");
#endif
      if ( strcmp(ifa->ifa_name, IFNamePtr) == 0) {
        rc = NOERROR;
        *sockAddrPtr = *ifa->ifa_addr;
        break;
      }
  }

  freeifaddrs(ifaddr);

  return rc;
}



/***********************************************************
* Function: int getIFInfo(char *arrayOfIFInfoStructs[]) 
*       
* Explanation: The caller passes in an array that
*              is to be filled in by this routine
*              with the names of all network interfaces
*              that are 'up' and NOT lo (local IF).
*              The number of IF names inserted in the
*              array is returned to the caller.
*
* inputs:   
*      char *arrayIFNames[] : array of ptrs to IF name strings 
*        Note: we could pass this as char **arrayIFNames
*
* outputs: returns a ERROR OR a number >=0 representing
*          the number of IF's that are 'up'  
*          This routine mallocs the memory for the array
*          The ownership of the memory transfers to the caller. 
*          when the callers ptr is set to point to the arrayOfPtrs
*         
*
***************************************************/
int getIFInfo(struct IFInfoStruct  *arrayOfIFInfo[]) 
{
  char *IFName = NULL;
  struct ifaddrs *ifaddr, *ifa;
  bool loopFlag=true;
  int arrayIndex=0;
  struct IFInfoStruct *myIFInfoStruct = NULL;
  char protocol[IFNAMSIZ]  = {0};

  if (getifaddrs(&ifaddr) == -1) {
    perror("getIFnames: failed getifaddrs");
    return -1;
  }

    /* Walk through linked list, maintaining head pointer so we
       can free list later .
       Each IF name is placed in a string whose ptr is added
       to the arrayOfPtrs;
    */
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
      if (ifa->ifa_addr == NULL ||
          ifa->ifa_addr->sa_family != AF_PACKET) continue;

#ifdef TRACEME 
      printf("cmp: %s and %s \n", ifa->ifa_name, "lo");
#endif
      if ( strcmp(ifa->ifa_name, "lo") == 0)
        continue;

      myIFInfoStruct  = malloc (sizeof (struct IFInfoStruct));
      if (myIFInfoStruct == NULL){
        perror("malloc myIFInfo failed ");
        return -1;
      }
      IFName = malloc (IFNAMSIZ*sizeof(char));
      if (IFName == NULL){
        perror("malloc IFName ");
        return -1;
      }
      strncpy(IFName, ifa->ifa_name, IFNAMSIZ);
      arrayOfIFInfo[arrayIndex++] = myIFInfoStruct; 
      if (isWireless(ifa->ifa_name, protocol)) {
        myIFInfoStruct->isWireless = true;
      }
      if ( (ifa->ifa_flags && IFF_BROADCAST) ) {
        myIFInfoStruct->isBroadcastAddr = true;
      } else
        myIFInfoStruct->isBroadcastAddr = false;
    }
  freeifaddrs(ifaddr);
  return arrayIndex;
}




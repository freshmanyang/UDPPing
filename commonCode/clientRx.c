/***********************************************************
* Function: int clientRx(int readfd, int writefd, int transferSize)
*
* Explanation:  This routine is passed a read and write descriptor.
*               It returns an rc of success or failure
*               It sends the transferSize to the server and then receives 
*              data from the server until the server completes.
*               
*
* inputs:   read and write file descriptors 
*     int readfd :  the descriptor for input
*     int writefd : the descriptor for output
*     uint32_t transferSize
*    int chunkSize
*
* outputs: returns a EXIT_SUCCESS or EXIT_FAILURE
*
* notes: 
*
***********************************************************/
#include "common.h"
#include "utils.h"

#define TRACEME 1

/***********************************************************
* Function: int clientRx(int readfd, int writefd, unsigned int transferSize, int chunkSize)
*
* Explanation:  This must be called by the main program
*               to set the Version. 
* inputs:   
*   int readfd
*   int writefd 
*   uint32_t transferSize
*   int chunkSize
*
* outputs:
*      returns EXIT_FAILURE or EXIT_SUCCESS
*
* notes: 
*
**************************************************/
int clientRx(int readfd, int writefd, unsigned int transferSize, int chunkSize)
{
 size_t	len =0;
 int rc = EXIT_SUCCESS;
 unsigned int readCount=0;
 char transferSizeText[128];
 char *transferSizePtr = transferSizeText;

  sprintf(transferSizePtr,"%d",transferSize);
  len = strlen(transferSizePtr);		
  
#ifdef TRACEME
  printf("clientRx(%d): sending a transferSize of %d bytes (string:%s, len:%d)  to server  \n",
           (int) getpid(),transferSize, transferSizePtr, (int) len);
#endif

  rc = write(writefd, transferSizePtr, len);
  if (rc < 0) {
    printf("clientRx: write error over pipe rc:%d  errno:%d\n" ,rc, errno);
    rc = EXIT_FAILURE;
  } else {
#ifdef TRACEME
    printf("clientRx: successfully sent the string %s to server, rc :%d\n",transferSizePtr,rc);
#endif
  }


  //Read the desired amount of data....
  rc =  RxData(transferSize, readfd, chunkSize);
  if (rc < 0) {
    printf("clientRx:  ERROR from RxData ?? %d \n" ,rc);
    rc = EXIT_FAILURE;
  } else {
    readCount  = rc;
    printf("clientRx: RxData returns %d bytes read \n",rc);
  }

  if (rc == EXIT_FAILURE) 
    printf("clientRx: exit loop in ERROR , readCount:%d, errno:%d \n",readCount, errno);
  else { 
    if (readCount != transferSize) {
      rc = EXIT_FAILURE;
      printf("clientRx: exit with ERROR: PARIAL RX ??  readCount:%d transferSize:%d errno:%d \n", 
           readCount, transferSize, errno);
    } else {
      rc = EXIT_SUCCESS;
      printf("clientRx: exit loop SUCCESSFULLY send transferSize:%d readCount:%d, errno:%d \n", 
           transferSize, readCount, errno);
    }
  }

  return rc;
}

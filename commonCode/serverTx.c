/***********************************************************
* Function: int serverTx(int readfd, int writefd, int chunkSize)
*
* Explanation:  This routine is passed a read and write descriptor.
*               It is also passed the amount of data to send.
*               It sends the requested amount (or less), returning the amount sent to the caller.
*
* inputs:   read and write file descriptors 
*     int readfd
*     int writefd :
*
* outputs: returns a EXIT_FAILURE
*               or number bytes transferred
* notes: 
*
***********************************************************/
#include "common.h"

#include "utils.h"

#define TRACEME 1 

/***********************************************************
* Function: int serverTx(int readfd, int writefd, int chunkSize)
*
* Explanation:  This must be called by the main program
*               to set the Version. 
* inputs:   
*   int readfd
*   int writefd 
*   int chunkSize
*
* outputs:
*      returns EXIT_FAILURE or EXIT_SUCCESS
*
* notes: 
*
**************************************************/
int serverTx(int readfd, int writefd, int chunkSize)
{
int fd;
ssize_t	n =0;;
char	buff[MAXLINE+1];
int rc = EXIT_SUCCESS;
char *inputPtr = (char *)buff;
unsigned int readCount=0;
unsigned int writeCount=0;
int transferSize = 0;

#ifdef TRACEME
  printf("serverTx(%d): ) Entered....chunkSize : %d   \n",
           (int) getpid(),chunkSize);
#endif

  /* read string from IPC channel, assume it is the transfer size */
  n = 0;
  rc =  read(readfd, buff,MAXLINE);
  if (rc == 0) {
    printf("serverTx: read  end-of-file while reading pathname...which is a HARD ERROR, errno:%d ", errno);
    rc = EXIT_FAILURE;
  } else if (rc< 0) {
    printf("serverTx: read error,  HARD ERROR - errno: %d  ", errno);
    rc = EXIT_FAILURE;
  } else {
    n = rc;
    transferSize=atoi(buff);
#ifdef TRACEME1
    printf("serverTx: succeeded to read %d bytes (string:%s),  transferSize: %d  ",(int)n,buff, transferSize);
    printf("serverTx: buf 0 - 7 : %c %c %c %c %c %c %c  \n " , 
          buff[0], buff[1], buff[2], buff[3], buff[4], buff[5], buff[6]);
#endif
    rc = EXIT_SUCCESS;
  }
  if (rc == EXIT_SUCCESS) {
#ifdef TRACEME
    printf("serverTx: read %d bytes ,  transferSize: %d \n",(int)n, transferSize);
#endif
  } else {
    printf("serverTx: read %d bytes ,  HARD ERROR   transferSize:%d,   errno:%d \n",
            (int)n, transferSize,errno);
    return EXIT_FAILURE;
  }

//  rc =  sendData(4, writefd,chunkSize);
//  printf("serverTx: sent initial 4 bytes  rc : %d  \n", rc);
  rc =  sendData(transferSize, writefd,chunkSize);
  printf("serverTx: exiting, rc : %d  \n", rc);

  return rc;
}





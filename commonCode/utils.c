/*********************************************************
* Module Name:  Tools common routines 
*
* File Name:    utils.c
*
* Last update: 10/5/2019
*
* Notes:
*
*   -to print an unsigned long long and uint64_t : printf(%llu, %"PRIu64",ulonglongX,uint64X);
*   -to print an unsigned long and uint32_t : printf(%lu, %"PRIu32",ulongX,uint64X);
*         This is a gcc macro...depending on the system, these could be the same.
*   -issue 'file testProgram' to determine if it was compiled for 32 or 64 bit mode
*   - to compile a program in 32 bit mode (if you are on a 64 bit machine), add compile option -m32
*        -Might get  a compile error: sys/cdefs.h: No such file or directory
*        -Solutions posted on the Internet: 
*            install libc6-dev-i386. 
*            best answer might be  to install gcc-multilib and g++-multilib
*                           -this gives 32 and 64 bit builders
* Check: 
*   -eliminate  ntohd(double network) - they are  not correct ....???
*   -question:  diff between X86 and AMD64
*
*********************************************************/
#include "common.h"
#include "utils.h"

//char Version[] = "8.0";   
char Version[MAX_LINE_SIZE] = "0.0";

#define TRACE_ERRORS 0 
//#define TRACEME 0 

#define CHUNKSIZE 1000000

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/***********************************************************
* Function: int getSubStringIndex(char *stringPtr, char *termString)
*
* Explanation: find the first occurance of terminator
*      and replaces it with string terminator.
*      Useful to replace a \n with a \0 to create a string
*
* Remember in C:   a "" is a string, a  '' is a character
*
* inputs:  
*      char *stringPtr:  callers string
*      char *termString :  character that is used to delimit the substring.
*
* outputs: returns ERROR or the size of the new string (not including the new string string TERM)
*
************************************************/
int getSubStringIndex(char *stringPtr, char *termString)
{
  int rc = NOERROR;
  int index=0;
  int sizeString=0;
  int tmpSize=0;

  sizeString=strlen(stringPtr);
  tmpSize=strlen(termString);

#ifdef TRACEME
  printf("getSubStringIndex: sizeString:%d sizeTermString:%d string: %s, termString :%s\n", 
         sizeString,tmpSize, stringPtr, termString);
#endif

  //strcspn: string lib call that finds the lenght of the substring before the termString
  //Therefore, the return can be used to as the index to add a string term
  index = strcspn(stringPtr,termString);
#ifdef TRACEME
  printf("getSubStringIndex: index:%d in string: %s, termString :%s\n", index, stringPtr, termString);
#endif
  if (index >= 0) {
    stringPtr[index] = '\0';
  } else 
    rc = ERROR;

  if (rc != ERROR) {
    rc = index;
  } 

#ifdef TRACEME
  if (rc != ERROR) 
    printf("getSubStringIndex: index:%d subString:%s\n", index, stringPtr);
  else {
    printf("getSubStringIndex: Exit in ERROR ??? index:%d \n", index);
  }
#endif

  return rc;
}

/***********************************************************
* Function: void setVersion(double versionLevel) {
*
* Explanation:  This must be called by the main program
*               to set the Version. 
* inputs:   The version number 
*
* outputs:
*
* notes: 
*
**************************************************/
void setVersion(double versionLevel) {

char *versionPtr = Version;
  sprintf(versionPtr,"Version %1.2f",versionLevel);
}

/***********************************************************
* Function: char *getVersion() 
*
* Explanation: Returns a ptr to the version string.
* inputs:    
*
* outputs: Returns a ptr to the global string
*
* notes: 
*
**************************************************/
char *getVersion() {
   return (Version);
}

/***********************************************************
*  function: int processErrnoArray(FILE *outputFID, int *arrayPtr,  int size)
*
* Explanation: dumps to the output param the contents of the int array
*          which contains all errors that were recorded.
*
* inputs:    
*   FILE *outputFID: file desc 
*   int *arrayPtr : ptr to array of ints (holds errno's) 
*   int size : number of errors that occured
*
* outputs: Returns SUCCESS or ERROR
*
* notes: 
*
**************************************************/
int processErrnoArray(FILE *outputFID, int *arrayPtr,  int size)
{
  int rc = SUCCESS;
  int i=0;

  if (size == 0)
    return rc;

  if (arrayPtr != NULL) {
    rc = ERROR;
    fprintf(stderr,"processErrnoArray: ERROR base size (%d) or arrayPtr\n",
           size);
  }
  else { 
    if (outputFID != NULL) 
      fprintf(outputFID,"\nEach %d errno values: ",size);
    else
      printf("\nEach %d errno values: ",size);

    for (i=0; i<size; i++) {
      if (outputFID != NULL) 
        fprintf(outputFID,"%d ",arrayPtr[i]);
      else 
        printf("%d ",arrayPtr[i]);
    }
  }
  return rc;
}


void DieWithUserMessage(const char *msg, const char *detail) {
  fputs(msg, stderr);
  fputs(": ", stderr);
  fputs(detail, stderr);
  fputc('\n', stderr);
  exit(1);
}

void DieWithSystemMessage(const char *msg) {
  perror(msg);
  exit(1);
}

void DieWithError(char *errorMessage)
{
    perror(errorMessage);
    exit(EXIT_FAILURE);
}

void die(const char *msg) {
  if (errno == 0) {
    /* Just the specified message--no error code */
    puts(msg);
  } else {
    /* Message WITH error code/name */
    perror(msg);
  }
  printf("Die message: %s \n", msg);
  
  /* DIE */
  exit(EXIT_FAILURE);
}

void setUnblockOption(int sock, char unblock) 
{
  int opts = fcntl(sock, F_GETFL);
  if (unblock == 1)
    opts |= O_NONBLOCK;
  else
    opts &= ~O_NONBLOCK;
  fcntl(sock, F_SETFL, opts);
}

void sockBlockingOn(int sock) 
{ 
  setUnblockOption(sock, 0); 
}

void sockBlockingOff(int sock) 
{ 
   setUnblockOption(sock, 1); 
}

void setBlockingOn(int fd) 
{ 
  setUnblockOption(fd, 0); 
}

void setBlockingOff(int fd) 
{ 
  setUnblockOption(fd, 1); 
}


/*************************************************************
*
* Function: void swapbytes(void *_object, size_t size)
* 
* Summary: In-place swapping of bytes to match endianness of hardware
*
* Inputs:
*   *object : memory to swap in-place
*   size   : length in bytes
*           
*
* outputs:  
*     updates caller's object data
*
* notes: 
*    
*
***************************************************************/
void swapbytes(void *_object, size_t size)
{
  unsigned char *start, *end;
  if(!is_bigendian())
  {
    for ( start = (unsigned char *)_object, end = start + size - 1; start < end; ++start, --end )
    {
      unsigned char swap = *start;
      *start = *end;
      *end = swap;
    }
   }
}

/*************************************************************
*
* Function: uint64_t htonll (uint64_t host) 
* 
* Summary:  equivalent to htonl but operates on uint64_t 
*
* Inputs:
*   uint64_t host :  callers 64 bit data in host byte format
*           
*
* outputs:  
*   returns the host param in  network byte (Big Endian) format
*
*    
***************************************************************/
uint64_t htonll (uint64_t host) 
{
  uint64_t rvalue = host;
  if(!is_bigendian())
  {
    rvalue = htobe64(rvalue);
  }
  return rvalue;
}

/*************************************************************
*
* Function: uint64_t ntohll (uint64_t network) 
* 
* Summary:  equivalent to ntohl but operates on uint64_t 
*
* Inputs:
*   uint64_t network:   callers 64 bit data in network by format
*           
*
* outputs:  
*   returns the value of network in  network byte (Big Endian) format
*
*    
***************************************************************/
uint64_t ntohll (uint64_t network) 
{
  uint64_t rvalue = network;
  if(!is_bigendian())
  {
    rvalue = be64toh(rvalue);
  }
  return rvalue;
}

#if 0

/*************************************************************
*
* Function: uint64_t myhtonll(uint64_t host)
* 
*  Converts uint64_t host from host order to network byte order
*
* Inputs:
*   uint64_t host :  callers 8 bytes unsigned int in host byte format
*           
*
* outputs:  
*   returns host in network byte (Big Endian) format
*
* notes: Returns all 1's on error
*    
***************************************************************/
uint64_t htonll(uint64_t host)
{
  uint64_t  network;          //Holds network byte order representation
  uint64_t host1 = host;
  int sizeToSwap = sizeof(uint64_t);
  char *n = (char *)&network + sizeToSwap-1;
  char *h = (char *)&host1;
  unsigned int x = 1;
  char *y = (char *)&x;
  char z = 1;
  int i;

  if(is_bigendian())
  {
    return host;
  } else {

    for(i=0;i<sizeToSwap;i++)
    {
      *n = *h;
      n--;
      h++;
    }
  }

  return network;
}

/*************************************************************
*
* Function: uint64_t ntohll(uint64_t network)
* 
*  Converts 8 bytes from network byte order to host byte order
*
* Inputs:
*  double  network:  callers  number to be swapped
*           
*
* outputs:  
*   returns the number in the correct order 
*
*    
***************************************************************/
uint64_t ntohll(uint64_t network)
{
  uint64_t host;             //Holds host byte order representation
  int sizeToSwap = sizeof(uint64_t);
  char *n = (char *)&network + sizeToSwap-1;
  char *h = (char *)&host;
  unsigned int x = 1;
  char *y = (char *)&x;
  char z = 1;
  int i;

  if(!is_bigendian())
  {
    return network;
  }

  for(i=0;i<sizeToSwap;i++)
  {
    *h = *n;
    n--;
    h++;
  }
  return host;
}

#endif

void err_sys(const char *fmt, ...)
{
  exit(EXIT_FAILURE);
}


void sig_chld(int signo)
{
  pid_t	pid = 0; //Will hold the child's pid that was processed by waitpid
  pid_t	mypid = getpid();
  pid_t	ppid = getppid();
  int stat = 0;
  int loopCounter=0;

  printf("sig_chld:(%d:%d): Entered with param:%d \n", mypid,ppid,signo);

  //This while loop ensures we issue a waitpid for ALL child processes
  while ( (pid = waitpid(-1, &stat, WNOHANG)) > 0) {
    loopCounter++;
  }
  printf("sigchld:(%d:%d): Exiting: loopCounter:%d  last childpid:%d  stat:%d \n", 
        mypid, ppid, loopCounter, pid,  stat);

  return;
}

/*************************************************************
*
* Function: int is_bigendian() 
*
* Inputs:
*
* outputs:  
*         returns 1 if true  big endian else 0 
*
***************************************************************/
bool is_bigendian() 
{
  bool rc = true;
  const uint32_t bsti = 1;  // Byte swap test integer

  if ( (*(char*)&bsti) == 0 ) 
    rc = false;
  else 
    rc = true;

  return rc;
}


/****************************************
* routine: int myGetLine(char *stringPtr, int maxStringSize) 
*
* explanation: This is returns the next line from standard in
*              It is different from the standard lib's getline()
*
* inputs:
*     char *stringPtr : caller's reference to a buffer
*     int maxStringSize : max number of bytes in the string (not counting the NULL)
*
* outputs: returns the number of characters placed in the string NOT including the final NULL.
*          On error, an ERROR is returned
*
* Details/notes:
*    
*    The routine is different from getLine.  
*    We use fgets.  The man :
*         char *fgets(char *s, int size, FILE *stream);
*         fgets() returns s on success, and NULL on error or when end of file occurs while no characters have been read.
*         fgets includes the newline.  It adds a NULL character into the buffer.
*         The caller's size must be large enough to support the largest line of characters, possibly including 
*              a newline.  And also to allow the NULL terminator to be added.
*
****************************************/
int myGetLine(char *stringPtr, int maxStringSize) 
{
   int rc = ERROR;
   char *tmpRC = NULL;
   bool loopFlag = true;

   //Loop until we get a line at least of size 1 
   while (loopFlag == true) {

     tmpRC = fgets(stringPtr, maxStringSize, stdin);
     if (tmpRC  == NULL ) {
        rc = ERROR;
#ifdef TRACE_ERRORS
        fprintf (stderr,"myGetLine: WARNING: got a NULL from fgets\n");
#endif
        //Don't break...just loop to try again
     } else { 
        // man: size_t strlen(const char *s);
        //     The strlen() function returns the number of bytes in the string s.
        //     This does NOT include the NULL terminating character.
        rc = (int) strlen((const char *) stringPtr);
        // >0 implies success.  
        if (rc > 0) {
            loopFlag = false;
            break;
        }
     }
   }

#ifdef TRACE
   if (rc == ERROR )
      printf ("getLine: ERROR:  ???  \n");
   else {
      printf ("getLine: SUCCEEDED to acquire a string of size %i  \n", rc);
      printf ("getLine:%s", stringPtr);
   }
#endif

   return rc;
}

/****************************************
* routine:  char getChar(void) 
*
* explanation: This is returns the next character or CHAR_ERROR
*              if either EOF or an error occurs.
*
* inputs:
*
* outputs: returns a character or CHAR_ERROR that is either
*          a number, lower or upper case letter OR
*          an error octet of value 0xFF  
*
* Details/notes:
*    
*
****************************************/
char getChar(void) 
{

char myChar = CHAR_ERROR; 
bool loopFlag = true;
int rc = ERROR;

//   printf ("getChar:  Enter a character ");
   while ( loopFlag == true)  {
      rc = scanf ("%c", &myChar);
#ifdef TRACE
      printf ("getChar: scanf rc: %i, myChar:%c (hex:%hhx) \n", rc,myChar, myChar);
#endif
      if ( rc > 0 ) {
         rc = NOERROR;
         break;
      } else { 
#ifdef TRACE_ERRORS
         fprintf (stderr,"getChar: ERROR?  rc:%i,  myChar in hex: 0X%hhx) \n", rc, myChar);
#endif
         rc = ERROR; 
         break;
      }
   }

   if (rc == ERROR) { 
      myChar = CHAR_ERROR;
   } 
#ifdef TRACE
   printf ("getChar:  rc:%i, return char:%c (%hhx) \n", rc,myChar,myChar);
#endif
   return myChar;
}



/****************************************
* routine:  char getAlphaNumeric(void) 
*
* explanation: This is returns a single character- 
*     either a letter or a digit  OR a -1
*
* inputs:
*
* outputs: returns a character that is either
*          a number, lower or upper case letter OR
*          an error octet of value 0xFF  
*
* Details/notes:
*    
*
****************************************/
char getAlphaNumeric (void) 
{

unsigned char myChar = ' '; 
bool loopFlag = true;
int rc = NOERROR;
int loopCounter=0;

//   printf ("getAlphaNumeric:  Enter a number or letter  \n");
   while ( loopFlag == true)  {
      loopCounter++;
      myChar = getChar();
#ifdef TRACE
      printf ("getAlphaNumeric(%i):  myChar:%c 0X%hhx) \n",loopCounter, myChar, myChar);
#endif

      if (myChar == CHAR_ERROR) 
       rc = ERROR;
      else{
        rc = NOERROR;
      }

      if ( rc == NOERROR ) {
         //Process the next character....break if it's valid
         if ( (myChar >= 'a'  &&  myChar <= 'z') || (myChar >= 'A'  &&  myChar <= 'Z') ) {
            rc = NOERROR;
            break;
         } else if  ( myChar >= '0'  &&  myChar <= '9' ) {
            rc = NOERROR;
            break;
         } else {
           ;
#ifdef TRACE
            printf ("It's a special character, ignore  (myChar in hex: 0X%hhx) \n", myChar);
#endif
         }


      } else {
         ;
#ifdef TRACE_ERRORS 
         fprintf(stderr, "getAlphaNumeric: ERROR: rc = %i \n", rc);
#endif
         break;
      }
   }

   if (rc != NOERROR) { 
      myChar = CHAR_ERROR;
   }
#ifdef TRACE
   if (rc != NOERROR)  
     printf ("getAlphaNumeric(%i): Found valid character !! : %c \n",loopCounter, myChar);
   else 
     printf ("getAlphaNumeric(%i): DID NOT FIND valid character  : rc:%i, myChar:%c (hex:%hhx)  \n",loopCounter,rc,myChar, myChar);
#endif
   return myChar;
}

/****************************************
* routine:  char getChar(void) 
*
* explanation: This is returns the next character or CHAR_ERROR
*              if either EOF or an error occurs.
*
* inputs:
*
* outputs: returns a character or CHAR_ERROR that is either
*          a number, lower or upper case letter OR
*          an error octet of value 0xFF  
*
* Details/notes:
*    
*
****************************************/
int getNumber(void) 
{

int myNumber = ERROR;

bool loopFlag = true;
int rc = ERROR;

   while ( loopFlag == true)  {
      rc = scanf ("%i", &myNumber);
      if ( rc > 0 ) {
         rc = NOERROR;
         break;
      } else { 
         rc = ERROR; 
         break;
      }
   }

   if (rc == ERROR) { 
      myNumber = ERROR;
   } 

#ifdef TRACE
   printf ("getNumber: scanf rc: %i, myNumber:%i \n", rc,myNumber);
#endif

   return myNumber;

}


/********************************************************
* routine:  unsigned int ipflow_hash(struct in_addr dst, 
*           struct in_addr src, unsigned short sport, 
*           unsigned short dport,unsigned char ip_p)
*
* explanation:  Modified version of hash function found in NetBSD code 
*
* inputs: the complete flow info:
*   struct in_addr dst 
*   struct in_addr src
*   unsigned short sport 
*   unsigned short dport
*   unsigned char ip_p
*
* outputs:  returns an unsigned int hash
*
* Details/notes:
*    
*
****************************************/
unsigned int ipflow_hash(struct in_addr dst, struct in_addr src, unsigned short sport, unsigned short dport,unsigned char ip_p)
{
  unsigned int hash = ip_p;
  int idx;
  for (idx = 0; idx < 32; idx += IPFLOW_HASHBITS)
    hash += (dst.s_addr >> (32 - idx)) + (src.s_addr >> idx) + (sport >> idx) + (dport >> idx);
  return hash & (IPFLOW_HASHSIZE-1);
}


/****************************************
* routine:  double myDoubleAvg(double *arrayPtr, uint32_t numberSamples, FILE *outputFID)
*
* explanation: This computes the avg of the array of doubles
*              It otutputFID is valid, the array contents are placed in the file.
*
* inputs:
*      double *arrayPtr:ptr to the array of doubles
*      uint32_t numberSamples: number of samples in the array
*      FILE *outputFID :  file descriptor. If not NULL, array dumped to file.
*
* outputs: returns the avg of the array. A DOUBLE_ERROR is returned on ERROR.
*
* Details/notes:
*    
****************************************************/
double myDoubleAvg(double *arrayPtr, uint32_t numberSamples, FILE *outputFID)
{
  double myAvg = 0;
  double mySum = 0;
  int i=0;


  if (arrayPtr == NULL)
    return DOUBLE_ERROR;

  for (i=0;i<numberSamples;i++) 
  {
    mySum +=  arrayPtr[i];
  }

  if (numberSamples > 0)
  {
    myAvg = mySum/numberSamples;
    if (outputFID !=NULL) {
      for (i=0; i<numberSamples; i++) {
        fprintf(outputFID,"%2.12f \n",arrayPtr[i]);
      }
    }
  }
  return myAvg;
}

uint64_t myLLavg(uint64_t  *arrayPtr, uint32_t numberSamples) 
{
  uint64_t myAvg = 0;
  uint64_t mySum = 0;
  int i;

  for (i=0;i<numberSamples;i++) 
  {
    mySum += (uint64_t) arrayPtr[i];
  }

  if (numberSamples > 0)
    myAvg = mySum/numberSamples;

  return myAvg;
}


/***********************************************************
* Function: int lockFile(FILE *lockFD)
*
* Explanation: locks the file identified by the stream param 
*
* inputs:   
*   FILE *lockFD: stream ptr
*
* outputs: returns an  ERROR or NOERROR
*
**************************************************/
int lockFile(FILE *lockFD)
{
  int rc = NOERROR;

  rc= flock(fileno(lockFD), LOCK_EX);

  return rc;
}

/***********************************************************
* Function: int unLockFile(FILE *lockFD)
*
* Explanation: unlocks the file identified by the stream param 
*
* inputs:   
*   FILE *lockFD: stream ptr
*
* outputs: returns an  ERROR or NOERROR
*
**************************************************/
int unLockFile(FILE *lockFD)
{
  int rc = NOERROR;

  rc= flock(fileno(lockFD), LOCK_UN);

  return rc;
}

/***********************************************************
* Function: int writeLine(double curTime,char *dataPtr, FILE *outFD, uint32_t sizeData, int mode)
*
* Explanation:writes the line of data to the file. 
*
* inputs:   
*   double curTime: time when data was obtained. set to -1 if it is not to be written.
*   char *dataPtr:  text data to write to file
*   FILE *outFD : target file descriptor
*   uint32_t sizeData: number of bytes of data
*   mode:  0:assume character string; 1:assume datablock
*
* outputs: returns an  ERROR or NOERROR
*
**************************************************/
int writeLine(double curTime,char *dataPtr, FILE *outFD, uint32_t sizeData, int mode)
{
  int rc = NOERROR;

  if (outFD == NULL){
     printf("writeLine(%f): ERROR: Bad output File handle \n",curTime);
     rc = ERROR;
  }
  else {
    if (mode == 0) {
      if (curTime == -1)
        fprintf(outFD,"%s\n", dataPtr);
      else 
        fprintf(outFD,"%12.9f %s\n", curTime, dataPtr);
    } else if (mode == 1) {
      if (curTime == -1)
        rc = fwrite(dataPtr, 1, sizeData, outFD);
      else {
        fprintf(outFD,"%12.9f\n", curTime);
        rc = fwrite(dataPtr, 1, sizeData, outFD);
      }
      if (rc == ERROR) {
        printf("writeLine: HARD ERROR on write ?? sizeData:%d \n",sizeData);
        perror("writeLine: HARD ERROR on write ??");
      } else
        rc = NOERROR;
    }
  }
  return rc;
}

/***********************************************************
* Function: int writeFile(double curTime, void *dataPtr,char *outputFileName,uint32_t sizeData, int mode)
*
* Explanation:writes a summary of gps data to the output file.
*      The file will need to be opened, flocked, written, unlocked, closed.
*
* inputs:   
*
* outputs: returns an  ERROR or NOERROR
*
**************************************************/
int writeFile(double curTime, void *dataPtr,char *outputFileName,uint32_t sizeData, int mode)
{
  int rc = NOERROR;
  FILE *fp = NULL;

#ifdef TRACEME
    printf("writeFile: opening file %s, size:%d, mode:%d \n",outputFileName,sizeData,mode);
#endif 

  fp = fopen(outputFileName,"w");

  if (fp==NULL) {
    printf("writeFile: HARD ERROR: Failed to open file %s \n",outputFileName);
    perror("writeFile: HARD ERROR: Failed to open file");
    rc = ERROR;
  } else 
  {

#ifdef TRACEME
    printf("writeFile: opening LOCKED file %s \n",outputFileName);
#endif 
    rc= flock(fileno(fp), LOCK_EX);
    rc = writeLine(curTime,dataPtr, fp, sizeData, mode);
    rc= flock(fileno(fp), LOCK_UN);
  }

  if (fp != NULL)
    fclose(fp);

  return rc;
}




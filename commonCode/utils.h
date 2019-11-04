/************************************************************************
* File:  utils.h
*
* Purpose:
*   This include file is for a set of utility functions 
*
* Notes:
*
* Last update: 8/4/2019
*
************************************************************************/
#ifndef	__utils_h
#define	__utils_h

#include "common.h"
#include <sys/wait.h>

int processErrnoArray( FILE *outputFD, int *arrayPtr,  int size);

double myDoubleAvg(double *arrayPtr, uint32_t numberSamples,FILE *outputFID);
uint64_t myLLavg(uint64_t  *arrayPtr, uint32_t numberSamples); 

//Replace stringPtr with substring that is stringPtr UNITL first termString found
int getSubStringIndex(char *stringPtr, char *termString);

void setVersion(double versionLevel);
char *getVersion();

void err_sys(const char *fmt, ...);
void sig_chld(int signo);

//void die(const char *msg);
void DieWithError(char *errorMessage); 
void DieWithUserMessage(const char *msg, const char *detail);
void DieWithSystemMessage(const char *msg);

void swapbytes(void *_object, size_t size);
bool is_bigendian();

#ifdef LINUX
uint64_t htonll(uint64_t host) ;
uint64_t ntohll(uint64_t network) ;
#endif
#ifdef DARWIN
//uint64_t htonll(uint64_t host) ;
//uint64_t ntohll(uint64_t network) ;
#endif 

#ifdef BSD
uint64_t htonll(uint64_t host) ;
uint64_t ntohll(uint64_t network) ;
#endif 

void setUnblockOption(int sock, char unblock);
void sockBlockingOn(int sock);
void sockBlockingOff(int sock); 
void setBlockingOn(int fd); 
void setBlockingOff(int fd); 

char getChar(void);
char getAlphaNumeric (void);

int myGetLine(char *stringPtr, int maxStringSize);
int getNumber(void);

/* should not be a multiple of 8 */
#define	IPFLOW_HASHBITS 20 
#define	IPFLOW_HASHSIZE	(1 << IPFLOW_HASHBITS)

unsigned int ipflow_hash(struct in_addr dst, struct in_addr src, unsigned short sport, unsigned short dport,unsigned char ip_p);

int lockFile(FILE *lockFD);

int unLockFile(FILE *lockFD);

int writeLine(double curTime,char *dataPtr, FILE *outFD, uint32_t sizeData, int mode);

int writeFile(double curTime, void *dataPtr,char *outputFileName,uint32_t maxSize, int mode);

#endif



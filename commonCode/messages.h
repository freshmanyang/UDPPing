/************************************************************************
* File:  messages.h
*
* Purpose:
*   This include file defines the formats for the different messages 
*       in the system.
*   Provides the pack and unpack routines that should be used to
*   when sending/receiving msg's over a network.
*
* Notes: These are the main messages used
*     MessageHeaderDefault   
*     BSMMsg; TS, GPS coords
*     WirelessStatsMsg:
*     NetStatsMsg:
*     SystemStatsMsg:
*     TGIFControlMsg: 
*
* NOTE:
*     GPSMsg:  same as BSMMsg
*  TGIFHeader: no used
*  TGIFHeartbeat:  same as a BMSMsg
*
* Last update:  10/17/2019
*
************************************************************************/
#ifndef	__messages_h
#define	__messages_h

#include "./gpsdHelper.h"

//defines max mesg size - needs to be a bit larger than possible max message
#define MAX_DATA_BUFFER 64128 
//defines min mesg size - we'll assume can be as small as 4 bytes
#define MIN_DATA_BUFFER 4

//The following are the min/max amount of USER data supported in a single sendto/receive (i.e., UDP only!!)
//It does NOT include the overhead data needed / used by our message header (or any TCP/IP/Frame headers)
#define MESSAGEMIN 0
#define MESSAGE_DEFAULT_SIZE 24
//Note: for UDP, this will cause frag. although if using localhost the mtu is usually >60Kbytes
#define MESSAGEMAX 50000

#define MAX_MSG_HDR 128 
#define MsgHdrSize 16
#define DATAMsgHdrSize 20
#define ACKMsgSize 18


/******************************************
* MESSAGE FORMAT:  This specifies the msg header that is used to 
*                  carry any of our MESSAGE TYPES. 
*
*  Tools should call this the msgFormat parameter
********************************************/
//
//This is the original format used in Fall 2016 tool
//maps to struct messageHeaderDefault
//msgFormat = 0
#define MSG_FORMAT_DEFAULT    0
#define MSG_FORMAT_BSM        1
#define MSG_FORMAT_TGIF       2
#define MSG_FORMAT_TGIF_HEARTBEAT    3

//Set of TGIF msgs formats
#define MSG_FORMAT_TGIF_DISCOVERY    4
#define MSG_FORMAT_TGIF_CONTROL      5
#define MSG_FORMAT_TGIF_DATA         6

//Other possibilities
#define MSG_FORMAT_WAVE     16
#define MSG_FORMAT_MQTT     32       
#define MSG_FORMAT_PROTOBUFV3  33


/******************************************
* MESSAGE TYPES:  Specifies the application level formats 
*                 That would be placed in the data portion
*                 of the MSG container
*
*  These define the structure of the 'payload' -
*  This data would follow the message header (whatever type is in use)
*  The message header mode field should be a union so it also servers as the code : type
********************************************/
/*possible structured  msg types */
#define NOMSG   0  
#define DATAMSG 1
#define ACKMSG  2
#define CONTROLMSG  3
#define DISCOVERY_MSG  4
#define DNS_MSG  5
#define ARP_MSG  6
#define HEARTBEAT_MSG  8
#define TGIF_CONTROL_MSG  11
#define TGIF_DATA_MSG  12
#define WAVE_MSG  16

//A BSM_MSG is == MSG_FORMAT_BSM
#define BSM_MSG  17

//possible unstructured msg types   
#define MSG_JSON        34
#define MSG_HTML        34
#define MSG_ASN         34
#define MSG_PROTOBUFV3  33





/******************************************
* Next are the structs that define 
*   each message format 
********************************************/


//Correspnds to msgFormat 0 
typedef struct {
  uint16_t size;
  uint16_t mode;
  uint32_t sequenceNum;
  uint64_t ts_sec;    //client places current timespec tv_sec
  uint64_t ts_nsec;   //client places nsec
                        //If this is to be echoed, the server
                        //places its time sent info
} messageHeaderDefault;


//Attempt to model a standard BSM - TODO:  Check IETF's IP BSM msg format
//#define BSM_MSG  17
//#define MSG_FORMAT_BSM        1
//msgFormat 1   
//Get actual struct from the SDK
// velocity ??
//msgFormat  1 
typedef struct {
  uint8_t  msgType;     //type of msg
  uint8_t code;         //further specifies msg:  type:code
  uint16_t msgHdrsize; //size in bytes of the header
  uint16_t dataSize;
  uint64_t  nodeID;   //node ID  - based on first IF mac addr
  uint32_t sequenceNum;
  uint32_t ts_sec;     //client places current timespec tv_sec
  uint32_t ts_nsec;    //client places nsec
                        //If this is to be echoed, the server
  uint16_t timeSoure;  //0:localGPSD;1:remote GPSD; 2:Chrony;3:NTP;4:localClock
  uint32_t timeError;
  uint32_t latitude;
  uint32_t longitude;
  uint32_t locationAccuracy;
  uint32_t velocity;
  uint32_t accuracy;
  uint32_t distance;
  uint32_t acceleration;
} BSMMsg;


//msgFormat 2
typedef struct {
  uint8_t  msgType;     //type of msg
  uint8_t code;         //further specifies msg:  type:code
  uint16_t msgHdrsize; //size in bytes of the header
  uint16_t dataSize;
  uint64_t  nodeID;   //node ID  - based on first IF mac addr
  uint32_t sequenceNum;
  uint32_t ts_sec;     //client places current timespec tv_sec
  uint32_t ts_nsec;    //client places nsec
                        //If this is to be echoed, the server
  uint16_t timeSoure;  //0:localGPSD;1:remote GPSD; 2:Chrony;3:NTP;4:localClock
  uint32_t timeError;
  uint32_t latitude;
  uint32_t longitude;
  uint32_t elevation;
  uint32_t velocity;
  uint32_t latError;
  uint32_t lonError;

} TGIFMsgHeader;

//msgType 3
typedef struct {
  uint8_t  msgType;     //type of msg
  uint8_t code;         //further specifies msg:  type:code
  uint16_t msgHdrsize; //size in bytes of the header
  uint16_t dataSize;
  uint32_t nodeID;   //node ID - hash based on octet1,2 (XOR) octets 3,4 (XOR) octets 5,6
  uint32_t sequenceNum;
  uint32_t ts_sec;     //client places current timespec tv_sec
  uint32_t ts_nsec;    //client places nsec
  uint16_t timeSource;  //0:localGPSD;1:remote GPSD; 2:Chrony;3:NTP;4:localClock
  uint32_t latitude;   //gps info
  uint32_t longitude;
  uint32_t elevation;
  uint32_t velocity;
  uint32_t latError;
  uint32_t lonError;
  int32_t SignalQuality;
  int32_t RSSI;
} TGIFHeartbeat;

typedef struct{
  uint32_t sequenceNum;
  uint32_t ts_sec;
  uint32_t ts_nsec;
} TGIFACK;


/****************************************
* These might be useful internally as msg's are created 
**************************************/

typedef  struct{
  uint8_t  msgType;     //type of msg
  uint8_t code;         //further specifies msg:  type:code
  uint16_t msgHdrsize; //size in bytes of the header
  uint16_t dataSize;
  uint64_t  nodeID;   //node ID  - based on first IF mac addr
  uint32_t sequenceNum;
  uint32_t ts_sec;     //client places current timespec tv_sec
  uint32_t ts_nsec;    //client places nsec
                        //If this is to be echoed, the server
  uint16_t timeSoure;  //0:localGPSD;1:remote GPSD; 2:Chrony;3:NTP;4:localClock
  uint32_t timeError;
  uint32_t mode;
	double latitude;
	double longitude;
	double altitude;
	double velocity;
	double latError;
	double longError;
	double altError;
} GPSMsg;

typedef struct {
  uint8_t  msgType;     //type of msg
  uint8_t code;         //further specifies msg:  type:code
  uint16_t msgHdrsize; //size in bytes of the header
  uint16_t dataSize;
  uint64_t  nodeID;   //node ID  - based on first IF mac addr
  uint32_t sequenceNum;
  uint32_t ts_sec;     //client places current timespec tv_sec
  uint32_t ts_nsec;    //client places nsec
                        //If this is to be echoed, the server
  uint16_t timeSoure;  //0:localGPSD;1:remote GPSD; 2:Chrony;3:NTP;4:localClock
  uint32_t timeError;
  uint32_t typeDevice; // 802.11, LTE-A,  
  uint32_t deviceMode;  //current mode of device (e.g., 802.11n, ...)
  char  IFName[32];     //interface name
  uint32_t Channel;  // channel id
//The following can be instantaneous or averaged over the timeInterval
//If timeInterval, the RxMeasure are in bps the node must clear stats each interval 
//   And measures like SINR,RSSI are avg over the interval
// if timeInternet is 0, RxMeasure is bytes rx'ed, SINR is instantaneous...
  uint32_t RxMeasure;
  uint32_t TxMeasure;
  uint32_t Power;    // Tx power or received power
  uint32_t TxMCS;      //modulation and coding -averaged or last used
  uint32_t RxMCS;      //modulation and coding -averaged or last used
  uint32_t SINR;      //
  uint32_t RSSI;     //
  uint32_t utilization;  //
  uint32_t channelErrors;  // means diff things depending on the rat
  uint32_t timeIntervalSecs;
  uint32_t timeIntervalNSecs;
  char nextIFName[32];       //name of next Wirieless IF. Empty if last or no others
} WirelessStatsMsg;

//Generic- appropriate for wireless or ethernet
typedef struct {
  uint8_t  msgType;     //type of msg
  uint8_t code;         //further specifies msg:  type:code
  uint16_t msgHdrsize; //size in bytes of the header
  uint16_t dataSize;
  uint64_t  nodeID;   //node ID  - based on first IF mac addr
  uint32_t sequenceNum;
  uint32_t ts_sec;     //client places current timespec tv_sec
  uint32_t ts_nsec;    //client places nsec
                        //If this is to be echoed, the server
  uint16_t timeSoure;  //0:localGPSD;1:remote GPSD; 2:Chrony;3:NTP;4:localClock
  uint32_t timeError;
  uint32_t typeDevice; // 802.11, LTE-A,  
  uint32_t deviceMode;  //current mode of device (OFF, 1Gbps, ....)
  char  IFName[32];     //interface name
  uint32_t RxBytes;
  uint32_t TxBytes;
  uint32_t timeIntervalSecs;
  uint32_t timeIntervalNSecs;
  uint32_t signalLevel;     //means diff things depending on typeDevice
  char nextIFName[32];       //name of next IF. Empty if last or no others
} NetStatsMsg;


typedef struct {
  uint8_t  msgType;     //type of msg
  uint8_t code;         //further specifies msg:  type:code
  uint16_t msgHdrsize; //size in bytes of the header
  uint16_t dataSize;
  uint64_t  nodeID;   //node ID  - based on first IF mac addr
  uint32_t sequenceNum;
  uint32_t ts_sec;     //client places current timespec tv_sec
  uint32_t ts_nsec;    //client places nsec
                        //If this is to be echoed, the server
  uint16_t timeSoure;  //0:localGPSD;1:remote GPSD; 2:Chrony;3:NTP;4:localClock
  uint32_t timeError;
  uint32_t load1;      //1 minute load avg
  uint32_t load2;      //5 minute load avg
  uint32_t load3;      //15 minute load avg
  uint32_t memAvail;    //Amount of mem availabl minute load avg
  uint32_t memFree;    //Amount of mem free
  uint32_t memUsed;    //Amount of mem used
} SystemStatsMsg;

typedef struct {
  uint8_t  msgType;     //type of msg
  uint8_t code;         //further specifies msg:  type:code
  uint16_t msgHdrsize; //size in bytes of the header
  uint16_t dataSize;
  uint64_t  nodeID;   //node ID  - based on first IF mac addr
  uint32_t sequenceNum;
  uint32_t ts_sec;     //client places current timespec tv_sec
  uint32_t ts_nsec;    //client places nsec
                        //If this is to be echoed, the server
  uint16_t timeSoure;  //0:localGPSD;1:remote GPSD; 2:Chrony;3:NTP;4:localClock
  uint32_t timeError;
//  char [....]   data specific to  msgType:code
} TGIFControlMsg;


/*****************************************
* The rest are not used...
**************************************/

typedef struct {
  int      mode;       //type of fix: 0,1,2,3  0 means no fix or error
  uint32_t ts_sec;     //client places current timespec tv_sec
  uint32_t ts_nsec;    //client places nsec
  uint32_t latitude;
  uint32_t longitude;
  uint32_t elevation;
  uint32_t accuracy;  //larger of the lat or long error
} GPSBrief;


//From the original perfTool - Fall 2016 
typedef struct {
  uint16_t size;
  uint16_t mode;
  time_t timestamp;
  uint32_t sequenceNum;
  struct timeval timeSent;
  uint64_t timeSeconds;
  uint64_t microSeconds;
} messageHeaderDefaultOLD;



typedef struct {
  uint16_t size;
  uint16_t mode;
  uint16_t gpsCap; // indicates if gps was captured
  uint32_t sequenceNum;
  uint32_t timeSentSeconds;
  uint32_t timeSentUSeconds;
  uint32_t timeRxSeconds;
  uint32_t timeRxUSeconds;
  uint32_t OWDelay;      //the sender fills this in if it can estimate the one way latency
} messageHeaderTGIF;


//Correspnds to msgFormat 0 

//From V3 and initially in V4 - this is what Anjan's version used 
typedef struct {
  uint16_t size;
  uint16_t mode;
  uint16_t gpsCap; // indicates if gps was captured
  uint32_t sequenceNum;
  uint32_t timeSentSeconds;
  uint32_t timeSentUSeconds;
  uint32_t timeRxSeconds;
  uint32_t timeRxUSeconds;
  uint32_t OWDelay;      //the sender fills this in if it can estimate the one way latency
} messageHeaderV4;





//To be completed....
typedef struct {
  uint32_t SINR;
  uint32_t RSSI;
  uint32_t Channel;
  uint32_t Power;
} WirelessMACStatsBrief;


//FromAnjan's old code 
typedef struct {
  uint16_t size;
  uint16_t mode;
  uint16_t gpsCap; // indicates if gps was captured
  uint32_t sequenceNum;
  uint32_t timeSentSeconds;
  uint32_t timeSentUSeconds;
  uint32_t timeRxSeconds;
  uint32_t timeRxUSeconds;
  uint32_t OWDelay;      //the sender fills this in if it can estimate the one way latency
} messageHeaderAnjan;

typedef  struct{
	int mode;
	double latitude;
	double longitude;
	double altitude;
	double speed;
	double climb;
	double track;
	double latAcc;
	double longAcc;
	double altAcc;
	double speedAcc;
	double climbAcc;
	double trackAcc;
} gpsDataAnjan;


typedef struct {
	messageHeaderAnjan hdr;
	gpsDataAnjan gps;
	//char msg[MESSAGEMAX];
} payloadAnjan; //structure for packet content

typedef struct {
	int *sock;
	struct sockaddr_in *ServerAddress;
	struct gps_data_t gpsdata;
} ConInfo;



typedef struct {
  messageHeaderV4 hdr;
  gpsDataAnjan gps;
//  char msg[MESSAGEMAX];
} payload; 

messageHeaderDefault *createDefaultMsgHdr( uint32_t sequenceNum, uint16_t mode, struct timeval *myTime);
void printDefaultMessageHeader(messageHeaderDefault *myHdr);
int getMsgOverhead( uint16_t msgFormat);

int packMsg(void *msgPtr, void *unpackedMsgPtr, int maxSize, int encodeType);
int unPackMsg(void *msgPtr, void *unpackedMsgPtr, int maxSize, int encodeType);

//Should have these somewhere
//int packMsgHdrToNetworkBuffer(struct MsgHdr *myMsgHdr, void *networkBufferPtr, uint32_t bufSize);
//int packDataMSGToNetworkBuffer(struct DataMsg *myMsg, void  *networkBufferPtr, uint32_t bufSize);
//struct MsgHdr *unpackMsgHdrToNetworkBuffer(void *netBufferPtr, uint32_t bufSize);

#if 0
struct MsgHdr *createMsgHdr( uint32_t sequenceNum, uint16_t mode, uint16_t msgType, 
                             uint32_t timeSentSeconds, uint32_t timeSentUSeconds);

struct DataMsg *createDataMsg(struct MsgHdr *myHdr, uint32_t dataSize, void *myData); 
struct ACKMsg *createACKMsg(struct MsgHdr *myHdr, uint32_t RxSequenceNum); 
struct EchoDataMsg *createEchoDataMsg(uint32_t sequenceNum, uint16_t mode, uint32_t dataSize, void *myData);
 
int compareMsgHdrs( struct MsgHdr *MsgHdr1, struct MsgHdr *MsgHdr2);
int compareAckMsgs( struct ACKMsg *Msg1Ptr, struct ACKMsg *Msg2Ptr);
int compareDataMsgs( struct DataMsg *Msg1Ptr, struct DataMsg *Msg2Ptr);
int compareEchoDataMsgs( struct EchoDataMsg *Msg1Ptr, struct EchoDataMsg *Msg2Ptr);

int freeDataMsg(struct DataMsg *myMsg);
int freeACKMsg(struct ACKMsg *myMsg);
int freeEchoDataMsg(struct EchoDataMsg *myMsg);

int packMsgHdrToNetworkBuffer(struct MsgHdr *myMsgHdr, void *networkBufferPtr, uint32_t bufSize);
int packACKMSGToNetworkBuffer(struct ACKMsg *myMsg, void *networkBufferPtr, uint32_t bufSize);
int packDataMSGToNetworkBuffer(struct DataMsg *myMsg, void  *networkBufferPtr, uint32_t bufSize);
int packEchoDataMSGToNetworkBuffer(struct EchoDataMsg *myMsg, void  *networkBufferPtr, uint32_t bufSize);

//struct MsgHdr *unpackMsgHdrToNetworkBuffer(void *netBufferPtr, uint32_t bufSize);
//struct DataMsg *unpackNetworkBufferToDataMsg(void *networkBufferPtr, uint32_t bufSize);
//struct ACKMsg *unpackNetworkBufferToACKMsg(void *networkBufferPtr, uint32_t bufsize);
int unpackNetworkBufferToMsgHdr(struct MsgHdr **CallersMsgPtr, void *netBufferPtr, uint32_t bufSize);
int unpackNetworkBufferToDataMsg(struct DataMsg **CallersDataMsgPtr, void *networkBufferPtr, uint32_t bufSize);
int unpackNetworkBufferToACKMsg(struct ACKMsg **CallersAckMsgPtr, void *networkBufferPtr, uint32_t bufsize);
int unpackNetworkBufferToEchoDataMsg(struct EchoDataMsg **CallersEchoDataMsgPtr, void *networkBufferPtr, uint32_t bufSize);




/* Header on all OpenFlow packets. */
struct ofp_header {
uint8_t version;            /* OFP_VERSION. */
uint8_t type;               /* One of the OFPT_ constants. */
uint16_t length;            /* Length including this ofp_header. */
uint32_t xid;               /* Transaction id associated with this packet.
                               Replies use the same id as was in the request
                               to facilitate pairing. */
};

#endif

#endif



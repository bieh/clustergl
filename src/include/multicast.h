/*********************************************
	Libraries
**********************************************/

/* select Libs */
#include <sys/types.h>
#include <sys/time.h>

/* socket Libs */
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <dlfcn.h>
#include <netinet/tcp.h>

#include <iostream>
#include <string>
using std::string;

/* C Libs*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

/*********************************************
	Global Constants
**********************************************/
/* max sizes */
const int MAX_CONTENT = 1280;
const int HEADER_SIZE = 16;
const int MAX_PACKET_SIZE = MAX_CONTENT + HEADER_SIZE;

/* macro to check if a given bit pos is set or not */
#define CHECK_BIT(var,pos) ((var) & (1<<(pos)))

/* positions of various bits */
#define ACKPOS 9
#define NACKPOS 8
#define FINPOS 1
#define RQAPOS 0

/*********************************************
	Header Layout
**********************************************/

/*  
    0                   1                   2                   3   
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                         Frame Number                          |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                      Byte Offset Number                       |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                               |           |A|N|           |F|R|
   |   	      Packet Size          |           |C|C|           |I|Q|
   |                               |           |K|K|           |N|A|
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                             Checksum                          |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |															   |
   /                             data                              /
   |															   |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	
	Where 	ACK = if this is an ack message (reply messages)
			NCK	= if this is a nack message (reply messages)
			FIN = if this is the last packet of the frame (send messages)
			RQA = if the packet requires an ACK of not (send messages)
*/

struct braden_packet
{
	uint32_t frameNumber;
	uint32_t offsetNumber;
	uint16_t packetSize;
	uint16_t packetFlags;
	int checksum;
};

/*********************************************
	Client
**********************************************/

class Client
{
public:
	Client();
	void createMulticastSocket();
	void createTCPSocket();
	void sendTCP_ACK();
	void sendTCP_NACK();
	int readMulticastPacket(void *buf, size_t count);
	int writeData(void *buf, size_t count);
	int readData(void *buf, size_t count);
};

/*********************************************
	Server
**********************************************/

class Server
{
public:
	Server();
	void createMulticastSocket();
	void connectTCPSockets();
	int writeData(void *buf, size_t count);
	int readData(void *buf, size_t count);
	int writeMulticastPacket(void *buf, size_t count, bool requiresACK);
	bool readTCP_packet(int timeout);
	bool readTCPPacket(void *buf, size_t count, int timeout);
	bool checkACKList();
};

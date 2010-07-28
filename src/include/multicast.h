/*********************************************
	Libraries
**********************************************/

/* Not everyone has the headers for this, so improvise */
#ifndef MCAST_JOIN_SOURCE_GROUP
#define MCAST_JOIN_SOURCE_GROUP 46

struct group_source_req
{
	/* Interface index.  */
	uint32_t gsr_interface;

	/* Group address.  */
	struct sockaddr_storage gsr_group;

	/* Source address.  */
	struct sockaddr_storage gsr_source;
};

#endif

/*********************************************
	Global Multicast Header Structure
**********************************************/

struct multicast_header
{
	uint32_t frameNumber;
	uint32_t offsetNumber;
	uint16_t packetSize;
	uint16_t packetFlags;
};

/* positions of various bits */
#define ACKPOS 9
#define NACKPOS 8
#define DATAPOS 4
#define FINPOS 1
#define RQAPOS 0

/*********************************************
	Global Constants
**********************************************/
/* max sizes */
const int MAX_CONTENT = 1280;
const int MAX_PACKET_SIZE = MAX_CONTENT + sizeof(multicast_header);

/* macro to check if a given bit pos is set or not */
#define CHECK_BIT(var,pos) ((var >> pos) & 1)

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
   |								   |
   /                             data                              /
   |								   |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
	
	Where 	ACK = if this is an ack message (reply messages)
			NCK	= if this is a nack message (reply messages)
			FIN = if this is the last packet of the frame (send messages)
			RQA = if the packet requires an ACK of not (send messages)
*/

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
	int sendTCP_DATA(void *data, size_t size);
	int readMulticastPacket(void *buf, size_t count);
	int writeData(void *buf, size_t count);
	int readData(void *buf, size_t count);
	bool pullData();
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
	bool flushData(void);
	bool flushDataWorker(uint32_t startingOffset, int bufNum);
	int readData(void *buf, size_t count);
	int writeMulticastPacket(void *buf, size_t count, bool finalPacket, int offset);
	int readTCP_packet(int timeout, uint32_t expectedFrame);
	bool readTCPPacket(void *buf, size_t count, int timeout);
	bool checkACKList(int list);
	bool checkDATAList();
};

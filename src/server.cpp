#include "include/main.h"
#include "include/multicast.h"

extern string addresses[5];

/*********************************************************
	Server Globals
*********************************************************/

int multicastSocket;
int mSockets[5];
int numConnections = 4;

/* select timer */
timeval mytime;

/* timers */
struct timeval tokenStart, tokenEnd;
long tokenSeconds, tokenuSeconds;

/* array of bits which you set or clear to indicate events of interest */
fd_set rfds;

uint32_t ackList[5];
uint32_t ackList2[5];

/* max number of sockets to listen to */
int sd_max;

/* packet to read headers into */
multicast_header * serverPacket = (multicast_header *)malloc(sizeof(multicast_header));

uint32_t serverFrameNumber = 1;
uint32_t serverOffsetNumber = 0;

struct sockaddr_in *group;
struct group_source_req group_source_req;
struct sockaddr_in *source;

struct buffer_t {
	unsigned char buffer[16*1024*1024];
	unsigned int length;
};

struct retransmit_packets {
	unsigned char buffer[MAX_PACKET_SIZE];
	unsigned int length;
	unsigned int frame;
}
/* buffer storage for retransmitting the last packet */
retransmit_packets[2];

/* buffer storage and info */
buffer_t storedBuffers[2];
int curBuffer = 1;
uint32_t framesStored[2];
bool firstPacket[2];

/*********************************************************
	Server Configuration
*********************************************************/
/* number of tokens per second that can be sent 
 * e.g. 1Gbps = 125000000			*/

int tokens = 125000000;

/* port number */
int server_tcp_port = 1414;

/*********************************************************
	Server
*********************************************************/

void addToBuffer(void *data, unsigned int len)
{
	/* ensure the data being added will not overflow the buffer */
	if(storedBuffers[curBuffer].length + len < sizeof(storedBuffers[curBuffer].buffer)) {
		memcpy(static_cast<void*>(&storedBuffers[curBuffer].buffer[storedBuffers[curBuffer].length]), data, len);
		storedBuffers[curBuffer].length+=len;
	}
	else {
		printf("storedBuffer too small!\n");
	}
}

Server::Server()
{
	/* set the start time */
	gettimeofday(&tokenStart, NULL);
	createMulticastSocket();
	connectTCPSockets();
}

/*********************************************************
	Multicast Socket
*********************************************************/

void Server::createMulticastSocket()
{		
		/* reset all counter values ready to start transmitting */
		storedBuffers[0].length = 0;
		storedBuffers[1].length = 0;
		framesStored[1] = -1;
		framesStored[0] = 0;
		firstPacket[0] = true;
		firstPacket[1] = true;
		for(int i = 0; i < numConnections; i++) {
		    ackList[i] = 0;
		    ackList2[i] = 0;
		}

		
		/* create the socket */
		multicastSocket = socket(AF_INET,SOCK_DGRAM,getprotobyname("udp")->p_proto);
		socklen_t socklen = sizeof(struct sockaddr_storage);
		struct sockaddr_in bindaddr;
		u_char loop = 1;

		/* First bind to the port */
		bindaddr.sin_family = AF_INET;
		bindaddr.sin_port = htons(8990);
		bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);

		bind(multicastSocket,(struct sockaddr*)&bindaddr,sizeof(bindaddr));

		/* Now set up the SSM request */
		group_source_req.gsr_interface = 2; /* "any" interface */
		group=(struct sockaddr_in*)&group_source_req.gsr_group;
		source=(struct sockaddr_in*)&group_source_req.gsr_source;

		group->sin_family = AF_INET;
		inet_aton("232.1.1.1",&group->sin_addr);
		group->sin_port = 0;	 /* Ignored */
		/* Set the source to the name of the socket we created above */
		getsockname(multicastSocket,(struct sockaddr *)source, &socklen);
	
		setsockopt(multicastSocket,SOL_IP,MCAST_JOIN_SOURCE_GROUP, &group_source_req,
										sizeof(group_source_req));
		

		/* Enable reception of our own multicast */
		loop = 1;
		setsockopt(multicastSocket, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));

		/* Set the TTL on packets to 1, so the internet is not destroyed */
		loop=1;
		setsockopt(multicastSocket, IPPROTO_IP, IP_MULTICAST_TTL, &loop, sizeof(loop));

		/* Now we care about the port we send to */
		group->sin_port = htons(8991);
		
		printf("multicast socket created group 232.1.1.1 on port 8991!\n");
	    printf("ClusterGL: UDP buffer size is %d\n", bufferSize(SO_RCVBUF));
	    printf("ClusterGL: UDP buffer size is %d\n", setBufferSize(SO_RCVBUF, 300000));
}

int Server::bufferSize(int buf)
  {
    int value = 0;
    socklen_t size = sizeof(value);

    getsockopt(multicastSocket, SOL_SOCKET, buf,&value, &size);

    return value;
  }

int Server::setBufferSize(int buf, int bytes)
  {
    setsockopt(multicastSocket, SOL_SOCKET, buf, &bytes, sizeof(buf));

    return bufferSize(buf);
  }

/*********************************************************
	TCP Socket(s)
*********************************************************/

void Server::connectTCPSockets()
{
	/* Make socket and connect */
	for(int i = 0; i < numConnections; i++) {
		mSockets[i] = socket(PF_INET, SOCK_STREAM, 0);
	}

	/* set TCP options for socket */
	int one = 1;
	for(int i = 0; i < numConnections; i++) {
		setsockopt(mSockets[i], IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
		setsockopt(mSockets[i], SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
	}

	struct sockaddr_in mAddr[numConnections];
	
	/* connect socket(s) to server */
	for(int i = 0; i < numConnections; i++) {
		memset(&mAddr[i], 0, sizeof(mAddr[i]));
		mAddr[i].sin_family = AF_INET;
		mAddr[i].sin_addr.s_addr = inet_addr(addresses[i].c_str());
		mAddr[i].sin_port = htons(server_tcp_port);

		/* Establish connection */
		if (connect(mSockets[i],	(struct sockaddr *) &mAddr[i],sizeof(mAddr[i])) < 0) {
			printf("Failed to connect with server %s\n", addresses[i].c_str());
			mSockets[i] = 0;
			//return;
		}
	}

	/* macro to clear events of interest */
	FD_ZERO(&rfds);
	
	/* sd_max set to max of all sockets */
	sd_max = mSockets[0];
	for(int i = 0; i < numConnections; i++) {
		if(sd_max < mSockets[i]) {
			sd_max = mSockets[i];
		}
	}
	sleep(1);
}

/*********************************************************
	write
*********************************************************/


void Server::sendPulsePackets() {
	if(retransmit_packets[0].frame < retransmit_packets[1].frame) {
	sendto(multicastSocket, retransmit_packets[0].buffer, retransmit_packets[0].length, 0,
          (struct sockaddr*)group,sizeof(struct sockaddr_in)
          );
    sendto(multicastSocket, retransmit_packets[1].buffer, retransmit_packets[1].length, 0,
          (struct sockaddr*)group,sizeof(struct sockaddr_in)
          );
	}
	else {
    sendto(multicastSocket, retransmit_packets[1].buffer, retransmit_packets[1].length, 0,
          (struct sockaddr*)group,sizeof(struct sockaddr_in)
          );
    sendto(multicastSocket, retransmit_packets[0].buffer, retransmit_packets[0].length, 0,
          (struct sockaddr*)group,sizeof(struct sockaddr_in)
          );
    }
}

int Server::writeData(void *buf, size_t count)
{

	/* check for new ACKs, block until the data we are writing over has been ACKed */
	while (checkACKList(serverFrameNumber & 1) < storedBuffers[serverFrameNumber & 1].length && firstPacket[serverFrameNumber & 1]) {
		//printf("blocking waiting for ACKS in writeData!\n");
		//printf("values: %d < %d\n", checkACKList(serverFrameNumber & 1), storedBuffers[serverFrameNumber & 1].length);
		//printf("serverFrameNumber: %d\n", serverFrameNumber);
		readTCP_packet(25000);
		}
	
	/* if this is the first chunk of data, save it */
	if(firstPacket[curBuffer]) {
		firstPacket[curBuffer] = false;
		framesStored[curBuffer] = serverFrameNumber;
		storedBuffers[curBuffer].length = 0;
	}
	/* simply add the data to the buffer, which will be flushed later */
	addToBuffer(buf,count);
	return count;
}

bool Server::flushData(void)
{
	int previousPacketSize = 0;

	if (storedBuffers[curBuffer].length == 0)
			return true; /* ignore zero byte frames, this won't work, but paulh says it will. */
				/* 2 minutes later: Ok, that worked.  Sorry to blame you paulh. */

	/* Now we reset acks */
	for(int i = 0; i < numConnections; i++) {
		if(curBuffer == 0)
			ackList[i] = 0;
		else
			ackList2[i] = 0;
	}

	flushDataWorker(0, storedBuffers[curBuffer].length, curBuffer, 30);
	firstPacket[curBuffer] = true;
	serverFrameNumber++;
	curBuffer = serverFrameNumber & 1;

	return true; /* Success */
}

bool Server::flushDataWorker(uint32_t startingOffset, uint32_t endingOffest, int bufNum, int windowSize)
{
	//printf("flushDataWorker: %d %d\n", startingOffset, storedBuffers[bufNum].length);
	int packets = 0;
	while(startingOffset < storedBuffers[bufNum].length){
			if(packets != 20) {
				packets++;
				int to_write = (int)storedBuffers[bufNum].length-(int)startingOffset < MAX_CONTENT 
					? (int)storedBuffers[bufNum].length-startingOffset 
					: MAX_CONTENT;
		
						writeMulticastPacket(
							static_cast<void*>(&storedBuffers[bufNum].buffer[startingOffset]), 
							to_write, 
							startingOffset+to_write == storedBuffers[bufNum].length, 
							startingOffset+to_write == storedBuffers[bufNum].length || packets == 10,
							startingOffset, 
							framesStored[bufNum]);
	
						startingOffset += to_write;
						serverOffsetNumber = startingOffset;
			}
			else {
				while(checkACKList(bufNum&1) < MAX_CONTENT * 10) {
			//printf("sent 20 packets! check for ACKS!\n");
					readTCP_packet(25000);
				}
				packets++;
			}		
		}
	return true; /* Success */
}

int Server::writeMulticastPacket(void *buf, size_t count, bool finalPacket, bool requireACK, int offset, uint32_t frame)
{
	unsigned char * fullPacket = (unsigned char *) malloc(sizeof(multicast_header)+count);
	/* set flags */
	short flags = 0;
	flags |= requireACK << RQAPOS;
	flags |= finalPacket << FINPOS;
	
	/* fill in header values */	
	serverPacket->frameNumber = frame;
	serverPacket->offsetNumber = offset;
	serverPacket->packetFlags = flags;
	serverPacket->packetSize = count;
	
	/* storage for full packet to send one datagram */
	int sendVal = 0;
 	memcpy(fullPacket, serverPacket, sizeof(multicast_header));
	memcpy(fullPacket+ sizeof(multicast_header), buf, count);

	/* send the full packet */
	sendVal = sendto(multicastSocket, fullPacket, sizeof(multicast_header)+count, 0,
				(struct sockaddr*)group,sizeof(struct sockaddr_in)
				);

	if(sendVal == -1)	{
    		fprintf(stderr, "sendto() failed\n");
	}
	if(sendVal != (int) count+sizeof(multicast_header)) {
		printf("sendto kind of failing: %d %d\n", sendVal, count+sizeof(multicast_header));
	}
	if(finalPacket) {
		memcpy(retransmit_packets[frame&1].buffer, fullPacket, count+sizeof(multicast_header));
		retransmit_packets[frame&1].length = count+sizeof(multicast_header);
		retransmit_packets[frame&1].frame = frame;
	}
	free(fullPacket);
}

/*********************************************************
	read
*********************************************************/

int Server::readData(void *buf, size_t count)
{
	//printf("reading Data!: %d\n", count);
	/* block until all outstanding data is ACKed */
	while (checkACKList(0) < storedBuffers[0].length ||
		   checkACKList(1) < storedBuffers[1].length) {
		//printf("waiting for ACKs before reading data!\n");
		readTCP_packet(25000);
		}
		
	//printf("all ACKS up to date, will now read DATA\n");
	for(int i = 0; i < numConnections; i++) {
		int remaining = count;
		int ret = 0;
		while(remaining > 0) {
			ret = read(mSockets[i], (unsigned char *) buf + count - remaining, remaining);
			remaining -= ret;
		}
	}
	return count;
}

/*********************************************************
	check ACKS
*********************************************************/

/* returns the smallest ACK value received so far */
uint32_t Server::checkACKList(int list) {
	uint32_t min;
	if(list == 0)
		min = ackList[0];
	else
		min = ackList2[0];
	for(int i = 1; i < numConnections; i++) {
		if(list == 0) {
			if(ackList[i] < min) {
				min = ackList[i];
			}
		}
		else {
			if(ackList2[i] < min) {
				min = ackList2[i];
			}
		}
	}
	return min;
}

/*********************************************************
	TCP Control messaging
*********************************************************/

int Server::readTCP_packet(int timeout) {
	
	fd_set rfds;
	FD_ZERO(&rfds);

	for(int i = 0; i < numConnections; i++) {
		/* macro to set each socket to listen to */
		if (ackList[i] < storedBuffers[0].length || ackList2[i] < storedBuffers[1].length) {
			FD_SET(mSockets[i],&rfds);
		}
	}
	
	/* wait for 0 seconds, timeout microseconds */
	mytime.tv_sec=0;
	mytime.tv_usec= timeout;

	/* attempt to read from select sockets */
	int valReady = select(sd_max+1,&rfds,(fd_set *)0,(fd_set *)0,&mytime);
	
	if(valReady < 0) {
		printf("select error!\n");
	}
	else if(valReady == 0) {
		printf("select timeout: sending pulse Packets!\n");
		sendPulsePackets();
	}
	else {
		//printf("%d socket(s) to read from! timeout: %d\n", valReady, mytime.tv_usec);
		for(int i = 0; i < numConnections; i++) 
		{
			int ret = 0;
			/* if the socket still hasn't acked and read is mSockets[i]*/
			if(FD_ISSET(mSockets[i],&rfds)) {
				int remaining = sizeof(multicast_header);
				while(remaining > 0) {
					ret = read(mSockets[i], serverPacket + sizeof(multicast_header)-remaining, remaining);
					remaining -= ret;
				}
				if(remaining < 0) {
					printf("SLEEPING: got more than I wanted! asked for: %d, got %d\n", sizeof(multicast_header), sizeof(multicast_header) - remaining);
					sleep(1);
				}
				
				if(CHECK_BIT(serverPacket->packetFlags, NACKPOS)) {
					fprintf(stderr,"got a NACK! from %d for frame: %d, offset %d\n", i, serverPacket->frameNumber, serverPacket->offsetNumber);
					if(framesStored[0] == serverPacket->frameNumber)
						flushDataWorker(serverPacket->offsetNumber, storedBuffers[0].length, 0, 30);
					else
						flushDataWorker(serverPacket->offsetNumber, storedBuffers[1].length, 1, 30);
				}
				else if(CHECK_BIT(serverPacket->packetFlags, ACKPOS)){
					//printf("got an ACK! from: %d frame: %d, offset: %d\n", i, serverPacket->frameNumber, serverPacket->offsetNumber);
					if((serverPacket->frameNumber & 1) == 0)
						ackList[i] = serverPacket->offsetNumber;
					else
						ackList2[i] = serverPacket->offsetNumber;	
				}
				else{
					printf("SLEEPING: got something weird!\n");
					sleep(1);
				}
			}
		}
	}
	return valReady;
}


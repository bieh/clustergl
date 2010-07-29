#include "include/main.h"
#include "include/multicast.h"

extern string addresses[5];

/*********************************************************
	Server Globals
*********************************************************/

int multicastSocket;
int mSockets[5];
int numConnections = 3;

/* select timer */
timeval mytime;

/* timers */
struct timeval tokenStart, tokenEnd;
long tokenSeconds, tokenuSeconds;

/* array of bits which you set or clear to indicate events of interest */
fd_set rfds;

bool ackList[5];
bool ackList2[5];

/* max number of sockets to listen to */
int sd_max;

/* packet to read headers into */
multicast_header * serverPacket = (multicast_header *)malloc(sizeof(multicast_header));

uint32_t serverFrameNumber = 1;

struct sockaddr_in *group;
struct group_source_req group_source_req;
struct sockaddr_in *source;

struct buffer_t {
	char buffer[16*1024*1024];
	unsigned int length;
};

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
	/* set ackLists to true */
	for(int i = 0; i < numConnections; i++) {
		ackList[i] = true;
		ackList2[i] = true;	
	}

}

/*********************************************************
	Multicast Socket
*********************************************************/

void Server::createMulticastSocket()
{		

		storedBuffers[0].length = 0;
		storedBuffers[1].length = 0;
		framesStored[1] = -1;
		framesStored[0] = 0;
		firstPacket[0] = true;
		firstPacket[1] = true;
		
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
}

/*********************************************************
	write
*********************************************************/

int Server::writeData(void *buf, size_t count)
{
	//printf("writeData! %d %d\n", count, serverFrameNumber);
	//usleep(500000);

	/* check for new ACKs, block until the data we are writing over has been ACKed */
	while (!checkACKList(serverFrameNumber & 1)) {
		//printf("readTCP_packet(100, framesStored[curBuffer]! %d %d\n", serverFrameNumber, serverFrameNumber & 1);
		readTCP_packet(12000, framesStored[curBuffer]);
		if(!checkACKList(serverFrameNumber & 1)) {
			printf("timeout occured!! %d \n", serverFrameNumber);
		}
	}
	
	/* if this is the first chunk of data, save it */
	if(firstPacket[curBuffer]) {
		firstPacket[curBuffer] = false;
		framesStored[curBuffer] = serverFrameNumber;
		//printf("frame number in %d is now %d\n", curBuffer, serverFrameNumber);
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

	//fprintf(stderr,"Frame %d: Flushing %d bytes: \n", serverFrameNumber, storedBuffers[curBuffer].length);
	/* Send the data, then wait for an ACK or a NAK.  If we get a NAK, revisit
	 * sending all the data. 
         */
	
	/* Now we reset acks */
	for(int i = 0; i < numConnections; i++) {
		if(curBuffer == 0)
			ackList[i] = false;
		else
			ackList2[i] = false;
	}

	flushDataWorker(0, curBuffer);
	firstPacket[curBuffer] = true;
	serverFrameNumber++;
	curBuffer = serverFrameNumber & 1;

	return true; /* Success */
}

bool Server::flushDataWorker(uint32_t startingOffset, int bufNum)
{
	int packets = 0;
	//printf("flushDataWorker: %d %d\n", startingOffset, storedBuffers[bufNum].length);
	while(startingOffset < storedBuffers[bufNum].length){
		packets++;
		int to_write = (int)storedBuffers[bufNum].length-(int)startingOffset < MAX_CONTENT 
			? (int)storedBuffers[bufNum].length-startingOffset 
			: MAX_CONTENT;
			//if(tokens - to_write > 0) {
				writeMulticastPacket(
					static_cast<void*>(&storedBuffers[bufNum].buffer[startingOffset]), 
					to_write, 
					startingOffset+to_write == storedBuffers[bufNum].length, startingOffset, framesStored[bufNum]);
				startingOffset += to_write;
				tokens -= to_write;
			//}
			/*else {
			    	/* add to the token bucket 
				gettimeofday(&tokenEnd, NULL);
				tokenSeconds  = tokenEnd.tv_sec  - tokenStart.tv_sec;
				tokenuSeconds = tokenEnd.tv_usec - tokenStart.tv_usec;
			
				tokens += (tokenSeconds * 125000000) + (tokenuSeconds * 900);
				if(tokens > 125000000) {
					tokens = 125000000;
				}
				gettimeofday(&tokenStart, NULL);
				//usleep(1);
			}*/
		}
	return true; /* Success */
}

int Server::writeMulticastPacket(void *buf, size_t count, bool finalPacket, int offset, uint32_t frame)
{
	/* set flags */
	short flags = 0;
	flags |= finalPacket << RQAPOS;
	flags |= finalPacket << FINPOS;

	/* fill in header values */	
	serverPacket->frameNumber = frame;
	serverPacket->offsetNumber = offset;
	serverPacket->packetFlags = flags;
	serverPacket->packetSize = count;
	
	//printf("multicast packet details: %d %d %d %d\n", serverPacket->frameNumber, serverPacket->offsetNumber, serverPacket->packetFlags, serverPacket->packetSize);
	/* storage for full packet to send one datagram */
	unsigned char * fullPacket = (unsigned char *) malloc(sizeof(multicast_header)+MAX_PACKET_SIZE);
 	memcpy(fullPacket, serverPacket, sizeof(multicast_header));
	memcpy(fullPacket+ sizeof(multicast_header), buf, count);

	/* send the full packet */
	if(sendto(multicastSocket,
				fullPacket,sizeof(multicast_header)+count,
				0,
				(struct sockaddr*)group,sizeof(struct sockaddr_in)
				) == -1) {
    		fprintf(stderr, "sendto() failed\n");
	}
	free(fullPacket);
}

/*********************************************************
	read
*********************************************************/

int Server::readData(void *buf, size_t count)
{
//	printf("reading Data!: %d\n", count);
	/* block until all outstanding data is ACKed */
	while (!checkACKList((serverFrameNumber -2) & 1)) {
		readTCP_packet(12000, (serverFrameNumber - 2));
	}
	//printf("halfway ACK\n");
	while (!checkACKList((serverFrameNumber-1) & 1)) {
		readTCP_packet(12000, serverFrameNumber - 1);
	}
	//printf("all ACKS up to date\n");

	
	for(int i = 0; i < numConnections; i++) {
		int remaining = count;
		int ret = 0;
		while(remaining > 0) {
			ret = read(mSockets[i], (char *) buf + count - remaining, count);
			remaining -= ret;
			//printf("reading %d, read this time: %d!\n", count, ret);
		}
	}
	//printf("data READ!\n");
	return count;
}

/*********************************************************
	check ACKS
*********************************************************/

bool Server::checkACKList(int list) {
	for(int i = 0; i < numConnections; i++) {
		if(list == 0) {
			if(!ackList[i]) {
				return false;
			}
		}
		else {
			if(!ackList2[i]) {
				return false;
			}
		}
	}
	return true;
}

/*********************************************************
	TCP Control messaging
*********************************************************/

int Server::readTCP_packet(int timeout, uint32_t expectedFrame) {
	int list = expectedFrame & 1;
	fd_set rfds;
	FD_ZERO(&rfds);
	for(int i = 0; i < numConnections; i++) {
		/* macro to set each socket to listen to */
		if ((!ackList[i] && list == 0) || (!ackList2[i] && list == 1))
			FD_SET(mSockets[i],&rfds);
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
		////printf("select timeout!\n");
	}
	else {
		//printf("%d socket(s) to read from!\n", valReady);
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
				
				/*check if from the current frame */
				if(serverPacket->frameNumber != expectedFrame) {
					fprintf(stderr,"unexpected packet for frame number: %d expecting %d, from client #%d, ret %d\n", 
							serverPacket->frameNumber, 
							expectedFrame,
							i,
							ret
							);
				}
				else {
					if(CHECK_BIT(serverPacket->packetFlags, NACKPOS)) {
						fprintf(stderr,"got a NACK! for frame: %d, framesStored %d & %d\n", serverPacket->frameNumber, framesStored[0], framesStored[1]);
						/* its a NACK, so resend every bit of data after missing packet */
						if(framesStored[0] == serverPacket->frameNumber) {
							flushDataWorker(serverPacket->offsetNumber, 0);
							if(framesStored[1] > serverPacket->frameNumber)	{
								flushDataWorker(0, 1);
							}	
						}
						else {
							flushDataWorker(serverPacket->offsetNumber, 1);
							if(framesStored[0] > serverPacket->frameNumber)	{
								flushDataWorker(0, 0);
							}
						}
					}
					else if(CHECK_BIT(serverPacket->packetFlags, ACKPOS)){
						//printf("got an ACK! from: %d frame: %d\n", i, expectedFrame);
						if(list == 0)
							ackList[i] = true;
						else
							ackList2[i] = true;	
					}
					else{
						printf("got something weird!\n");	
					}
				}
			}
		}
	}
	return valReady;
}


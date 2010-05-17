#include "include/multicast.h"
#include <assert.h>

/* Not everyone has the headers for this so improvise */
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

extern string addresses[5];

/*********************************************************
	Server Globals
*********************************************************/

int multicastSocket;
int mSockets[5];
int numConnections = 4;

int tokens = 125000000;
/* timers to not transmit lots of NACKS */
struct timeval tokenStart, tokenEnd;
long tokenSeconds, tokenuSeconds;


//char* address = (char *) "127.0.0.1";
bool ackList[5];

/* NACK checking values */
uint32_t packetCount = 0;
uint32_t checkNacksFreq = 800;

/* select timer */
timeval mytime;

/* array of bits which you set or clear to indicate events of interest */
fd_set rfds;

/* max number of sockets to listen to */
int sd_max;

/* packet to read headers into */
braden_packet * serverPacket = (braden_packet *)malloc(sizeof(braden_packet));

uint32_t serverFrameNumber = 0;
uint32_t serverOffsetNumber = 0;
int server_tcp_port = 1414;
struct sockaddr_in *group;
struct group_source_req group_source_req;
struct sockaddr_in *source;

int curbuffer = 0;
struct buffer_t {
	char buffer[16*1024*1024];
	unsigned int length;
};
buffer_t buffers[2];

/*********************************************************
	Server
*********************************************************/

void write_to_buffer(void *data, unsigned int len)
{
	buffer_t *buf = &buffers[curbuffer];
	assert(buf->length + len < sizeof(buf->buffer));
	memcpy(static_cast<void*>(&buf->buffer[buf->length]), data, len);
	buf->length+=len;
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
	//printf("Connected to tcp socket!\n");

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
	write_to_buffer(buf,count);
	return count;
}

bool Server::flushData(void)
{
	int previousPacketSize = 0;

	buffer_t *buf = &buffers[curbuffer];
	int lastoffset = 0;

	if (buf->length == 0)
			return true; /* ignore zero byte frames, this won't work, but paulh says it will. */
				/* 2 minutes later: Ok, that worked.  Sorry to blame you paulh. */

	serverFrameNumber++;
	serverOffsetNumber = 0;
	int packets = 0;
	/* Now we wait for acks */
	for(int i = 0; i < numConnections; i++) {
		ackList[i] = false;	
	}
	//fprintf(stderr,"Frame %d: Flushing %d bytes\n", serverFrameNumber, buf->length);
	/* Send the data, then wait for an ACK or a NAK.  If we get a NAK, revisit
	 * sending all the data. 
         */
	do {
		while(serverOffsetNumber < buf->length) {
			int to_write = (int)buf->length-(int)serverOffsetNumber < MAX_CONTENT 
					? (int)buf->length-serverOffsetNumber 
					: MAX_CONTENT;
					//if(tokens - to_write > 0) {
						packets++;
						writeMulticastPacket(
							static_cast<void*>(&buf->buffer[serverOffsetNumber]), 
							to_write, 
							serverOffsetNumber+to_write == buf->length);
							lastoffset = serverOffsetNumber;
						serverOffsetNumber += to_write;
						tokens -= to_write;
						if(packets % checkNacksFreq == 0) {
							readTCP_packet(0);
						}
					//	if(packets == 62) {
					//		usleep(1);
					//	}
					//}
					//else {
					    /* add to the token bucket */
						/*gettimeofday(&tokenEnd, NULL);

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

		/* If we hit a timeout, then resend the last packet. */
		if (readTCP_packet(12000) == 0) {
				fprintf(stderr,"Frame %d: Timeout, retransmitting last packet (offset %d of %d): Waiting on",
						serverFrameNumber,
						serverOffsetNumber, 
						buf->length);
				for(int i=0;i<numConnections; ++i) {
					if (!ackList[i])
						fprintf(stderr," %d",i);
				}
				fprintf(stderr,"\n");
				serverOffsetNumber = lastoffset;
		}
	} while (!checkACKList());

	//fprintf(stderr,"Frame %d: Flushed %d bytes\n", serverFrameNumber, buf->length);
	/* Reset the buffer position to 0 so we can start adding new data to it. */
	buf->length = 0;

	return true; /* Success */
}

int Server::writeMulticastPacket(void *buf, size_t count, bool requiresACK)
{
	/* set flags */
	short flags = 0;
	flags |= requiresACK << RQAPOS;

	/* fill in header values */
	serverPacket->frameNumber = serverFrameNumber;
	serverPacket->offsetNumber = serverOffsetNumber;
	serverPacket->packetFlags = flags;
	serverPacket->packetSize = count;

	/* storage for full packet to send one datagram */
	unsigned char * fullPacket = (unsigned char *) malloc(MAX_PACKET_SIZE);
 	memcpy(fullPacket, serverPacket, sizeof(braden_packet));
	memcpy(fullPacket+ sizeof(braden_packet), buf, count);

	/* send the full packet */
	if(sendto(multicastSocket,
				fullPacket,sizeof(braden_packet)+count,
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
	//printf("reading data!\n");
	for(int i = 0; i < numConnections; i++) {
		int remaining = count;
		int ret = 0;
		while(remaining > 0) {
			ret = read(mSockets[i], (char *) buf + count - remaining, count);
			remaining -= ret;
		}
	}
	return count;
}

/*********************************************************
	check ACKS
*********************************************************/

bool Server::checkACKList() {
	for(int i = 0; i < numConnections; i++) {
		if(!ackList[i]) {
			return false;
		}	
	}
	return true;
}

/*********************************************************
	TCP Control messaging
*********************************************************/

int Server::readTCP_packet(int timeout) {
	
	fd_set rfds;
	FD_ZERO(&rfds);
	for(int i = 0; i < numConnections; i++) {
		/* macro to set each socket to listen to */
		if (!ackList[i])
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
			if(ackList[i] && FD_ISSET(mSockets[i],&rfds)){
				fprintf(stderr,"Ignoring packet on unexpected socket #%d (%d)\n",i,mSockets[i]);
				valReady--;
			}
			int ret = 0;
			/* if the socket still hasn't acked and read is mSockets[i]*/
			if(!ackList[i] && FD_ISSET(mSockets[i],&rfds)) {
				int remaining = sizeof(braden_packet);
				/* FIXME: This won't work. because if you get half of a
				 * braden_packet, you'll overwrite the first half with the
				 * second half when the second half turns up
				 */
				while(remaining > 0) {
					ret = read(mSockets[i], serverPacket + sizeof(braden_packet)-remaining, remaining);
					remaining -= ret;
				}
				
				/*check if from the current frame */
				if(serverPacket->frameNumber != serverFrameNumber) {
					fprintf(stderr,"unexpected packet for frame number: %d expecting %d, from client #%d, ret %d\n", 
							serverPacket->frameNumber, 
							serverFrameNumber,
							i,
							ret
							);
					usleep(100000);
				}
				else {
					if(CHECK_BIT(serverPacket->packetFlags, NACKPOS)) {
						fprintf(stderr,"got a NACK! %d offset now %d\n", serverOffsetNumber, serverPacket->offsetNumber);

						/* its a NACK, so reset offsetNumber */
						serverOffsetNumber = serverPacket->offsetNumber;
					}
					else if(CHECK_BIT(serverPacket->packetFlags, ACKPOS)){
						//printf("got an ACK! from: %d frame: \n", i, serverFrameNumber);
						ackList[i] = true;		
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


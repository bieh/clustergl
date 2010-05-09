#include "include/multicast.h"

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
int numConnections = 5;
//char* address = (char *) "127.0.0.1";
bool ackList[5];

/* NACK checking values */
uint32_t packetCount = 0;
uint32_t checkNacksFreq = 10;

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
int server_tcp_port = 1313;
struct sockaddr_in *group;
struct group_source_req group_source_req;
struct sockaddr_in *source;

/*********************************************************
	Server
*********************************************************/

Server::Server()
{
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
		bindaddr.sin_port = htons(9990);
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
		group->sin_port = htons(9991);
		
		printf("multicast socket created group 232.1.1.1 on port 9991!\n");
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
	serverFrameNumber++;
	serverOffsetNumber = 0;
	int previousPacketSize = 0;

	for(int i = 0; i < numConnections; i++) {
		ackList[i] = false;	
	}
	
	//printf("frame: %d sending %d total bytes!\n", serverFrameNumber, count);	
	
	while(!checkACKList()) 
	{

		/* Now send all packets */
		while(serverOffsetNumber < count) 
		{

			/* set the size of packet to send */
			int packetSize = count-serverOffsetNumber;
			if(packetSize > MAX_CONTENT) packetSize = MAX_CONTENT;

			//if(packetCount != 100 && packetCount != 102 && packetCount != 104)
			writeMulticastPacket((unsigned char *)buf+serverOffsetNumber, packetSize, 
									serverOffsetNumber + packetSize == count);
			packetCount++;	

			/* change counters to reflect the packet that has been sent */
			serverOffsetNumber += packetSize;
			previousPacketSize = packetSize;

			/* if this is not the last packet, check for NACKS */
			if(serverOffsetNumber < count && packetCount % checkNacksFreq == 0) {
				readTCP_packet(1);
			}
		}
		/* now wait for acks, with longer timeout val */
		int rec = 0;
		rec += readTCP_packet(1000);
		if(rec < numConnections) {
			rec += readTCP_packet(2000);
		}
		if(rec < numConnections) {
		    rec += readTCP_packet(2000);
		}

	
		if(checkACKList()) {
		}
		else {
			/* if no ACK received (timed out), resend the previous packet 
			to hopefully flush the ACK (or force a NACK) through */
			//printf("%d frame, resending data, got %d so far!\n", serverFrameNumber, rec);	
			serverOffsetNumber -= previousPacketSize;
		}
	}
	return count;
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

	/* send them 
	if(sendto(fd,serverPacket,sizeof(braden_packet),0,(struct sockaddr*)group,sizeof(struct sockaddr_in)) == -1) {
    		f//printf(stderr, "sendto() failed\n");
	}
	if(sendto(fd,buf,count,0,(struct sockaddr*)group,sizeof(struct sockaddr_in)) == -1) {
    		f//printf(stderr, "sendto() failed\n");
	}*/
	////printf("sending %d bytes!\n", count);
	/* send the full packet */
	if(sendto(multicastSocket,fullPacket,sizeof(braden_packet)+count,0,(struct sockaddr*)group,sizeof(struct sockaddr_in)) == -1) {
    		fprintf(stderr, "sendto() failed\n");
	}
}

/*********************************************************
	read
*********************************************************/

int Server::readData(void *buf, size_t count)
{
	//printf("reading data!\n");
	for(int i = 0; i < numConnections; i++) {
		int ret = read(mSockets[i], buf, count);
	}
	return count;
}

/*********************************************************
	check ACKS
*********************************************************/

bool Server::checkACKList() {
	bool allACKS = true;
	for(int i = 0; i < numConnections; i++) {
		if(!ackList[i]) {
		allACKS = false;		
		}	
	}
	return allACKS;
}

/*********************************************************
	TCP Control messaging
*********************************************************/

int Server::readTCP_packet(int timeout) {
	
	for(int i = 0; i < numConnections; i++) {
		/* macro to set each socket to listen to */
		FD_SET(mSockets[i],&rfds);
	}
	
	/* wait for 0 seconds, timeout microseconds */
	mytime.tv_sec=0;
	mytime.tv_usec= timeout;

	/* attempt to read from select sockets */
	int valReady = select(sd_max+1,&rfds,(fd_set *)0,(fd_set *)0,&mytime);
	if(valReady < 0) {
		//printf("select error!\n");
	}
	else if(valReady == 0) {
		////printf("select timeout!\n");
	}
	else {
		//printf("%d socket(s) to read from!\n", valReady);
		for(int i = 0; i < numConnections; i++) 
		{
			/* if the socket still hasn't acked and read is mSockets[i]*/
			if(!ackList[i] && FD_ISSET(mSockets[i],&rfds)) {
				int remaining = sizeof(braden_packet);
				int ret = 0;
					while(remaining > 0) {
						ret = read(mSockets[i], serverPacket, sizeof(braden_packet));
						remaining -= ret;
					}
				
					/*check if from the current frame */
					if(serverPacket->frameNumber != serverFrameNumber) {
						//printf("frame number: %d expecting %d\n", serverPacket->frameNumber, serverFrameNumber);
					}
					else {
						if(CHECK_BIT(serverPacket->packetFlags, NACKPOS)) {
							printf("got a NACK! %d offset now %d\n", serverOffsetNumber, serverPacket->offsetNumber);

							/* its a NACK, so reset offsetNumber */
							serverOffsetNumber = serverPacket->offsetNumber;
						}
						else if(CHECK_BIT(serverPacket->packetFlags, ACKPOS)){
							//printf("got an ACK! %d\n", i);
							ackList[i] =  true;		
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


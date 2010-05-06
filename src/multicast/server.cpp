#include "main.h"

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

/*********************************************************
	Server Globals
*********************************************************/

int fd;
int mSocket[5];
int numConnections = 1;
char* address = (char *) "127.0.0.1";
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
braden_packet * newPacket = (braden_packet *)malloc(sizeof(braden_packet));

uint32_t frameNumber = 1;
uint32_t offsetNumber = 0;
int tcp_port = 1234;
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
		fd = socket(AF_INET,SOCK_DGRAM,getprotobyname("udp")->p_proto);
		socklen_t socklen = sizeof(struct sockaddr_storage);
		struct sockaddr_in bindaddr;
		u_char loop = 1;

		/* First bind to the port */
		bindaddr.sin_family = AF_INET;
		bindaddr.sin_port = htons(9990);
		bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);

		bind(fd,(struct sockaddr*)&bindaddr,sizeof(bindaddr));

		/* Now set up the SSM request */
		group_source_req.gsr_interface = 0; /* "any" interface */
		group=(struct sockaddr_in*)&group_source_req.gsr_group;
		source=(struct sockaddr_in*)&group_source_req.gsr_source;

		group->sin_family = AF_INET;
		inet_aton("232.1.1.1",&group->sin_addr);
		group->sin_port = 0;	 /* Ignored */
		/* Set the source to the name of the socket we created above */
		getsockname(fd,(struct sockaddr *)source, &socklen);
	
		setsockopt(fd,SOL_IP,MCAST_JOIN_SOURCE_GROUP, &group_source_req,
										sizeof(group_source_req));
		


		/* Enable reception of our own multicast */
		loop = 1;
		//setsockopt(fd, IPPROTO_IP, IP_MULTICAST_LOOP, &loop, sizeof(loop));

		/* Set the TTL on packets to 1, so the internet is not destroyed */
		loop=1;
		setsockopt(fd, IPPROTO_IP, IP_MULTICAST_TTL, &loop, sizeof(loop));

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
		mSocket[i] = socket(PF_INET, SOCK_STREAM, 0);
	}

	/* set TCP options for socket */
	int one = 1;
	for(int i = 0; i < numConnections; i++) {
		setsockopt(mSocket[i], IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
		setsockopt(mSocket[i], SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
	}

	struct sockaddr_in mAddr[numConnections];
	
	/* connect socket(s) to server */
	for(int i = 0; i < numConnections; i++) {
		memset(&mAddr[i], 0, sizeof(mAddr));
		mAddr[i].sin_family = AF_INET;
		mAddr[i].sin_addr.s_addr = inet_addr(address);
		mAddr[i].sin_port = htons(tcp_port);

		/* Establish connection */
		if (connect(mSocket[i],	(struct sockaddr *) &mAddr[i],sizeof(mAddr[i])) < 0) {
			printf("Failed to connect with server\n");
			mSocket[i] = 0;
			return;
		}
	}
	printf("Connected to tcp socket!\n");

	/* macro to clear events of interest */
	FD_ZERO(&rfds);
	
	/* sd_max set to max of all sockets */
	sd_max = mSocket[0];
	for(int i = 0; i < numConnections; i++) {
		if(sd_max < mSocket[i]) {
			sd_max = mSocket[i];
		}
	}
}

/*********************************************************
	send
*********************************************************/

int Server::send(void *buf, size_t count)
{
	offsetNumber = 0;
	int previousPacketSize = 0;

	for(int i = 0; i < numConnections; i++) {
		ackList[i] = false;	
	}
	
	printf("sending %d total bytes!\n",  count);	
	
	while(!checkACKList()) 
	{

		/* Now send all packets */
		while(offsetNumber < count) 
		{

			/* set the size of packet to send */
			int packetSize = count-offsetNumber;
			if(packetSize > MAX_CONTENT) packetSize = MAX_CONTENT;

			if(packetCount != 100 && packetCount != 102 && packetCount != 104)
			writeMulticastPacket((unsigned char *)buf+offsetNumber, packetSize, 
									offsetNumber + packetSize == count);
			packetCount++;	

			/* change counters to reflect the packet that has been sent */
			offsetNumber += packetSize;
			previousPacketSize = packetSize;

			/* if this is not the last packet, check for NACKS */
			if(offsetNumber < count && packetCount % checkNacksFreq == 0) {
				readTCP_packet(1);
			}
		}
		/* now wait for acks, with longer timeout val */
		readTCP_packet(100);
	
		if(checkACKList()) {
		}
		else {
			/* if no ACK received (timed out), resend the previous packet 
			to hopefully flush the ACK (or force a NACK) through */
			offsetNumber -= previousPacketSize;
		}
	}
	return 1;
}

int Server::writeMulticastPacket(void *buf, size_t count, bool requiresACK)
{
	/* set flags */
	short flags = 0;
	flags |= requiresACK << RQAPOS;

	/* fill in header values */
	newPacket->frameNumber = frameNumber;
	newPacket->offsetNumber = offsetNumber;
	newPacket->packetFlags = flags;
	newPacket->packetSize = count;

	/* storage for full packet to send one datagram */
	unsigned char * fullPacket = (unsigned char *) malloc(MAX_PACKET_SIZE);
 	memcpy(fullPacket, newPacket, sizeof(braden_packet));
	memcpy(fullPacket+ sizeof(braden_packet), buf, count);

	/* send them 
	if(sendto(fd,newPacket,sizeof(braden_packet),0,(struct sockaddr*)group,sizeof(struct sockaddr_in)) == -1) {
    		fprintf(stderr, "sendto() failed\n");
	}
	if(sendto(fd,buf,count,0,(struct sockaddr*)group,sizeof(struct sockaddr_in)) == -1) {
    		fprintf(stderr, "sendto() failed\n");
	}*/

	/* send the full packet */
	if(sendto(fd,fullPacket,sizeof(braden_packet)+count,0,(struct sockaddr*)group,sizeof(struct sockaddr_in)) == -1) {
    		fprintf(stderr, "sendto() failed\n");
	}
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

bool Server::readTCP_packet(int timeout) {
	
	for(int i = 0; i < numConnections; i++) {
		/* macro to set each socket to listen to */
		FD_SET(mSocket[i],&rfds);
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
		//printf("select timeout!\n");
	}
	else {
		printf("%d socket(s) to read from!\n", valReady);
		for(int i = 0; i < numConnections; i++) 
		{
			/* if the socket to read is mSocket[i]*/
			if(FD_ISSET(mSocket[i],&rfds)) {
				int remaining = sizeof(braden_packet);
				int ret = 0;
					while(remaining > 0) {
						ret = read(mSocket[i], newPacket, sizeof(braden_packet));
						remaining -= ret;
					}
				
					/*check if from the current frame */
					if(newPacket->frameNumber != frameNumber) {
						printf("frame number: %d expecting %d\n", newPacket->frameNumber, frameNumber);
					}
					else {
						if(CHECK_BIT(newPacket->packetFlags, NACKPOS)) {
							printf("got a NACK! %d offset now %d\n", offsetNumber, newPacket->offsetNumber);

							/* its a NACK, so reset offsetNumber */
							offsetNumber = newPacket->offsetNumber;
						}
						else if(CHECK_BIT(newPacket->packetFlags, ACKPOS)){
							printf("got an ACK!\n");
							ackList[i] =  true;		
						}
						else{
							printf("got something weird!\n");	
						}
					}
			}
		}
	}
	return false;
}


#include "main.h"

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

/*********************************************************
	Client Globals
*********************************************************/

/* multicast socket */
int fd;
char* hostAddress = (char *) "10.1.18.243";

/* TCP socket */
int mSocket;
int tcp_fd;
int tcp_port = 1234;

/* packet counting variables */
uint32_t frameNumber = 1;
uint32_t offsetNumber = 0;

/* timers to not transmit lots of NACKS */
struct timeval start, end;
long mtime, seconds, useconds;    

/* packet header to reuse*/
braden_packet * newPacket = (braden_packet *)malloc(sizeof(braden_packet));

/*********************************************************
	Client
*********************************************************/

Client::Client()
{
	createMulticastSocket();
	createTCPSocket();
}


/*********************************************************
	Create Multicast Connection
*********************************************************/

void Client::createMulticastSocket()
{
		/* create required structures */
		struct group_source_req group_source_req;
		struct sockaddr_in *group;
		struct sockaddr_in *source;
		fd = socket(AF_INET,SOCK_DGRAM,getprotobyname("udp")->p_proto);
		struct sockaddr_in bindaddr;

		/* Setup the socket to listen on */
		bindaddr.sin_family = AF_INET;
		bindaddr.sin_port = htons(9991);
		bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		bind(fd,(struct sockaddr*)&bindaddr,sizeof(bindaddr));

		/* Set up the connection to the group */
		group_source_req.gsr_interface = 0;
		group=(struct sockaddr_in*)&group_source_req.gsr_group;
		source=(struct sockaddr_in*)&group_source_req.gsr_source;

		/* Group is 232.1.1.1 */
		group->sin_family = AF_INET;
		inet_aton("232.1.1.1",&group->sin_addr);

		/* Source is [ip address] */
		group->sin_port = 0;
		source->sin_family = AF_INET;
		if (inet_aton(hostAddress,&source->sin_addr) == 0) {
		  printf("error: %s\n", (hostAddress));					  ;
		}
		source->sin_port = 0;

		setsockopt(fd,SOL_IP,MCAST_JOIN_SOURCE_GROUP, &group_source_req, sizeof(group_source_req));

		printf("listening to multicast group 232.1.1.1 on port 9991!\n");

}

/*********************************************************
	Create TCP Connection
*********************************************************/

void Client::createTCPSocket()
{
	/* create TCP socket */
	if ((mSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		printf("Failed to create socket\n");
		exit(1);
	}

	struct sockaddr_in addr, clientaddr;

	/* Construct the server sockaddr_in structure */
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(tcp_port);


	/* set TCP options */
	int one = 1;
	setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
	setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

	/* Bind the server socket */
	if (bind(mSocket, (struct sockaddr *) &addr,
	sizeof(addr)) < 0) {
		printf("Failed to bind the server socket\n");
		exit(1);
	}

	/* Listen */
	if (listen(mSocket, 1) < 0) {
		printf("Failed to listen on server socket\n");
		exit(1);
	}

	unsigned int clientlen = sizeof(clientaddr);
	int client = 0;

	/* Wait for connection */
	printf("waiting for tcp connection!\n");
	if ((client = accept(mSocket, (struct sockaddr *) &clientaddr, &clientlen)) < 0) {
		printf("Failed to accept client connection\n");
		exit(1);
	}
	tcp_fd = client;
	printf("tcp connected!\n");
}

/*********************************************************
	Multicast Reading
*********************************************************/

int Client::read(void *buf, size_t count) 
{
		offsetNumber = 0;
		gettimeofday(&start, NULL);
		/* Now read packets */
		while(offsetNumber < count) {
		  //printf("receiving packet!\n");
		  int packetSize = count-offsetNumber;
 		  if(packetSize > MAX_CONTENT) packetSize = MAX_CONTENT;
		  readMulticastPacket((unsigned char *) buf+offsetNumber, packetSize);	
		}
		printf("all packets arrived!\n");
		return 0;
}

int Client::readMulticastPacket(void *buf, size_t count)
{
	/* storage for full packet to read one datagram */
	unsigned char * fullPacket = (unsigned char *) malloc(MAX_PACKET_SIZE);
	int ret=recv(fd, fullPacket,sizeof(braden_packet)+count,0);
	
	/* copy over to correct places */
 	memcpy(newPacket, fullPacket, sizeof(braden_packet));
	
	/* check header */
	bool correctHeader = true;

	if(newPacket->frameNumber != frameNumber) {
		printf("frame number: %d expecting %d ret %d\n", newPacket->frameNumber, frameNumber, ret);
		correctHeader = false;
	}

	if(newPacket->offsetNumber != offsetNumber) {
		printf("readOffset number: %d expecting %d ret %d\n", newPacket->offsetNumber, offsetNumber, ret);
		correctHeader = false;
	}
	if(newPacket->packetSize != count) {
		printf("packetSize number: %d expecting %d ret %d\n", newPacket->packetSize, count, ret);
	}
	
	if(correctHeader)
	{
		/* copy data and return */
		memcpy(buf, fullPacket+ sizeof(braden_packet), count);
		offsetNumber += count;

		/* check if ACK needs to be sent */
		if(CHECK_BIT(newPacket->packetFlags, RQAPOS)) {
			sendTCP_ACK();
		}
		
		return count;
	}
	else
	{
		if(newPacket->offsetNumber > offsetNumber)
		{
			/* send a NACK, as a packet is missing */
			sendTCP_NACK();
		}
		else
		{
			/* do nothing, someone else NACKed and is catching up */
			printf("ignoring packet!\n");
		}

	/* packet is not the next in the sequence, so return 0 bytes */
	return 0;
	}
}

/*********************************************************
	TCP Control messaging
*********************************************************/


void Client::sendTCP_ACK()
{
	printf("sending ACK!\n");
	
	/* set flags */
	short flags = 0;
	flags |=  1 << ACKPOS;
	newPacket->packetFlags = flags;

	/* fill in header values */
	newPacket->frameNumber = frameNumber;
	newPacket->offsetNumber = offsetNumber;

	/* send it */
	sendTCPPacket(newPacket, sizeof(braden_packet));
}

void Client::sendTCP_NACK()
{
	
	/* check that we're not spamming NACKS */
	gettimeofday(&end, NULL);

	seconds  = end.tv_sec  - start.tv_sec;
	useconds = end.tv_usec - start.tv_usec;

	mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
	
	/* if theres been enough time, then NACK */
	if(mtime > 0) {
		printf("sending NACK! %ld since last NACK\n", mtime);
		/* set flags */
		short flags = 0;
		flags |=  1 << NACKPOS;
		newPacket->packetFlags = flags;
	
		/* fill in header values */
		newPacket->frameNumber = frameNumber;
		newPacket->offsetNumber = offsetNumber;

		/* send it */
		sendTCPPacket(newPacket, sizeof(braden_packet));

		gettimeofday(&start, NULL);
	}
}

int Client::sendTCPPacket(void *buf, size_t count)
{
	int remaining = count;
	int ret = 0;
	while(remaining > 0) {
	ret = write(tcp_fd, buf, count);
	remaining -= ret;
	printf("sent: %d!\n", ret);
	}
}


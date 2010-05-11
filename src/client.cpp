#include "include/multicast.h"
#include <assert.h>

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

extern string addresses[5];

/*******************************************************n
Client Globals
*********************************************************/

/* multicast socket */
int multi_fd;
char* hostAddress = (char *) "192.168.22.101";

/* TCP socket */
int mSocket;
int tcp_fd;
int client_tcp_port = 1313;

/* packet counting variables */
uint32_t clientFrameNumber = 0;
uint32_t clientOffsetNumber = 0;

/* timers to not transmit lots of NACKS */
struct timeval start, end;
long mtime, seconds, useconds;    

/* packet header to reuse*/
braden_packet * clientPacket = (braden_packet *)malloc(sizeof(braden_packet));

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
		multi_fd = socket(AF_INET,SOCK_DGRAM,getprotobyname("udp")->p_proto);
		struct sockaddr_in bindaddr;

		/* Setup the socket to listen on */
		bindaddr.sin_family = AF_INET;
		bindaddr.sin_port = htons(9991);
		bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		bind(multi_fd,(struct sockaddr*)&bindaddr,sizeof(bindaddr));

		/* Set up the connection to the group */
		group_source_req.gsr_interface = 2;
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

		setsockopt(multi_fd,SOL_IP,MCAST_JOIN_SOURCE_GROUP, &group_source_req, sizeof(group_source_req));

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
	addr.sin_port = htons(client_tcp_port);


	/* set TCP options */
	int one = 1;
	setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
	setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

	/* Bind the server socket */
	if (bind(mSocket, (struct sockaddr *) &addr,
	sizeof(addr)) < 0) {
		printf("Client: Failed to bind the server socket \n");
		exit(1);
	}

	/* Listen */
	if (listen(mSocket, 1) < 0) {
		printf("Client: Failed to listen on server socket\n");
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

char buffer[16*1024*1024];
int offset;
int length;

bool last_packet = false;

bool Client::pullData(void)
{
	last_packet = false;
	/* reset counter values */
	clientFrameNumber++;
	clientOffsetNumber = 0;
	gettimeofday(&start, NULL);

	//printf("frame: %d reading %d total bytes!\n", clientFrameNumber, count);

	/* Now read packets */
	while(!last_packet)
	{
		//int packetSize = count-clientOffsetNumber;
 		//if(packetSize > MAX_CONTENT) packetSize = MAX_CONTENT;
		int packetSize = MAX_CONTENT;
		length+=readMulticastPacket(&buffer[length], packetSize);	
	}
	offset=0;
	//printf("all packets arrived!\n");
	return true;
}

int Client::readData(void *buf, size_t count) 
{
	assert(offset+(int)count < length);
	memcpy(buf,&buffer[offset],count);
	offset+=count;
	return count;
}

int Client::readMulticastPacket(void *buf, size_t count)
{
	/* storage for full packet to read one datagram */
	unsigned char * fullPacket = (unsigned char *) malloc(MAX_PACKET_SIZE);
	int ret=recv(multi_fd, fullPacket, sizeof(braden_packet)+MAX_PACKET_SIZE,0);
	
	/* copy over to correct places */
 	memcpy(clientPacket, fullPacket, sizeof(braden_packet));
	
	/* check header */
	bool correctHeader = true;

	if(clientPacket->frameNumber != clientFrameNumber) {
		//printf("frame number: %d expecting %d ret %d\n", clientPacket->frameNumber, clientFrameNumber, ret);
		correctHeader = false;
	}

	if(clientPacket->offsetNumber != clientOffsetNumber) {
		//printf("readOffset number: %d expecting %d ret %d\n", clientPacket->offsetNumber, clientOffsetNumber, ret);
		correctHeader = false;
	}
	if(clientPacket->packetSize != count) {
		//printf("packetSize number: %d expecting %d ret %d\n", clientPacket->packetSize, count, ret);
	}
	
	if(correctHeader)
	{
		/* copy data and return */
		count = ret - sizeof(braden_packet);
		memcpy(buf, fullPacket+ sizeof(braden_packet), count);
		clientOffsetNumber += count;

		/* check if ACK needs to be sent */
		if(CHECK_BIT(clientPacket->packetFlags, RQAPOS)) {
			sendTCP_ACK();
			last_packet = true;
		}
		free(fullPacket);
		return count;
	}
	else
	{
		if(clientPacket->frameNumber == clientFrameNumber 
				&& clientPacket->offsetNumber > clientOffsetNumber)
		{
			/* send a NACK, as a packet is missing */
			sendTCP_NACK();
		}
		else
		{
			/* do nothing, someone else NACKed and is catching up */
			//printf("ignoring packet!\n");
		}

		/* packet is not the next in the sequence, so return 0 bytes */
		free(fullPacket);
		return 0;
	}
}

/*********************************************************
	TCP Control messaging
*********************************************************/


void Client::sendTCP_ACK()
{
	//printf("sending ACK!\n");
	
	/* set flags */
	short flags = 0;
	flags |=  1 << ACKPOS;
	clientPacket->packetFlags = flags;

	/* fill in header values */
	clientPacket->frameNumber = clientFrameNumber;
	clientPacket->offsetNumber = clientOffsetNumber;

	/* send it */
	writeData(clientPacket, sizeof(braden_packet));
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
		printf("sending NACK! frame %d\n", clientFrameNumber);
		/* set flags */
		short flags = 0;
		flags |=  1 << NACKPOS;
		clientPacket->packetFlags = flags;
	
		/* fill in header values */
		clientPacket->frameNumber = clientFrameNumber;
		clientPacket->offsetNumber = clientOffsetNumber;

		/* send it */
		writeData(clientPacket, sizeof(braden_packet));

		gettimeofday(&start, NULL);
	}
}

int Client::writeData(void *buf, size_t count)
{
	int remaining = count;
	int ret = 0;
	while(remaining > 0) {
	ret = write(tcp_fd, buf, count);
	remaining -= ret;
	//printf("sent: %d!\n", ret);
	}
	return count;
}


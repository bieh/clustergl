#include "include/main.h"
#include "include/multicast.h"
#include <assert.h>

/********************************************************
	Main Globals (Loaded from config file)
********************************************************/

extern string addresses[5];
extern char * multicastServer;

/*******************************************************n
Client Globals
*********************************************************/

/* multicast socket */
int multi_fd;

/* TCP socket */
int mSocket;
int tcp_fd;
int client_tcp_port = 1414;

/* packet counting variables */
uint32_t clientFrameNumber = 0;
uint32_t clientOffsetNumber = 0;

/* timers to not transmit lots of NACKS */
struct timeval start, end;
long mtime, seconds, useconds;    

/* packet header to reuse*/
multicast_header * clientPacket = (multicast_header *)malloc(sizeof(multicast_header));

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
		struct group_source_req group_source_req_local;
		struct sockaddr_in *group;
		struct sockaddr_in *source;
		multi_fd = socket(AF_INET,SOCK_DGRAM,getprotobyname("udp")->p_proto);
		struct sockaddr_in bindaddr;

		/* Setup the socket to listen on */
		bindaddr.sin_family = AF_INET;
		bindaddr.sin_port = htons(8991);
		bindaddr.sin_addr.s_addr = htonl(INADDR_ANY);
		bind(multi_fd,(struct sockaddr*)&bindaddr,sizeof(bindaddr));

		/* Set up the connection to the group */
		group_source_req_local.gsr_interface = 2;
		group=(struct sockaddr_in*)&group_source_req_local.gsr_group;
		source=(struct sockaddr_in*)&group_source_req_local.gsr_source;

		/* Group is 232.1.1.1 */
		group->sin_family = AF_INET;
		inet_aton("232.1.1.1",&group->sin_addr);

		/* Source is [ip address] */
		group->sin_port = 0;
		source->sin_family = AF_INET;
		if (inet_aton(multicastServer,&source->sin_addr) == 0) {
		  printf("error: %s\n", (multicastServer));
		}
		source->sin_port = 0;

		setsockopt(multi_fd,SOL_IP,MCAST_JOIN_SOURCE_GROUP, &group_source_req_local, sizeof(group_source_req));

		printf("listening to multicast group 232.1.1.1 on port 8991!\n");

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
	if (bind(mSocket, 
			(struct sockaddr *) &addr, sizeof(addr)) < 0) {
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

	length=0;
	//printf("frame: %d reading %d total bytes!\n", clientFrameNumber, count);

	/* Now read packets */
	while(!last_packet)
	{
		length+=readMulticastPacket(&buffer[length], MAX_CONTENT);	
	}
	offset=0;
	//fprintf(stderr,"Frame: %d Size: %d\n", clientFrameNumber, length);
	return true;
}

int Client::readData(void *buf, size_t count) 
{
	if (offset+(int)count > length) {
			fprintf(stderr,"Trying to read %d bytes from %d @%d\n",
					count, length-offset, offset);
	}
	assert(offset+(int)count <= length);

	memcpy(buf,&buffer[offset],count);
	offset+=count;
	return count;
}

int Client::readMulticastPacket(void *buf, size_t maxsize)
{
	/* storage for full packet to read one datagram */
	unsigned char * fullPacket = (unsigned char *) malloc(sizeof(multicast_header)+MAX_PACKET_SIZE);
	int ret=recv(multi_fd, fullPacket, sizeof(multicast_header)+MAX_PACKET_SIZE,0);

	/* copy over to correct places */
 	memcpy(clientPacket, fullPacket, sizeof(multicast_header));

	if(clientPacket->frameNumber != clientFrameNumber) {
		printf("frame number: %d expecting %d offset %d\n", clientPacket->frameNumber, clientFrameNumber, clientPacket->offsetNumber);
		free(fullPacket);
		return false;
	}

	if (clientOffsetNumber < clientPacket->offsetNumber) {
	//	fprintf(stderr,"frame number: %d expecting offset %d, got %d\n", 
//				clientFrameNumber,
//				clientOffsetNumber,
//				clientPacket->offsetNumber);
		sendTCP_NACK();
		return false; /* Fail. */
	}

	if(clientPacket->offsetNumber != clientOffsetNumber) {
	//	printf("frame number: %d readOffset number: %d expecting %d ret %d\n", 
//			clientPacket->frameNumber, 
//			clientPacket->offsetNumber, 
//			clientOffsetNumber, 
//			ret);
		free(fullPacket);
		return false;
	}

	/* copy payload data */
	memcpy(buf, fullPacket+ sizeof(multicast_header), ret-sizeof(multicast_header));
	clientOffsetNumber += ret-sizeof(multicast_header);
	/* check if final packet */
	if(CHECK_BIT(clientPacket->packetFlags, FINPOS)) {
		last_packet = true;
	}
	/* check if ACK needs to be sent */
	if(CHECK_BIT(clientPacket->packetFlags, RQAPOS)) {
		sendTCP_ACK();
		/* If this asks for an ACK, then send it. */
	}
	free(fullPacket);

	/* Return the amount of payload */
	return ret-sizeof(multicast_header);
}

/*********************************************************
	TCP Control messaging
*********************************************************/


void Client::sendTCP_ACK()
{
	//printf("got: %d bytes, sending ACK for frame: %d!\n", clientOffsetNumber, clientFrameNumber);
	
	/* set flags */
	short flags = 0;
	flags |=  1 << ACKPOS;
	clientPacket->packetFlags = flags;

	/* fill in header values */
	clientPacket->frameNumber = clientFrameNumber;
	clientPacket->offsetNumber = clientOffsetNumber;

	/* send it */
	writeData(clientPacket, sizeof(multicast_header));
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
		writeData(clientPacket, sizeof(multicast_header));

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


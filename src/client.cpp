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

/*********************************************************
	Client
*********************************************************/

Client::Client()
{
	mtime = 99999;
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
	    printf("ClusterGL: UDP buffer size is %d\n", bufferSize(SO_RCVBUF));
	    printf("ClusterGL: UDP buffer size is %d\n", setBufferSize(SO_RCVBUF, 300000));

}

int Client::bufferSize(int buf)
  {
    int value = 0;
    socklen_t size = sizeof(value);

    getsockopt(multi_fd, SOL_SOCKET, buf,&value, &size);

    return value;
  }

int Client::setBufferSize(int buf, int bytes)
  {
    setsockopt(multi_fd, SOL_SOCKET, buf, &bytes, sizeof(buf));

    return bufferSize(buf);
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

unsigned char buffer[16*1024*1024];
int offset=0;
int length=0;

bool last_packet = false;

bool Client::pullData(void)
{
	last_packet = false;
	/* reset counter values */
	clientFrameNumber++;
	clientOffsetNumber = 0;
	gettimeofday(&start, NULL);

	length=0;
	//printf("frame: %d reading unknown total bytes!\n", clientFrameNumber);

	/* Now read packets */
	while(!last_packet)
	{
	//	printf("reading packets for frame: %d, offset %d\n", clientFrameNumber, clientOffsetNumber);
		length+=readMulticastPacket(&buffer[0], MAX_CONTENT);	
	//	printf("returned!\n");
	}
	offset=0;
//	fprintf(stderr,"Frame: %d Size: %d, clientOffsetNumber %d\n", clientFrameNumber, length, clientOffsetNumber);
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

int Client::readMulticastPacket(unsigned char *buf, size_t maxsize)
{
	/* storage for full packet to read one datagram */
	unsigned char * fullPacket = (unsigned char *) malloc(MAX_PACKET_SIZE);
	
	multicast_header multicastPacket;

	int ret=recv(multi_fd, fullPacket, MAX_PACKET_SIZE,0);
	
	memcpy(&multicastPacket, fullPacket, sizeof(multicast_header));

	/* if the server is ahead a frame from us, PANIC! */
	if(multicastPacket.frameNumber > clientFrameNumber) {
		printf("frame behind! PANIC!\n");
		//sendTCP_NACK();
		free(fullPacket);
		return 0;
	}

	/* if the server is behind us, ignore it */
  	if(multicastPacket.frameNumber < clientFrameNumber) {
		printf("frame infront, don't panic!\n");
	 	 free(fullPacket);
		return 0;
	}

	/* if the server is on the same frame, but ahead, NACK! */
	if (clientOffsetNumber < multicastPacket.offsetNumber) {
		printf("offset behind! %d < %d\n", clientOffsetNumber, multicastPacket.offsetNumber);		
		sendTCP_NACK();
		free(fullPacket);
		return 0;
		}
		
	/* if the server is on the same frame, but behind, ignore it */
	if(clientOffsetNumber > multicastPacket.offsetNumber) {
		printf("ignoring Packet: %d > %d\n", clientOffsetNumber, multicastPacket.offsetNumber);
		free(fullPacket);
		return 0;
	}
	
	/* if everything above did not return, then it is the next packet we want. Copy payload data */
	memcpy(buf+clientOffsetNumber, fullPacket+sizeof(multicast_header), multicastPacket.packetSize);

	clientOffsetNumber += multicastPacket.packetSize;
	
	/* check if final packet */
	if(CHECK_BIT(multicastPacket.packetFlags, FINPOS)) {
		last_packet = true;
	}
	/* check if ACK needs to be sent */
	if(CHECK_BIT(multicastPacket.packetFlags, RQAPOS)) {
		multicastPacket.offsetNumber += multicastPacket.packetSize;
		//printf("sending ACK! Frame %d Offset %d\n", multicastPacket.frameNumber, multicastPacket.offsetNumber);
		/* set flags */
	    short flags = 1 << ACKPOS;

		multicastPacket.packetFlags = flags;
    	/* send it */
	    write(tcp_fd, (const unsigned char *) &multicastPacket, sizeof(multicast_header));
	}
	free(fullPacket);

	/* Return the amount of payload */
	return multicastPacket.packetSize;
}

/*********************************************************
	TCP Control messaging
*********************************************************/

void Client::sendTCP_NACK()
{
	/* check that we're not spamming NACKS */
	gettimeofday(&end, NULL);

	seconds  = end.tv_sec  - start.tv_sec;
	useconds = end.tv_usec - start.tv_usec;

	mtime = ((seconds) * 1000 + useconds/1000.0) + 0.5;
	printf("trying to send a NACK: %d\n", mtime);	
	if(mtime > 100) {
		printf("sending NACK! frame %d offset: %d\n", clientFrameNumber, clientOffsetNumber);
		/* set flags */
		short flags = 0;
		flags |=  1 << NACKPOS;
		multicast_header NACKheader;
		
		NACKheader.packetFlags = flags;
	
		/* fill in header values */
		NACKheader.frameNumber = clientFrameNumber;
		NACKheader.offsetNumber = clientOffsetNumber;
		
		/* send it */
		write(tcp_fd, &NACKheader, sizeof(multicast_header));

		gettimeofday(&start, NULL);
	}
}

int Client::writeData(void *buf, size_t count)
{
	//printf("writing data, size: %d\n", count);
	int value;
	memcpy(&value, buf, sizeof(int));
	//printf("data contents: %d\n", value);
	int remaining = count;
	int ret = 0;
	while(remaining > 0) {
		ret = write(tcp_fd, (const unsigned char *) buf+count-remaining, remaining);
		remaining -= ret;
	}
	return count;
}

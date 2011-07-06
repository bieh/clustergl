/********************************************************
	Headers
********************************************************/

#include "main.h"

const int recieveBufferSize = 268435456;
//const int recieveBufferSize = sizeof(Instruction) * MAX_INSTRUCTIONS;
uint32_t iRecieveBufPos = 0;
uint32_t bytesRemaining = 0;;

//big buffer
static byte mRecieveBuf[recieveBufferSize];

/*********************************************************
	Net Server Module
*********************************************************/

NetSrvModule::NetSrvModule()
{

	if ((mSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		LOG("Failed to create socket\n");
		exit(1);
	}

	struct sockaddr_in addr, clientaddr;

	//Construct the server sockaddr_in structure
	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(gConfig->serverPort);

	//set TCP options
	int one = 1;
	setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
	setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

	//Bind the server socket
	if (bind(mSocket, (struct sockaddr *) &addr,
			sizeof(addr)) < 0) {
		LOG("Failed to bind the server socket NetSrvModule\n");
		exit(1);
	}
	//Listen
	if (listen(mSocket, 1) < 0) {
		LOG("Failed to listen on server socket\n");
		exit(1);
	}
	
	LOG("Waiting for connection on port %d\n", gConfig->serverPort);

	unsigned int clientlen = sizeof(clientaddr);
	int client = 0;
	// Wait for client connection
	if ((client = accept(mSocket, (struct sockaddr *) &clientaddr, &clientlen)) < 0) {
		LOG("Failed to accept client connection\n");
		exit(1);
	}
	mClientSocket = new BufferedFd(client);
	LOG("%s connected\n", string(inet_ntoa(clientaddr.sin_addr)).c_str() );
}


/*********************************************************
	Net Server Process Instructions
*********************************************************/

bool NetSrvModule::process(list<Instruction> &list)
{
	//Read instructions off the network and insert them into the list
	//First read the uint32 that tells us how many instructions we will have
	uint32_t num = 0;

	int len = internalRead((byte *)&num, sizeof(uint32_t));
	if(len != sizeof(uint32_t) ) {
		LOG("Read error\n");
		return false;
	}

	for(uint32_t x=0;x<num;x++) {
		Instruction i;

		int r = internalRead((byte *)&i, sizeof(Instruction));
		if(r != sizeof(Instruction)) {
			LOG("Read error (%d)\n", r);
			return false;
		}

		//Now see if we're expecting any buffers
		for(int n=0;n<3;n++) {
			int l = i.buffers[n].len;

			if(l > 0) {
				i.buffers[n].buffer = (byte *) malloc(l);
				i.buffers[n].needClear = true;
				internalRead(i.buffers[n].buffer, l);
			}
		}		
	
		list.push_back(i);		
	}

	return true;
}


void NetSrvModule::reply(Instruction *instr, int i)
{
	internalWrite(instr->buffers[i].buffer, instr->buffers[i].len);
}


/*********************************************************
	Net Server Sync
*********************************************************/

bool NetSrvModule::sync()
{	
/*
	if(mClientSocket->read((byte *)&a, sizeof(uint32_t)) != sizeof(uint32_t)) {
		LOG("Connection problem NetSrvModule (didn't recv sync)!\n");
		return false;
	}
	if(mClientSocket->write((byte *)&a, sizeof(uint32_t)) != sizeof(uint32_t)) {
		LOG("Connection problem NetSrvModule (didn't send sync)!\n");
		return false;
	}
*/	

	return true;
}


/*********************************************************
	Net Server Run Decompression
*********************************************************/

int NetSrvModule::internalRead(byte *input, int nByte)
{
	//LOG("Read: %d (%d)\n", nByte, bytesRemaining);
	
	if(bytesRemaining <= 0){
		recieveBuffer();
	}
	
	memcpy(input, mRecieveBuf + iRecieveBufPos, nByte);
	iRecieveBufPos += nByte;
	bytesRemaining -= nByte;

	return nByte;
}


void NetSrvModule::recieveBuffer(void)
{	
	int compSize = 0;	
	
	//first read the original number of bytes coming
	mClientSocket->read((byte *)&bytesRemaining, sizeof(uint32_t));
	
	//then read the buffer
	mClientSocket->read(mRecieveBuf, bytesRemaining);
	iRecieveBufPos = 0;
	
	//LOG("bytesRemaining = %d\n", bytesRemaining);
}


/*********************************************************
	Net Server Run Compression
*********************************************************/

int NetSrvModule::internalWrite(byte *input, int nByte)
{
	int ret = mClientSocket->write(input, nByte);
	return ret;
}

/********************************************************
	Headers
********************************************************/

#include "main.h"
#include "multicast.h"

#include <zconf.h>
#include <zlib.h>
#include <netinet/tcp.h>

/********************************************************
	Main Globals (Loaded from config file)
********************************************************/

extern bool usingSendCompression;
extern bool usingReplyCompression;
extern int port;

/*********************************************************
	Net Server Globals
*********************************************************/

NetCompressModule *compressor;
Client *client;
int prevInstruction = 0;

const int recieveBufferSize = sizeof(Instruction) * MAX_INSTRUCTIONS;
uint32_t iRecieveBufPos = 0;
uint32_t bytesRemaining = 0;;

bool multi = true;

//big buffer
byte mRecieveBuf[recieveBufferSize];

/*********************************************************
	Net Server Module
*********************************************************/

NetSrvModule::NetSrvModule()
{
	//reset BPS calculations
	frames = 0;
	netBytes = 0;
	netBytes2 = 0;

	if(usingSendCompression || usingReplyCompression) {
		compressor = new NetCompressModule();
	}

	if(multi) {
	client = new Client();
	}
	else {
		if ((mSocket = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
			LOG("Failed to create socket\n");
			exit(1);
		}

		struct sockaddr_in addr, clientaddr;

		//Construct the server sockaddr_in structure
		memset(&addr, 0, sizeof(addr));
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = htons(port);

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
}


/*********************************************************
	Net Server Process Instructions
*********************************************************/

bool NetSrvModule::process(list<Instruction> &list)
{
	//Read instructions off the network and insert them into the list
	//First read the uint32 that tells us how many instructions we will have
	uint32_t num = 0;

	int len = myRead((byte *)&num, sizeof(uint32_t));
	int a=sizeof(uint32_t);
	if(len < a ) {
		LOG("Read error\n");
		return false;
	}

	// Variables for processing Deltas
	std::list<Instruction>::iterator pIter = (*prevFrame).begin();

	for(uint32_t x=0;x<num;x++) {
		Instruction i;

		int r = myRead((byte *)&i, sizeof(Instruction));
		if(r != sizeof(Instruction)) {
			LOG("Read error (%d)\n", r);
			LOG("Last Instruction %d, %d, \n", prevInstruction, x);
			perror("NetSrvMod Instruction");
			return false;
		}
		else {
			prevInstruction = i.id;
		}
		//Now see if we're expecting any buffers
		for(int n=0;n<3;n++) {
			int l = i.buffers[n].len;

			if(l > 0) {
				i.buffers[n].buffer = (byte *) malloc(l);
				i.buffers[n].needClear = true;
				myRead(i.buffers[n].buffer, l);
			}
		}
		if(i.id == CGL_REPEAT_INSTRUCTION) {
			//decrease num, as we won't need to read these instructions from the socket
			num -= i.args[0];
			for (uint32_t a = 0;a <(uint32_t)i.args[0];a++) {
				Instruction p;

				p.id = pIter->id;
				for (int j =0;j<MAX_ARG_LEN;j++)
					p.args[j] = pIter->args[j];

				//Now see if we're expecting any buffers
				for(int n=0;n<3;n++) {
					uint32_t l = pIter->buffers[n].len;

					if(l > 0) {
						p.buffers[n].buffer = (byte *) malloc(l);
						p.buffers[n].needClear = true;
						p.buffers[n].len = l;
						//TODO fix me!
						if(!(pIter->buffers[n].buffer)) {
							LOG("copying a cleared buffer, something wrong here!!\n");
						}
						memcpy((byte *)(p.buffers[n].buffer), &(*pIter->buffers[n].buffer),l);
					}
				}
				list.push_back(p);
				if (pIter != (*prevFrame).end())
					pIter++;
			}
			//decrease x, as CGL_REPEAT_INSTRUCTION does not count as an instruction
			x--;
		}
		else {
			list.push_back(i);
			if (pIter != (*prevFrame).end()) pIter++;
		}
	}
	return true;
}


void NetSrvModule::reply(Instruction *instr, int i)
{
	myWrite(instr->buffers[i].buffer, instr->buffers[i].len);
}


/*********************************************************
	Net Server Sync
*********************************************************/

bool NetSrvModule::sync()
{
	netBytes += sizeof(uint32_t);
	netBytes2 += sizeof(uint32_t);
	uint32_t a;
	if(multi) {
		LOG("syncing NetSrvModule::sync\n");
		if(client->readData((byte *)&a, sizeof(uint32_t)) != sizeof(uint32_t)) {
			LOG("Connection problem NetSrvModule (didn't recv sync)!\n");
			return false;
		}
		if(client->writeData((byte *)&a, sizeof(uint32_t)) != sizeof(uint32_t)) {
			LOG("Connection problem NetSrvModule (didn't send sync)!\n");
			return false;
		}
	}
	else {
		if(mClientSocket->read((byte *)&a, sizeof(uint32_t)) != sizeof(uint32_t)) {
			LOG("Connection problem NetSrvModule (didn't recv sync)!\n");
			return false;
		}
		if(mClientSocket->write((byte *)&a, sizeof(uint32_t)) != sizeof(uint32_t)) {
			LOG("Connection problem NetSrvModule (didn't send sync)!\n");
			return false;
		}
	}

	return true;
}


/*********************************************************
	Net Server Run Decompression
*********************************************************/

int NetSrvModule::myRead(byte *input, int nByte)
{

	if(bytesRemaining == 0)
		recieveBuffer();
	memcpy(input, mRecieveBuf + iRecieveBufPos, nByte);
	iRecieveBufPos += nByte;
	bytesRemaining -= nByte;

	return nByte;
}


void NetSrvModule::recieveBuffer(void)
{	
	int compSize = 0;	
	if(multi) {
		if(usingSendCompression) {
			client->readData((byte *)&bytesRemaining, sizeof(uint32_t));
			client->readData((byte *)&compSize, sizeof(uint32_t));
			Bytef *in = (Bytef*) malloc(compSize);	
			client->readData(in, compSize);
			compressor->myDecompress(mRecieveBuf, bytesRemaining, in, compSize);
			iRecieveBufPos = 0;
			free(in);
		}
		else {
			client->readData((byte *)&bytesRemaining, sizeof(uint32_t));
			client->readData(mRecieveBuf, bytesRemaining);
			iRecieveBufPos = 0;
		}
	}
	else {
		if(usingSendCompression) {
			//first read the original number of bytes coming
			mClientSocket->read((byte *)&bytesRemaining, sizeof(uint32_t));

			//read the size of the compressed packet

			mClientSocket->read((byte *)&compSize, sizeof(uint32_t));

			//LOG("recieving buffer of size: %d!\n", bytesRemaining);
			//then read in that many bytes
			Bytef *in = (Bytef*) malloc(compSize);
			mClientSocket->read(in, compSize);

			compressor->myDecompress(mRecieveBuf, bytesRemaining, in, compSize);
			iRecieveBufPos = 0;
			free(in);
		}
		else {
			//first read the original number of bytes coming
			mClientSocket->read((byte *)&bytesRemaining, sizeof(uint32_t));
			//read the buffer
			mClientSocket->read(mRecieveBuf, bytesRemaining);
			iRecieveBufPos = 0;
		}
	}
}


/*********************************************************
	Net Server Run Compression
*********************************************************/

int NetSrvModule::myWrite(byte *input, int nByte)
{

	if(multi) {
		if(usingReplyCompression) {
			//create room for new compressed buffer
			uLongf CompBuffSize = (uLongf)(nByte + (nByte * 0.1) + 12);
			Bytef *out = (Bytef*) malloc(CompBuffSize);
			int newSize = compressor->myCompress(input, nByte, out);

			//write the size of the next instruction
			if(!client->writeData((byte*)&newSize, sizeof(uint32_t))) {
				LOG("Connection problem!\n");
			}
			//write the old size of the next instruction
			if(!client->writeData( (byte*)&nByte, sizeof(uint32_t))) {
				LOG("Connection problem!\n");
			}

			//write the compressed instruction
			int ret =client->writeData(out, newSize);
			//cheack and set return value to keep caller happy
			if(ret == newSize)
				ret = nByte;

			//calculate bandwidth requirements
			netBytes += nByte;
			netBytes2 += sizeof(uint32_t) * 2;
			netBytes2 += newSize;
			free(out);

			return ret;
		}
		else {
			int ret = client->writeData(input, nByte);
			netBytes += nByte;
			netBytes2 += nByte;
			return ret;
		}
	}
	else {
		if(usingReplyCompression) {
			//create room for new compressed buffer
			uLongf CompBuffSize = (uLongf)(nByte + (nByte * 0.1) + 12);
			Bytef *out = (Bytef*) malloc(CompBuffSize);
			int newSize = compressor->myCompress(input, nByte, out);

			//write the size of the next instruction
			if(!mClientSocket->write( (byte*)&newSize, sizeof(uint32_t))) {
				LOG("Connection problem!\n");
			}
			//write the old size of the next instruction
			if(!mClientSocket->write( (byte*)&nByte, sizeof(uint32_t))) {
				LOG("Connection problem!\n");
			}

			//write the compressed instruction
			int ret =mClientSocket->write(out, newSize);
			//cheack and set return value to keep caller happy
			if(ret == newSize)
				ret = nByte;

			//calculate bandwidth requirements
			netBytes += nByte;
			netBytes2 += sizeof(uint32_t) * 2;
			netBytes2 += newSize;
			free(out);

			return ret;
		}
		else {
			int ret = mClientSocket->write(input, nByte);
			netBytes += nByte;
			netBytes2 += nByte;
			return ret;
		}
	}
}

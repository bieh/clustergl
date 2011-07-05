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

extern bool symphony;
extern bool usingSendCompression;
extern bool usingReplyCompression;
extern bool useRepeat;
extern int port;
extern bool useSYMPHONYnodes[5];
extern string addresses[5];
extern bool multicast;

/*********************************************************
	Net Client Globals
*********************************************************/

int bufferSavings = 0;
int incomingSize = 0;
int outgoingSize = 0;
NetCompressModule *compressor2;
Server *server;

const int sendBufferSize = 268435456;
//const int sendBufferSize = sizeof(Instruction) * MAX_INSTRUCTIONS;
uint32_t iSendBufPos = 0;
uint32_t bytesLeft = sendBufferSize;

//big buffer to store instructions before sending
byte mSendBuf[sendBufferSize];

/*********************************************************
	Net Client Module
*********************************************************/

NetClientModule::NetClientModule()
{

	//set the number of sockets to create/use
	if(symphony) numConnections = 5;
	else numConnections = 1;

	if(usingSendCompression | usingReplyCompression) {
		//make a compressor object (if required)
		compressor2 = new NetCompressModule();
	}
	
	if(multicast) {
		server = new Server();
	}
	else {
		//Make each socket and connect
		for(int i = 0; i < numConnections; i++) {
			if(useSYMPHONYnodes[i]) mSocket[i] = socket(PF_INET, SOCK_STREAM, 0);
		}

		//set TCP options for each socket
		int one = 1;
		for(int i = 0; i < numConnections; i++) {
			if(useSYMPHONYnodes[i]) {
				setsockopt(mSocket[i], IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
				setsockopt(mSocket[i], SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
				if(mSocket[i] == 0) {
					LOG("Couldn't make socket!\n");
					return;
				}
			}
		}

		struct sockaddr_in mAddr[5];

		//connect each socket to server
		for(int i = 0; i < numConnections; i++) {
			if(useSYMPHONYnodes[i]) {
				memset(&mAddr[i], 0, sizeof(mAddr[i]));
				mAddr[i].sin_family = AF_INET;
				mAddr[i].sin_addr.s_addr = inet_addr(addresses[i].c_str());
				mAddr[i].sin_port = htons(port);
				//Establish connection
				if (connect(mSocket[i],
					(struct sockaddr *) &mAddr[i],
				sizeof(mAddr[i])) < 0) {
					LOG("Failed to connect with server '%s:%d'\n", addresses[i].c_str(), port);
					mSocket[i] = 0;
					return;
				}
				LOG("Connected to remote pipeline on %s:%d\n", addresses[i].c_str(), port);
			}
		}
	}

	//reset BPS calculations
	frames = 0;
	netBytes = 0;
	netBytes2 = 0;

}


/*********************************************************
	Net Client Process Instrctions
*********************************************************/

bool NetClientModule::process(list<Instruction> &list)
{
	
	//First send the total number
	uint32_t num = list.size();
	//LOG("********num instructions netClient: %d!\n", num);
	fflush(stdout);
	if(multicast) {
		netBytes += sizeof(uint32_t);
	} 
	else {
		netBytes += sizeof(uint32_t) * numConnections;
	}
	if(!myWrite(&num, sizeof(uint32_t))) {
		LOG("Connection problem!\n");
		return false;
	}

	//Count of duplicate instructions
	uint32_t sameCount = 0;

	//Now send the instructions
	int counter = 0;
	for(std::list<Instruction>::iterator iter = list.begin(), pIter = (*prevFrame).begin();
	iter != list.end(); iter++) {
								 //yuck
		Instruction *i = &(*iter);
		bool mustSend = false;

		for(int n=0;n<3;n++) {
			if(i->buffers[n].len > 0) {
				//If we expect a buffer back then we must send Instruction
				if(i->buffers[n].needReply) mustSend = true;
			}
		};

		if (useRepeat			 //value from config to enable/disable deltas
			&& i->id == pIter->id//if the instruction has the same id as previous
			&& !mustSend && i->id//mustSend is set when expecting a reply
		//	&& sameCount < 150//stops sameBuffer filling up indefinitely (is this needed?)
			&& i->id != 1499
		) {
			//assume the instruction is the same
			bool same = true;
			//compare all arguments
			for (int a=0;a<MAX_ARG_LEN;a++) {
				if (i->args[a]!=pIter->args[a]) {
					same = false;
					break;
				}
			}

			//if arguments the same, compare all buffers
			if (same) {
				for (int a=0;a<3;a++) {
					if (i->buffers[a].len > 0) {
						if(i->buffers[a].len != pIter->buffers[a].len) {
							same = false;
							break;
						}
						else if (i->buffers[a].needClear != pIter->buffers[a].needClear) {
							same = false;
							break;
						}
						else if (memcmp(i->buffers[a].buffer,pIter->buffers[a].buffer,i->buffers[a].len) != 0) {
							same = false;
							break;
						}
					}
				}
			}

			//if arguments and buffers match, the instruction is identical
			if (same) {
				sameCount++;
				if (pIter != (*prevFrame).end())
					pIter++;
				continue;
			}
		}
		//printf("same count: %d\n", sameCount);
		if (sameCount> 0) {		 // send a count of the duplicates before this instruction
			Instruction * skip = (Instruction *)malloc(sizeof(Instruction));
			if (skip == 0) {
				LOG("ERROR: Out of memory\n");
				exit(-1);
			}
			skip->id = CGL_REPEAT_INSTRUCTION;
			memcpy(skip->args, &sameCount, sizeof(uint32_t));
			//printf("sameCount: %d\n", sameCount);
			for(int i=0;i<3;i++) {
				skip->buffers[i].buffer = NULL;
				skip->buffers[i].len = 0;
				skip->buffers[i].needClear = false;
			}
			if(multicast) {
				netBytes += sizeof(Instruction);
			}
			else {
				netBytes += sizeof(Instruction) * numConnections;
			}
			if(myWrite(skip, sizeof(Instruction))!= sizeof(Instruction)) {
				LOG("Connection problem (didn't send instruction)!\n");
				return false;
			}
			sameCount = 0;		 // reset the count and free the memory
			free(skip);
		}

		// now send the new instruction
		netBytes += sizeof(Instruction) * numConnections;
		if(myWrite(i, sizeof(Instruction)) != sizeof(Instruction)) {
			LOG("Connection problem (didn't send instruction)!\n");
			return false;
		}

		//Now see if we need to send any buffers
		for(int n=0;n<3;n++) {
			int l = i->buffers[n].len;

			if(l > 0) {
				netBytes += l * numConnections;
				if(myWrite(i->buffers[n].buffer, l) != l) {
					LOG("Connection problem (didn't write buffer %d)!\n", l);
					return false;
				}
				//And check if we're expecting a buffer back in response
				if(i->buffers[n].needReply) {

					sendBuffer();
					if(int x = myRead(i->buffers[n].buffer, l) != l) {
						LOG("Connection problem NetClient (didn't recv buffer %d got: %d)!\n", l, x);
						return false;
					}
					//LOG("got buffer back!\n");
				}
			}
		}
		if (pIter != (*prevFrame).end()) pIter++;
		counter++;
	}

	//send any instructions that are remaining in the CGL_REPEAT_INSTRUCTION buffer
	if (sameCount> 0) {			 // send a count of the duplicates before this instruction
		Instruction * skip = (Instruction *)malloc(sizeof(Instruction));
		if (skip == 0) {
			LOG("ERROR: Out of memory\n");
			exit(-1);
		}
		skip->id = CGL_REPEAT_INSTRUCTION;
		skip->args[0] = (uint32_t) sameCount;
		//printf("sameCount: %d\n", sameCount);
		for(int i=0;i<3;i++) {
			skip->buffers[i].buffer = NULL;
			skip->buffers[i].len = 0;
			skip->buffers[i].needClear = false;
		}
		netBytes += sizeof(Instruction) * numConnections;
		if(myWrite(skip, sizeof(Instruction))!= sizeof(Instruction)) {
			LOG("Connection problem (didn't send instruction)!\n");
			return false;
		}
		sameCount = 0;			 // reset the count and free the memory
		free(skip);
	}
	sendBuffer();

	return true;
}


/*********************************************************
	Net Client Sync
*********************************************************/

bool NetClientModule::sync()
{
	//ensure all remaining messages are sent instantly
//	sendBuffer(); TODO FIXME TESTING
	uint32_t * a = (uint32_t *)malloc(sizeof(uint32_t));
	*a = 987654;
	if(multicast) {
		if(server->writeData(a, sizeof(uint32_t)) != sizeof(uint32_t)) {
			LOG("Connection problem NetClientModule (didn't send sync)!\n");
			return false;
		}
		server->flushData();
		if(server->readData(a, sizeof(uint32_t)) != sizeof(uint32_t)) {
			LOG("Connection problem NetClientModule (didn't recv sync)!\n");
			return false;
		}
		if (*a!=987654) {
			LOG("Sync returned unexpected magic (%08x)\n",*a);
			return false;
		}
	}
	else {
		netBytes += sizeof(uint32_t) * numConnections;
		netBytes2 += sizeof(uint32_t) * numConnections;
		for(int i = 0; i < numConnections; i++) {
			if(useSYMPHONYnodes[i]) {
				int fd = mSocket[i];
				if(write(fd, a, sizeof(uint32_t)) != sizeof(uint32_t)) {
					LOG("Connection problem NetClientModule (didn't send sync)!\n");
					return false;
				}

				if(read(fd, a, sizeof(uint32_t)) != sizeof(uint32_t)) {
					LOG("Connection problem NetClientModule (didn't recv sync)!\n");
					return false;
				}
				if (*a!=987654)
					return false;
			}
		}
	}
	free(a);
	return true;
}


/*********************************************************
	Net Client Run Compression
*********************************************************/

int NetClientModule::myWrite(void* buf, int nByte)
{

	if(bytesLeft - nByte > 0) {
		memcpy(mSendBuf + iSendBufPos, buf, nByte);
		iSendBufPos += nByte;
		bytesLeft -= nByte;
	}
	else {
		sendBuffer();
	}
	return nByte;
}


int NetClientModule::myWrite(void* buf, unsigned nByte)
{

	if(bytesLeft - nByte > 0) {
		memcpy(mSendBuf + iSendBufPos, buf, nByte);
		iSendBufPos += nByte;
		bytesLeft -= nByte;
	}
	else {
		sendBuffer();
	}
	return nByte;
}


int NetClientModule::myWrite(void* buf, long unsigned nByte)
{
	if(bytesLeft - nByte > 0) {
		memcpy(mSendBuf + iSendBufPos, buf, nByte);
		iSendBufPos += nByte;
		bytesLeft -= nByte;
	}
	else {
		sendBuffer();
	}
	return nByte;
}


void NetClientModule::sendBuffer()
{
	if(iSendBufPos > 0) {
		if(usingSendCompression) {
			//create room for new compressed buffer
			//TODO: change the size CompBuffSize according to compress method
			uLongf CompBuffSize = (uLongf)(iSendBufPos + (iSendBufPos * 0.1) + 12);
			Bytef *out = (Bytef*) malloc(CompBuffSize);
			uint32_t newSize = compressor2->myCompress(mSendBuf, iSendBufPos, out);
			if(multicast) {
				//first write the original size of the buffer
				if(!server->writeData(&iSendBufPos, sizeof(uint32_t))) {
					LOG("Connection problem!\n");
				}

				//then write the compressed size of the buffer
				if(!server->writeData(&newSize, sizeof(uint32_t))) {
					LOG("Connection problem!\n");
				}

				//then write the actual buffer
				if(!server->writeData(out, newSize)) {
					LOG("Connection problem!\n");
				}
				server->flushData();
			}
			else {
				//send the compressed buffer to each socket
				for(int i = 0; i < numConnections; i++) {
					if(useSYMPHONYnodes[i]) {
						uint32_t fd = mSocket[i];

						//first write the original size of the buffer
						if(!write(fd, &iSendBufPos, sizeof(uint32_t))) {
							LOG("Connection problem!\n");
						}

						//then write the compressed size of the buffer
						if(!write(fd, &newSize, sizeof(uint32_t))) {
							LOG("Connection problem!\n");
						}

						//then write the actual buffer
						if(!write(fd, out, newSize)) {
							LOG("Connection problem!\n");
						}
					}

				}
			}
			free(out);
			iSendBufPos = 0;
			bytesLeft = sendBufferSize;
			netBytes2 += (newSize + (sizeof(uint32_t) * 2)) * numConnections;
		}
		else {
			if(multicast) {
				//first write the original size of the buffer
				if(!server->writeData(&iSendBufPos, sizeof(uint32_t))) {
					LOG("Connection problem!\n");
				}

				//then write the actual buffer
				if(!server->writeData(mSendBuf, iSendBufPos)) {
					LOG("Connection problem!\n");
				}		
				server->flushData();
			}
			else {
				//send the buffer to each socket
				for(int i = 0; i < numConnections; i++) {
					if(useSYMPHONYnodes[i]) {
						uint32_t fd = mSocket[i];

						//first write the original size of the buffer
						if(!write(fd, &iSendBufPos, sizeof(uint32_t))) {
							LOG("Connection problem!\n");
						}

						//then write the actual buffer
						if(!write(fd, mSendBuf, iSendBufPos)) {
							LOG("Connection problem!\n");
						}
					}
				}
			}
			 //should + sizeof(int) * numConnections overhead
			netBytes2 += iSendBufPos * numConnections;
			bytesLeft = sendBufferSize;
			iSendBufPos = 0;
		}
		
	}
	//server->flushData();
}


/*********************************************************
	Net Client Run Decompression
*********************************************************/

int NetClientModule::myRead(void *buf, size_t count)
{
	size_t *a = &count;
	uint32_t compressedSize = 0;
	uint32_t origSize = 0;
	Bytef *in = (Bytef*) malloc(compressedSize);
	uint32_t ret[5]={0,};
	if(multicast) {
		if(usingReplyCompression) {
			//read the size of the compressed packet
			int c = server->readData(&compressedSize, sizeof(uint32_t));
			//read the size of the original packet
			int d = server->readData(&origSize, sizeof(uint32_t));

			//read the size of the incoming packet
			//then read the compressed packet data and uncompress
				
			ret[0] = server->readData(in, compressedSize);
			compressor2->myDecompress(buf, count, in, compressedSize);
			free(in);
		}
		else {
			ret[0] = server->readData(buf, count);
		}
	}
	else {
		//read in replys from each socket
		for(int i = 0; i < numConnections; i++) {
			if(useSYMPHONYnodes[i]) {
				int fd = mSocket[i];

				if(usingReplyCompression) {

					//read the size of the compressed packet
					int c = read(fd, &compressedSize, sizeof(uint32_t));
					//read the size of the original packet
					int d = read(fd, &origSize, sizeof(uint32_t));

					//read the size of the incoming packet
					//then read the compressed packet data and uncompress
				
					ret[i] = read(fd, in, compressedSize);


					if(ret[i] == compressedSize)
						ret[i] = origSize;
					else
						return 0;	 //return error
					if(i == 0)		 //only bother to decompress/process the first one
						compressor2->myDecompress(buf, count, in, compressedSize);
					free(in);
				}
				else {
					if(i == 0) {
						int remaining = count;
						while(remaining > 0) {
							ret[i] = read(fd, buf+(count-remaining), remaining);
							remaining -= ret[i];
						}
						ret[0] = count;
					}
					else {
						byte * tempBuf = (byte *) malloc(count);
						int remaining = count;
						while(remaining > 0) {
							ret[i] = read(fd, tempBuf+(count-remaining), remaining);
							remaining -= ret[i];
						}
						free(tempBuf);
					}
				}
			}
		}
	}
	return ret[0];
}

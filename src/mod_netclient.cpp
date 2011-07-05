/*******************************************************************************
	Network client module
*******************************************************************************/

#include "main.h"

#include <zconf.h>
#include <zlib.h>
#include <netinet/tcp.h>

const int SEND_BUFFER_SIZE = 268435456;

uint32_t iSendBufPos = 0;
uint32_t bytesLeft = SEND_BUFFER_SIZE;

//big buffer to store instructions before sending
byte mSendBuf[SEND_BUFFER_SIZE];

/*******************************************************************************
	Network client module connect / setup
*******************************************************************************/
NetClientModule::NetClientModule()
{    
	//Make each socket
	for(int i=0;i<gConfig->numRenderers;i++) {
		mSocket.push_back(socket(PF_INET, SOCK_STREAM, 0));
	}

	//set TCP options for each socket
	int one = 1;
	for(int i=0;i<(int)mSockets.size();i++){		
		setsockopt(mSocket[i], IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
		setsockopt(mSocket[i], SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
		if(mSocket[i] == 0){
			LOG("Couldn't make socket!\n");
			return;
		}
	}

	//connect each socket to server
	for(int i=0;i<(int)mSockets.size();i++) {
		memset(&mAddr, 0, sizeof(mAddr));
		mAddr.sin_family = AF_INET;
		mAddr.sin_addr.s_addr = inet_addr(addresses[i].c_str());
		mAddr.sin_port = htons(port);
		//Establish connection
		if (connect(mSocket[i],
		            (struct sockaddr *) &mAddr,
		            sizeof(mAddr)) < 0) {
			LOG("Failed to connect with server '%s:%d'\n", addresses[i].c_str(), port);
			mSocket[i] = 0;
			exit(1);
		}
		LOG("Connected to remote pipeline on %s:%d\n", addresses[i].c_str(), port);
		}
	}

    iByteCount = 0;
}



/*******************************************************************************
	Send each instruction
*******************************************************************************/
bool NetClientModule::process(list<Instruction> &list)
{
	
	//First send the total number
	uint32_t num = list.size();
		
	if(!myWrite(&num, sizeof(uint32_t))) {
		LOG("Connection problem!\n");
		return false;
	}

	//Now send the instructions
	int counter = 0;
	for(std::list<Instruction>::iterator iter = list.begin(), 
	    pIter = (*prevFrame).begin();iter != list.end(); iter++) {								
		Instruction *i = &(*iter);
		
		bool mustSend = false;

		for(int n=0;n<3;n++) {
			if(i->buffers[n].len > 0) {
				//If we expect a buffer back then we must send everything now
				if(i->buffers[n].needReply) mustSend = true;
			}
		};

		// now send the new instruction
		if(internalWrite(i, sizeof(Instruction)) != sizeof(Instruction)) {
			LOG("Connection problem (didn't send instruction)!\n");
			return false;
		}

		//Now see if we need to send any buffers
		for(int n=0;n<3;n++) {
			int l = i->buffers[n].len;

			if(l > 0) {
				netBytes += l * numConnections;
				if(internalWrite(i->buffers[n].buffer, l) != l) {
					LOG("Connection problem (didn't write buffer %d)!\n", l);
					return false;
				}
				//And check if we're expecting a buffer back in response
				if(i->buffers[n].needReply) {

					sendBuffer();
					if(int x = internalRead(i->buffers[n].buffer, l) != l) {
						LOG("Connection problem: NetClient (didn't recv buffer %d got: %d)!\n", l, x);
						return false;
					}
					//LOG("got buffer back!\n");
				}
			}
		}
		if (pIter != (*prevFrame).end()) pIter++;
		counter++;
	}
	
	sendBuffer(); //send anything that's outstanding

	return true;
}


/*********************************************************
	Net Client Sync
*********************************************************/

bool NetClientModule::sync()
{
    LOG("NetClientModule::sync() requested, but not implemented\n");
	
	return true;
}



int NetClientModule::internalWrite(void* buf, int nByte)
{
    iByteCount += sizeof(Instruction) * numConnections;
    		
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
	LOG("NetClientModule::sendBuffer()\n");
}


/*********************************************************
	Net Client Run Decompression
*********************************************************/

int NetClientModule::internalRead(void *buf, size_t count)
{
	//Read from each client.
	LOG("NetClientModule::internalRead() requested, but not implemented\n");
}

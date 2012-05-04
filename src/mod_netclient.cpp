/*******************************************************************************
	Network client module
*******************************************************************************/

#include "main.h"


const int SEND_BUFFER_SIZE = 268435456;//1294276

uint32_t iSendBufPos = 0;
uint32_t bytesLeft = SEND_BUFFER_SIZE;

//big buffer to store instructions before sending
byte mSendBuf[SEND_BUFFER_SIZE];

int totalSent;

/*******************************************************************************
	Network client module connect / setup
*******************************************************************************/
NetClientModule::NetClientModule()
{    
	//Make each socket
	for(int i=0;i<gConfig->numOutputs;i++){	
		struct addrinfo hints, *res;
	
		memset(&hints, 0, sizeof hints);
		hints.ai_family = AF_UNSPEC;
		hints.ai_socktype = SOCK_STREAM;
		
		string addr = gConfig->outputAddresses[i];
		int port = gConfig->outputPorts[i];
		
		getaddrinfo(addr.c_str(), toString(port).c_str(), &hints, &res);
		
		int s = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
		
		if(s == 0){
			LOG("Couldn't make socket!\n");
			return;
		}
				
		//set TCP options 
		int one = 1;

		setsockopt(s, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
		setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
				
		struct sockaddr_in mAddr; 

		int c = connect(s, res->ai_addr, res->ai_addrlen);

	    if(c < 0) {
			LOG("Failed to connect with server '%s:%d' - error %s\n", 
					addr.c_str(), port, strerror( errno ));
			exit(1);
		}
		LOG("Connected to remote pipeline on %s:%d\n", addr.c_str(), port);	
	
		mSockets.push_back(s);
	}

}



/*******************************************************************************
	Send each instruction
*******************************************************************************/
bool NetClientModule::process(vector<Instruction *> *list)
{
	
	//First send the total number
	uint32_t num = list->size();
		
	if(!internalWrite(&num, sizeof(uint32_t))) {
		LOG("Connection problem!\n");
		return false;
	}

	//Now send the instructions
	int counter = 0;
	for(int n=0;n<(int)list->size();n++){
		
		Instruction *i = (*list)[n];
		
		//Check for NULL buffers and reset length if necessary
		for(int n=0;n<3;n++) {
			int l = i->buffers[n].len;

			if(l > 0 && !i->buffers[n].buffer) {
				i->buffers[n].len = 0;
			}
		}			
		
		/*
		byte len = (sizeof(Instruction) - MAX_ARG_LEN) + i->arglen;
		
		// now send the new instruction
		//first the length byte
		if(internalWrite(&len, 1) != 1) {
			LOG("Connection problem (didn't send instruction)!\n");
		}
		
		//now the struct itself	
		if(internalWrite(i, len) != len) {
			LOG("Connection problem (didn't send instruction)!\n");
			return false;
		}
		*/
		//now the struct itself	
		if(internalWrite(i, sizeof(Instruction)) != sizeof(Instruction)) {
			LOG("Connection problem (didn't send instruction)!\n");
			return false;
		}

		//Now see if we need to send any buffers
		for(int n=0;n<3;n++) {
			int l = i->buffers[n].len;

			if(l > 0) {
				
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
		counter++;
	}
	
	sendBuffer(); //send anything that's outstanding
	
	Stats::count("mod_netclient write", totalSent);
	totalSent = 0;

	return true;
}



/*******************************************************************************
 Sync
*******************************************************************************/
bool NetClientModule::sync()
{
    LOG("NetClientModule::sync() requested, but not implemented\n");
	
	return true;
}




/*******************************************************************************
 Write to the network
*******************************************************************************/
int NetClientModule::internalWrite(void* buf, int nByte)
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
	if(iSendBufPos == 0){
		return;
	}
	
	byte *mCompressedBuf = NULL;
	
	if(gConfig->networkCompression){		
		mCompressedBuf = Compression::getBuf();
		iSendBufPos = Compression::compress(mSendBuf, iSendBufPos);
	}
	
	//LOG("NetClientModule::sendBuffer(%d)\n", iSendBufPos);
	for(int i=0;i<(int)mSockets.size();i++){
		uint32_t a = write(mSockets[i], &iSendBufPos, sizeof(iSendBufPos));
		uint32_t b = 0;	
		if(!gConfig->networkCompression){		
			b = write(mSockets[i], mSendBuf, iSendBufPos);			
		}else{
			b = write(mSockets[i], mCompressedBuf, iSendBufPos);
		}
		if(a != sizeof(iSendBufPos) && b != iSendBufPos){
			LOG("Failure to send: %d/%d\n", i, iSendBufPos);
		}		
		totalSent += a;
		totalSent += b;
	}
	//LOG("sent ok\n");
	iSendBufPos = 0;
	bytesLeft = SEND_BUFFER_SIZE;
	
}



/*******************************************************************************
 Read from the network
*******************************************************************************/
int NetClientModule::internalRead(void *buf, size_t count)
{
	//LOG("About to read %d\n", count);
	uint32_t n = 0;
	//Read from each renderer
	for(int i=0;i<(int)mSockets.size();i++){
		n = 0;
		byte *d = (byte *)buf;
		while(n < count){
		    n += read(mSockets[i], d + n, count);		  
		    //LOG("%d\n", n);
		}		
	}
	//LOG("done\n");
	return n;
}

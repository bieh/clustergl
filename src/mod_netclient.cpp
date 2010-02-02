#include "main.h"
#include <zconf.h>
#include <zlib.h>
#include <netinet/tcp.h>

/*********************************************************
	Net Client Globals
*********************************************************/

int bufferSavings = 0;
int incomingSize = 0;
int outgoingSize = 0;
NetCompressModule *compressor2;
bool useSendCompression = false;
bool useRecieveCompression = false;
bool useCGLrepeat = false;

const int sendBufferSize = sizeof(Instruction) * MAX_INSTRUCTIONS;
int iSendBufPos = 0;
int bytesLeft = sendBufferSize;

//big buffer
byte mSendBuf[sendBufferSize];
 
string addresses[5] = {"127.0.0.1", "192.168.22.102", "192.168.22.103", "192.168.22.104", "192.168.22.105"};


/*********************************************************
	Net Client Module
*********************************************************/

NetClientModule::NetClientModule(int port, bool sendCompression, bool recieveCompression, int compressMethod, bool repeatInstruction){
	#ifdef SYMPHONY
		numConnections = 5;
	#else
		numConnections = 1;
	#endif

        //Make the socket and connect
	for(int i = 0; i < numConnections; i++) {
		mSocket[i] = socket(PF_INET, SOCK_STREAM, 0);
	}
	
	useSendCompression = sendCompression;
	useRecieveCompression = recieveCompression;

	useCGLrepeat = repeatInstruction;

	if(useSendCompression | useRecieveCompression) {
		//make a compressor object
		compressor2 = new NetCompressModule(compressMethod);
	}
	
	//set TCP options
	int one = 1;
	for(int i = 0; i < numConnections; i++) {
		setsockopt(mSocket[i], IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
		setsockopt(mSocket[i], SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));
		if(mSocket[i] == 0){
			LOG("Couldn't make socket!\n");
			return;
		}
	}

	
	struct sockaddr_in mAddr[5];
	
	for(int i = 0; i < numConnections; i++) {
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
	
	//reset BPS calculations
	netBytes = 0;   
	netBytes2 = 0; 

	
}

/*********************************************************
	Net Client Process Instrctions
*********************************************************/

bool NetClientModule::process(list<Instruction> &list){
	//Send all the commands in the list down the socket
	//LOG("processing:\n");
	for(int i = 0; i < numConnections; i++) {
		if(mSocket[i] == 0){
			return false;
		}
	}
	
	//First send the total number
	uint32_t num = list.size();
	//LOG("num instructions netClient: %d!\n", num);
	fflush(stdout);
	netBytes += sizeof(uint32_t) * numConnections;
	if(!myWrite(&num, sizeof(uint32_t))){
		LOG("Connection problem!\n");
		return false;
	}

	//Count of duplicate instructions
	uint32_t sameCount = 0;

	//Now send the instructions
	int counter = 0;
	for(std::list<Instruction>::iterator iter = list.begin(), pIter = (*prevFrame).begin(); 
	    iter != list.end(); iter++){
	    Instruction *i = &(*iter); //yuck
	   // if(i->id == 1499)
            //	LOG("ID:%d %d %d\n",i->id, counter, num);
	    bool mustSend = false;

	    for(int n=0;n<3;n++){		
		if(i->buffers[n].len > 0){
		  	//LOG("MustSend:%d\n",i->id);
			//If we expect a buffer back then we must send Instruction
			if(i->buffers[n].needReply) mustSend = true;
		}
	    };
	    //if(mustSend)
	//	LOG("mustSend %d!\n", counter);
	    if (i->id == pIter->id 		
		&& !mustSend && i->id 			
		  && useCGLrepeat //value from config to enable/disable deltas
		  && sameCount < 100		//stops sameBuffer filling up indefinitely
		) {
			bool same = true;
			for (int a=0;a<MAX_ARG_LEN;a++){
				if (i->args[a]!=pIter->args[a]){
					same = false;
					break;
				}
			}
			if (same){
				for (int a=0;a<3;a++) {						
					if (i->buffers[a].len > 0){
						if(i->buffers[a].len != pIter->buffers[a].len){
							same = false;
							break;
						}
						else if (i->buffers[a].needClear != pIter->buffers[a].needClear){
							same = false;
							break;
						}
						else if (memcmp(i->buffers[a].buffer,pIter->buffers[a].buffer,i->buffers[a].len) != 0){
							same = false;
							break;
						}
					}
				}
			}
			if (same) {
				sameCount++;
				if (pIter != (*prevFrame).end()) 
					pIter++;
				continue; 
			}	
	    }

	    if (sameCount> 0){ // send a count of the duplicates before this instruction
		//LOG("SKIP: %d\n", sameCount);
		Instruction * skip = (Instruction *)malloc(sizeof(Instruction));		
		if (skip == 0){
			LOG("ERROR: Out of memory\n");
			exit(-1);	
		}
		skip->id = CGL_REPEAT_INSTRUCTION;
		skip->args[0] = (uint32_t) sameCount;
		for(int i=0;i<3;i++){
			skip->buffers[i].buffer = NULL;
			skip->buffers[i].len = 0;
			skip->buffers[i].needClear = false;
		}
		netBytes += sizeof(Instruction) * numConnections;
		if(myWrite(skip, sizeof(Instruction))!= sizeof(Instruction)){
		    	LOG("Connection problem (didn't send instruction)!\n");
		    	return false;
		}
		sameCount = 0; // reset the count and free the memory
		free(skip);
	    }
	   
	    // now send the new instruction
		netBytes += sizeof(Instruction) * numConnections;
	    if(myWrite(i, sizeof(Instruction)) != sizeof(Instruction)){
	    		LOG("Connection problem (didn't send instruction)!\n");
	    		return false;
	    }
	    
	    //Now see if we need to send any buffers
		for(int n=0;n<3;n++){
			int l = i->buffers[n].len;
		
			if(l > 0){
				//LOG("buffer: %d\n", l);
				netBytes += l * numConnections;	
				if(myWrite(i->buffers[n].buffer, l) != l){
					LOG("Connection problem (didn't write buffer %d)!\n", l);
					return false;
				}
				//And check if we're expecting a buffer back in response
				if(i->buffers[n].needReply){
					
					sendBuffer();
					//LOG("sent buffer!\n");
					if(int x = myRead(i->buffers[n].buffer, l) != l){
						LOG("Connection problem (didn't recv buffer %d got: %d)!\n", l, x);
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
	    if (sameCount> 0){ // send a count of the duplicates before this instruction
		//LOG("SKIP: %d\n", sameCount);
		Instruction * skip = (Instruction *)malloc(sizeof(Instruction));		
		if (skip == 0){
			LOG("ERROR: Out of memory\n");
			exit(-1);	
		}
		skip->id = CGL_REPEAT_INSTRUCTION;
		skip->args[0] = (uint32_t) sameCount;
		for(int i=0;i<3;i++){
			skip->buffers[i].buffer = NULL;
			skip->buffers[i].len = 0;
			skip->buffers[i].needClear = false;
		}
		netBytes += sizeof(Instruction) * numConnections;
		if(myWrite(skip, sizeof(Instruction))!= sizeof(Instruction)){
		    	LOG("Connection problem (didn't send instruction)!\n");
		    	return false;
		}
		sameCount = 0; // reset the count and free the memory
		free(skip);
	    }
	    sendBuffer();

	return true;
}

/*********************************************************
	Net Client Sync
*********************************************************/

bool NetClientModule::sync(){
	int * a = (int *)malloc(sizeof(int));
	*a = 987654;
	netBytes += sizeof(int) * numConnections;

	if(myWrite(a, sizeof(int)) != sizeof(int)){
		LOG("Connection problem (didn't send sync)!\n");
		return false;
	}
	
	sendBuffer();

	if(myRead(a, sizeof(int)) != sizeof(int)){
		LOG("Connection problem (didn't recv sync)!\n");
		return false;
	}
	if (*a!=987654)
		return false;
	free(a);
	return true;
}

/*********************************************************
	Net Client Run Compression
*********************************************************/

int NetClientModule::myWrite(void* buf, int nByte){
	
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

int NetClientModule::myWrite(void* buf, unsigned nByte){

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

int NetClientModule::myWrite(void* buf, long unsigned nByte){
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

void NetClientModule::sendBuffer() {

	if(iSendBufPos > 0) {
		if(useSendCompression) {
			//LOG("sending buffer of size: %d!\n", iSendBufPos);
				//create room for new compressed buffer
			//TODO: change the size CompBuffSize according to compress method
			uLongf CompBuffSize = (uLongf)(iSendBufPos + (iSendBufPos * 0.1) + 12);
			Bytef *out = (Bytef*) malloc(CompBuffSize);
			int newSize = compressor2->myCompress(mSendBuf, iSendBufPos, out);
			
			for(int i = 0; i < numConnections; i++) {
				int fd = mSocket[i];
				//first write the original size of the buffer
				if(!write(fd, &iSendBufPos, sizeof(int))){
					LOG("Connection problem!\n");
				}
				//LOG("original size: %d!\n", iSendBufPos);
				//then write the compressed size of the buffer
				if(!write(fd, &newSize, sizeof(int))){
					LOG("Connection problem!\n");
				}
				//LOG("compressed size: %d!\n", newSize);
				//then write the actual buffer
				if(!write(fd, out, newSize)){
					LOG("Connection problem!\n");
				}
				
				//LOG("iSendBufPos %d\n", iSendBufPos);
			}
			free(out);
			iSendBufPos = 0;
			bytesLeft = sendBufferSize;
			netBytes2 += (newSize + (sizeof(int) * 2)) * numConnections;
		}
		else {
			for(int i = 0; i < numConnections; i++) {
				int fd = mSocket[i];
				//first write the original size of the buffer
				if(!write(fd, &iSendBufPos, sizeof(int))){
					LOG("Connection problem!\n");
				}
				//then write the actual buffer
				if(!write(fd, mSendBuf, iSendBufPos)){
					LOG("Connection problem!\n");
				}
			}
		
			bytesLeft = sendBufferSize;
			netBytes2 += iSendBufPos * numConnections;	//should add sizeof(int) * numConnections overhead
			iSendBufPos = 0;
		}
	}	
}

/*********************************************************
	Net Client Run Decompression
*********************************************************/

int NetClientModule::myRead(void *buf, size_t count){
	int ret = 0;
	for(int i = 0; i < numConnections; i++) {
			int fd = mSocket[i];

		if(useRecieveCompression) {
			size_t *a = &count;
			//read the size of the compressed packet
			int compressedSize = 0;
			int c = read(fd, &compressedSize, sizeof(int));

			//read the size of the original packet
			int origSize = 0;
			int d = read(fd, &origSize, sizeof(int));

			//read the size of the incoming packet
			//then read the compressed packet data and uncompress
			Bytef *in = (Bytef*) malloc(compressedSize);
			ret = read(fd, in, compressedSize);
		//	if(ret != compressedSize)
		//		return 0;
			compressor2->myDecompress(buf, count, in, compressedSize);
			free(in);
		}
		else {
			//LOG("waiting for a message!\n");
			ret = read(fd, buf, count);
		}
	}
	return ret;
}


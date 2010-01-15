#include "main.h"
#include <zconf.h>
#include <zlib.h>
#include <netinet/tcp.h>

/*********************************************************
	Net Client Globals
*********************************************************/

int incomingSize = 0;
int outgoingSize = 0;
NetCompressModule *compressor2;
bool useSendCompression = false;
bool useRecieveCompression = false;
bool useCGLrepeat = false;

const int sendBufferSize = sizeof(Instruction) * MAX_INSTRUCTIONS * 8;
int iSendBufPos = 0;
int bytesLeft = sendBufferSize;

//big buffer
byte mSendBuf[sendBufferSize];


/*********************************************************
	Net Client Module
*********************************************************/

NetClientModule::NetClientModule(string address, int port, bool sendCompression, bool recieveCompression, bool repeatInstruction){
        //Make the socket and connect
	mSocket = socket(PF_INET, SOCK_STREAM, 0); 
	
	useSendCompression = sendCompression;
	useRecieveCompression = recieveCompression;

	useCGLrepeat = repeatInstruction;

	if(useSendCompression | useRecieveCompression) {
		//make a compressor object
		compressor2 = new NetCompressModule();
	}
	
	//set TCP options
	int one = 1;
	setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
	setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

	if(mSocket == 0){
		LOG("Couldn't make socket!\n");
		return;
	}
	
	struct sockaddr_in mAddr;
	
	memset(&mAddr, 0, sizeof(mAddr));   
	mAddr.sin_family = AF_INET;   
	mAddr.sin_addr.s_addr = inet_addr(address.c_str()); 
	mAddr.sin_port = htons(port);  
	//Establish connection
	if (connect(mSocket,
                (struct sockaddr *) &mAddr,
                sizeof(mAddr)) < 0) {
		LOG("Failed to connect with server '%s:%d'\n", address.c_str(), port);
		mSocket = 0;
		return;
	}
	
	//reset BPS calculations
	netBytes = 0;   
	netBytes2 = 0; 

	LOG("Connected to remote pipeline on %s:%d\n", address.c_str(), port);
}

/*********************************************************
	Net Client Process Instrctions
*********************************************************/

bool NetClientModule::process(list<Instruction> &list){
	//Send all the commands in the list down the socket
	//TODO: Don't do so many send() calls!
	//LOG("processing:\n");
	if(mSocket == 0){
		return false;
	}
	
	//First send the total number
	uint32_t num = list.size();
	//LOG("num size %d!\n", num);
	fflush(stdout);
	netBytes += sizeof(uint32_t);
	if(!myWrite(mSocket, &num, sizeof(uint32_t))){
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

            //LOG("ID:%d\n",i->id);
	    bool mustSend = false;

	    for(int n=0;n<3;n++){		
		if(i->buffers[n].len > 0){
		  	//LOG("MustSend:%d\n",i->id);
			//If we expect a buffer back then we must send Instruction
			if(i->buffers[n].needReply) mustSend = true;
		}
	    };
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
					if (i->buffers[a].len >0 && pIter->buffers[a].len >0){
						if (i->buffers[a].len != pIter->buffers[a].len){
							same = false;
							break;
						}
						else if (i->buffers[a].needClear != pIter->buffers[a].needClear){
							same = false;
							break;
						}
						else if (!memcmp(i->buffers[a].buffer,pIter->buffers[a].buffer,i->buffers[a].len)){
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
		netBytes += sizeof(Instruction);
		if(myWrite(mSocket, skip, sizeof(Instruction))!= sizeof(Instruction)){
		    	LOG("Connection problem (didn't send instruction)!\n");
		    	return false;
		}
		sameCount = 0; // reset the count and free the memory
		free(skip);
	    }
	   
	    // now send the new instruction
		netBytes += sizeof(Instruction);
	    if(myWrite(mSocket, i, sizeof(Instruction)) != sizeof(Instruction)){
	    		LOG("Connection problem (didn't send instruction)!\n");
	    		return false;
	    }
	    
	    //Now see if we need to send any buffers
		for(int n=0;n<3;n++){
			int l = i->buffers[n].len;
		
			if(l > 0){
				//LOG("buffer: %d\n", l);
				netBytes += l;	
				if(myWrite(mSocket, i->buffers[n].buffer, l) != l){
					LOG("Connection problem (didn't write buffer %d)!\n", l);
					return false;
				}
				//And check if we're expecting a buffer back in response
				if(i->buffers[n].needReply){
					
					sendBuffer(mSocket);
					if(myRead(mSocket, i->buffers[n].buffer, l) != l){
						LOG("Connection problem (didn't recv buffer %d)!\n", l);
						return false;
					}
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
		netBytes += sizeof(Instruction);
		if(myWrite(mSocket, skip, sizeof(Instruction))!= sizeof(Instruction)){
		    	LOG("Connection problem (didn't send instruction)!\n");
		    	return false;
		}
		sameCount = 0; // reset the count and free the memory
		free(skip);
	    }
		
	    sendBuffer(mSocket);

	return true;
}

/*********************************************************
	Net Client Sync
*********************************************************/

bool NetClientModule::sync(){
	int * a = (int *)malloc(sizeof(int));
	*a = 987654;
	netBytes += sizeof(int);

	if(myWrite(mSocket, a, sizeof(int)) != sizeof(int)){
		LOG("Connection problem (didn't send sync)!\n");
		return false;
	}
	
	sendBuffer(mSocket);

	if(myRead(mSocket, a, sizeof(int)) != sizeof(int)){
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

int NetClientModule::myWrite(int fd, void* buf, int nByte){
	
	if(bytesLeft - nByte > 0) {
		memcpy(mSendBuf + iSendBufPos, buf, nByte);
		iSendBufPos += nByte;
		bytesLeft -= nByte;
	}
	else {
		sendBuffer(fd);
	}
	return nByte;

	/*if(useCompress) {
		//create room for new compressed buffer
		uLongf CompBuffSize = (uLongf)(nByte + (nByte * 0.1) + 12);
		Bytef *out = (Bytef*) malloc(CompBuffSize);
		int newSize = compressor2->myCompress(buf, nByte, out);

		//write the size of the next instruction
		if(!write(fd, &newSize, sizeof(int))){
			LOG("Connection problem!\n");
		}
		//write the old size of the next instruction
		if(!write(fd, &nByte, sizeof(int))){
			LOG("Connection problem!\n");
		}
	
		//write the compressed instruction
		int ret = write(fd, out, newSize);
		//check and set return value to keep caller happy
		if(ret == newSize)
			ret = nByte;
		netBytes2 += sizeof(int) * 2;
		netBytes2 += newSize;
		free(out);
		return ret;
	}
	else {
		int ret = write(fd, buf, nByte);
		netBytes2 += nByte;
		return ret;
	}*/
}

int NetClientModule::myWrite(int fd, void* buf, unsigned nByte){

	if(bytesLeft - nByte > 0) {
		memcpy(mSendBuf + iSendBufPos, buf, nByte);
		iSendBufPos += nByte;
		bytesLeft -= nByte;
	}
	else {
		sendBuffer(fd);
	}
	return nByte;
	/*if(useCompress) {
		//create room for new compressed buffer
		uLongf CompBuffSize = (uLongf)(nByte + (nByte * 0.1) + 12);
		Bytef *out = (Bytef*) malloc(CompBuffSize);
		int newSize = compressor2->myCompress(buf, nByte, out);

		//write the size of the next instruction
		if(!write(mSocket, &newSize, sizeof(int))){
			LOG("Connection problem!\n");
		}
		//write the old size of the next instruction
		if(!write(mSocket, &nByte, sizeof(int))){
			LOG("Connection problem!\n");
		}
	
		//write the compressed instruction
		int ret = write(fd, out, newSize);
		//check and set return value to keep caller happy
		if(ret == newSize)
			ret = nByte;
		netBytes2 += sizeof(int) * 2;
		netBytes2 += newSize;
		free(out);
		return ret;
	}
	else {
		int ret = write(fd, buf, nByte);
		netBytes2 += nByte;
		return ret;
	} */
}

int NetClientModule::myWrite(int fd, void* buf, long unsigned nByte){
	
	if(bytesLeft - nByte > 0) {
		memcpy(mSendBuf + iSendBufPos, buf, nByte);
		iSendBufPos += nByte;
		bytesLeft -= nByte;
	}
	else {
		sendBuffer(fd);
	}
	return nByte;
	/*
	if(useCompress) {
		//create room for new compressed buffer
		uLongf CompBuffSize = (uLongf)(nByte + (nByte * 0.1) + 12);
		Bytef *out = (Bytef*) malloc(CompBuffSize);
		int newSize = compressor2->myCompress(buf, nByte, out);

		//write the size of the next instruction
		if(!write(mSocket, &newSize, sizeof(int))){
			LOG("Connection problem!\n");
		}
		//write the old size of the next instruction
		if(!write(mSocket, &nByte, sizeof(int))){
			LOG("Connection problem!\n");
		}
	
		//write the compressed instruction
		int ret = write(fd, out, newSize);
		//check and set return value to keep caller happy
		if(ret == newSize)
			ret = nByte;
		netBytes2 += sizeof(int) * 2;
		netBytes2 += newSize;
		free(out);
		return ret;
	}
	else {
		int ret = write(fd, buf, nByte);
		netBytes2 += nByte;
		return ret;
	}*/
}

void NetClientModule::sendBuffer(int fd) {
	if(iSendBufPos > 0) {
		if(useSendCompression) {
			//LOG("sending buffer of size: %d!\n", iSendBufPos);

			//create room for new compressed buffer
			uLongf CompBuffSize = (uLongf)(iSendBufPos + (iSendBufPos * 0.1) + 12);
			Bytef *out = (Bytef*) malloc(CompBuffSize);
			int newSize = compressor2->myCompress(mSendBuf, iSendBufPos, out);

			//first write the original size of the buffer
			if(!write(fd, &iSendBufPos, sizeof(int))){
				LOG("Connection problem!\n");
			}
			//LOG("original size: %d!\n", iSendBufPos);
			//first write the compressed size of the buffer
			if(!write(fd, &newSize, sizeof(int))){
				LOG("Connection problem!\n");
			}
			//LOG("compressed size: %d!\n", newSize);
			//then write the actual buffer
			if(!write(fd, out, newSize)){
				LOG("Connection problem!\n");
			}
			//reset values
			iSendBufPos = 0;
			bytesLeft = sendBufferSize;
			netBytes2 += newSize + (sizeof(int) * 2);
			//LOG("iSendBufPos %d\n", iSendBufPos);
		}
		else {
			//first write the original size of the buffer
			if(!write(fd, &iSendBufPos, sizeof(int))){
				LOG("Connection problem!\n");
			}

			//then write the actual buffer
			if(!write(fd, mSendBuf, iSendBufPos)){
				LOG("Connection problem!\n");
			}
			
			bytesLeft = sendBufferSize;
			netBytes2 += iSendBufPos;	//should add sizeof(int) overhead
			iSendBufPos = 0;
		}
	}
}

/*********************************************************
	Net Client Run Decompression
*********************************************************/

int NetClientModule::myRead(int fd, void *buf, size_t count){
	
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
		int ret = read(fd, in, compressedSize);
		if(ret == compressedSize)
			ret = origSize;
		compressor2->myDecompress(buf, count, in, compressedSize);
		free(in);
		return ret;
	}
	else {
		//LOG("waiting for a message!\n");
		int ret = read(fd, buf, count);
		return ret;
	}
}


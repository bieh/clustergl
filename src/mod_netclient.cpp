#include "main.h"
#include <zconf.h>
#include <zlib.h>

int incomingSize = 0;
int outgoingSize = 0;
NetCompressModule *compressor2;

NetClientModule::NetClientModule(string address, int port){
        //Make the socket and connect
	mSocket = socket(PF_INET, SOCK_STREAM, 0); 

	//make a compressor object
	compressor2 = new NetCompressModule();

	//setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));

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

bool NetClientModule::process(list<Instruction> &list){
	//Send all the commands in the list down the socket
	//TODO: Don't do so many send() calls!

	if(mSocket == 0){
		return false;
	}
	
	//First send the total number
	uint32_t num = list.size();
	//LOG("num of instructions: %d\n", num);
	netBytes += sizeof(uint32_t);
	if(!myWrite(mSocket, &num, sizeof(uint32_t))){
		LOG("Connection problem!\n");
		return false;
	}

	//Count of duplicate instructions
	uint32_t sameCount = 0;

	//Now send the instructions
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
	    }

	    if (i->id == pIter->id 		
		&& !mustSend
		//&& false //uncomment to disable deltas
		) {
			bool same = true;
			for (int a=0;a<MAX_ARG_LEN;a++){
				if (i->args[a]!=pIter->args[a])
					same = false;	
			}
			if (same){
				for (int a=0;a<3;a++)
					if (i->buffers[a].len >0 && pIter->buffers[a].len >0){
						if (i->buffers[a].len != pIter->buffers[a].len)
							same = false;
						else if (i->buffers[a].needClear != pIter->buffers[a].needClear)
							same = false;
						else if (!memcmp(i->buffers[a].buffer,pIter->buffers[a].buffer,i->buffers[a].len)){
							same = false;
						}
					}
			}
			if (same) {
				sameCount++;
				if (pIter != (*prevFrame).end()) pIter++;
				continue; 
			}	
	    }

	    if (sameCount> 0){ // send a count of the duplicates before this instruction
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
				netBytes += l;
				if(myWrite(mSocket, i->buffers[n].buffer, l) != l){
					LOG("Connection problem (didn't write buffer %d)!\n", l);
					return false;
				}
				//And check if we're expecting a buffer back in response
				if(i->buffers[n].needReply){
					if(myRead(mSocket, i->buffers[n].buffer, l) != l){
						LOG("Connection problem (didn't recv buffer %d)!\n", l);
						return false;
					}
				}
			}
		}
		if (pIter != (*prevFrame).end()) pIter++;
	}

	    if (sameCount> 0){ // send a count of the duplicates before this instruction
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
	return true;
}

bool NetClientModule::sync(){
	int * a = (int *)malloc(sizeof(int));
	*a = 987654;
	if(myWrite(mSocket, a, sizeof(int)) != sizeof(int)){
		LOG("Connection problem (didn't send sync)!\n");
		return false;
	}
	if(myRead(mSocket, a, sizeof(int)) != sizeof(int)){
		LOG("Connection problem (didn't recv sync)!\n");
		return false;
	}
	if (*a!=987654)
		return false;
	free(a);
	return true;
}

int NetClientModule::myWrite(int fd, void* buf, int nByte){
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
	//cheack and set return value to keep caller happy
	if(ret == newSize)
		ret = nByte;
	netBytes2 += sizeof(int) * 2;
	netBytes2 += newSize;
	free(out);
	return ret;
}

int NetClientModule::myWrite(int fd, void* buf, unsigned nByte){
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
	//cheack and set return value to keep caller happy
	if(ret == newSize)
		ret = nByte;
	netBytes2 += sizeof(int) * 2;
	netBytes2 += newSize;
	free(out);
	return ret;
}

int NetClientModule::myWrite(int fd, void* buf, long unsigned nByte){
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
	//cheack and set return value to keep caller happy
	if(ret == newSize)
		ret = nByte;
	netBytes2 += sizeof(int) * 2;
	netBytes2 += newSize;
	free(out);
	return ret;
}

int NetClientModule::myRead(int fd, void *buf, size_t count){
//LOG("READING:\n");
	size_t *a = &count;
	//read the size of the compressed packet
	int compressedSize = 0;
	read(fd, &compressedSize, sizeof(int));

	//read the size of the original packet
	int origSize = 0;
	read(fd, &origSize, sizeof(int));

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


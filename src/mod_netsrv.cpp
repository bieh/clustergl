#include "main.h"
#include <zconf.h>
#include <zlib.h>
#include <netinet/tcp.h>

/*********************************************************
	Net Server Globals
*********************************************************/

NetCompressModule *compressor;
int prevInstruction = 0;
bool useDecompression = false;
bool useReplyCompression = false;

const int recieveBufferSize = sizeof(Instruction) * MAX_INSTRUCTIONS * 8;
int iRecieveBufPos = 0;
int bytesRemaining = 0;;

//big buffer
byte mRecieveBuf[recieveBufferSize];

/*********************************************************
	Net Server Module
*********************************************************/

NetSrvModule::NetSrvModule(int port, bool decompression, bool replyCompression, int compressionMethod){

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
	
	//reset BPS calculations
	netBytes = 0;   
	netBytes2 = 0; 
	
	useDecompression = decompression;
	useReplyCompression = replyCompression;
	if(useDecompression || useReplyCompression) {
		compressor = new NetCompressModule(compressionMethod);
	}

	//set TCP options
	int one = 1;
	setsockopt(mSocket, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
	setsockopt(mSocket, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

	//Bind the server socket
	if (bind(mSocket, (struct sockaddr *) &addr,
		                       sizeof(addr)) < 0) {
		LOG("Failed to bind the server socket\n");
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

/*********************************************************
	Net Server Process Instructions
*********************************************************/

bool NetSrvModule::process(list<Instruction> &list){
	//Read instructions off the network and insert them into the list
	//First read the uint32 that tells us how many instructions we will have
	uint32_t num = 0;
	
	int len = myRead((byte *)&num, sizeof(uint32_t));
	int a=sizeof(uint32_t);
	if(len < a ){
		LOG("Read error\n");
		return false;
	}

	//LOG("Reading %d instructions!\n", num);
	// Variables for processing Deltas
	std::list<Instruction>::iterator pIter = (*prevFrame).begin();
	
	for(uint32_t x=0;x<num;x++){
		//LOG("Reading instruction %d!\n", x);
		
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
		//LOG("Instruction %d\n", i.id);
		}
		//Now see if we're expecting any buffers
		for(int n=0;n<3;n++){
			int l = i.buffers[n].len;
						
			if(l > 0){
				//LOG("Reading buffer of size %d (%d)\n", l, i.id);
				i.buffers[n].buffer = (byte *) malloc(l);
				//need clear was set at the other end
				i.buffers[n].needClear = true;
				myRead(i.buffers[n].buffer, l);
			}			
		}
		if(i.id == CGL_REPEAT_INSTRUCTION){
			//LOG("SKIP: %d\n", i.args[0]);
			//decrease num, as we won't need to read these instructions from the socket
			num -= i.args[0];
			for (uint32_t a = 0;a <(uint32_t)i.args[0];a++){
				Instruction p;
				
				p.id = pIter->id;
				//LOG("SKIP INSTRUCT: %d\n", p.id);	
				for (int j =0;j<MAX_ARG_LEN;j++)
					p.args[j] = pIter->args[j];
				
				//Now see if we're expecting any buffers
				for(int n=0;n<3;n++){
					int l = pIter->buffers[n].len;
								
					if(l > 0){
						//LOG("%d SKIP with buffers!, %d\n", p.id, l);
						p.buffers[n].buffer = (byte *) malloc(l);
						p.buffers[n].needClear = true;
						p.buffers[n].len = l;

						if(!(pIter->buffers[n].buffer)) {
							LOG("copying a cleared buffer, something wrong here!!\n");
						}
						memcpy((byte *)(p.buffers[n].buffer), &(*pIter->buffers[n].buffer),l);
					}			
				}
				list.push_back(p);
				//LOG("INSTRUCTION: %d\n", p.id );
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

void NetSrvModule::reply(Instruction *instr, int i) {

	//LOG("reply size: %d, for instruction: %d\n", instr->buffers[i].len, instr->id);
	myWrite(instr->buffers[i].buffer, instr->buffers[i].len);
}

/*********************************************************
	Net Server Sync
*********************************************************/

bool NetSrvModule::sync() {

	int a;
	if(myRead((byte *)&a, sizeof(int)) != sizeof(int)){
		LOG("Connection problem (didn't recv sync)!\n");
		return false;
	}
	if(myWrite((byte *)&a, sizeof(int)) != sizeof(int)){
		LOG("Connection problem (didn't send sync)!\n");
		return false;
	}	
	return true;
}

/*********************************************************
	Net Server Run Decompression
*********************************************************/

int NetSrvModule::myRead(byte *input, int nByte) {
	
	if(bytesRemaining == 0)
		recieveBuffer();
	memcpy(input, mRecieveBuf + iRecieveBufPos, nByte);
	iRecieveBufPos += nByte;
	bytesRemaining -= nByte;

	return nByte;
}

void NetSrvModule::recieveBuffer(void) {
	if(useDecompression) {
		//LOG("recieving buffer!\n");
		//first read the original number of bytes coming
		mClientSocket->read((byte *)&bytesRemaining, sizeof(int));
		//LOG("original size: %d!\n", bytesRemaining);
		//read the size of the compressed packet
		int compSize = 0;
		mClientSocket->read((byte *)&compSize, sizeof(int));
		//LOG("compressed size: %d!\n", compSize);
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
		mClientSocket->read((byte *)&bytesRemaining, sizeof(int));
		//read the buffer
		mClientSocket->read(mRecieveBuf, bytesRemaining);
		iRecieveBufPos = 0;
	}
}
/*********************************************************
	Net Server Run Compression
*********************************************************/

int NetSrvModule::myWrite(byte *input, int nByte) {

	if(useReplyCompression) {
		//create room for new compressed buffer
		uLongf CompBuffSize = (uLongf)(nByte + (nByte * 0.1) + 12);
		Bytef *out = (Bytef*) malloc(CompBuffSize);
		int newSize = compressor->myCompress(input, nByte, out);

		//write the size of the next instruction
		if(!mClientSocket->write( (byte*)&newSize, sizeof(int))){
			LOG("Connection problem!\n");
		}
		//write the old size of the next instruction
		if(!mClientSocket->write( (byte*)&nByte, sizeof(int))){
			LOG("Connection problem!\n");
		}

		//write the compressed instruction
		int ret =mClientSocket->write(out, newSize);
		//cheack and set return value to keep caller happy
		if(ret == newSize)
			ret = nByte;

		//calculate bandwidth requirements
		netBytes += nByte; 
		netBytes2 += sizeof(int) * 2;  
		netBytes2 += newSize; 
		free(out);

		return ret;
	}
	else {
		//LOG("sending a message!\n");
		int ret = mClientSocket->write(input, nByte);
		netBytes += nByte;
		netBytes2 += nByte;
		return ret;
	}
}



#include "main.h"
#include <zconf.h>
#include <zlib.h>
#include <netinet/tcp.h>

  NetCompressModule *compressor;

NetSrvModule::NetSrvModule(int port){

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

	compressor = new NetCompressModule();

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
	
	//LOG("Reading %d instructions!\n\n", num);

	// Variables for processing Deltas
	uint32_t count = -1;
	std::list<Instruction>::iterator pIter = (*prevFrame).begin();
	
	for(uint32_t i=0;i<num;i++){
		Instruction i;		
		count++;
		
		int r = myRead((byte *)&i, sizeof(Instruction));
		if(r != sizeof(Instruction)){
			LOG("Read error (%d)\n", r);
			perror("NetSrvMod Instruction");
			return false;
		}
		//Now see if we're expecting any buffers
		for(int n=0;n<3;n++){
			int l = i.buffers[n].len;
						
			if(l > 0){
				//LOG("Reading buffer of size %d (%d)\n", l, i.id);
				i.buffers[n].buffer = new byte[l];
				i.buffers[n].needClear = true;	
				myRead(i.buffers[n].buffer, l);
			}			
		}
		
		if(i.id == CGL_REPEAT_INSTRUCTION){
			for (uint32_t a = count;count<a+(uint32_t)i.args[0];count++){
				Instruction p;
				
				p.id = pIter->id;
				for (int j =0;j<MAX_ARG_LEN;j++)
					p.args[j] = pIter->args[j];

				//Now see if we're expecting any buffers
				for(int n=0;n<3;n++){
					int l = pIter->buffers[n].len;
								
					if(l > 0){
						p.buffers[n].buffer = new byte[l];
						p.buffers[n].needClear = true;			
						memcpy((byte *)(p.buffers[n].buffer), &(*pIter->buffers[n].buffer),l);
					}			
				}
				list.push_back(p);
				num--;
				if (pIter != (*prevFrame).end()) pIter++;
				
			}
			count--;
			num++;
		}
		else{
			//LOG("READ INSTRUCTION: %d on stack\n", i.id);
			list.push_back(i);
			if (pIter != (*prevFrame).end()) pIter++;
		}
	}
	return true;
}

void NetSrvModule::reply(Instruction *instr, int i){
	LOG("reply size: %d, for instruction: %d\n", instr->buffers[i].len, instr->id);
	myWrite(instr->buffers[i].buffer, instr->buffers[i].len);
}

bool NetSrvModule::sync(){
	//LOG("SYNC\n");
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

int NetSrvModule::myRead(byte *input, int nByte){
	//LOG("READING:\n");
	//read the size of the compressed packet
	int compressedSize = 0;
	mClientSocket->read((byte *)&compressedSize, sizeof(int));

	//read the size of the original packet
	int origSize = 0;
	mClientSocket->read((byte *)&origSize, sizeof(int));
	if(origSize < 4)
		LOG("READING: %d\n", origSize);
	//then read the compressed packet data and uncompress
	Bytef *in = (Bytef*) malloc(compressedSize);
	int ret = mClientSocket->read(in, compressedSize);
	if(ret == compressedSize)
		ret = origSize;
	compressor->myDecompress(input, nByte, in, compressedSize);
	free(in);
	//LOG("READ:\n");
	return ret;
}

int NetSrvModule::myWrite(byte *input, int nByte){
	//LOG("WRITING: %d\n", nByte);
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
	netBytes += nByte; 
	netBytes2 += sizeof(int) * 2;  
	netBytes2 += newSize; 
	free(out);
	//LOG("size: %d, size2: %d\n", nByte, newSize + 8);
	return ret;
}



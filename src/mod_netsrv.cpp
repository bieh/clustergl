#include "main.h"

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
		
	//LOG("Waiting\n");

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
	
	int len = mClientSocket->read((byte *)&num, sizeof(uint32_t));
	
	if(len < sizeof(uint32_t)){
		LOG("Read error\n");
		return false;
	}
	
	//LOG("Reading %d instructions!\n", num);
	
	for(uint32_t i=0;i<num;i++){
		Instruction i;			
	
		int r = mClientSocket->read((byte *)&i, sizeof(Instruction));
		
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
				
				mClientSocket->read(i.buffers[n].buffer, l);
			}			
		}
		
		list.push_back(i);
	}
	
	return true;
}

void NetSrvModule::reply(Instruction *instr, int i){

	//int n = 7;
	//memcpy(instr->buffers[i].buffer, &n, sizeof(int));

	mClientSocket->write(instr->buffers[i].buffer, instr->buffers[i].len);
}

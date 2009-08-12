#include "main.h"

NetSrvModule::NetSrvModule(int port){
#ifdef MULTICAST
	/////////////////////////////////////////////////
	// SET up multicast
	//////////////////////////
	
   // open a UDP socket
   recvSocket = socket(PF_INET, SOCK_DGRAM, IPPROTO_IP);
   if ( recvSocket < 0 ) LOG("Error creating socket"), exit(recvSocket);

   saddr.sin_family = PF_INET;
   saddr.sin_port = htons(4096); // listen on port 4096
   saddr.sin_addr.s_addr = htonl(INADDR_ANY); // bind socket to any interface

   if ( bind(recvSocket, (struct sockaddr *)&saddr, sizeof(struct sockaddr_in)) < 0 )
     LOG("Error binding socket to interface"), exit(-1);

   struct ip_mreq imreq;
   imreq.imr_multiaddr.s_addr = inet_addr("224.0.0.1");
   imreq.imr_interface.s_addr = INADDR_ANY; // use DEFAULT interface

   // JOIN multicast group on default interface
   if ( setsockopt(recvSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
	      (const void *)&imreq, sizeof(struct ip_mreq)) < 0 )
     LOG("Error joining multicast group"), exit(-1);
#endif

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
//LOG("b4, m:%d c:%d",mSocket,client);
#ifdef MULTICAST	
	mSocket = client;
	mClientSocket = new BufferedFd(recvSocket,saddr);
#else
	mClientSocket = new BufferedFd(client);
#endif
//LOG("b4, m:%d c:%d",mSocket,client);
		
	LOG("%s connected\n", string(inet_ntoa(clientaddr.sin_addr)).c_str() );
}

bool NetSrvModule::process(list<Instruction> &list){

	//Read instructions off the network and insert them into the list
	//First read the uint32 that tells us how many instructions we will have
	uint32_t num = 0;
	
	int len = mClientSocket->read((byte *)&num, sizeof(uint32_t));
	int a=sizeof(uint32_t);
	if(len < a ){
		LOG("Read error\n");
		return false;
	}
	
	//LOG("Reading %d instructions!\n", num);

	// Variables for processing Deltas
	uint32_t count = -1;
	std::list<Instruction>::iterator pIter = (*prevFrame).begin();
	
	for(uint32_t i=0;i<num;i++){
		Instruction i;		
		count++;
		
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
			list.push_back(i);
			if (pIter != (*prevFrame).end()) pIter++;
		}
	}
	return true;
}

void NetSrvModule::reply(Instruction *instr, int i){
	mClientSocket->write(instr->buffers[i].buffer, instr->buffers[i].len);
}



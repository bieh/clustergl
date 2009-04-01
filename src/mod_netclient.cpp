#include "main.h"

NetClientModule::NetClientModule(string address, int port){

	//Make the socket and connect
	mSocket = socket(PF_INET, SOCK_STREAM, 0); 
	
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
	
	if(!write(mSocket, &num, sizeof(uint32_t))){
		LOG("Connection problem!\n");
		return false;
	}
	
	//LOG("Sending %d\n", num);
	
	
	//Now send the instructions
	for(std::list<Instruction>::iterator iter = list.begin(); 
	    iter != list.end(); iter++){
	    
	    Instruction *i = &(*iter); //yuck
	    
	    if(write(mSocket, i, sizeof(Instruction)) != sizeof(Instruction)){
	    	LOG("Connection problem (didn't send instruction)!\n");
	    	return false;
	    }
	    
	    //Now see if we need to send any buffers
		for(int n=0;n<3;n++){
			int l = i->buffers[n].len;
						
			if(l > 0){
				if(write(mSocket, i->buffers[n].buffer, l) != l){
					LOG("Connection problem (didn't write buffer %d)!\n", l);
					return false;
				}
				
				//LOG("Wrote buffer of size %d (%d)\n", l, i->id);
				
				//And check if we're expecting a buffer back in response
				if(i->buffers[n].needReply){
					//LOG("Waiting for reply\n");
					if(read(mSocket, i->buffers[n].buffer, l) != l){
						LOG("Connection problem (didn't recv buffer %d)!\n", l);
						return false;
					}
					//LOG("Got reply!\n");
				}
			}
		}
	}
	
	return true;
}
		

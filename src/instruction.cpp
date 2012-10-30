/*******************************************************************************
	ClusterGL - Instruction object utils
*******************************************************************************/

#include "main.h"


Instruction::Instruction() {
	id = 0;
	for(int i=0;i<3;i++) {
		buffers[i].buffer = NULL;
		buffers[i].len = 0;
		buffers[i].needClear = false;
		buffers[i].needReply = false;
		buffers[i].needRemoteReply = false;
		buffers[i].hash = 0;
	}
			
	memset(args, 0, MAX_ARG_LEN);
}

Instruction::~Instruction() {
	for(int i=0; i<3; i++) {
		if(buffers[i].buffer)
			free(buffers[i].buffer);
	}
}

Instruction *Instruction::copy(){
	Instruction *r = new Instruction();
	memcpy(r, this, (sizeof(Instruction) - MAX_ARG_LEN) + arglen);
	for(int i=0;i<3;i++) {
		if(buffers[i].buffer) {
			r->buffers[i].buffer = (byte*) malloc(buffers[i].len);
			memcpy(r->buffers[i].buffer, buffers[i].buffer, buffers[i].len);
		}
	}
	return r;
}

void Instruction::clear() {
	//printf("Deleted\n");
	arglen = 0;

	for(int i=0;i<3;i++) {
		if(buffers[i].buffer && buffers[i].needClear) {
			//printf("deleted (%d)\n", id);
			//previously delete buffers[i].buffer
			free(buffers[i].buffer);
			buffers[i].buffer = NULL;
			buffers[i].len = 0;
			buffers[i].hash = 0;
		}
	}
	
}

bool Instruction::needReply(){	
	for(int i=0;i<3;i++) {
		if(buffers[i].buffer && buffers[i].needReply){
			return true;
		}
	}
	return false;
}
	
bool Instruction::compare(Instruction *other){

	//instr id
	if(other->id != id){
		return false;
	}
	
	if(other->arglen != arglen){
		return false;
	}
			
	//argument data.
	if(memcmp((void *)args, (void *)other->args, arglen) != 0){		
		return false;
	}
	
	//buffers
	for(int i=0;i<3;i++){
	
		//if one has a buffer and another doesn't...
		if(buffers[i].buffer && !other->buffers[i].buffer){
			return false;
		}
		
		if(!buffers[i].buffer && other->buffers[i].buffer){
			return false;
		}
		
		//if we have a buffer at all
		if(!buffers[i].buffer){
			continue;
		}
		
		//if sizes of buffers are different
		if(buffers[i].len != other->buffers[i].len){
			return false;
		}
		
		//finally check the memory. this is the most expensive bit
		if(memcmp(	(void *)buffers[i].buffer, 
					(void *)other->buffers[i].buffer,
					buffers[i].len) != 0){
			return false;
		}				
	}
	
	
	//the same!
	return true;
}


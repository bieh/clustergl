/*******************************************************************************
				instruction.h
*******************************************************************************/

//TODO: will this be enough?
#define MAX_ARG_LEN 48

class InstructionBuffer
{
public:
	byte *buffer;
	uint32_t len;
	bool needClear;
	bool needReply;
};

class Instruction
{
public:
	Instruction() {
		id = 0;
		for(int i=0;i<3;i++) {
			buffers[i].buffer = NULL;
			buffers[i].len = 0;
			buffers[i].needClear = false;
			buffers[i].needReply = false;
		}
	}

	void clear() {
		//printf("Deleted\n");

		for(int i=0;i<3;i++) {
			if(buffers[i].buffer && buffers[i].needClear) {
				//printf("deleted (%d)\n", id);
				//previously delete buffers[i].buffer
				free(buffers[i].buffer);
				buffers[i].buffer = NULL;
				buffers[i].len = 0;
			}
		}
	}
	
	bool compare(Instruction *other){
		if(other->id != id){
			return false;
		}
		
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
	
	uint16_t id;
	byte args[MAX_ARG_LEN];

	InstructionBuffer buffers[3];
};

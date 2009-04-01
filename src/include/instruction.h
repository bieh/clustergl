/*******************************************************************************
							instruction.h
*******************************************************************************/

//TODO: will this be enough?
#define MAX_ARG_LEN 128

class InstructionBuffer{
public:
	byte *buffer;
	uint32_t len;
	bool needClear;
	bool needReply;
};

class Instruction{
public:
	Instruction(){
		id = 0;
		for(int i=0;i<3;i++){
			buffers[i].buffer = NULL;
			buffers[i].len = 0;
			buffers[i].needClear = false;
		}
	}
	
	void clear(){
		//printf("Deleted\n");
		
		for(int i=0;i<3;i++){
			if(buffers[i].buffer && buffers[i].needClear){
				//printf("deleted (%d)\n", id);
				delete buffers[i].buffer;
				
				buffers[i].buffer = NULL;
				buffers[i].len = 0;
			}
		}
	}
	uint16_t id;
	byte args[MAX_ARG_LEN];
	
	InstructionBuffer buffers[3];
};



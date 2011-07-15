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
	bool needRemoteReply;
};

class Instruction
{
public:
	Instruction();
	void clear();	
	bool compare(Instruction *other);
	Instruction *copy();
	bool needReply();
	
	uint16_t id;
	byte args[MAX_ARG_LEN];
	byte arglen;

	InstructionBuffer buffers[3];
};

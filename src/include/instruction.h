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
	uint32_t hash;
	uint32_t hashlen;
};

class Instruction
{
public:
	Instruction();
	~Instruction();
	void clear();	
	bool compare(Instruction *other);
	Instruction *copy();
	bool needReply();
	
	uint16_t id;
	byte arglen;

	InstructionBuffer buffers[3];

	//Must be the last thing...
	byte args[MAX_ARG_LEN];
};

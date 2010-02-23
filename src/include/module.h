/*******************************************************************************
	The module interface
*******************************************************************************/

/*********************************************
	Module object
**********************************************/
class Module
{
	public:
		Module(){}

								 //used for comparing deltas (only used in NetClient and NetSrv)
		list<Instruction> *prevFrame;
		ulong netBytes;			 // variable to calculate network usage
		ulong netBytes2;		 // variable to calculate compressed network usage
		uint32_t frames;

		virtual bool process(list<Instruction> &i)=0;
		virtual void reply(Instruction *instr, int i){}
		virtual bool sync()=0;
};

/*********************************************
	A module that captures from a program
**********************************************/
class AppModule : public Module
{
	public:
		AppModule(string command);

		bool init(string command);
		bool process(list<Instruction> &i);
		bool sync();
};

/*********************************************
	A module that outputs to stdout
**********************************************/
class TextModule : public Module
{
	public:
		TextModule();
		bool init();
		bool process(list<Instruction> &i);
		bool sync();
};

/*********************************************
	A module that outputs to GL
**********************************************/
class ExecModule : public Module
{
	bool makeWindow();

	public:
		ExecModule();

		bool init();
		bool process(list<Instruction> &i);
		bool sync();
};

/*********************************************
 Network server module. This recvs commands
 and thus should go at the *start* of the local
 pipe. Blocks till a client connects.
**********************************************/
class NetSrvModule : public Module
{
	int mSocket;
	BufferedFd *mClientSocket;
	public:
		NetSrvModule();

		bool process(list<Instruction> &i);
		void reply(Instruction *instr, int i);
		bool sync();
		int myRead(byte *input, int nByte);
		int myWrite(byte *input, int nByte);
		void recieveBuffer(void);
};

/*********************************************
 Network client module. This sends commands,
 and thus should go at the *end* of the local
 pipe (probably on the app side).
**********************************************/
class NetClientModule : public Module
{
	int mSocket[5];
	int numConnections;
	public:
		NetClientModule();

		bool process(list<Instruction> &i);
		bool sync();
		int myWrite(void* buf, int nByte);
		int myWrite(void* buf, unsigned nByte);
		int myWrite(void* buf, long unsigned nByte);
		int myRead(void *buf, size_t count);
		void sendBuffer();
};

/*********************************************
 Network compress module. This compresses each
 instruction/buffer to send over the network.
 Internal to NetSrvModule & NetClientModule
**********************************************/
class NetCompressModule : public Module
{
	public:
		NetCompressModule();

		bool process(list<Instruction> &i);
		void reply(Instruction *instr, int i);
		bool sync();
		int myCompress(void *input, int nByte, void *output);
		int myDecompress(void *dest, int destLen, void *source, int sourceLen);
};

/*********************************************
 InsertModule. Inserts additional instructions
 into the list for adding text or pictures
 over the top of the program.
**********************************************/
class InsertModule : public Module
{
	public:
		InsertModule();

		bool init();
		bool process(list<Instruction> &i);
		void reply(Instruction *instr, int i);
		bool sync();
};

/*********************************************
 Profiling Module, calculates the top 5
 Instructions getting called, and the top 5
 Instructions using the most buffer space
**********************************************/
class ProfileModule : public Module
{
	public:
		ProfileModule();

		bool process(list<Instruction> &i);
		void reply(Instruction *instr, int i);
		bool sync();
		void resetCounts();
		void output();
		void outputBuffers();
};

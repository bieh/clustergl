/*******************************************************************************
							The module interface
*******************************************************************************/


/*********************************************
			Module object
**********************************************/
class Module{
public:
	Module(){}

	list<Instruction> *prevFrame;	//used for comparing deltas (only used in NetClient and NetSrv)
	ulong netBytes;			// variable to calculate network usage
	ulong netBytes2;			// variable to calculate compressed network usage

	virtual bool process(list<Instruction> &i)=0;
	virtual void reply(Instruction *instr, int i){}
	virtual bool sync()=0;
};



/*********************************************
	A module that captures from a program
**********************************************/
class AppModule : public Module{
public:
	AppModule(string command);

	bool init(string command);	
	bool process(list<Instruction> &i);
	bool sync();
};


/*********************************************
	A module that outputs to stdout
**********************************************/
class TextModule : public Module{
public:
	TextModule();
	bool init();	
	bool process(list<Instruction> &i);
	bool sync();
};


/*********************************************
	A module that outputs to GL
**********************************************/
class ExecModule : public Module{
	bool makeWindow();
	
	int iScreenX;
	int iScreenY;
	int iOffsetX;
	int iOffsetY;
	int origWidth;
	int origHeight;
	int iScaleX;
	int iScaleY;
public:
	ExecModule(int sizeX, int sizeY, int offsetX, int offsetY, int scaleX, int scaleY);

	bool init();	
	bool process(list<Instruction> &i);
	bool sync();
};


/*********************************************
 Network server module. This recvs commands
 and thus should go at the *start* of the local
 pipe. Blocks till a client connects. 
**********************************************/
class NetSrvModule : public Module{
  int mSocket;
  BufferedFd *mClientSocket;
public:
	NetSrvModule(int port, bool decompression, bool replyCompression, int compressionLevel);
	
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
class NetClientModule : public Module{
  int mSocket;
public:
	NetClientModule(string address, int port, bool sendCompression, bool recieveCompression, int compressLevel, bool repeatInstruction);
	
	bool process(list<Instruction> &i);
	bool sync();
	int myWrite(int fd, void* buf, int nByte);
	int myWrite(int fd, void* buf, unsigned nByte);
	int myWrite(int fd, void* buf, long unsigned nByte);
	int myRead(int fd, void *buf, size_t count);
	void sendBuffer(int fd);
};

/*********************************************
 Network compress module. This compresses each
 instruction/buffer to send over the network.
**********************************************/
class NetCompressModule : public Module{
  int compressLevel;
public:
	NetCompressModule(int level);
	
	bool process(list<Instruction> &i);
	void reply(Instruction *instr, int i);
	bool sync();
	int myCompress(void *input, int nByte, void *output);
	int myDecompress(void *dest, int destLen, void *source, int sourceLen);
};

/*********************************************
Network keyboard module. Sends keyboard 
instuctions to each screen.
**********************************************/
class NetKeyboardModule : public Module{
public:
	NetKeyboardModule();
	
	bool process(list<Instruction> &i);
	void reply(Instruction *instr, int i);
	bool sync();
};

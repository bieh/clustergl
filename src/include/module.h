/*******************************************************************************
	ClusterGL - module.h
*******************************************************************************/

//Modules can operate on either instructions or byte bufs. Most of them should
//work on the instruction level, but things like compression or network read
//operate on bytes
const int MOD_TYPE_INSTR = 1; //takes/emits a list of instructions
const int MOD_TYPE_BYTES = 2; //takes/emits a byte buffer

/*******************************************************************************
	The main module interface
*******************************************************************************/
class Module
{
protected:
	vector<Instruction *> *mListResult;
	
public:
	Module(){}

    //input
	virtual bool process(vector<Instruction *> *i){}
	virtual bool process(byte *buf, int len){}
	
	//output
	virtual vector<Instruction *> *resultAsList(){return mListResult;}
	virtual byte *resultAsBytes(int *len){}
	
	//Config defaults
	virtual int getInputFormat(){return MOD_TYPE_INSTR;}
	virtual int getOutputFormat(){return MOD_TYPE_INSTR;}
	
	void setListResult(vector<Instruction *> *i){
		mListResult = i;
	}
	
	virtual void reply(Instruction *instr, int i){}
	virtual bool sync()=0;
};



/*******************************************************************************
	Capture from a program
*******************************************************************************/
class AppModule : public Module
{
public:
	AppModule(string command);

	bool init(string command);
	bool process(vector<Instruction *> *i);
	bool sync();
};


/*******************************************************************************
	Output to stdout
*******************************************************************************/
class TextModule : public Module
{
public:
	TextModule();
	bool init();
	bool process(vector<Instruction *> *i);
	bool sync();
};



/*******************************************************************************
	Output to OpenGL (executing the commands)
*******************************************************************************/
class ExecModule : public Module
{
    bool makeWindow();

public:
	ExecModule();

	bool init();
	bool process(vector<Instruction *> *i);
	bool sync();
};

/*******************************************************************************
 Network server module. This recvs commands and thus should go at the *start* 
 of the local pipeline. Blocks till a client connects.
*******************************************************************************/
class NetSrvModule : public Module
{
    int mSocket;
    BufferedFd *mClientSocket;
    
    int internalRead(byte *input, int nByte);
    int internalWrite(byte *input, int nByte);
    
    void recieveBuffer(void);
	
public:
	NetSrvModule();

	bool process(vector<Instruction *> *i);
	void reply(Instruction *instr, int i);
	bool sync();

};


/*******************************************************************************
  Network client module. This sends commands, and thus should go at the *end* 
  of the local pipe (probably on the app side).
*******************************************************************************/
class NetClientModule : public Module
{
    vector<int> mSockets;
    int numConnections;
    
    int internalWrite(void* buf, int nByte);
	int internalRead(void *buf, size_t count);
	
	void sendBuffer();
	
public:
	NetClientModule();

	bool process(vector<Instruction *> *i);
	bool sync();
		
	//Config defaults
	int getInputFormat(){return MOD_TYPE_INSTR;}
	int getOutputFormat(){return MOD_TYPE_INSTR;}

};

/*******************************************************************************
  Network client module. Like NCM, but multicasty!
*******************************************************************************/
class MulticastNetClientModule : public Module
{
public:
	MulticastNetClientModule();

	bool process(vector<Instruction *> *i);
	bool sync();
};


/*******************************************************************************
 Network compress module. This compresses each instruction/buffer to send over 
 the network. 
*******************************************************************************/
class NetCompressModule : public Module
{
	int myCompress(void *input, int nByte, void *output);
	int myDecompress(void *dest, int destLen, void *source, int sourceLen);
	
public:
	NetCompressModule();

	bool process(vector<Instruction *> *i);
	void reply(Instruction *instr, int i);
	bool sync();

};


/*******************************************************************************
 Insertion module. Insert instructions into a frame at runtime
*******************************************************************************/
class InsertModule : public Module
{
public:
	InsertModule();

	bool init();
	bool process(vector<Instruction *> *i);
	void reply(Instruction *instr, int i);
	bool sync();
};


/*******************************************************************************
 Profiling module
*******************************************************************************/
class ProfileModule : public Module
{
public:
	ProfileModule();

	bool process(vector<Instruction *> *i);
	void reply(Instruction *instr, int i);
	bool sync();
	void resetCounts();
	void output();
	void outputBuffers();
};



/*******************************************************************************
 Delta module. Remove duplicate instructions, replace with CGL_REPEAT's
*******************************************************************************/
class DeltaEncodeModule : public Module
{
	vector<Instruction *> lastFrame;
	
	Instruction *makeSkip(uint32_t n);
public:
	DeltaEncodeModule();
    
    //input
	bool process(vector<Instruction *> *i);
	
	//output
	vector<Instruction *> *resultAsList();
	
	//Config defaults
	int getInputFormat(){return MOD_TYPE_INSTR;}
	int getOutputFormat(){return MOD_TYPE_INSTR;}
	
	bool sync(){}
};

class DeltaDecodeModule : public Module
{	
	vector<Instruction *> lastFrame;
public:
	DeltaDecodeModule();
    
    //input
	bool process(vector<Instruction *> *i);
	
	//output
	vector<Instruction *> *resultAsList();
	
	//Config defaults
	int getInputFormat(){return MOD_TYPE_INSTR;}
	int getOutputFormat(){return MOD_TYPE_INSTR;}
	
	bool sync(){}
};

/*******************************************************************************
	ClusterGL - module.h
*******************************************************************************/

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
	virtual bool process(vector<Instruction *> *i){return i != NULL;}
	virtual bool process(byte *buf, int len){return (buf != NULL && len);}
	
	//output
	virtual vector<Instruction *> *resultAsList(){return mListResult;}
	
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
    
    bool handleViewMode(Instruction *i);

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
};


/*******************************************************************************
 Network server and client modules that uses OpenPGM for multicast
*******************************************************************************/
class MulticastSrvModule : public Module
{
    int mSocket;
    BufferedFd *mClientSocket;
    
    int internalRead(byte *input, int nByte);
    int internalWrite(byte *input, int nByte);
    
    void recieveBuffer(void);
	
public:
	MulticastSrvModule();

	bool process(vector<Instruction *> *i);
	void reply(Instruction *instr, int i);
	bool sync();

};

class MulticastClientModule : public Module
{
    vector<int> mSockets;
    int numConnections;
    
    int internalWrite(void* buf, int nByte);
	int internalRead(void *buf, size_t count);
	
	void sendBuffer();
	
public:
	MulticastClientModule();

	bool process(vector<Instruction *> *i);
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
	
	bool sync(){return true;}
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
		
	bool sync(){return true;}
};



/*******************************************************************************
 DuplicateBuffer module. Keeps a LRU of buffers+hash.
*******************************************************************************/
typedef LRUCache<uint32_t,byte *> lru_cache;	

class DuplicateBufferEncodeModule : public Module
{
	lru_cache *mLRU;
public:
	DuplicateBufferEncodeModule();
    
    //input
	bool process(vector<Instruction *> *i);
	
	//output
	vector<Instruction *> *resultAsList();
	
	bool sync(){return true;}
};

class DuplicateBufferDecodeModule : public Module
{	
	lru_cache *mLRU;
public:
	DuplicateBufferDecodeModule();
    
    //input
	bool process(vector<Instruction *> *i);
	
	//output
	vector<Instruction *> *resultAsList();
	
	bool sync(){return true;}
};



/*******************************************************************************
	Data compression global. Not really a global, but it used to be
*******************************************************************************/
class Compression{
public:
	static int decompress(void *dest, int destLen, int sourceLen);
	static int compress(void *input, int nByte);
	static byte *getBuf();
};

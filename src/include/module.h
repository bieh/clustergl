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
	int netBytes;			// variable to calculate network usage

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
public:
	ExecModule(int sizeX, int sizeY, int offsetX, int offsetY);

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
	NetSrvModule(int port);
	
	bool process(list<Instruction> &i);
	void reply(Instruction *instr, int i);
	bool sync();
};

/*********************************************
 Network client module. This sends commands,
 and thus should go at the *end* of the local
 pipe (probably on the app side). 
**********************************************/
class NetClientModule : public Module{
  int mSocket;
public:
	NetClientModule(string address, int port);
	
	bool process(list<Instruction> &i);
	bool sync();
};

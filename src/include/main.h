/*******************************************************************************
							main.h
*******************************************************************************/

/*********************************************
		     MULTICAST (or TCP)
**********************************************/
//#define MULTICAST true

/*********************************************
			Internal headers
**********************************************/
#include "libs.h"
#include "utils.h"
#include "instruction.h"
#include "module.h"

//Quick LOG hack that we can make a proper log system out of later
#define LOG printf

#define CGL_REPEAT_INSTRUCTION 1498
//#define MAX_INSTRUCTIONS 500000

/*********************************************
			Main application object
**********************************************/
class App{
	vector<Module *> mModules;  
	
	void init();
public:
	list<Instruction> oneFrame; //space to hold Instructions for current and previous frames
	list<Instruction> twoFrame; 

	//called when we're invoked from the command line
	int run(int argc, char **argv);
	
	//called when we're invoked from LD_PRELOAD
	int run_shared();
	
	bool tick();
	
	void debug();
	
};

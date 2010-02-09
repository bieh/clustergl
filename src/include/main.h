/*******************************************************************************
	main.h
*******************************************************************************/

/*********************************************
	Internal headers
**********************************************/
#include "libs.h"
#include "utils.h"
#include "instruction.h"
#include "module.h"

//Quick LOG hack that we can make a proper log system out of later
#define LOG printf

// used because SYMPHONY does not support all OpenGL commands (only version 1.4)
#define  SYMPHONY true
const float SYMPHONY_SCREEN_WIDTH = 1680.0;
const float SYMPHONY_SCREEN_TOTAL_WIDTH = 8880.0;
const float SYMPHONY_SCREEN_TOTAL_HEIGHT = 4560.0;
const float SYMPHONY_SCREEN_GAP = 120.0;

#define CGL_REPEAT_INSTRUCTION 1498
#define MAX_INSTRUCTIONS 500000

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

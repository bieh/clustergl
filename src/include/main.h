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


/*********************************************
			Main application object
**********************************************/
class App{
	vector<Module *> mModules;
	
	void init();
public:

	//called when we're invoked from the command line
	int run(int argc, char **argv);
	
	//called when we're invoked from LD_PRELOAD
	int run_shared();
	
	bool tick();
	
	void debug();
	
};

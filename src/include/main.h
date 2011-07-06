/*******************************************************************************
	ClusterGL - main.h
*******************************************************************************/


/*******************************************************************************
	Internal headers
*******************************************************************************/
#include "libs.h"
#include "config.h"
#include "utils.h"
#include "instruction.h"
#include "module.h"

/*******************************************************************************
	Main application object
*******************************************************************************/
class App
{
	vector<Module *> mModules;
	
	void init(bool shared, const char *id);
	
	void stats_begin();
	void stats_end();
	
	bool bIsIntercept;
	
	// pointer to the current frame
    list<Instruction> *thisFrame;
    	
public:

	//called when we're invoked from the command line
	int run(int argc, char **argv);

	//called when we're invoked from LD_PRELOAD
	int run_shared();

	bool tick();

	void debug();

};

//Global config instance
extern Config *gConfig;

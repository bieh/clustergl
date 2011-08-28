/*******************************************************************************
	ClusterGL - main.h
*******************************************************************************/

#define ENABLE_DL_INTERCEPT

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
		    	
public:

	//called when we're invoked from the command line
	int run(int argc, char **argv);

	//called when we're invoked from LD_PRELOAD
	//Will return false if we're not configured to run off this source
	bool run_shared(string src);

	bool tick();

	void debug();

};

extern bool bIsIntercept;

//Global config instance
extern Config *gConfig;

//mod_text
void LOG_INSTRUCTION(Instruction *instr);

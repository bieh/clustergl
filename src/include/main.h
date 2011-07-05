/*******************************************************************************
	ClusterGL - main.h
*******************************************************************************/


/*******************************************************************************
	Internal headers
*******************************************************************************/
#include "libs.h"
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
    
    //Counters
    uint32_t totalFrames;	
    time_t totalTime, prevTime;
    uint32_t startTime = 0;
    uint32_t endTime = 0;
	
public:

    //space to hold Instructions for current and previous frames
	list<Instruction> oneFrame;
	list<Instruction> twoFrame;

	//called when we're invoked from the command line
	int run(int argc, char **argv);

	//called when we're invoked from LD_PRELOAD
	int run_shared();

	bool tick();

	void debug();

};

//Global config instance
extern Config *gConfig;

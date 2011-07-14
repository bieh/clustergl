/*******************************************************************************
	ClusterGL - main.cpp
*******************************************************************************/
#include "main.h"


/*******************************************************************************
	Globals
*******************************************************************************/
static bool bHasInit = false;

Config *gConfig = NULL;

/*******************************************************************************
	Entry if invoked as a renderer output
*******************************************************************************/
int App::run(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr,"usage: %s <window id>\n", argv[0]);
		exit(0);
	}
	
	if(bHasInit) {
		return 1;
	}
	
	init(false, argv[1]);
	
	//Set up the module chain	
	mModules.push_back(new NetSrvModule());
	mModules.push_back(new DeltaDecodeModule()); 	
	mModules.push_back(new ExecModule());

	while( tick() ){ 
	    //run tick() until we decide to bail
	}
	
	return 0;
}


/*******************************************************************************
	Entry if invoked as capture (shared library)
*******************************************************************************/
int App::run_shared()
{
	//This is needed to ensure that multiple calls to SDL_Init() don't cause
	//us to do the wrong thing.
	if(bHasInit) {
		return 1;
	}
	
	init(true, "capture");
	
	for(int i=0;i<gConfig->numOutputs;i++){
		LOG("Found output: %s:%d\n", 
				gConfig->outputAddresses[i].c_str(), 
				gConfig->outputPorts[i]);
	}
			
	//Set up the module chain
	mModules.push_back(new AppModule(""));
	mModules.push_back(new DeltaEncodeModule()); 	
	mModules.push_back(new NetClientModule());

	//Return control to the parent process.

	return 0;
}

/*******************************************************************************
	Load the config file, init various internal variables
*******************************************************************************/
void App::init(bool shared, const char *id)
{
    printf("**********************************************\n");
	printf(" ClusterGL(%s - %s)\n", bIsIntercept ? "intercept" : "renderer", id);
	printf("**********************************************\n");	

    bIsIntercept = shared;
    
    char *configFile = "cgl.conf";
    
    if(getenv("CGL_CONFIG_FILE")){
    	configFile = getenv("CGL_CONFIG_FILE");
    }

	gConfig = new Config(configFile, string(id ? id : "null"));

	bHasInit = true;
}


/*******************************************************************************
	Main loop
*******************************************************************************/
bool App::tick()
{
	//LOG("tick()\n");
		
	vector<Instruction *> *thisFrame = new vector<Instruction *>();
	
	if(gConfig->enableStats){
	    stats_begin();
	}

	//Go through each module and process the frame
	for(int i=0;i<(int)mModules.size();i++) {
		Module *m = mModules[i];
		
		m->setListResult(thisFrame);
		
		if( !m->process(thisFrame) ) {
			LOG("Failed to process frame (in %d), bailing out\n", i);
			return false;
		}
		
		//TODO: handle bytes and such
		thisFrame = m->resultAsList();
		
		if(!thisFrame){
			LOG("!thisFrame\n");
			return false;
		}
	}

	//return appropriate frames
	for(int n=0;n<(int)thisFrame->size();n++){
		Instruction *iter = (*thisFrame)[n];
		for(int i=0;i<3;i++) {
			//If we need a reply, send it
			//But only do so if /we/ created the instruction
			if(iter->buffers[i].needReply /*&& iter->buffers[i].needClear*/) {
				LOG("need a reply\n");
				LOG_INSTRUCTION(iter);
				mModules[0]->reply(iter, i);
			}
		}
	}


	if(gConfig->enableStats){
	    stats_end();
	}
	
	for(int i=0;i<(int)thisFrame->size();i++){
		Instruction *iter = (*thisFrame)[i];
		iter->clear();
	}

	delete thisFrame;

	return true;
}

/*******************************************************************************
	Begin stats run
*******************************************************************************/
void App::stats_begin(){

}

/*******************************************************************************
	End stats run
*******************************************************************************/
void App::stats_end(){
 
}


/*******************************************************************************
	main()
*******************************************************************************/
App *theApp = NULL;

int main( int argc, char **argv )
{
	theApp = new App();
	int ret = theApp->run(argc, argv);
	delete theApp;
	return ret;
}

//The shared object entry is now in mod_app

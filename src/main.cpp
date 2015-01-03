/*******************************************************************************
	ClusterGL - main.cpp
*******************************************************************************/
#include "main.h"


/*******************************************************************************
	Globals
*******************************************************************************/
static bool bHasInit = false;
bool bIsIntercept = false;

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
	//mModules.push_back(new DuplicateBufferDecodeModule());
	mModules.push_back(new ExecModule());

	while( tick() ){ 
		//run tick() until we decide to bail
	}
	
	return 0;
}


/*******************************************************************************
	Entry if invoked as capture (shared library)
*******************************************************************************/
bool App::run_shared(string src)
{
	//This is needed to ensure that multiple calls to SDL_Init() don't cause
	//us to do the wrong thing.
	if(bHasInit) {
		return false;
	}
	
	//Load the config file
	init(true, "capture");
	
	//Make sure we're launching from the correct source. This means that mixing
	//SDL and Xorg works properly when we're fork()'ing. ie: OpenArena
	if(src != gConfig->interceptMode){
		LOG("Ignored spurious launch (possibly fork()?): %s vs %s\n", 
			src.c_str(), gConfig->interceptMode.c_str());
		LOG("If this is intended, change 'interceptMode' to '%s' in the config file\n", src.c_str());
		return false;
	}
	
	for(int i=0;i<gConfig->numOutputs;i++){
		LOG("Found output: %s:%d\n", 
				gConfig->outputAddresses[i].c_str(), 
				gConfig->outputPorts[i]);
	}
	
	//Write our pid out
	if(gConfig->capturePidFile != ""){
		FILE *f = fopen(gConfig->capturePidFile.c_str(), "w");
		char pid[64];
		sprintf(pid, "%d", getpid());
		fwrite(pid, strlen(pid), 1, f);
		fclose(f);
	}
	
			
	//Set up the module chain
	for (vector<string>::iterator it = gConfig->capturePipeline.begin(); it != gConfig->capturePipeline.end(); ++it) {
		Module* m;
		if(*it == "app") m = new AppModule("");
		else if(*it == "delta") m = new DeltaEncodeModule();
        	else if(*it == "netclient") m = new NetClientModule();
        	else if(*it == "text") m = new TextModule();
        	else if(*it == "profile") m = new ProfileModule();
        	else {
			LOG("Unknown module: %s\n", it->c_str());
			exit(1);
		}
		mModules.push_back(m);
	}

	//Return control to the parent process.
	return true;
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
	
	char *configFile = (char *)"/etc/cgl.conf";
	
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
			if(iter->buffers[i].needReply && iter->buffers[i].needRemoteReply) {
				//LOG("need a reply %d\n", i);
				//LOG_INSTRUCTION(iter);
				mModules[0]->reply(iter, i);
				
				iter->buffers[i].needReply = false;
				
				Stats::increment("Pipeline stalls due to replies");
			}
		}
	}


	if(gConfig->enableStats){
		stats_end();
	}
	
	for(int i=0;i<(int)thisFrame->size();i++){
		bool mustdelete = false;
		Instruction *iter = (*thisFrame)[i];
		if(iter->id==CGL_REPEAT_INSTRUCTION)
			mustdelete = true;
		//LOG_INSTRUCTION(iter);
		iter->clear();

		// this has to be deleted on renderers only
		// because they dynamically copy instructions in DeltaDecodeModule
		if(!bIsIntercept || mustdelete)
			delete iter;
	}
	thisFrame->clear();

	delete thisFrame;
	
	//LOG("tick() done\n");

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
 	Stats::update();
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

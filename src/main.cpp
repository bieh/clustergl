/*******************************************************************************
	ClusterGL - main.cpp
*******************************************************************************/
#include "main.h"


/*******************************************************************************
	Globals
*******************************************************************************/
static bool bHasInit = false;



/*******************************************************************************
	Entry if invoked as a renderer output
*******************************************************************************/
int App::run(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr,"usage: %s <window id>\n", argv[0]);
		exit(0);
	}
	LOG("Window ID: %s\n", argv[1]);
	
	if(bHasInit) {
		return 1;
	}
	
	init(false, argv[1]);

	LOG("Loading modules for network server and renderer output\n");
	
	//Set up the module chain	
	mModules.push_back(new NetSrvModule());
	//mModules.push_back(new InsertModule());
	mModules.push_back(new ExecModule());
	//mModules.push_back(new TextModule()); //output OpenGL method calls to console

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
	
	init(true, NULL);
		
	LOG("Loading modules for application intercept\n");
	
	//Set up the module chain
	mModules.push_back(new AppModule(""));
	//mModules.push_back(new TextModule()); //output OpenGL method calls to console
		
	//calculate instruction usage for the current program	
	if(gConfig->enableProfile) {
		mModules.push_back(new ProfileModule());
	}
	
	mModules.push_back(new NetClientModule());
    //mModules.push_back(new TextModule());

	//Return control to the parent process.

	return 0;
}

/*******************************************************************************
	Load the config file, init various internal variables
*******************************************************************************/
void App::init(bool shared, const char *id)
{
    bIsIntercept = shared;

	gConfig = new Config("config.conf");
	
	thisFrame = NULL;
	totalFrames = 0;
	totalTime = 0;
	prevTime = 0;
	startTime = 0;
	endTime = 0;

	LOG("\n");
	LOG("**********************************************\n");
	LOG(" ClusterGL(%s - %s)\n", bIsIntercept ? "intercept" : "renderer", id);
	LOG("**********************************************\n");
	bHasInit = true;
}


/*******************************************************************************
	Main loop
*******************************************************************************/
bool App::tick()
{
	totalFrames++; //used to calculate when to sync
	
	if(gConfig->enableStats){
	    stats_begin();
	}
/*
	//Make sure we have a real frame
	if (!thisFrame) {
		thisFrame = &oneFrame;
		for(int i=0;i<(int)mModules.size();i++)
			mModules[i]->prevFrame = &twoFrame;
	}
*/

	//Go through each module and process the frame
	for(int i=0;i<(int)mModules.size();i++) {
		Module *m = mModules[i];
		if( !m->process(*thisFrame) ) {
			LOG("Failed to process frame (in %d), bailing out\n", i);
			return false;
		}
	}

	//return appropriate frames
	for(std::list<Instruction>::iterator iter = thisFrame->begin();
	    iter != (*thisFrame).end(); iter++) {
		for(int i=0;i<3;i++) {
			//A bit dodgy. This is how we determine if it was created on this
			//end of the network
			if(iter->buffers[i].needReply && iter->buffers[i].needClear) {
				mModules[0]->reply(&(*iter), i);
			}
		}
	}
/*
	//Sync frames if necessary
	if(gConfig->syncRate > 0) {
		if (totalFrames % syncRate == 0 && totalFrames > 0) {
			for(int i=0;i<(int)mModules.size();i++) {
				Module *m = mModules[i];
				if( !m->sync() ) {
					LOG("Failed to sync frame (in %d), bailing out\n", i);
					return false;
				}
			}
		}
	}
*/

	if(gConfig->enableStats){
	    stats_end();
	}

/*	
	//Swap frames
	for(int i=0;i<(int)mModules.size();i++){
		mModules[i]->prevFrame = thisFrame;

        
	if(thisFrame == &oneFrame){
		thisFrame = &twoFrame;
	}else{
		thisFrame = &oneFrame;
	}

	//clear previous frames
	for(std::list<Instruction>::iterator iter = thisFrame->begin();
	iter != (*thisFrame).end(); iter++) {
		iter->clear();
	}
	thisFrame->clear();    
*/	

	return true;
}

/*******************************************************************************
	Begin stats run
*******************************************************************************/
void App::stats_begin(){

    if(totalTime == 0){
		time(&totalTime);
		time(&prevTime);
	}

	time_t curTime;
	time(&curTime);
	
	
	LOG("Last %ld Seconds:\n", curTime - prevTime);
	
	// First calculate FPS	
	int FPS = 0;
		
	if(FPS > 0) {
	    LOG("ClusterGL2 Average FPS:\t\t\t%ld\n",
		    FPS/(curTime - prevTime));
	}

	// Now Calculate KBPS (Kbytes per second)
	int bytes = 0;
	int bytes_compressed = 0;
	/*
	if(bytes > 0) {
		LOG("ClusterGL2 Average KBPS:\t\t%lf\n",
			(bytes/(curTime - prevTime))/1024.0);
		LOG("ClusterGL2 Average compressed KBPS:\t%lf\n\n",
			(bytes2/(curTime - prevTime))/1024.0);
	}
	*/
	
	time(&prevTime);
	
	if(gConfig->enableProfile){
    	//profile->output();
	}

}

/*******************************************************************************
	End stats run
*******************************************************************************/
void App::stats_end(){

    endTime = SDL_GetTicks();    
	uint32_t diff = endTime - startTime;
	int FPS = 0;
	/*
	for(int i=0;i<(int)mModules.size();i++) {
        Module *m = mModules[i];
        FPS += m->frames;
        m->frames = 0;
    }
    */
    
    /*
    if(FPS > 0 && intercept) {
        LOG("ClusterGL2 FPS: %f\n", 1000.0/diff);
    }
    */

	startTime = SDL_GetTicks();
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

#include "main.h"

/********************************************************
	Main Globals (Loaded from config file)
********************************************************/

bool bHasInit = false;
int sizeX;
int sizeY;
int sizeSYMPHONYX;
int sizeSYMPHONYY;
int offsetX;
int offsetY;
float scaleX;
float scaleY;
string dnNumber;
int port;
int syncRate;
int compressingMethod;
bool usingSendCompression;
bool usingReplyCompression;
bool useRepeat;
bool useSYMPHONYnodes[5];
			
/********************************************************
	Application Object
********************************************************/
int App::run(int argc, char **argv){
    	if (argc != 2) {
       		fprintf(stderr,"usage: cgl <SYMPHONY number. Use 0 for testing, 1-5 for SYMPHONY>\n");
	       exit(0);
	}
	dnNumber = argv[1];
	if(bHasInit){
		return 1;
	}
	init();
	
	LOG("Loading modules for network server and renderer output on port: %d\n", port);
	mModules.push_back(new NetSrvModule(port, usingSendCompression, usingReplyCompression, compressingMethod));
	mModules.push_back(new ExecModule(sizeX, sizeY, offsetX, offsetY, scaleX, scaleY));
	//mModules.push_back(new TextModule()); //output OpenGL method calls to console
	
	while( tick() ){ }
	return 0;
}

int App::run_shared(){
	//This is needed to ensure that multiple calls to SDL_Init() don't cause
	//us to do the wrong thing.
	if(bHasInit){
		return 1;
	}
	init();
	
	LOG("Loading modules for application intercept\n");
	mModules.push_back(new AppModule(""));
	//mModules.push_back(new TextModule()); //output OpenGL method calls to console
	mModules.push_back(new NetClientModule(port, usingSendCompression, usingReplyCompression, compressingMethod, useRepeat));

	//Return control to the parent process.
	
	return 0;
}

void App::init(){
	//load values in from config file
    cfg_opt_t opts[] = {
	CFG_SIMPLE_INT((char *)("sizeX"), &sizeX),
	CFG_SIMPLE_INT((char *)("sizeY"), &sizeY),
	CFG_SIMPLE_INT((char *)("sizeSYMPHONYX"), &sizeSYMPHONYX),
	CFG_SIMPLE_INT((char *)("sizeSYMPHONYY"), &sizeSYMPHONYY),
	CFG_SIMPLE_INT((char *)("offsetX"), &offsetX),
        CFG_SIMPLE_INT((char *)("offsetY"), &offsetY),
        CFG_SIMPLE_INT((char *)("port"), &port),
	CFG_FLOAT((char *)("scaleX"), 1.0f, CFGF_NONE),
        CFG_FLOAT((char *)("scaleY"), 1.0f, CFGF_NONE),
	CFG_SIMPLE_INT((char *)("syncRate"), &syncRate),
	CFG_SIMPLE_INT((char *)("compressingMethod"), &compressingMethod),
	CFG_SIMPLE_BOOL((char *)("usingSendCompression"), &usingSendCompression),
	CFG_SIMPLE_BOOL((char *)("usingReplyCompression"), &usingReplyCompression),
	CFG_SIMPLE_BOOL((char *)("useCGLRepeat"), &useRepeat),
	CFG_BOOL_LIST((char *)("SYMPHONYnodes"), (char *)"{false}", CFGF_NONE),
        CFG_END()
    };
    cfg_t *cfg;

    cfg = cfg_init(opts, CFGF_NONE);
    cfg_parse(cfg, "config.conf");
    for(uint32_t i = 0; i < cfg_size(cfg, "SYMPHONYnodes"); i++)
    	useSYMPHONYnodes[i] = cfg_getnbool(cfg, "SYMPHONYnodes", i);
    
    scaleX = cfg_getfloat(cfg,(char *) "scaleX");
    scaleY = cfg_getfloat(cfg,(char *) "scaleY");
    cfg_free(cfg);


	//adjust offset and size values for symphony display nodes
	#ifdef SYMPHONY
		int dn = atoi(dnNumber.c_str());
		offsetX = (int) (SYMPHONY_SCREEN_WIDTH + SYMPHONY_SCREEN_GAP) * (dn - 1);
		sizeX = sizeSYMPHONYX;
		sizeY = sizeSYMPHONYY;
	#endif

	LOG("\n");
	LOG("**********************************************\n");
	LOG("                 ClusterGL\n");
	LOG("**********************************************\n");
	bHasInit = true;
}

void App::debug(){
	LOG("******************* %d modules ***************\n", mModules.size());
} 

list<Instruction> *thisFrame = NULL; // pointer to the current frame
uint32_t frames = 0, totFrames = 0; //used for Calculations
time_t totalTime = 0, prevTime = 0;

bool App::tick(){
	frames++;
	totFrames++; //used to calculate when to SYNC

	if (totalTime == 0){ // initlise time for calculating statistics
	   time(&totalTime);	
	   time(&prevTime);
	}

	time_t curTime;
	time(&curTime);
	// Output FPS and BPS
	if (curTime - prevTime>= 5){ //maybe need more precision	
		LOG("Last %ld Seconds:\n", curTime - prevTime);	
		// First calculate FPS
		LOG("ClusterGL2 Average FPS:\t\t\t%ld\n",
			frames/(curTime - prevTime));
		frames = 0;

		// Now Calculate KBPS (Kbytes per second)
		int bytes = 0;
		int bytes2 = 0;
		for(int i=0;i<(int)mModules.size();i++){
			Module *m = mModules[i];	
			bytes += m->netBytes;
			bytes2 += m->netBytes2;
			m->netBytes = 0;
			m->netBytes2 = 0;
			if(i == (int)mModules.size() - 1 && bytes > 0){
				LOG("ClusterGL2 Average KBPS:\t\t%lf\n",
					(bytes/(curTime - prevTime))/1024.0);
				LOG("ClusterGL2 Average compressed KBPS:\t%lf\n\n",
					(bytes2/(curTime - prevTime))/1024.0);
			}
		}
	        time(&prevTime);	
	}

	//Sync frames
	if(syncRate > 0)
	{
		if (totFrames%syncRate == 0 && totFrames > 0){
			for(int i=0;i<(int)mModules.size();i++){
				Module *m = mModules[i];		
				if( !m->sync() ){
					LOG("Failed to sync frame (in %d), bailing out\n", i);
					return false;
				}
			}
		}
	}

	if (thisFrame == NULL){//initlize frames
		thisFrame = &oneFrame;
		for(int i=0;i<(int)mModules.size();i++)
			mModules[i]->prevFrame = &twoFrame;
	}
	
	for(int i=0;i<(int)mModules.size();i++){
		Module *m = mModules[i];		
		if( !m->process(*thisFrame) ){
			LOG("Failed to process frame (in %d), bailing out\n", i);
			return false;
		}
	}
	
	
	for(std::list<Instruction>::iterator iter = thisFrame->begin(); 
	    iter != (*thisFrame).end(); iter++){
	    
	    for(int i=0;i<3;i++){	    
	    	//A bit dodgy. This is how we determine if it was created on this
	    	//end of the network
	    	if(iter->buffers[i].needReply && iter->buffers[i].needClear){
	    		mModules[0]->reply(&(*iter), i);
	    	}
	    }	    
		//iter->clear();
	}
	
	//swap frames
	for(int i=0;i<(int)mModules.size();i++)
		mModules[i]->prevFrame = thisFrame;
	if (thisFrame == &oneFrame)
		thisFrame = &twoFrame;
	else
		thisFrame = &oneFrame;
	for(std::list<Instruction>::iterator iter = thisFrame->begin(); // clear previous frame so this frame can be drawn
	    iter != (*thisFrame).end(); iter++){
		iter->clear();
	}
	thisFrame->clear();
	
	return true;
}
			
/********************************************************
	Entry Points
********************************************************/
App *theApp = NULL;

int main( int argc, char **argv )
{		
	theApp = new App();
	int ret = theApp->run(argc, argv);	
	delete theApp;	
	return ret;
}

//The shared object entry is now in mod_app

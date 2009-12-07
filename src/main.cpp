#include "main.h"

bool bHasInit = false;
/* default values */
int sizeX = 800;
int sizeY = 600;
int offsetX = 0;
int offsetY = 0;
string dnNumber = "0";
int port;

/*******************************************************************************
				Application object
*******************************************************************************/
int App::run(int argc, char **argv){
    	if (argc != 2) {
       		fprintf(stderr,"usage: cgl <SYMPHONY number. Use 0 for testing>\n");
	       exit(0);
	}
	dnNumber = argv[1];
	if(bHasInit){
		return 1;
	}
	init();
	
	LOG("Loading modules for network server and renderer output on port: %d\n", port);
	mModules.push_back(new NetSrvModule(port));
	mModules.push_back(new ExecModule(sizeX, sizeY, offsetX, offsetY));
	//mModules.push_back(new TextModule());
	
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
	//mModules.push_back(new TextModule());
	mModules.push_back(new NetClientModule("127.0.0.1", port+11));
	mModules.push_back(new NetClientModule("127.0.0.1", port+12));
	mModules.push_back(new NetClientModule("127.0.0.1", port+13));
	mModules.push_back(new NetClientModule("127.0.0.1", port+14));
	mModules.push_back(new NetClientModule("127.0.0.1", port+15));
	//mModules.push_back(new NetClientModule("127.0.0.1", port+16));

#ifdef SYMPHONY
	// Symphony ip addys (if this is run from dn1 then we use local host above)
	//mModules.push_back(new NetClientModule("192.168.22.101", 12345));//dn1
	mModules.push_back(new NetClientModule("192.168.22.102", 12345));//dn2
	mModules.push_back(new NetClientModule("192.168.22.103", 12345));//dn3
	mModules.push_back(new NetClientModule("192.168.22.104", 12345));//dn4
	mModules.push_back(new NetClientModule("192.168.22.105", 12345));//dn5
#endif

	
	//Return control to the parent process.
	
	return 0;
}

void App::init(){
    cfg_opt_t opts[] = {
	CFG_SIMPLE_INT((char *)("sizeX"), &sizeX),
	CFG_SIMPLE_INT((char *)("sizeY"), &sizeY),
	CFG_SIMPLE_INT((char *)("offsetX"), &offsetX),
        CFG_SIMPLE_INT((char *)("offsetY"), &offsetY),
        CFG_SIMPLE_INT((char *)("port"), &port),
        CFG_END()
    };
    cfg_t *cfg;

    cfg = cfg_init(opts, 0);
    cfg_parse(cfg, "config.conf");
    cfg_free(cfg);

	#ifdef SYMPHONY
		int dn = atoi(dnNumber.c_str());
		offsetX = offsetX * ((1680 + 120) * (dn - 1));
	#else
		//ugly horrible way of setting up windows
		/*int num = atoi(dnNumber.c_str());
		port = port + num-10;
		if(num == 11)
		{
		offsetX = 0;
		offsetY = 0;
		}
		else if(num == 12)
		{
		offsetX = 160;
		offsetY = 0;
		}
		else if(num == 13)
		{
		offsetX = 320;
		offsetY = 0;
		}
		else if(num == 14)
		{
		offsetX = 480;
		offsetY = 0;
		}
		else if(num == 15)
		{
		offsetX = 640;
		offsetY = 0;
		}*/

	#endif

	LOG("\n");
	LOG("**********************************************\n");
	LOG("               ClusterGL\n");
	LOG("**********************************************\n");
	bHasInit = true;
}

void App::debug(){
	LOG("******************* %d modules ********************\n", mModules.size());
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
		// First calculat FPS
		LOG("ClusterGL2 Average FPS:\t\t\t%ld\n",
			frames/(curTime - prevTime));
		frames = 0;

		// Now Calculate BPS (bytes per second)
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

	//Sync every 20 frames
	if (totFrames%20 == 0){
		for(int i=0;i<(int)mModules.size();i++){
			Module *m = mModules[i];		
			if( !m->sync() ){
				LOG("Failed to sync frame (in %d), bailing out\n", i);
				return false;
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
	    iter->clear();
	}
	
	// swap frames
	for(int i=0;i<(int)mModules.size();i++)
		mModules[i]->prevFrame = thisFrame;
	if (thisFrame == &oneFrame)
		thisFrame = &twoFrame;
	else
		thisFrame = &oneFrame;
	thisFrame->clear();// clear previous frame so this frame can be drawn
	
	return true;
}


/*******************************************************************************
				Entry points
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

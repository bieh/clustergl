/********************************************************
	Headers
********************************************************/

#include "main.h"

/********************************************************
	Main Globals
********************************************************/

bool bHasInit = false;
bool profileApp = false;
ProfileModule *profile = NULL;
bool intercept = false;

/********************************************************
	Main Globals (Loaded from config file)
********************************************************/

/* testing or display wall */
bool symphony;

/* screen sizes */
int sizeX;
int sizeY;
int sizeSYMPHONYX;
int sizeSYMPHONYY;
int offsetX;
int offsetY;
float scaleX;
float scaleY;
int fakeWindowX;
int fakeWindowY;
bool glFrustumUsage;
bool bezelCompensation;

/* connection */
//string addresses[5];
int port;
bool multicast;
char * multicastServer;

/* clusterGL configs */
int syncRate;
int compressingMethod;
bool usingSendCompression;
bool usingReplyCompression;
bool useRepeat;

bool useSYMPHONYnodes[5];
string addresses[5];

uint32_t startTime = 0;
uint32_t endTime = 0;

/********************************************************
	Application Object
********************************************************/

int App::run(int argc, char **argv)
{
	if (argc != 2) {
		fprintf(stderr,"usage: cgl <SYMPHONY number. Use 0 for testing, 1-5 for SYMPHONY>\n");
		exit(0);
	}
	LOG("Console DNnumber: %s\n", argv[1]);
	int dnNumber = atoi(argv[1]);
	if(bHasInit) {
		return 1;
	}
	init(false, dnNumber);

	LOG("Loading modules for network server and renderer output on port: %d\n", port);
	mModules.push_back(new NetSrvModule());
	//mModules.push_back(new InsertModule());
	mModules.push_back(new ExecModule());
	//mModules.push_back(new TextModule()); //output OpenGL method calls to console

	while( tick() ){ }
	return 0;
}


int App::run_shared()
{
	//This is needed to ensure that multiple calls to SDL_Init() don't cause
	//us to do the wrong thing.
	if(bHasInit) {
		return 1;
	}
	init(true, -1);
	LOG("Loading modules for application intercept\n");
	intercept = true;
	mModules.push_back(new AppModule(""));
	//mModules.push_back(new TextModule()); //output OpenGL method calls to console
	if(profileApp) {
		profile = new ProfileModule();
		//calculate instruction usage for the current program
		mModules.push_back(profile);
	}
	mModules.push_back(new NetClientModule());
//	mModules.push_back(new TextModule());
	//Return control to the parent process.

	return 0;
}

void App::init(bool shared, int dn)
{
	//load values in from config file
	cfg_opt_t opts[] = {
		CFG_SIMPLE_BOOL((char *)("symphony"), &symphony),
		CFG_SIMPLE_INT((char *)("sizeSYMPHONYX"), &sizeSYMPHONYX),
		CFG_SIMPLE_INT((char *)("sizeSYMPHONYY"), &sizeSYMPHONYY),
		CFG_BOOL_LIST((char *)("SYMPHONYnodes"), (char *)"{false}", CFGF_NONE),
		CFG_STR_LIST((char *)("addresses"), (char *)"127.0.0.1", CFGF_NONE),
		CFG_SIMPLE_INT((char *)("sizeX"), &sizeX),
		CFG_SIMPLE_INT((char *)("sizeY"), &sizeY),
		CFG_SIMPLE_INT((char *)("offsetX"), &offsetX),
		CFG_SIMPLE_INT((char *)("offsetY"), &offsetY),
		CFG_SIMPLE_INT((char *)("fakeWindowX"), &fakeWindowX),
		CFG_SIMPLE_INT((char *)("fakeWindowY"), &fakeWindowY),
		CFG_SIMPLE_BOOL((char *)("bezelCompensation"), &bezelCompensation),
		CFG_SIMPLE_BOOL((char *)("glFrustumUsage"), &glFrustumUsage),
		CFG_SIMPLE_INT((char *)("port"), &port),
		CFG_SIMPLE_BOOL((char *)("multicast"), &multicast),
		CFG_SIMPLE_STR((char *)("multicastServer"), &multicastServer),
		CFG_FLOAT((char *)("scaleX"), 1.0f, CFGF_NONE),
		CFG_FLOAT((char *)("scaleY"), 1.0f, CFGF_NONE),
		CFG_SIMPLE_INT((char *)("syncRate"), &syncRate),
		CFG_SIMPLE_INT((char *)("compressingMethod"), &compressingMethod),
		CFG_SIMPLE_BOOL((char *)("usingSendCompression"), &usingSendCompression),
		CFG_SIMPLE_BOOL((char *)("usingReplyCompression"), &usingReplyCompression),
		CFG_SIMPLE_BOOL((char *)("useCGLRepeat"), &useRepeat),
		CFG_END()
	};
	cfg_t *cfg;

	cfg = cfg_init(opts, CFGF_NONE);
	cfg_parse(cfg, "config.conf");

	//process lists in the config file
	for(uint32_t i = 0; i < cfg_size(cfg, "SYMPHONYnodes"); i++)
		useSYMPHONYnodes[i] = cfg_getnbool(cfg, "SYMPHONYnodes", i);

	for(uint32_t i = 0; i < cfg_size(cfg, "addresses"); i++) {
		addresses[i] = string(cfg_getnstr(cfg, "addresses", i));
	}

	//process floats in the config file
	scaleX = cfg_getfloat(cfg,(char *) "scaleX");
	scaleY = cfg_getfloat(cfg,(char *) "scaleY");
	cfg_free(cfg);

	//adjust offset and size values for symphony display nodes
	// (saves having 5 unique config files)
	if(symphony) {
		offsetX = (int) (SYMPHONY_SCREEN_WIDTH + SYMPHONY_SCREEN_GAP) * (dn - 1);
		sizeX = sizeSYMPHONYX;
		sizeY = sizeSYMPHONYY;
	}
	LOG("\n");
	LOG("**********************************************\n");
	LOG("                 ClusterGL(%s)\n", shared ? "intercept" : "renderer");
	LOG("**********************************************\n");
	bHasInit = true;
}


void App::debug()
{
	LOG("******************* %d modules ***************\n", mModules.size());
}

/********************************************************
	Tick (main loop)
********************************************************/

// pointer to the current frame
list<Instruction> *thisFrame = NULL;
uint32_t totFrames = 0;			 //used for Calculations
time_t totalTime = 0, prevTime = 0;

bool App::tick()
{
	totFrames++;				 //used to calculate when to SYNC
/*
	if (totalTime == 0) {		 // initlise time for calculating statistics
		time(&totalTime);
		time(&prevTime);
	}

	time_t curTime;
	time(&curTime);
	// Output FPS and BPS
								 //maybe need more precision
	//if (curTime - prevTime>= 5) {
		LOG("Last %ld Seconds:\n", curTime - prevTime);
		// First calculate FPS
		int FPS = 0;
		for(int i=0;i<(int)mModules.size();i++) {
			Module *m = mModules[i];
			FPS += m->frames;
			m->frames = 0;
		}
		if(FPS > 0) {
		LOG("ClusterGL2 Average FPS:\t\t\t%ld\n",
			FPS/(curTime - prevTime));
		}
*/

	/*	// Now Calculate KBPS (Kbytes per second)
		int bytes = 0;
		int bytes2 = 0;
		for(int i=0;i<(int)mModules.size();i++) {
			Module *m = mModules[i];
			bytes += m->netBytes;
			bytes2 += m->netBytes2;
			m->netBytes = 0;
			m->netBytes2 = 0;
		}
		if(bytes > 0) {
			LOG("ClusterGL2 Average KBPS:\t\t%lf\n",
				(bytes/(curTime - prevTime))/1024.0);
			LOG("ClusterGL2 Average compressed KBPS:\t%lf\n\n",
				(bytes2/(curTime - prevTime))/1024.0);
		}
		time(&prevTime);
		if(profileApp) profile->output();
	//}*/

	//initlize frames
	if (thisFrame == NULL) {
		thisFrame = &oneFrame;
		for(int i=0;i<(int)mModules.size();i++)
			mModules[i]->prevFrame = &twoFrame;
	}

	//process frames
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

	//Sync frames
	if(syncRate > 0) {
		if (totFrames%syncRate == 0 && totFrames > 0) {
			for(int i=0;i<(int)mModules.size();i++) {
				Module *m = mModules[i];
				if( !m->sync() ) {
					LOG("Failed to sync frame (in %d), bailing out\n", i);
					return false;
				}
			}
		}
	}

	//swap frames
	for(int i=0;i<(int)mModules.size();i++)
		mModules[i]->prevFrame = thisFrame;
	if (thisFrame == &oneFrame)
		thisFrame = &twoFrame;
	else
		thisFrame = &oneFrame;

	//clear previous frames
	for(std::list<Instruction>::iterator iter = thisFrame->begin();
	iter != (*thisFrame).end(); iter++) {
		iter->clear();
	}
	thisFrame->clear();

	/*endTime = SDL_GetTicks();
	uint32_t diff = endTime - startTime;
	int FPS = 0;
	for(int i=0;i<(int)mModules.size();i++) {
        Module *m = mModules[i];
        FPS += m->frames;
        m->frames = 0;
    }
        if(FPS > 0 && intercept) {
        LOG("ClusterGL2 FPS: %f\n", 1000.0/diff);
        }

	startTime = SDL_GetTicks();*/
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

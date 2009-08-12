#include "main.h"

bool bHasInit = false;

/*******************************************************************************
							Application object
*******************************************************************************/
int App::run(int argc, char **argv){
    	if (argc != 5) {
       		fprintf(stderr,"usage: cgl <sizeX> <sizeY> <offsetX> <offsetY>\n");
	       exit(0);
	}

	if(bHasInit){
		return 1;
	}

	init();
	
	LOG("Loading modules for network server and renderer output\n");
	
	mModules.push_back(new NetSrvModule(12345));
	//mModules.push_back(new TextModule());
	mModules.push_back(new ExecModule(atoi(argv[1]), atoi(argv[2]), atoi(argv[3]), atoi(argv[4])));
	
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
	mModules.push_back(new NetClientModule("127.0.0.1", 12345));
	//mModules.push_back(new NetClientModule("130.217.244.59", 12345));
	
	//Return control to the parent process.
	
	return 0;
}

void App::init(){
	LOG("\n");
	
	LOG("**********************************************\n");
	LOG("               ClusterGL\n");
	LOG("**********************************************\n");

	bHasInit = true;
}

void App::debug(){
	LOG("******************* %d modules ********************\n", mModules.size());
} 

list<Instruction> *thisFrame = NULL;
	int frames = 0, totFrames = 0;
	time_t totalTime = 0, prevTime = 0;

bool App::tick(){
	frames++;
	totFrames++;
	if (totalTime == 0){		
	   time(&totalTime);	
	   time(&prevTime);
	}
	time_t curTime;
	time(&curTime);
	if (curTime - prevTime>= 5){ //maybe ned more precision
		// send it to stderr so we can redirect away from app output 
		fprintf(stderr,"CGL2 AVG FPS so far:%ld AVG FPS last 5secs:%ld\n", 
			totFrames/(curTime - totalTime),
			frames/(curTime - prevTime));
		frames = 0;
	        time(&prevTime);		
	}

	if (thisFrame ==NULL){
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
	if (thisFrame == &oneFrame){
		thisFrame = &twoFrame;
	}
	else{
		thisFrame = &oneFrame;
	}
	thisFrame->clear();
	
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

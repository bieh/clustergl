#include "main.h"

bool bHasInit = false;

/*******************************************************************************
							Application object
*******************************************************************************/
int App::run(int argc, char **argv){

	if(bHasInit){
		return 1;
	}

	init();
	
	LOG("Loading modules for network server and renderer output\n");
	
	mModules.push_back(new NetSrvModule(12345));
	//mModules.push_back(new TextModule());
	mModules.push_back(new ExecModule(640, 480, 0, 0));
	
	while( tick() ){ }

	return 0;
}

int App::run_shared(){
	
	if(bHasInit){
		return 1;
	}

	init();
	
	LOG("Loading modules for application intercept\n");
	
	mModules.push_back(new AppModule(""));
	//mModules.push_back(new TextModule());
	mModules.push_back(new NetClientModule("127.0.0.1", 12345));
	
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

list<Instruction> thisFrame;

bool App::tick(){

	//if(mModules.size() > 0){
	//	LOG("Tick %d\n", mModules.size());
	//}
	
	for(int i=0;i<(int)mModules.size();i++){
		//LOG("%d\n", i);
		Module *m = mModules[i];		
		if( !m->process(thisFrame) ){
			LOG("Failed to process frame (in %d), bailing out\n", i);
			return false;
		}
	}
	
	
	for(std::list<Instruction>::iterator iter = thisFrame.begin(); 
	    iter != thisFrame.end(); iter++){
	    
	    for(int i=0;i<3;i++){
	    
	    	//A bit dodgy. This is how we determine if it was created on this
	    	//end of the network
	    	if(iter->buffers[i].needReply && iter->buffers[i].needClear){
	    		mModules[0]->reply(&(*iter), i);
	    	}
	    }
	    
	    iter->clear();
	}
	
	thisFrame.clear();
	
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

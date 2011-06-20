#include "main.h"

bool Application::init(vector<string> args){

	if(!mSurface.init(1024, 768, false)){ //888, 456
		return false;
	}
	
	if(!mPresentation.init(args)){
		return false;
	}
	
	LOG("Init done!\n");
	
	return true;
}

bool Application::shutdown(){
	
	if(!mSurface.shutdown()){
		return false;
	}
	
	mPresentation.shutdown();
	
	return true;
}

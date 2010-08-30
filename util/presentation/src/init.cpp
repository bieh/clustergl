#include "main.h"

bool Application::init(vector<string> args){

	if(!mSurface.init(888, 456, false)){
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

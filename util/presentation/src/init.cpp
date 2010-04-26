#include "main.h"

bool Application::init(){

	if(!mSurface.init(640, 480, false)){
		return false;
	}
	
	LOG("Init done!\n");
	
	return true;
}

bool Application::shutdown(){
	
	if(!mSurface.shutdown()){
		return false;
	}
	
	return true;
}

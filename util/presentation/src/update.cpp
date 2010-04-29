#include "main.h"

bool Application::update(){

	processEvents();

	return true;
}


void Application::onMouseEvent(int button, int event){

}

void Application::onKeyEvent(int keycode, int event){

	if(event != SDL_KEYUP){
		return;
	}

	if(keycode == SDLK_ESCAPE){
		requestShutdown();
	}
	
	if(keycode == SDLK_SPACE){
		mPresentation.next();
	}
}


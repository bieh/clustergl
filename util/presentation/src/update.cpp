#include "main.h"

extern bool presentationFinished;

bool Application::update(){

	processEvents();

	if(presentationFinished){
		requestShutdown();
	}

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
	
	if(keycode == SDLK_p){
		mPresentation.prev();
	}

	if(keycode == SDLK_r){
        mPresentation.toggleReadabilityMovement();
	}
}


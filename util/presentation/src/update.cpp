#include "main.h"

bool Application::update(){

	processEvents();

	return true;
}


void Application::onMouseEvent(int button, int event){

}

void Application::onKeyEvent(int keycode, int event){
	requestShutdown();
}


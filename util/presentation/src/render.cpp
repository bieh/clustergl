#include "main.h"

void Application::render(){

	mSurface.beginRender();
		
	mPresentation.render();
	
	mSurface.endRender();
}

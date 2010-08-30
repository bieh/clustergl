#include "main.h"

void Application::render(){

	//LOG("2\n");

	mSurface.beginRender();
		
	mPresentation.render();

	mSurface.begin2D();

	mPresentation.render2D();
	
	mSurface.endRender();
}

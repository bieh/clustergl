#include "main.h"

void Application::render(){

	mSurface.beginRender();
		
	mPresentation.render();

	mSurface.begin2D();

	mPresentation.render2D();
	
	mSurface.endRender();
}

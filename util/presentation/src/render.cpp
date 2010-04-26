#include "main.h"

void Application::render(){

	mSurface.beginRender();
	
	gluLookAt(	5, 5, 5, 
				0, 0, 0, 
				0, 1, 0	);
	
	mPresentation.render();
	
	mSurface.endRender();
}

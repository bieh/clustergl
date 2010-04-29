#include "main.h"

void Application::render(){

	mSurface.beginRender();
	
	gluLookAt(	0, 0, 1, 
				0, 0, 0, 
				0, 1, 0	);
	
	mPresentation.render();
	
	mSurface.endRender();
}

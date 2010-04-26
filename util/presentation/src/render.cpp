#include "main.h"

void Application::render(){

	mSurface.beginRender();
	
	gluLookAt(	10, 10, 10, 
				0, 0, 0, 
				0, 1, 0	);
	
	mGLU.cube(1,1,1);
	
	mSurface.endRender();
}

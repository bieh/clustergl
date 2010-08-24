#include "main.h"

void Presentation::doSimpleTransition(bool init){

	static float fZoom = 1.0f;
	static float fZoomVel = 0.1f;
	static float x = 0.0f;
	static float xVel = 0.0f;
	static float xTarget = 0.0f;
	
	if(init){
		x = iCurrentSlide * 3;
		xTarget = x + 3;
		xVel = 0.0f;
	}
	
	gluLookAt(	x, 0, fZoom, 
				x, 0, 0, 
				0, 1, 0	);
				
	fZoom += fZoomVel;
	x += xVel;
	
	if(fZoom >= 10.0f){
		fZoomVel = 0.0f;
		fZoom = 10.0f;
		xVel = 0.1f;
	}
	
	
	if(x >= xTarget){
		fZoomVel = -0.1f;
		x = xTarget;
	}
	
	if(fZoom < 1.0f){
		fZoom = 1.0f;
		fZoomVel = 0.1f;
		isTransition = false;
	}
	
	
	glPushMatrix();
	
	for(int i=0;i<(int)mSlides.size();i++){	
	
		mSlides[i]->bind();
		
		float h = 1.0f; //0.67f;
		float w = 1.0f; //0.93f;
		
		glTranslatef(3, 0, 0);
	
		glBegin(GL_QUADS);
			// Front Face
			glNormal3f( 0.0f, 0.0f, 1.0f);		// Normal Pointing Towards Viewer
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, -h,  0);	// Point 1 (Front)
			glTexCoord2f(1.0f, 0.0f); glVertex3f(w, -h,  0);	// Point 2 (Front)
			glTexCoord2f(1.0f, 1.0f); glVertex3f(w,  h,  0);	// Point 3 (Front)
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-w,  h,  0);	// Point 4 (Front)
		glEnd();
	}
	
	glPopMatrix();

}

#include "main.h"

Slide::Slide(){
    mFull = NULL;
    mThumb = NULL;
}

bool Slide::loadFull(){
        
	Texture *full = new Texture(mFullFilename);

	if(!full || !full->isLoaded()){
        return false;
	}
	mFull = full;
	return true;
}







bool HitInFace::init(){
	x = iCurrentSlide * 5;
	xTarget = x + 5;
	xVel = 0.0f;
	fZoomVel = 0.1f;
	fZoom = 1.1f;
	
	LOG("Started transition\n");
	
	return true;
}


bool HitInFace::render(){
	
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
		fZoomVel = 0.0f;
		LOG("Ended transition\n");
		return false; //finished!
	}	
	
	glPushMatrix();
	
	for(int i=0;i<(int)mSlides.size();i++){	
	
		mSlides[i]->mThumb->bind();
		
		float h = 0.74f * 1.14f;
		float w = 1.52f * 1.08f;
		
		glTranslatef(5, 0, 0);
	
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
	
	return true; //keep going

}

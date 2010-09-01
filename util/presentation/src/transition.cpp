#include "main.h"



/*******************************************************************************
						slide
*******************************************************************************/
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







/*******************************************************************************
						hit in face
*******************************************************************************/
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



/*******************************************************************************
						collapse
*******************************************************************************/

bool Collapse::init(){

	stage = 0;
	size = 1.0f;
	
	LOG("Started transition\n");
	
	return true;
}


bool Collapse::render(){
	
	gluLookAt(	0, 0, 1, 
				0, 0, 0, 
				0, 1, 0	);
				
	
	
	float h = 0.74f / size;
	float w = 1.52f * size;

	if(stage == 0){
        size *= 0.99f;

        if(size < 0.01f){
            stage++;
            size = 0.01f;
        }
        
    	mSlides[iCurrentSlide-1]->mThumb->bind();
	
	}else if(stage == 1){
        size *= 1.01f;

        if(size > 1.0f){
            size = 1.0f;
            return false;
        }
        
	    mSlides[iCurrentSlide]->mThumb->bind();
	}

	
	
	glBegin(GL_QUADS);
		// Front Face
		glNormal3f( 0.0f, 0.0f, 1.0f);		// Normal Pointing Towards Viewer
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, -h,  0);	// Point 1 (Front)
		glTexCoord2f(1.0f, 0.0f); glVertex3f(w, -h,  0);	// Point 2 (Front)
		glTexCoord2f(1.0f, 1.0f); glVertex3f(w,  h,  0);	// Point 3 (Front)
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-w,  h,  0);	// Point 4 (Front)
	glEnd();
	
	
	return true; //keep going

}




/*******************************************************************************
						rotate
*******************************************************************************/

Rotate::Rotate(int x, int y, int z){
    AX = x; AY = y; AZ = z;
}


bool Rotate::init(){

	stage = 0;
	size = 1.0f;
	
	LOG("Started transition\n");
	
	return true;
}




bool Rotate::render(){
	
	gluLookAt(	0, 0, 1, 
				0, 0, 0, 
				0, 1, 0	);
				
	
	
	float h = 0.74f;
	float w = 1.52f;

	float scaleX = 1.0f;
	float scaleY = 1.0f;

	glRotatef(size, AX, AY, AZ);

	size ++;

	float end = (AX * 180) + (AY * 180) + (AZ * 180);
	float half = end/2;

	if(size >= end){
        return false;
	}	

	if(size < half)
        mSlides[iCurrentSlide-1]->mThumb->bind();
    else{
        mSlides[iCurrentSlide]->mThumb->bind();

        if(AY == 1)
            scaleX = -1;

        if(AX == 1)
            scaleY = -1;

        if(AX == 1 && AZ == 1){
            scaleX = 1.0; scaleY = 1.0;
        }
        
    }
	
	glBegin(GL_QUADS);
		// Front Face
		glNormal3f( 0.0f, 0.0f, 1.0f);		// Normal Pointing Towards Viewer
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, -h,  0);	// Point 1 (Front)
		glTexCoord2f(1.0f * scaleX, 0.0f); glVertex3f(w, -h,  0);	// Point 2 (Front)
		glTexCoord2f(1.0f * scaleX, 1.0f * scaleY); glVertex3f(w,  h,  0);	// Point 3 (Front)
		glTexCoord2f(0.0f, 1.0f * scaleY); glVertex3f(-w,  h,  0);	// Point 4 (Front)
	glEnd();
	
	
	return true; //keep going

}


/*******************************************************************************
						collapse
*******************************************************************************/

bool Fade::init(){

	stage = 0;
	size = 1.0f;
	
	LOG("Started transition\n");
	
	return true;
}


bool Fade::render(){
	
	gluLookAt(	0, 0, 1.0f, 
				0, 0, 0, 
				0, 1, 0	);
				
	
	
	float h = 0.7f;
	float w = 1.365f;
	
	mSlides[iCurrentSlide]->mThumb->bind();

	size -= 0.01f;

	if(size < 0.0f){
	    glDisable(GL_BLEND);
	    glColor4f(1,1,1,1);
        return false;
	}
	
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glColor4f(1,1,1,size);


    mSlides[iCurrentSlide]->mThumb->bind();
    
    glColor4f(1 - size,1 - size,1 - size,1 - size);

    glBegin(GL_QUADS);
	    // Front Face
	    glNormal3f( 0.0f, 0.0f, 1.0f);		// Normal Pointing Towards Viewer
	    glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, -h,  0);	// Point 1 (Front)
	    glTexCoord2f(1.0f, 0.0f); glVertex3f(w, -h,  0);	// Point 2 (Front)
	    glTexCoord2f(1.0f, 1.0f); glVertex3f(w,  h,  0);	// Point 3 (Front)
	    glTexCoord2f(0.0f, 1.0f); glVertex3f(-w,  h,  0);	// Point 4 (Front)
    glEnd();

    mSlides[iCurrentSlide-1]->mThumb->bind();

    glColor4f(size,size,size,size);

    glBegin(GL_QUADS);
	    // Front Face
	    glNormal3f( 0.0f, 0.0f, 1.0f);		// Normal Pointing Towards Viewer
	    glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, -h,  0);	// Point 1 (Front)
	    glTexCoord2f(1.0f, 0.0f); glVertex3f(w, -h,  0);	// Point 2 (Front)
	    glTexCoord2f(1.0f, 1.0f); glVertex3f(w,  h,  0);	// Point 3 (Front)
	    glTexCoord2f(0.0f, 1.0f); glVertex3f(-w,  h,  0);	// Point 4 (Front)
    glEnd();

	
	return true; //keep going

}


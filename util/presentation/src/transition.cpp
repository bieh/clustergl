#include "main.h"


float h = 0.7f;
float w = 0.93f;


float randFloat(float min, float max){
    return min + (((float)rand()/(float)RAND_MAX) * (max - min));
}

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
		
		float h2 = h * 1.14f;
		float w2 = w * 1.08f;
		
		glTranslatef(5, 0, 0);
	
		glBegin(GL_QUADS);
			// Front Face
			glNormal3f( 0.0f, 0.0f, 1.0f);		// Normal Pointing Towards Viewer
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-w2, -h2,  0);	// Point 1 (Front)
			glTexCoord2f(1.0f, 0.0f); glVertex3f(w2, -h2,  0);	// Point 2 (Front)
			glTexCoord2f(1.0f, 1.0f); glVertex3f(w2,  h2,  0);	// Point 3 (Front)
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-w2,  h2,  0);	// Point 4 (Front)
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
				
	
	
	float h2 = h / size;
	float w2 = w * size;

	if(stage == 0){
        size *= 0.97f;

        if(size < 0.01f){
            stage++;
            size = 0.01f;
        }
        
    	mSlides[iCurrentSlide-1]->mThumb->bind();
	
	}else if(stage == 1){
        size *= 1.03f;

        if(size > 1.0f){
            size = 1.0f;
            return false;
        }
        
	    mSlides[iCurrentSlide]->mThumb->bind();
	}

	
	
	glBegin(GL_QUADS);
		// Front Face
		glNormal3f( 0.0f, 0.0f, 1.0f);		// Normal Pointing Towards Viewer
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-w2, -h2,  0);	// Point 1 (Front)
		glTexCoord2f(1.0f, 0.0f); glVertex3f(w2, -h2,  0);	// Point 2 (Front)
		glTexCoord2f(1.0f, 1.0f); glVertex3f(w2,  h2,  0);	// Point 3 (Front)
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-w2,  h2,  0);	// Point 4 (Front)
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
				
	
	
	mSlides[iCurrentSlide]->mFull->bind();

	size -= 0.01f;

	if(size < 0.0f){
	    glDisable(GL_BLEND);
	    glColor4f(1,1,1,1);
        return false;
	}
	
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_ONE, GL_ONE);
	glColor4f(1,1,1,size);


    mSlides[iCurrentSlide]->mFull->bind();
    
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




/*******************************************************************************
						starwars
*******************************************************************************/

bool StarWars::init(){

	stage = 1;
	size = 0.0f;
	
	LOG("Started transition\n");
	
	return true;
}


bool StarWars::render(){
	
	gluLookAt(	0, 0, 1, 
				0, 0, 0, 
				0, 1, 0	);
				
	
    if(stage == 0){     
       /*
        glRotatef(size, 1, 0, 0);	
        size -=5;

        if(size <= -45){
            size = 0;
            stage++;
        }
        */
        stage++;
    }else if(stage == 1){
        glTranslatef(0, 0, -size);

        int angle = -(size * 30);
        
        glRotatef(angle, 1, 0, 0);	

        if(angle == -90){
            stage++;
        }

        size += 0.01f;

         mSlides[iCurrentSlide-1]->mThumb->bind();

    
    }
    else if(stage == 2){
        glTranslatef(0, 0, -size);

        int angle = -(size * 30);
        
        glRotatef(angle, 1, 0, 0);	


        size -= 0.01f;

        if(size <= 0){
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
						tumble
*******************************************************************************/

bool Tumble::init(){

	stage = 0;
	size = 0.0f;
	
	LOG("Started transition\n");
	
	return true;
}


bool Tumble::render(){
	
	gluLookAt(	0, 0, 1, 
				0, 0, 0, 
				0, 1, 0	);
				
	

    mSlides[iCurrentSlide-1+stage]->mThumb->bind();


    if(stage == 0){
        size += 0.005f;
        
        glRotatef(size * 180, 0, 0, 1);
    }	
    else if(stage == 1){
        size -= 0.005f;
        
        glRotatef((size * -1) * 180, 0, 0, 1);
    }	

    glTranslatef(0, 0, -size * 10);
    
    if(size >= 1.0f){
        stage = 1;
    }else if(size <= 0.0f){
        return false;
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
						shatter
*******************************************************************************/

class Fragment{
public:
    float x, y;
    float x2, y2;

    float px, py;
    float vx, vy;
};




bool Shatter::init(){

	stage = 0;
	size = 0.0f;
	
	LOG("Started transition\n");

	for(int i=0;i<10;i++){
        Fragment *f = new Fragment();
        f->x = randFloat(0, 1);
        f->y = randFloat(0, 1);

        f->x2 = f->x + 0.1f;
        f->y2 = f->y + 0.1f;
    }
	
	return true;
}


bool Shatter::render(){
	
	gluLookAt(	0, 0, 1, 
				0, 0, 0, 
				0, 1, 0	);
				
	
	
    mSlides[iCurrentSlide-1+stage]->mThumb->bind();

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
						bounce
*******************************************************************************/



bool Bounce::init(){

	stage = 0;
	size = 0.0f;

	top = 1.5f;
	bottom = 0.0f;
	topvel = -0.001f;
	bottomvel = 0.0f;
	
	LOG("Started transition\n");

	return true;
}


bool Bounce::render(){
	
	gluLookAt(	0, 0, 1, 
				0, 0, 0, 
				0, 1, 0	);
				
	

    mSlides[iCurrentSlide]->mThumb->bind();

	glBegin(GL_QUADS);
		// Front Face
		glNormal3f( 0.0f, 0.0f, 1.0f);		// Normal Pointing Towards Viewer
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, -h + top,  0);	// Point 1 (Front)
		glTexCoord2f(1.0f, 0.0f); glVertex3f(w, -h + top,  0);	// Point 2 (Front)
		glTexCoord2f(1.0f, 1.0f); glVertex3f(w,  h + top,  0);	// Point 3 (Front)
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-w,  h + top,  0);	// Point 4 (Front)
	glEnd();



	mSlides[iCurrentSlide-1]->mThumb->bind();

	float o = bottomvel * -10;

	glBegin(GL_QUADS);
		// Front Face
		glNormal3f( 0.0f, 0.0f, 1.0f);		// Normal Pointing Towards Viewer
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-w - o, -h + bottom,  0);	// Point 1 (Front)
		glTexCoord2f(1.0f, 0.0f); glVertex3f(w + o, -h + bottom,  0);	// Point 2 (Front)
		glTexCoord2f(1.0f, 1.0f); glVertex3f(w + o,  h + bottom,  0);	// Point 3 (Front)
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-w - o,  h + bottom,  0);	// Point 4 (Front)
	glEnd();

	top += topvel;
	bottom += bottomvel;
    topvel -= 0.0001f;

	if((top - (h * 2)) < bottom){
        bottomvel = topvel * 0.75f;
        topvel *= -0.75f;
	}

	bottomvel *= 0.99f;

    if(top <= 0.0f){
        return false;
    }
	
	
	return true; //keep going

}




/*******************************************************************************
						spin
*******************************************************************************/

Spin::Spin(int x, int y, int z){
      AX = x; AY = y; AZ = z;
}

bool Spin::init(){

	stage = 0;
	size = 0.001f;
	vel = 0.1f;
	
	LOG("Started transition\n");
	
	return true;
}




bool Spin::render(){
	
	gluLookAt(	0, 0, 1, 
				0, 0, 0, 
				0, 1, 0	);
				
	
    glRotatef(size, AX, AY, AZ);

    if(stage == 0){
        size += vel;
        
	    mSlides[iCurrentSlide-1]->mThumb->bind();
	}
    else if(stage == 1){
        size -= vel;
        
	    mSlides[iCurrentSlide]->mThumb->bind();
	}

    if(vel > 5){
        stage++;
    }

    if(vel <= 0){
        return false;
    }

    if(stage == 0)
        vel += 0.05f;
    else
        vel -= 0.05f;
		
	glBegin(GL_QUADS);
		// Front Face
		glNormal3f( 0.0f, 0.0f, 1.0f);		// Normal Pointing Towards Viewer
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, -h,  0);	// Point 1 (Front)
		glTexCoord2f(1.0f, 0.0f); glVertex3f(w, -h,  0);	// Point 2 (Front)
		glTexCoord2f(1.0f , 1.0f); glVertex3f(w,  h,  0);	// Point 3 (Front)
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-w,  h,  0);	// Point 4 (Front)
	glEnd();
	
	
	return true; //keep going

}



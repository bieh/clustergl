#include "main.h"

bool Presentation::init(vector<string> files){

	LOG("Starting presentation init\n");
	
	mSlides.clear();
		
	for(int i=0;i<(int)files.size();i++){
			
		//LOG("loading '%s'\n", files[i].c_str());
	
		Texture *t = new Texture(files[i]);
		
		if(t->isLoaded()){
		
			mSlides.push_back(t);	
			LOG("Loaded slide '%s'\n", files[i].c_str());
		}else{
			ERR("Failed to load slide '%s'\n", files[i].c_str());
			delete t;
			return false;
		}
	
	}
	
	LOG("Done presentation init (%d slides)\n", mSlides.size());
	
	iCurrentSlide = 0;
	isTransition = false;
		
	return true;	
}

void Presentation::render(){

	if(isTransition){
		doSimpleTransition(transitionInit);
		transitionInit = false;
	}else{
		
		gluLookAt(	0, 0, 1, 
				0, 0, 0, 
				0, 1, 0	);

		mSlides[iCurrentSlide]->bind();
		
		float h = 0.7f; //0.67f;
 		float w = 1.4f; //0.93f;
	
		glBegin(GL_QUADS);
			// Front Face
			glNormal3f( 0.0f, 0.0f, 1.0f);		// Normal Pointing Towards Viewer
			glTexCoord2f(0.0f, 0.0f); glVertex3f(-w, -h,  0);	// Point 1 (Front)
			glTexCoord2f(1.0f, 0.0f); glVertex3f(w, -h,  0);	// Point 2 (Front)
			glTexCoord2f(1.0f, 1.0f); glVertex3f(w,  h,  0);	// Point 3 (Front)
			glTexCoord2f(0.0f, 1.0f); glVertex3f(-w,  h,  0);	// Point 4 (Front)
		glEnd();
	}
}

void Presentation::update(){
	
	
}

void Presentation::next(){
	iCurrentSlide++;
	
	if(iCurrentSlide >= mSlides.size()){
		iCurrentSlide = 0;
	}
	
	LOG("Next! (%d)\n", iCurrentSlide);
	
	isTransition = true;
	transitionInit = true;
}

void Presentation::prev(){

}


void Presentation::shutdown(){

	LOG("presentation shutdown (%d slides)\n", mSlides.size());

	for(int i=0;i<(int)mSlides.size();i++){
		delete mSlides[i];
	}
	mSlides.clear();
}

#include "main.h"


vector<Slide *> mSlides;
int iCurrentSlide;


bool Presentation::init(vector<string> files){

	LOG("Starting presentation init\n");
	
	mSlides.clear();
		
	for(int i=0;i<(int)files.size();i++){
			
		//LOG("loading '%s'\n", files[i].c_str());

		string thumbFile = files[i];

		//thumbnail is in a 'thumbs' directory
		//man, c++ is annoying to do this in
		int r = thumbFile.rfind("/");
		if(r == string::npos){ r = 2; thumbFile = "./" + thumbFile; }
		thumbFile.replace(r, 1, "/thumbs/");

		//LOG("thumb: %s, %s\n", files[i].c_str(), thumbFile.c_str());
	
		Texture *thumb = new Texture(thumbFile);
		
		if(thumb->isLoaded()){

		    Slide *s = new Slide();
		    s->mThumb = thumb;
		    s->mFull = NULL;
		    s->mFullFilename = files[i];
		    s->mThumbFilename = thumbFile;
		
			mSlides.push_back(s);	
			LOG("Loaded thumb '%s'\n", thumbFile.c_str());
		}else{
			ERR("Failed to load slide '%s'\n", thumbFile.c_str());
			delete thumb;
			return false;
		}
	
	}
	
	LOG("Done presentation init (%d slides)\n", mSlides.size());
	
	iCurrentSlide = 0;
	isTransition = false;

	mTransitions.clear();
	
	//set up transitions
	mTransitions.push_back(new HitInFace());
		
	return true;	
}

Slide *Presentation::getNextSlide(){
	if((iCurrentSlide + 1) < mSlides.size()){
		return mSlides[iCurrentSlide + 1];
	}else{
		return NULL;
	}
}

Slide *Presentation::getCurrentSlide(){
	return mSlides[iCurrentSlide];
}

Slide *Presentation::getPrevSlide(){
	if((iCurrentSlide - 1) >= 0){
		return mSlides[iCurrentSlide - 1];
	}else{
		return NULL;
	}
}

void Presentation::render2D(){
    if(isTransition){

    }else{

        Slide *s = getCurrentSlide();
        Slide *n = getNextSlide();
        Slide *p = getPrevSlide();
        
        
        bool didLoad = false;

        if(!s->mFull || !s->mFull->isLoaded()){
            LOG("need to load %d\n", iCurrentSlide);
            if(!s->loadFull()){
                LOG("Failed to load full texture!\n");
                return;
            }
            didLoad = true;
        }
        
        
        if(allowCache && !didLoad){
		    
		    if(!n->mFull || !n->mFull->isLoaded()){
		        LOG("caching %d\n", iCurrentSlide + 1);
		        if(!n->loadFull()){
		            LOG("Failed to load full texture!\n");
		            return;
		        }
		    }
		    
		    allowCache = false;
		    didLoad = true;
		    
		    if(p && p->mFull && p->mFull->isLoaded()){
		    	p->mFull->destroy();
		    }
		}
			    
    
		s->mFull->bind();	

		float w = 888;
		float h = 456;	
	
		glBegin(GL_QUADS);
			// Front Face
			glTexCoord2f(0.0f, 0.0f); glVertex2f(0, 0);	// Point 1 (Front)
			glTexCoord2f(1.0f, 0.0f); glVertex2f(w, 0);	// Point 2 (Front)
			glTexCoord2f(1.0f, -1.0f); glVertex2f(w,  h);	// Point 3 (Front)
			glTexCoord2f(0.0f, -1.0f); glVertex2f(0,  h);	// Point 4 (Front)
		glEnd();
		
		allowCache = true;
	}
}

void Presentation::render(){

	if(isTransition){
		if(transitionInit){
			mCurrentTransition->init();
		}
		transitionInit = false;
		
		isTransition = mCurrentTransition->render();
		
		if(!isTransition){
			allowCache = false;
		}
	}else{
		
	}
}

void Presentation::update(){
	
	
}

void Presentation::next(){
	iCurrentSlide++;
	
	if(iCurrentSlide >= mSlides.size()){
		iCurrentSlide = 0;
	}
	
	LOG("Next! (%d, %d)\n", iCurrentSlide, mTransitions.size());
	
	mCurrentTransition = mTransitions[0];
	
	isTransition = true;
	transitionInit = true;

	LOG("1\n");
}

void Presentation::prev(){
	iCurrentSlide--;
	LOG("Prev! (%d)\n", iCurrentSlide);
	isTransition = false;
}


void Presentation::shutdown(){

	LOG("presentation shutdown (%d slides)\n", mSlides.size());

	for(int i=0;i<(int)mSlides.size();i++){
		delete mSlides[i];
	}
	mSlides.clear();
}

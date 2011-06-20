#include "main.h"


vector<Slide *> mSlides;
int iCurrentSlide;

char *textFileRead(const char *filename) 
	{
		FILE *fp;
		char *content = NULL;

		int count=0;

		if (filename != NULL) {

			fp = fopen(filename,"rt");

			if (fp != NULL) {
										      
        	      		fseek(fp, 0, SEEK_END);
        			count = ftell(fp);
        			rewind(fp);

				if (count > 0) {
					content = (char *)malloc(sizeof(char) * (count+1));
					count = fread(content,sizeof(char),count,fp);
					content[count] = '\0';
				}
				fclose(fp);
										
			}
		}
	
		return content;
	}
	

bool presentationFinished = false;

bool Presentation::init(vector<string> files){

    srand((unsigned int)time(0));

	LOG("Starting presentation init\n");
	
	mSlides.clear();
		
	for(int i=0;i<(int)files.size();i++){
			
		//LOG("loading '%s'\n", files[i].c_str());

		string thumbFile = files[i];

		//thumbnail is in a 'thumbs' directory
		//man, c++ is annoying to do this in
		unsigned int r = thumbFile.rfind("/");
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
	
	LOG("Done presentation init (%d slides)\n", (int)mSlides.size());
	
	iCurrentSlide = 0;
	isTransition = false;
	bUseReadabilityMovement = false;

	mTransitions.clear();
	
	//set up transitions

	
    mTransitions.push_back(new HitInFace());
	mTransitions.push_back(new Collapse());
	mTransitions.push_back(new Rotate(1, 0, 0));
    mTransitions.push_back(new Rotate(0, 1, 0));
	mTransitions.push_back(new Rotate(1, 0, 1));
	mTransitions.push_back(new Fade());	
	mTransitions.push_back(new Tumble());	
	mTransitions.push_back(new Bounce());	
	mTransitions.push_back(new Bounce());	
	mTransitions.push_back(new StarWars());
    mTransitions.push_back(new Spin(1,0,0));
    mTransitions.push_back(new Spin(0,0,1));
	mTransitions.push_back(new Spin(0,1,0));
	mTransitions.push_back(new Spin(1,1,0));	
	mTransitions.push_back(new Spin(1,1,1));
    


	
	
	//mTransitions.push_back(new Shatter());

	
		
	return true;	
}

Slide *Presentation::getNextSlide(){
	if((iCurrentSlide + 1) < (int)mSlides.size()){
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

/*
		if(p && p->mFull && p->mFull->isLoaded()){
			p->mFull->destroy();
		}
*/
        
        if(allowCache && !didLoad){
		    
		    if(n && (!n->mFull || !n->mFull->isLoaded())){
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

        float x = 0;
        float y = 0;
		float w = 1024;//888
		float h = 768;//456

		if(bUseReadabilityMovement){
            static float move = 0.0f;
            move += 0.01f;

            float scale = 1.0f;

            glTranslatef(sinf(move) * scale, cosf(move) * scale, 0);

            x -= scale;
            y -= scale;
            w += scale;
            h += scale;
		}
	
		glBegin(GL_QUADS);
			// Front Face
			glTexCoord2f(0.0f, 0.0f); glVertex2f(x, y);	// Point 1 (Front)
			glTexCoord2f(1.0f, 0.0f); glVertex2f(w, y);	// Point 2 (Front)
			glTexCoord2f(1.0f, -1.0f); glVertex2f(w,  h);	// Point 3 (Front)
			glTexCoord2f(0.0f, -1.0f); glVertex2f(x,  h);	// Point 4 (Front)
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
	
	if(iCurrentSlide >= (int)mSlides.size()){
		iCurrentSlide = 0;
		presentationFinished = true;
	}
	
	LOG("Next! (%d, %d)\n", iCurrentSlide, (int)mTransitions.size());
	
	mCurrentTransition = mTransitions[rand() % mTransitions.size()];
	
	isTransition = true;
	transitionInit = true;

}

void Presentation::prev(){
	iCurrentSlide--;

	if(iCurrentSlide < 0) iCurrentSlide = mSlides.size() - 1;

	LOG("Prev! (%d)\n", iCurrentSlide);
	isTransition = false;
}


void Presentation::shutdown(){

	LOG("presentation shutdown (%d slides)\n", (int)mSlides.size());

	for(int i=0;i<(int)mSlides.size();i++){
		delete mSlides[i];
	}
	mSlides.clear();
}

bool Presentation::toggleReadabilityMovement(){
    bUseReadabilityMovement = !bUseReadabilityMovement;
    return bUseReadabilityMovement;
}

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
	
	return true;	
}

void Presentation::render(){

	mSlides[0]->bind();
	
	mGLU.cube(1,1,1);	

}

void Presentation::update(){
	
}

void Presentation::shutdown(){

	LOG("presentation shutdown (%d slides)\n", mSlides.size());

	for(int i=0;i<(int)mSlides.size();i++){
		delete mSlides[i];
	}
	mSlides.clear();
}

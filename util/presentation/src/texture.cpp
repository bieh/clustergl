#include "main.h"

Texture::Texture(string filename){
	load(filename, 0);
}
	
bool Texture::load(string filename, int flags){

	bIsLoaded = false;

	ilGenImages(1, &iDevilID);    
	ilBindImage(iDevilID);
			
	//Load the data	
	if(!ilLoadImage((char *)filename.c_str())){
		int err = ilGetError();
		ERR("error: %s, %d!\n", filename.c_str(), err);
		return false;
	}
					
	iSizeX = ilGetInteger(IL_IMAGE_WIDTH);
	iSizeY = ilGetInteger(IL_IMAGE_HEIGHT);
	
	//if(flags != TEXTURE_NO_GL){	
						
		mHandle = ilutGLBindTexImage(); //
				
		if(mHandle == 0){
			LOG("Failed to get handle!\n");
			return false;
		}
	//}	
	
	ilDeleteImages(1, &iDevilID);
	
	bIsLoaded = true;
		
	return true;
}
	
void Texture::bind(){

	if(!isLoaded()){
		LOG("Tried to bind a null texture!\n");
		return;
	}	
	
	

	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, mHandle);
	
	
}

void Texture::destroy(){
	glDeleteTextures(1, &mHandle);
	bIsLoaded = false;
}

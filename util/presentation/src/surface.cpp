#include "main.h"

int iScreenX, iScreenY;
bool bFullscreen;

void Surface::beginRender(){
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	
	glViewport(0,0, iScreenX, iScreenY);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	
	float ar = (float)iScreenX / (float)iScreenY;

	gluPerspective(70.0f,ar,0.5f,1000.0f);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    
    glEnable(GL_DEPTH_TEST);
	
   	//Now in 3D	
}

void Surface::begin2D(){

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

#ifndef ENABLE_CGL_COMPAT	
	//Make it ortho
	//(0,0) == top-left

	glOrtho(0, iScreenX, iScreenY, 0, 0, 1);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glDisable(GL_DEPTH_TEST);

	//now in 2D mode
#endif
			
}

void Surface::endRender(){

	SDL_GL_SwapBuffers( );
}


void Surface::resizeWindow( int width, int height )
{
	// Height / width ration 
	GLfloat ratio;

	// Protect against a divide by zero
	if ( height == 0 )
		height = 1;

	ratio = ( GLfloat )width / ( GLfloat )height;

	//Setup our viewport.
	glViewport( 0, 0, ( GLsizei )width, ( GLsizei )height );

	//change to the projection matrix and set our viewing volume.
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity( );
	
	//Set our perspective
//	gluPerspective( 45.0f, ratio, 0.1f, 100.0f );

//#ifndef ENABLE_CGL_COMPAT
	gluPerspective(70.0f,ratio,1.0f,10000.0f);
//#endif

	// Make sure we're chaning the model view and not the projection 
	glMatrixMode( GL_MODELVIEW );

	//Reset The View 
	glLoadIdentity( );
	
	iScreenX = width;
	iScreenY = height;	

}




/*********************************************
	Creates a window with specified attribs
**********************************************/
bool Surface::init(int sizeX, int sizeY, bool fullscreen){
 
 	int bpp = 32;
 	iScreenX = sizeX;
 	iScreenY = sizeY;
 	bFullscreen = fullscreen;

	//this holds some info about our display
	const SDL_VideoInfo *videoInfo;
		//initialize SDL 
	if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ){
		LOG( "Video initialization failed: %s\n", SDL_GetError( ) );
		return false;
	}

	SDL_EnableUNICODE(1);

	// Fetch the video info
	videoInfo = SDL_GetVideoInfo( );

	if ( !videoInfo ){
		LOG( "Video query failed: %s\n", SDL_GetError( ) );
		return false;
	}

	//If the size is 0, detect the native res
	if(sizeX <= 0.0f){
		sizeX = videoInfo->current_w;
	}

	if(sizeY <= 0.0f){
		sizeY = videoInfo->current_h;
	}

	if(bpp == 0){
		bpp = videoInfo->vfmt->BitsPerPixel;
	}

	//the flags to pass to SDL_SetVideoMode
	videoFlags  = SDL_OPENGL;         
	videoFlags |= SDL_GL_DOUBLEBUFFER;
	videoFlags |= SDL_HWPALETTE;     

	//Quick hack - our resizing doesn't work properly under windows.
	//easier just to disable it
#ifndef _WINDOWS
	videoFlags |= SDL_RESIZABLE;     
#endif

	//This checks to see if surfaces can be stored in memory 
	if ( videoInfo->hw_available )
		videoFlags |= SDL_HWSURFACE;
	else
		videoFlags |= SDL_SWSURFACE;

	// This checks if hardware blits can be done 
	if ( videoInfo->blit_hw )
		videoFlags |= SDL_HWACCEL;
	
	//Sets up OpenGL double buffering
	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	// Fullscreen?
	if(fullscreen){
		videoFlags |= SDL_FULLSCREEN;
	}

	// get a SDL surface 

	surface = SDL_SetVideoMode( iScreenX, iScreenY, bpp, videoFlags );

	// Verify there is a surface 
	if ( !surface ){
		LOG( "Video mode set failed: %s\n", SDL_GetError( ) );
		//return false;
	}

	//Set the window caption
	SDL_WM_SetCaption( "window", NULL );

	//Enable smooth shading 
    glShadeModel( GL_SMOOTH );

    //Set the background black
    glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );

    //Depth buffer setup
    glClearDepth( 1.0f );

    //Enables Depth Testing 
    glEnable( GL_DEPTH_TEST );

    //The Type Of Depth Test To Do 
    glDepthFunc( GL_LEQUAL );

    // Really Nice Perspective Calculations 
    glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
	
	//Set up GLEW
#ifndef ENABLE_CGL_COMPAT
	GLenum err = glewInit();
	if (GLEW_OK != err){
		ERR("Couldn't start GLEW: %s!\n", glewGetErrorString(err));
		return false;
	}
#endif

	//resize the initial window   
	resizeWindow( sizeX, sizeY );

	LOG("Made a %d/%d window @ %d bpp\n", sizeX, sizeY, bpp );
	
	
	
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION || 
		iluGetInteger(ILU_VERSION_NUM) < ILU_VERSION || 
		ilutGetInteger(ILUT_VERSION_NUM) < ILUT_VERSION) 
	{
		LOG("DevIL version is different...exiting!\n");
		return false;
	}
	
	ilInit();
	ilutRenderer(ILUT_OPENGL);
	
	
	return true;
}


bool Surface::shutdown(){
	return true;
}


void Surface::onResize(int x, int y){

    //handle resize event
    surface = SDL_SetVideoMode( x, y, 32, videoFlags );
    if ( !surface ){
	    ERR( "Could not get a surface after resize: %s\n", 
	    	SDL_GetError( ) );
	    requestShutdown();
	}
    resizeWindow( x, y );
}

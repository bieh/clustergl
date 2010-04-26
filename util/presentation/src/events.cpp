#include "main.h"


/*********************************************
		SDL event loop
**********************************************/
void Application::processEvents(){

	SDL_Event event;
   
    while ( SDL_PollEvent( &event ) ){
    
	    switch( event.type ){
					      
		case SDL_VIDEORESIZE:
			mSurface.onResize(event.resize.w, event.resize.h);
		    break;
		
		case SDL_QUIT:
		    //handle quit requests
		    requestShutdown();
		    break;
		
		case SDL_MOUSEBUTTONDOWN:			
			onMouseEvent(event.button.button, event.type); 			
			break;	
			
		case SDL_MOUSEBUTTONUP:
			onMouseEvent(event.button.button, event.type); 
			break;	
			
		case SDL_KEYDOWN:
			onKeyEvent(event.key.keysym.sym, event.type);
			break;
		
		case SDL_KEYUP:
			onKeyEvent(event.key.keysym.sym, event.type);
			break;
		
		default:
		    break;
		}
	}
	
	/*
    if (!done){
    
    	//Do one frames worth of work and figure out the length of time
    	uint32_t startTime = SDL_GetTicks();
		renderMain();
		updateMain();
		uint32_t endTime = SDL_GetTicks();
		
		//Figure out the scaling factor for FPS-independent movement
		uint32_t diff = endTime - startTime;
		fTimeScale = (float)diff * fTimeScaleScale;
		
		//Every hour, do a cleanup
		if(fCleanupTimer < 0.0f){
			ps()->doPeriodicCleanup();				
			fCleanupTimer = CLEANUP_TIMER;
		}
		
		//Update our various timers
		fCleanupTimer -= fTimeScale;
		fUptime += fTimeScale;
		fParticleFPS = fTimeScale;
	}
	*/
	
}



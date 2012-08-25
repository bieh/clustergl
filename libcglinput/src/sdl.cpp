#include "main.h"
#include <SDL/SDL.h>

/*******************************************************************************
 SDL intercept
*******************************************************************************/

//SDL functors
static int (*_SDL_Init)(unsigned int flags) = NULL;

static bool bHasInit = false;

extern "C" int SDL_Init(unsigned int flags) {
	
	LOG("SDL_Init (%d)\n", bHasInit);
	if (_SDL_Init == NULL) {
		_SDL_Init = (int (*)(unsigned int)) dlsym(RTLD_NEXT, "SDL_Init");
	}
		
	if(!_SDL_Init){
		printf("Couldn't find SDL_Init: %s\n", dlerror());
		exit(0);
	}
	
	int r = (*_SDL_Init)(flags);
				
	//Set up our internals
	if(!bHasInit && !mInject){
		bHasInit = true;

		mInject = new SDLInject();

		if(!run("sdl")){
			exit(1);
		}
	}else{
		LOG("Ignored a second/non-video SDL_Init()\n");
	}
	
	//LOG("SDL_Init finished\n");
	return r;
}


/*******************************************************************************
 SDL injection
*******************************************************************************/
void SDLInject::keydown(int keycode){
	// Create a user event to call the game loop.
    SDL_Event event;
 
    
    event.type = SDL_KEYDOWN;
    event.user.code = 0;
    event.key.keysym.sym = (SDLKey)keycode;
    event.user.data2 = 0;
    
    SDL_PushEvent(&event);
}

void SDLInject::keyup(int keycode){
	// Create a user event to call the game loop.
    SDL_Event event;
    
    event.type = SDL_KEYUP;
    event.user.code = 0;
    event.key.keysym.sym = (SDLKey)keycode;
    event.user.data2 = 0;
    
    SDL_PushEvent(&event);
}

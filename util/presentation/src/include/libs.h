/*******************************************************************************
							BSOD2 Client - libs.h
							
 This file includes all the common standard libraries that the client uses. It's
 nice to have them all in one place
 
 You'll need: SDL, DevIL, SDL_ttf, libconfuse, SDL_net, CEGUI, libGLEW
*******************************************************************************/


/*********************************************
					CEGUI
**********************************************/
#ifdef ENABLE_GUI
#include <CEGUI.h>
#endif

/*********************************************
					STL
**********************************************/
#include <iostream>
#include <string>
#include <vector>
#include <list>
#include <sstream>
#include <map>
#include <iterator>
#include <stack>
#include <cassert>
#include <limits>
#include <algorithm>
#include <fstream>
#include <cctype>


using std::string;
using std::vector;
using std::list;
using std::stack;
using std::map;

/*********************************************
					C stdlib
**********************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef _WINDOWS
#include <Winsock2.h>
#include <time.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#endif

//libconfuse
#include <confuse.h>


/*********************************************
					OpenGL
**********************************************/
#ifdef ENABLE_CGL_COMPAT
	#define GL_GLEXT_PROTOTYPES
#else
	#include <GL/glew.h>
#endif

#include <GL/gl.h>
#include <GL/glu.h>

/*********************************************
				SDL*
**********************************************/
#ifdef _WINDOWS
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_net.h>
#else
#include <SDL/SDL.h>
#include <SDL/SDL_ttf.h>
#include <SDL/SDL_net.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mutex.h>
#endif


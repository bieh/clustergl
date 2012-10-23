/*******************************************************************************
	libs.h
*******************************************************************************/

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
#include <sstream>

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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dlfcn.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/tcp.h>
#include <sys/types.h>
#include <sys/time.h>

#include <confuse.h>

#include <errno.h>

#include <zconf.h>
#include <zlib.h>


/*********************************************
	OpenGL
**********************************************/

//Sometimes we don't want the GL headers interfering
#ifndef NO_OPENGL_HEADERS
	#ifdef __APPLE__
		#include <SDL/SDL_opengl.h>
		#include <OpenGL/gltypes.h> // needed for GLuint64EXT and stuff
		// a little ugly but won't compile on Mac without this
		#define GL_FRAMEBUFFER_COMPLETE			0x8CD5
		#define GL_NUM_EXTENSIONS				0x821D
	#else
		#include <GL/gl.h>
		#include <GL/glx.h>
		#include <GL/glu.h>
	#endif
#endif


/*********************************************
	SDL*
**********************************************/
#include <SDL/SDL.h>
#include <SDL/SDL_net.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mutex.h>
#include <SDL/SDL_syswm.h>

typedef unsigned char byte;

#define MAX(a,b)         ((a < b) ?  (b) : (a))
#define MIN(a,b)  ((((a)-(b))&0x80000000) >> 31)? (a) : (b)

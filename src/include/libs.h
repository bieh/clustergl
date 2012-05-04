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

#ifdef _WINDOWS
#include <Winsock2.h>
#include <time.h>
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <dlfcn.h>
#include <netdb.h>
#include <unistd.h>
#include <netinet/tcp.h>
/* select Libs */
#include <sys/types.h>
#include <sys/time.h>
#endif

//libconfuse
#include <confuse.h>

#include <errno.h>

#include <zconf.h>
#include <zlib.h>

#include <netinet/tcp.h>
#include <stdlib.h>

/*********************************************
					OpenGL
**********************************************/

#ifndef NO_OPENGL_HEADERS

#ifdef __APPLE__
#include <SDL/SDL_opengl.h>
#else
#include <GL/glew.h>
#include <GL/gl.h>
#include <GL/glu.h>
#endif

#endif

/*********************************************
				SDL*
**********************************************/
#include <SDL/SDL.h>

#ifdef _WINDOWS
//#include <SDL_ttf.h>
#include <SDL_net.h>
#else
//#include <SDL/SDL_ttf.h>
#include <SDL/SDL_net.h>
#include <SDL/SDL_thread.h>
#include <SDL/SDL_mutex.h>
#include <SDL/SDL_syswm.h>
#endif

typedef unsigned char byte;

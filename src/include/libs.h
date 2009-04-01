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
#endif

//libconfuse
#include <confuse.h>


/*********************************************
					OpenGL
**********************************************/
//#include <GL/glew.h>
//#include <GL/gl.h>
//#include <GL/glu.h>

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
#endif

typedef unsigned char byte;


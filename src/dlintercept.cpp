/*******************************************************************************
	ClusterGL - dynamic library interception
*******************************************************************************/

#include "main.h"

#ifdef ENABLE_DL_INTERCEPT

#include <dlfcn.h>
#include <GL/glu.h>

/*
(notes on what this is and why, as it feels like something I'll forget)

Many apps use glxGetProcAddress() to load function pointers manually
This means they don't call our functions and so they don't get intercepted. 

Apps obtain the address of glXGetProcAddress() through calling 
dlopen("/usr/lib/libGL.so"), then dlsym("glxGetProcAddress") with that handle

We intercept dlsym(), and return our own copy of glXGetProcAddress() instead.
Our function uses RTLD_DEFAULT to return pointers to functions inside CGL. This
specifies local scope, instead of "go look in another library" scope, so our
functions are found instead of those in libGL

Note we have to use dlvsym() to look up the address to dlsym() in order to
handle the normal cases. TODO: is using "GLIBC_2.0" safe? Probably not.

*/


static void* (*o_dlsym) ( void *handle, const char *name )=0;
static void* (*o_dlopen) ( const char *file, int mode )=0;

extern "C" void *glXGetProcAddress(const GLubyte * str) { 
	return dlsym(RTLD_DEFAULT, (char *) str);
}

extern "C" void *glXGetProcAddressARB (const GLubyte * str) {
  
	return dlsym(RTLD_DEFAULT, (char *) str);
}


/*
extern "C" void *dlopen(const char *file, int mode){    
    if(!o_dlopen)
	    o_dlopen = (void*(*)(const char *file, int mode)) dlsym(RTLD_NEXT,"dlopen");
	    
    return (*o_dlopen)( file, mode );
}
*/

extern "C" void *dlsym(void *handle, const char *name){
	/*
	if(strcmp(name, "dlopen") != 0){
	    LOG( "dlsym was called (%s)\n", name );
	}
	*/
	
	if(strcmp(name, "glXGetProcAddressARB") == 0){
		return (void *)glXGetProcAddressARB;
	}
    
    if(strcmp(name, "glXGetProcAddress") == 0){
		return (void *)glXGetProcAddress;
	}
    
    if(!o_dlsym){
	    o_dlsym = (void*(*)(void *handle, const char *name)) dlvsym(RTLD_NEXT,"dlsym", "GLIBC_2.0");

		if(!o_dlsym){
			o_dlsym = (void*(*)(void *handle, const char *name)) dlvsym(RTLD_NEXT,"dlsym", "GLIBC_2.10");
		}

		if(!o_dlsym){
			printf("FAILED TO FIND DLSYM()\n");
		}else{
			printf("found dlsym\n");
		}
	}
	    
    return (*o_dlsym)( handle,name );
}

#endif

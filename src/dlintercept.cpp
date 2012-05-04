/*******************************************************************************
	ClusterGL - dynamic library interception
*******************************************************************************/

#include "main.h"

#ifdef ENABLE_DL_INTERCEPT

#include <dlfcn.h>
#include <GL/glu.h>

typedef unsigned char GLubyte;

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

//Try lots of different glibc versions. In my testing, only one is ever valid,
//so it's okay to return the first one we find.
void find_dlsym(){

	char buf[32];
	
	int maxver = 40;
	
	//Works on Ubuntu
	for(int a=0;a<maxver;a++){
		sprintf(buf, "GLIBC_2.%d", a);

		o_dlsym = (void*(*)(void *handle, const char *name)) dlvsym(RTLD_NEXT,"dlsym", buf);

		if(o_dlsym){
			LOG("Using %s\n", buf);
			return;
		}
	}
	
	//Works on Debian
	for(int a=0;a<maxver;a++){
		for(int b=0;b<maxver;b++){
			sprintf(buf, "GLIBC_2.%d.%d", a, b);
	
			o_dlsym = (void*(*)(void *handle, const char *name)) dlvsym(RTLD_NEXT,"dlsym", buf);

			if(o_dlsym){
				LOG("Using %s\n", buf);
				return;
			}
		}
	}

}

extern "C" void *glXGetProcAddress(const GLubyte * str) { 
	return dlsym(RTLD_DEFAULT, (char *) str);
}

extern "C" void *glXGetProcAddressARB (const GLubyte * str) {
	return dlsym(RTLD_DEFAULT, (char *) str);
}

extern "C" void *dlsym(void *handle, const char *name){

	if(strcmp(name, "glXGetProcAddressARB") == 0){
		return (void *)glXGetProcAddressARB;
	}
	
	if(strcmp(name, "glXGetProcAddress") == 0){
		return (void *)glXGetProcAddress;
	}
	
	if(!o_dlsym){
		find_dlsym();
	}
		
	return (*o_dlsym)( handle,name );
}

#endif

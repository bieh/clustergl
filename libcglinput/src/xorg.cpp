#include "main.h"


//http://www.doctort.org/adam/nerd-notes/x11-fake-keypress-event.html
//http://technofetish.net/repos/buffaloplay/xtest/keylockx.c

//Requires libxcb-xtest0-dev libxtst-dev

#include <X11/Xlib.h>
#include <X11/keysym.h>

#include <X11/extensions/XTest.h>

// The key code to be sent.
// A full list of available codes can be found in /usr/include/X11/keysymdef.h
#define KEYCODE XK_Down

/*******************************************************************************
 Xorg intercept
*******************************************************************************/
static Display *(*_XOpenDisplay)(const char *) = NULL;


static bool bHasInit = false;

extern "C" Display *XOpenDisplay(const char *display_name){

	LOG("XOpenDisplay (%d)\n", bHasInit);
	if (_XOpenDisplay == NULL) {
		_XOpenDisplay = (Display *(*)(const char *)) dlsym(RTLD_NEXT, "XOpenDisplay");
	}
		
	if(!_XOpenDisplay){
		LOG("Couldn't find XOpenDisplay(): %s\n", dlerror());
		exit(0);
	}
	
	Display *r = (*_XOpenDisplay)(display_name);
		
	//Set up our internals
	if(!bHasInit && !mInject){
		bHasInit = true;

		mInject = new XInject();

		if(!run("xorg")){
			exit(1);
		}
	}else{
		LOG("Ignored a second/non-video XOpenDisplay()\n");
	}
	
	return r;
}




// Function to create a keyboard event
XKeyEvent createKeyEvent(Display *display, Window &win,
                           Window &winRoot, bool press,
                           int keycode, int modifiers)
{
   XKeyEvent event;

   event.display     = display;
   event.window      = win;
   event.root        = winRoot;
   event.subwindow   = None;
   event.time        = CurrentTime;
   event.x           = 1;
   event.y           = 1;
   event.x_root      = 1;
   event.y_root      = 1;
   event.same_screen = True;
   event.keycode     = XKeysymToKeycode(display, keycode);
   event.state       = modifiers;

   if(press)
      event.type = KeyPress;
   else
      event.type = KeyRelease;

   return event;
}

void event(int keycode, bool press){

	// Obtain the X11 display.
	Display *display = _XOpenDisplay(0);
	if(display == NULL){
		LOG("Couldn't XOpenDisplay(0)\n");
		return;
	}

	// Get the root window for the current display.
	Window winRoot = XDefaultRootWindow(display);

	// Find the window which has the current keyboard focus.
	Window winFocus;
	int    revert;
	XGetInputFocus(display, &winFocus, &revert);

	// Send a fake key press event to the window.
	XKeyEvent event = createKeyEvent(display, winFocus, winRoot, press, keycode, 0);
	//XSendEvent(event.display, event.window, True, KeyPressMask, (XEvent *)&event);
	XTestFakeKeyEvent(event.display, event.keycode, press, CurrentTime);

	// Done.
	XCloseDisplay(display);
}

void XInject::keydown(int keycode){
	event(keycode, true);
}

void XInject::keyup(int keycode){
	event(keycode, false);
}



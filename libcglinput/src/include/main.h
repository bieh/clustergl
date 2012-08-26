
/*******************************************************************************
 libcglinput 
*******************************************************************************/

#include <string>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sstream>
#include <string.h>

using std::vector;
using std::string;

#define LOG printf


/*******************************************************************************
 Main functions
*******************************************************************************/
extern bool run(string methods);
extern bool begin_web();
extern bool begin_spacenav();


/*******************************************************************************
 Configuration
*******************************************************************************/
static const int MAX_AXIS = 512;

class Config{
public:
	Config(string filename);
	int axis_actions[MAX_AXIS][2];
	string device;
	int num_axis;
	int thresh[MAX_AXIS];
};

extern Config *mConfig;

/*******************************************************************************
 Injection interface
*******************************************************************************/

class IInject{
public:
	virtual void keydown(int keycode)=0;
	virtual void keyup(int keycode)=0;
};

class SDLInject : public IInject{
public:
	void keydown(int keycode);
	void keyup(int keycode);
};

class XInject : public IInject{
public:
	void keydown(int keycode);
	void keyup(int keycode);
};

extern IInject *mInject;

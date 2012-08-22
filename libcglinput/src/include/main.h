
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
class Config{
public:
	Config(string filename);
	int spacenav_codes[6];
	char *spacenav_device;
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

extern IInject *mInject;
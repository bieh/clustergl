#include "libs.h"
#include "vector.h"
#include "misc.h"
#include "glutil.h"
#include "surface.h"


//global configuration
extern int iScreenX;
extern int iScreenY;
extern bool bFullscreen;
extern void requestShutdown();

#define LOG printf
#define ERR printf


/*******************************************************************************
						Main app class
*******************************************************************************/
class Application{
public:
	Application();	
	int run(vector<string> args);	
	
	//event handling
	void processEvents();
	void onMouseEvent(int button, int event);
	void onKeyEvent(int keycode, int event);
	
private:

	Surface mSurface;	
	GLUtil mGLU;
	
	//mainloop functions
	bool init();
	bool update();
	void render();
	bool shutdown();
	
};

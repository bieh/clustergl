#include "libs.h"
#include "vector.h"
#include "misc.h"
#include "glutil.h"
#include "surface.h"
#include "texture.h"
#include "presentation.h"

//global configuration
extern int iScreenX;
extern int iScreenY;
extern bool bFullscreen;

//global functions
extern void requestShutdown();

//other global objects
extern GLUtil mGLU;

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
	Presentation mPresentation;
	
	//mainloop functions
	bool init(vector<string> args);
	bool update();
	void render();
	bool shutdown();
	
};

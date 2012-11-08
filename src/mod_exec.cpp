/*******************************************************************************
	mod_exec - this unpacks and executes the GL instructions. It also creates
	a SDL window to render to
*******************************************************************************/

#define NO_OPENGL_HEADERS

#include "main.h"

#undef NO_OPENGL_HEADERS

#include <GL/glew.h>

typedef void (*ExecFunc)(byte *buf);

/*******************************************************************************
	Globals
*******************************************************************************/

static ExecFunc mFunctions[1700];
static Instruction *mCurrentInstruction = NULL;
GLenum currentMode = GL_MODELVIEW;

int displayNumber = 0;
int currentBuffer = 0;
bool glFrustumUsage = true;
bool bezelCompensation = true;

/*******************************************************************************
	Module
*******************************************************************************/
ExecModule::ExecModule()
{
	init();

	//displayNumber = offsetX/(gConfig->screenWidth + gConfig->screenGap);

	if(!makeWindow()) {
		LOG("failed to make window!\n");
		exit(1);
	}
}


bool ExecModule::makeWindow()
{
	const SDL_VideoInfo *videoInfo;

	if ( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
		LOG( "Video initialization failed: %s\n", SDL_GetError());
		return false;
	}
	
	
	string title = "ClusterGL Output - " + gConfig->id;
	
	LOG("Set caption: %s\n", title.c_str());

	SDL_WM_SetCaption(title.c_str(), title.c_str());

	videoInfo = SDL_GetVideoInfo( );

	int videoFlags;

	//the flags to pass to SDL_SetVideoMode
	videoFlags  = SDL_OPENGL;
	videoFlags |= SDL_GL_DOUBLEBUFFER;
	videoFlags |= SDL_HWPALETTE;
	//videoFlags |= SDL_NOFRAME;

	if(gConfig->fullscreen)
		videoFlags |= SDL_FULLSCREEN;
								 
	if(videoInfo->hw_available ){
		videoFlags |= SDL_HWSURFACE;
	}else{
		videoFlags |= SDL_SWSURFACE;
	}
	
	if(videoInfo->blit_hw ){
		videoFlags |= SDL_HWACCEL;
	}

	SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );

	//Autodetect res
	int width = gConfig->sizeX*gConfig->scaleX;
	int height = gConfig->sizeY*gConfig->scaleY;

	//set window position
	std::stringstream stream;
	stream <<gConfig->positionX<<","<<gConfig->positionY;
	setenv("SDL_VIDEO_WINDOW_POS", stream.str().c_str(), true);

	//get a SDL surface
	SDL_Surface *surface = SDL_SetVideoMode(width, height, 32, videoFlags );

	if ( !surface ) {
		LOG( "Video mode set failed: %s\n", SDL_GetError());
		return false;
	}

	//Disable mouse pointer
	//SDL_ShowCursor(SDL_DISABLE);

	//Do this twice - above works for OSX, here for Linux
	//Yeah, I know.
	SDL_WM_SetCaption(title.c_str(), title.c_str());
	
		
	if (GLEW_OK != glewInit()) {
		LOG("GLEW failed to start up for some reason\n");
		return false;
	}

	return true;
}


/*******************************************************************************
	Instruction dispatch
*******************************************************************************/
bool ExecModule::process(vector<Instruction *> *list)
{

	for(int n=0;n<(int)list->size();n++){		
		Instruction *iter = (*list)[n];
		
		if(iter->id >= 1700 || !mFunctions[iter->id]) {
			LOG("Unimplemented method %d, bailing out\n", iter->id);
			return false;
		}

		mCurrentInstruction = iter;
		currentBuffer = 0;
		
		//LOG_INSTRUCTION(mCurrentInstruction);
		
		if(handleViewMode(iter)){
			continue;
		}else{
			mFunctions[iter->id](iter->args);
		}
	}
	
	Stats::count("mod_exec instruction count", list->size());
	
	return true;
}






/*******************************************************************************
	Module and unpacking utils
*******************************************************************************/
bool ExecModule::sync()
{
	return true;
}


byte *popBuf()
{
	currentBuffer++;
	return mCurrentInstruction->buffers[currentBuffer-1].buffer;
}

byte *popBuf(int *len){
	*len = mCurrentInstruction->buffers[currentBuffer].len;
	currentBuffer++;
	return mCurrentInstruction->buffers[currentBuffer-1].buffer;	
}


#define PUSHRET(TYPE) \
	void pushRet(TYPE val) \
	{ \
		int len = sizeof(TYPE);\
		byte *b = (byte *) malloc(len);\
		memcpy(b, &val, len);\
		mCurrentInstruction->buffers[currentBuffer].buffer = b;\
		mCurrentInstruction->buffers[currentBuffer].needClear = true;\
		mCurrentInstruction->buffers[currentBuffer].needReply = true;\
	}

PUSHRET(const GLuint);
PUSHRET(const GLdouble);
PUSHRET(const GLfloat);
PUSHRET(const GLint);
PUSHRET(const GLbyte);
PUSHRET(const GLubyte);

void pushRet(const GLubyte *val)
{
	int len = strlen((char *)val);
	byte *b = (byte *) malloc(len);
	memcpy(b, val, len);
	mCurrentInstruction->buffers[currentBuffer].buffer = b;
	mCurrentInstruction->buffers[currentBuffer].needClear = true;
	mCurrentInstruction->buffers[currentBuffer].needReply = true;
}


void pushRet(const GLchar * val)
{
	int len = strlen((char *)val);
	byte *b = (byte *) malloc(len);
	memcpy(b, val, len);
	mCurrentInstruction->buffers[currentBuffer].buffer = b;
	mCurrentInstruction->buffers[currentBuffer].needClear = true;
	mCurrentInstruction->buffers[currentBuffer].needReply = true;
}


/*******************************************************************************
	CGL special functions
*******************************************************************************/

//1499
static void EXEC_CGLSwapBuffers(byte *commandbuf)
{
	//LOG("Swap!\n");
	SDL_GL_SwapBuffers();
	
	Stats::increment("Rendered frames");
}


/*******************************************************************************
	Regular OpenGL functions
*******************************************************************************/

//0
static void EXEC_glNewList(byte *commandbuf)
{
	GLuint *list = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glNewList(*list, *mode);
}


//1
static void EXEC_glEndList(byte *commandbuf)
{

	glEndList();
}


//2
static void EXEC_glCallList(byte *commandbuf)
{
	GLuint *list = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	glCallList(*list);
}


//3
static void EXEC_glCallLists(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	
	//LOG("glCallLists(%d, %d)\n", *n, *type);

	glCallLists(*n, *type, (const GLvoid *)popBuf());
}


//4
static void EXEC_glDeleteLists(byte *commandbuf)
{
	GLuint *list = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLsizei *range = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glDeleteLists(*list, *range);
}


//5
static void EXEC_glGenLists(byte *commandbuf)
{
	GLsizei *range = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	
	GLint r = glGenLists(*range);
	
	//LOG("glGenLists(%d) -> %d\n", *range, r);
	
	pushRet(r);
}


//6
static void EXEC_glListBase(byte *commandbuf)
{
	GLuint *base = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	glListBase(*base);
}


//7
static void EXEC_glBegin(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	glBegin(*mode);
}


//8
static void EXEC_glBitmap(byte *commandbuf)
{
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLfloat *xorig = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *yorig = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *xmove = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *ymove = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glBitmap(*width, *height, *xorig, *yorig, *xmove, *ymove, (const GLubyte *)popBuf());
}


//9
static void EXEC_glColor3b(byte *commandbuf)
{
	GLbyte *red = (GLbyte*)commandbuf;   commandbuf += sizeof(GLbyte);
	GLbyte *green = (GLbyte*)commandbuf;     commandbuf += sizeof(GLbyte);
	GLbyte *blue = (GLbyte*)commandbuf;  commandbuf += sizeof(GLbyte);

	glColor3b(*red, *green, *blue);
}


//10
static void EXEC_glColor3bv(byte *commandbuf)
{

	glColor3bv((const GLbyte *)popBuf());
}


//11
static void EXEC_glColor3d(byte *commandbuf)
{
	GLdouble *red = (GLdouble*)commandbuf;   commandbuf += sizeof(GLdouble);
	GLdouble *green = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *blue = (GLdouble*)commandbuf;  commandbuf += sizeof(GLdouble);

	glColor3d(*red, *green, *blue);
}


//12
static void EXEC_glColor3dv(byte *commandbuf)
{

	glColor3dv((const GLdouble *)popBuf());
}


//13
static void EXEC_glColor3f(byte *commandbuf)
{
	GLfloat *red = (GLfloat*)commandbuf;     commandbuf += sizeof(GLfloat);
	GLfloat *green = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *blue = (GLfloat*)commandbuf;    commandbuf += sizeof(GLfloat);

	glColor3f(*red, *green, *blue);
}


//14
static void EXEC_glColor3fv(byte *commandbuf)
{

	glColor3fv((const GLfloat *)popBuf());
}


//15
static void EXEC_glColor3i(byte *commandbuf)
{
	GLint *red = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *green = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *blue = (GLint*)commandbuf;    commandbuf += sizeof(GLint);

	glColor3i(*red, *green, *blue);
}


//16
static void EXEC_glColor3iv(byte *commandbuf)
{

	glColor3iv((const GLint *)popBuf());
}


//17
static void EXEC_glColor3s(byte *commandbuf)
{
	GLshort *red = (GLshort*)commandbuf;     commandbuf += sizeof(GLshort);
	GLshort *green = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *blue = (GLshort*)commandbuf;    commandbuf += sizeof(GLshort);

	glColor3s(*red, *green, *blue);
}


//18
static void EXEC_glColor3sv(byte *commandbuf)
{

	glColor3sv((const GLshort *)popBuf());
}


//19
static void EXEC_glColor3ub(byte *commandbuf)
{
	GLubyte *red = (GLubyte*)commandbuf;     commandbuf += sizeof(GLubyte);
	GLubyte *green = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *blue = (GLubyte*)commandbuf;    commandbuf += sizeof(GLubyte);

	glColor3ub(*red, *green, *blue);
}


//20
static void EXEC_glColor3ubv(byte *commandbuf)
{

	glColor3ubv((const GLubyte *)popBuf());
}


//21
static void EXEC_glColor3ui(byte *commandbuf)
{
	GLuint *red = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *green = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLuint *blue = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	glColor3ui(*red, *green, *blue);
}


//22
static void EXEC_glColor3uiv(byte *commandbuf)
{

	glColor3uiv((const GLuint *)popBuf());
}


//23
static void EXEC_glColor3us(byte *commandbuf)
{
	GLushort *red = (GLushort*)commandbuf;   commandbuf += sizeof(GLushort);
	GLushort *green = (GLushort*)commandbuf;     commandbuf += sizeof(GLushort);
	GLushort *blue = (GLushort*)commandbuf;  commandbuf += sizeof(GLushort);

	glColor3us(*red, *green, *blue);
}


//24
static void EXEC_glColor3usv(byte *commandbuf)
{

	glColor3usv((const GLushort *)popBuf());
}


//25
static void EXEC_glColor4b(byte *commandbuf)
{
	GLbyte *red = (GLbyte*)commandbuf;   commandbuf += sizeof(GLbyte);
	GLbyte *green = (GLbyte*)commandbuf;     commandbuf += sizeof(GLbyte);
	GLbyte *blue = (GLbyte*)commandbuf;  commandbuf += sizeof(GLbyte);
	GLbyte *alpha = (GLbyte*)commandbuf;     commandbuf += sizeof(GLbyte);

	glColor4b(*red, *green, *blue, *alpha);
}


//26
static void EXEC_glColor4bv(byte *commandbuf)
{

	glColor4bv((const GLbyte *)popBuf());
}


//27
static void EXEC_glColor4d(byte *commandbuf)
{
	GLdouble *red = (GLdouble*)commandbuf;   commandbuf += sizeof(GLdouble);
	GLdouble *green = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *blue = (GLdouble*)commandbuf;  commandbuf += sizeof(GLdouble);
	GLdouble *alpha = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glColor4d(*red, *green, *blue, *alpha);
}


//28
static void EXEC_glColor4dv(byte *commandbuf)
{

	glColor4dv((const GLdouble *)popBuf());
}


//29
static void EXEC_glColor4f(byte *commandbuf)
{
	GLfloat *red = (GLfloat*)commandbuf;     commandbuf += sizeof(GLfloat);
	GLfloat *green = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *blue = (GLfloat*)commandbuf;    commandbuf += sizeof(GLfloat);
	GLfloat *alpha = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glColor4f(*red, *green, *blue, *alpha);
}


//30
static void EXEC_glColor4fv(byte *commandbuf)
{

	glColor4fv((const GLfloat *)popBuf());
}


//31
static void EXEC_glColor4i(byte *commandbuf)
{
	GLint *red = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *green = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *blue = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLint *alpha = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glColor4i(*red, *green, *blue, *alpha);
}


//32
static void EXEC_glColor4iv(byte *commandbuf)
{

	glColor4iv((const GLint *)popBuf());
}


//33
static void EXEC_glColor4s(byte *commandbuf)
{
	GLshort *red = (GLshort*)commandbuf;     commandbuf += sizeof(GLshort);
	GLshort *green = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *blue = (GLshort*)commandbuf;    commandbuf += sizeof(GLshort);
	GLshort *alpha = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glColor4s(*red, *green, *blue, *alpha);
}


//34
static void EXEC_glColor4sv(byte *commandbuf)
{

	glColor4sv((const GLshort *)popBuf());
}


//35
static void EXEC_glColor4ub(byte *commandbuf)
{
	GLubyte *red = (GLubyte*)commandbuf;     commandbuf += sizeof(GLubyte);
	GLubyte *green = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *blue = (GLubyte*)commandbuf;    commandbuf += sizeof(GLubyte);
	GLubyte *alpha = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);

	glColor4ub(*red, *green, *blue, *alpha);
}


//36
static void EXEC_glColor4ubv(byte *commandbuf)
{

	glColor4ubv((const GLubyte *)popBuf());
}


//37
static void EXEC_glColor4ui(byte *commandbuf)
{
	GLuint *red = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *green = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLuint *blue = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLuint *alpha = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glColor4ui(*red, *green, *blue, *alpha);
}


//38
static void EXEC_glColor4uiv(byte *commandbuf)
{

	glColor4uiv((const GLuint *)popBuf());
}


//39
static void EXEC_glColor4us(byte *commandbuf)
{
	GLushort *red = (GLushort*)commandbuf;   commandbuf += sizeof(GLushort);
	GLushort *green = (GLushort*)commandbuf;     commandbuf += sizeof(GLushort);
	GLushort *blue = (GLushort*)commandbuf;  commandbuf += sizeof(GLushort);
	GLushort *alpha = (GLushort*)commandbuf;     commandbuf += sizeof(GLushort);

	glColor4us(*red, *green, *blue, *alpha);
}


//40
static void EXEC_glColor4usv(byte *commandbuf)
{

	glColor4usv((const GLushort *)popBuf());
}


//41
static void EXEC_glEdgeFlag(byte *commandbuf)
{
	GLboolean *flag = (GLboolean*)commandbuf;    commandbuf += sizeof(GLboolean);

	glEdgeFlag(*flag);
}


//42
static void EXEC_glEdgeFlagv(byte *commandbuf)
{

	glEdgeFlagv((const GLboolean *)popBuf());
}


//43
static void EXEC_glEnd(byte *commandbuf)
{

	glEnd();
}


//44
static void EXEC_glIndexd(byte *commandbuf)
{
	GLdouble *c = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glIndexd(*c);
}


//45
static void EXEC_glIndexdv(byte *commandbuf)
{

	glIndexdv((const GLdouble *)popBuf());
}


//46
static void EXEC_glIndexf(byte *commandbuf)
{
	GLfloat *c = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glIndexf(*c);
}


//47
static void EXEC_glIndexfv(byte *commandbuf)
{

	glIndexfv((const GLfloat *)popBuf());
}


//48
static void EXEC_glIndexi(byte *commandbuf)
{
	GLint *c = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glIndexi(*c);
}


//49
static void EXEC_glIndexiv(byte *commandbuf)
{

	glIndexiv((const GLint *)popBuf());
}


//50
static void EXEC_glIndexs(byte *commandbuf)
{
	GLshort *c = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glIndexs(*c);
}


//51
static void EXEC_glIndexsv(byte *commandbuf)
{

	glIndexsv((const GLshort *)popBuf());
}


//52
static void EXEC_glNormal3b(byte *commandbuf)
{
	GLbyte *nx = (GLbyte*)commandbuf;    commandbuf += sizeof(GLbyte);
	GLbyte *ny = (GLbyte*)commandbuf;    commandbuf += sizeof(GLbyte);
	GLbyte *nz = (GLbyte*)commandbuf;    commandbuf += sizeof(GLbyte);

	glNormal3b(*nx, *ny, *nz);
}


//53
static void EXEC_glNormal3bv(byte *commandbuf)
{

	glNormal3bv((const GLbyte *)popBuf());
}


//54
static void EXEC_glNormal3d(byte *commandbuf)
{
	GLdouble *nx = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *ny = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *nz = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);

	glNormal3d(*nx, *ny, *nz);
}


//55
static void EXEC_glNormal3dv(byte *commandbuf)
{

	glNormal3dv((const GLdouble *)popBuf());
}


//56
static void EXEC_glNormal3f(byte *commandbuf)
{
	GLfloat *nx = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *ny = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *nz = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	glNormal3f(*nx, *ny, *nz);
}


//57
static void EXEC_glNormal3fv(byte *commandbuf)
{

	glNormal3fv((const GLfloat *)popBuf());
}


//58
static void EXEC_glNormal3i(byte *commandbuf)
{
	GLint *nx = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *ny = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *nz = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	glNormal3i(*nx, *ny, *nz);
}


//59
static void EXEC_glNormal3iv(byte *commandbuf)
{

	glNormal3iv((const GLint *)popBuf());
}


//60
static void EXEC_glNormal3s(byte *commandbuf)
{
	GLshort *nx = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);
	GLshort *ny = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);
	GLshort *nz = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);

	glNormal3s(*nx, *ny, *nz);
}


//61
static void EXEC_glNormal3sv(byte *commandbuf)
{

	glNormal3sv((const GLshort *)popBuf());
}


//62
static void EXEC_glRasterPos2d(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glRasterPos2d(*x, *y);
}


//63
static void EXEC_glRasterPos2dv(byte *commandbuf)
{

	glRasterPos2dv((const GLdouble *)popBuf());
}


//64
static void EXEC_glRasterPos2f(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glRasterPos2f(*x, *y);
}


//65
static void EXEC_glRasterPos2fv(byte *commandbuf)
{

	glRasterPos2fv((const GLfloat *)popBuf());
}


//66
static void EXEC_glRasterPos2i(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glRasterPos2i(*x, *y);
}


//67
static void EXEC_glRasterPos2iv(byte *commandbuf)
{

	glRasterPos2iv((const GLint *)popBuf());
}


//68
static void EXEC_glRasterPos2s(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glRasterPos2s(*x, *y);
}


//69
static void EXEC_glRasterPos2sv(byte *commandbuf)
{

	glRasterPos2sv((const GLshort *)popBuf());
}


//70
static void EXEC_glRasterPos3d(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glRasterPos3d(*x, *y, *z);
}


//71
static void EXEC_glRasterPos3dv(byte *commandbuf)
{

	glRasterPos3dv((const GLdouble *)popBuf());
}


//72
static void EXEC_glRasterPos3f(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glRasterPos3f(*x, *y, *z);
}


//73
static void EXEC_glRasterPos3fv(byte *commandbuf)
{

	glRasterPos3fv((const GLfloat *)popBuf());
}


//74
static void EXEC_glRasterPos3i(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *z = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glRasterPos3i(*x, *y, *z);
}


//75
static void EXEC_glRasterPos3iv(byte *commandbuf)
{

	glRasterPos3iv((const GLint *)popBuf());
}


//76
static void EXEC_glRasterPos3s(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glRasterPos3s(*x, *y, *z);
}


//77
static void EXEC_glRasterPos3sv(byte *commandbuf)
{

	glRasterPos3sv((const GLshort *)popBuf());
}


//78
static void EXEC_glRasterPos4d(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *w = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glRasterPos4d(*x, *y, *z, *w);
}


//79
static void EXEC_glRasterPos4dv(byte *commandbuf)
{

	glRasterPos4dv((const GLdouble *)popBuf());
}


//80
static void EXEC_glRasterPos4f(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *w = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glRasterPos4f(*x, *y, *z, *w);
}


//81
static void EXEC_glRasterPos4fv(byte *commandbuf)
{

	glRasterPos4fv((const GLfloat *)popBuf());
}


//82
static void EXEC_glRasterPos4i(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *z = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *w = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glRasterPos4i(*x, *y, *z, *w);
}


//83
static void EXEC_glRasterPos4iv(byte *commandbuf)
{

	glRasterPos4iv((const GLint *)popBuf());
}


//84
static void EXEC_glRasterPos4s(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *w = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glRasterPos4s(*x, *y, *z, *w);
}


//85
static void EXEC_glRasterPos4sv(byte *commandbuf)
{

	glRasterPos4sv((const GLshort *)popBuf());
}


//86
static void EXEC_glRectd(byte *commandbuf)
{
	GLdouble *x1 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *y1 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *x2 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *y2 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);

	glRectd(*x1, *y1, *x2, *y2);
}


//87
static void EXEC_glRectdv(byte *commandbuf)
{

	glRectdv((const GLdouble *)popBuf(), (const GLdouble *)popBuf());
}


//88
static void EXEC_glRectf(byte *commandbuf)
{
	GLfloat *x1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *y1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *x2 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *y2 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	glRectf(*x1, *y1, *x2, *y2);
}


//89
static void EXEC_glRectfv(byte *commandbuf)
{

	glRectfv((const GLfloat *)popBuf(), (const GLfloat *)popBuf());
}


//90
static void EXEC_glRecti(byte *commandbuf)
{
	GLint *x1 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *y1 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *x2 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *y2 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	glRecti(*x1, *y1, *x2, *y2);
}


//91
static void EXEC_glRectiv(byte *commandbuf)
{

	glRectiv((const GLint *)popBuf(), (const GLint *)popBuf());
}


//92
static void EXEC_glRects(byte *commandbuf)
{
	GLshort *x1 = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);
	GLshort *y1 = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);
	GLshort *x2 = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);
	GLshort *y2 = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);

	glRects(*x1, *y1, *x2, *y2);
}


//93
static void EXEC_glRectsv(byte *commandbuf)
{

	glRectsv((const GLshort *)popBuf(), (const GLshort *)popBuf());
}


//94
static void EXEC_glTexCoord1d(byte *commandbuf)
{
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glTexCoord1d(*s);
}


//95
static void EXEC_glTexCoord1dv(byte *commandbuf)
{

	glTexCoord1dv((const GLdouble *)popBuf());
}


//96
static void EXEC_glTexCoord1f(byte *commandbuf)
{
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glTexCoord1f(*s);
}


//97
static void EXEC_glTexCoord1fv(byte *commandbuf)
{

	glTexCoord1fv((const GLfloat *)popBuf());
}


//98
static void EXEC_glTexCoord1i(byte *commandbuf)
{
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glTexCoord1i(*s);
}


//99
static void EXEC_glTexCoord1iv(byte *commandbuf)
{

	glTexCoord1iv((const GLint *)popBuf());
}


//100
static void EXEC_glTexCoord1s(byte *commandbuf)
{
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glTexCoord1s(*s);
}


//101
static void EXEC_glTexCoord1sv(byte *commandbuf)
{

	glTexCoord1sv((const GLshort *)popBuf());
}


//102
static void EXEC_glTexCoord2d(byte *commandbuf)
{
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *t = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glTexCoord2d(*s, *t);
}


//103
static void EXEC_glTexCoord2dv(byte *commandbuf)
{

	glTexCoord2dv((const GLdouble *)popBuf());
}


//104
static void EXEC_glTexCoord2f(byte *commandbuf)
{
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glTexCoord2f(*s, *t);
}


//105
static void EXEC_glTexCoord2fv(byte *commandbuf)
{

	glTexCoord2fv((const GLfloat *)popBuf());
}


//106
static void EXEC_glTexCoord2i(byte *commandbuf)
{
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *t = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glTexCoord2i(*s, *t);
}


//107
static void EXEC_glTexCoord2iv(byte *commandbuf)
{

	glTexCoord2iv((const GLint *)popBuf());
}


//108
static void EXEC_glTexCoord2s(byte *commandbuf)
{
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *t = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glTexCoord2s(*s, *t);
}


//109
static void EXEC_glTexCoord2sv(byte *commandbuf)
{

	glTexCoord2sv((const GLshort *)popBuf());
}


//110
static void EXEC_glTexCoord3d(byte *commandbuf)
{
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *t = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *r = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glTexCoord3d(*s, *t, *r);
}


//111
static void EXEC_glTexCoord3dv(byte *commandbuf)
{

	glTexCoord3dv((const GLdouble *)popBuf());
}


//112
static void EXEC_glTexCoord3f(byte *commandbuf)
{
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *r = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glTexCoord3f(*s, *t, *r);
}


//113
static void EXEC_glTexCoord3fv(byte *commandbuf)
{

	glTexCoord3fv((const GLfloat *)popBuf());
}


//114
static void EXEC_glTexCoord3i(byte *commandbuf)
{
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *t = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *r = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glTexCoord3i(*s, *t, *r);
}


//115
static void EXEC_glTexCoord3iv(byte *commandbuf)
{

	glTexCoord3iv((const GLint *)popBuf());
}


//116
static void EXEC_glTexCoord3s(byte *commandbuf)
{
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *t = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *r = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glTexCoord3s(*s, *t, *r);
}


//117
static void EXEC_glTexCoord3sv(byte *commandbuf)
{

	glTexCoord3sv((const GLshort *)popBuf());
}


//118
static void EXEC_glTexCoord4d(byte *commandbuf)
{
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *t = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *r = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *q = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glTexCoord4d(*s, *t, *r, *q);
}


//119
static void EXEC_glTexCoord4dv(byte *commandbuf)
{

	glTexCoord4dv((const GLdouble *)popBuf());
}


//120
static void EXEC_glTexCoord4f(byte *commandbuf)
{
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *r = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *q = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glTexCoord4f(*s, *t, *r, *q);
}


//121
static void EXEC_glTexCoord4fv(byte *commandbuf)
{

	glTexCoord4fv((const GLfloat *)popBuf());
}


//122
static void EXEC_glTexCoord4i(byte *commandbuf)
{
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *t = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *r = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *q = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glTexCoord4i(*s, *t, *r, *q);
}


//123
static void EXEC_glTexCoord4iv(byte *commandbuf)
{

	glTexCoord4iv((const GLint *)popBuf());
}


//124
static void EXEC_glTexCoord4s(byte *commandbuf)
{
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *t = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *r = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *q = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glTexCoord4s(*s, *t, *r, *q);
}


//125
static void EXEC_glTexCoord4sv(byte *commandbuf)
{

	glTexCoord4sv((const GLshort *)popBuf());
}


//126
static void EXEC_glVertex2d(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glVertex2d(*x, *y);
}


//127
static void EXEC_glVertex2dv(byte *commandbuf)
{

	glVertex2dv((const GLdouble *)popBuf());
}


//128
static void EXEC_glVertex2f(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	glVertex2f(*x, *y);
}


//129
static void EXEC_glVertex2fv(byte *commandbuf)
{

	glVertex2fv((const GLfloat *)popBuf());
}


//130
static void EXEC_glVertex2i(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glVertex2i(*x, *y);
}


//131
static void EXEC_glVertex2iv(byte *commandbuf)
{

	glVertex2iv((const GLint *)popBuf());
}


//132
static void EXEC_glVertex2s(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glVertex2s(*x, *y);
}


//133
static void EXEC_glVertex2sv(byte *commandbuf)
{

	glVertex2sv((const GLshort *)popBuf());
}


//134
static void EXEC_glVertex3d(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glVertex3d(*x, *y, *z);
}


//135
static void EXEC_glVertex3dv(byte *commandbuf)
{

	glVertex3dv((const GLdouble *)popBuf());
}


//136
static void EXEC_glVertex3f(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glVertex3f(*x, *y, *z);
}


//137
static void EXEC_glVertex3fv(byte *commandbuf)
{

	glVertex3fv((const GLfloat *)popBuf());
}


//138
static void EXEC_glVertex3i(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *z = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glVertex3i(*x, *y, *z);
}


//139
static void EXEC_glVertex3iv(byte *commandbuf)
{

	glVertex3iv((const GLint *)popBuf());
}


//140
static void EXEC_glVertex3s(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glVertex3s(*x, *y, *z);
}


//141
static void EXEC_glVertex3sv(byte *commandbuf)
{

	glVertex3sv((const GLshort *)popBuf());
}


//142
static void EXEC_glVertex4d(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *w = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glVertex4d(*x, *y, *z, *w);
}


//143
static void EXEC_glVertex4dv(byte *commandbuf)
{

	glVertex4dv((const GLdouble *)popBuf());
}


//144
static void EXEC_glVertex4f(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *w = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glVertex4f(*x, *y, *z, *w);
}


//145
static void EXEC_glVertex4fv(byte *commandbuf)
{

	glVertex4fv((const GLfloat *)popBuf());
}


//146
static void EXEC_glVertex4i(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *z = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *w = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glVertex4i(*x, *y, *z, *w);
}


//147
static void EXEC_glVertex4iv(byte *commandbuf)
{

	glVertex4iv((const GLint *)popBuf());
}


//148
static void EXEC_glVertex4s(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *w = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glVertex4s(*x, *y, *z, *w);
}


//149
static void EXEC_glVertex4sv(byte *commandbuf)
{

	glVertex4sv((const GLshort *)popBuf());
}


//150
static void EXEC_glClipPlane(byte *commandbuf)
{
	GLenum *plane = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glClipPlane(*plane, (const GLdouble *)popBuf());
}


//151
static void EXEC_glColorMaterial(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glColorMaterial(*face, *mode);
}


//152
static void EXEC_glCullFace(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glCullFace(*mode);
}


//153
static void EXEC_glFogf(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glFogf(*pname, *param);
}


//154
static void EXEC_glFogfv(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glFogfv(*pname, (const GLfloat *)popBuf());
}


//155
static void EXEC_glFogi(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glFogi(*pname, *param);
}


//156
static void EXEC_glFogiv(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	glFogiv(*pname, (const GLint *)popBuf());
}


//157
static void EXEC_glFrontFace(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glFrontFace(*mode);
}


//158
static void EXEC_glHint(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glHint(*target, *mode);
}


//159
static void EXEC_glLightf(byte *commandbuf)
{
	GLenum *light = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glLightf(*light, *pname, *param);
}


//160
static void EXEC_glLightfv(byte *commandbuf)
{
	GLenum *light = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glLightfv(*light, *pname, (const GLfloat *)popBuf());
}


//161
static void EXEC_glLighti(byte *commandbuf)
{
	GLenum *light = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glLighti(*light, *pname, *param);
}


//162
static void EXEC_glLightiv(byte *commandbuf)
{
	GLenum *light = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glLightiv(*light, *pname, (const GLint *)popBuf());
}


//163
static void EXEC_glLightModelf(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glLightModelf(*pname, *param);
}


//164
static void EXEC_glLightModelfv(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glLightModelfv(*pname, (const GLfloat *)popBuf());
}


//165
static void EXEC_glLightModeli(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glLightModeli(*pname, *param);
}


//166
static void EXEC_glLightModeliv(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glLightModeliv(*pname, (const GLint *)popBuf());
}


//167
static void EXEC_glLineStipple(byte *commandbuf)
{
	GLint *factor = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLushort *pattern = (GLushort*)commandbuf;   commandbuf += sizeof(GLushort);

	glLineStipple(*factor, *pattern);
}


//168
static void EXEC_glLineWidth(byte *commandbuf)
{
	GLfloat *width = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glLineWidth(*width);
}


//169
static void EXEC_glMaterialf(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glMaterialf(*face, *pname, *param);
}


//170
static void EXEC_glMaterialfv(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	int i =0;
	glMaterialfv(*face, *pname, (const GLfloat *)popBuf(&i));
	
	//LOG("glMaterialfv: %d\n", i);
}


//171
static void EXEC_glMateriali(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glMateriali(*face, *pname, *param);
}


//172
static void EXEC_glMaterialiv(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glMaterialiv(*face, *pname, (const GLint *)popBuf());
}


//173
static void EXEC_glPointSize(byte *commandbuf)
{
	GLfloat *size = (GLfloat*)commandbuf;    commandbuf += sizeof(GLfloat);

	glPointSize(*size);
}


//174
static void EXEC_glPolygonMode(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glPolygonMode(*face, *mode);
}


//175
static void EXEC_glPolygonStipple(byte *commandbuf)
{

	glPolygonStipple((const GLubyte *)popBuf());
}


//176
static void EXEC_glScissor(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	glScissor(*x, *y, *width, *height);
}


//177
static void EXEC_glShadeModel(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glShadeModel(*mode);
}


//178
static void EXEC_glTexParameterf(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glTexParameterf(*target, *pname, *param);
}


//179
static void EXEC_glTexParameterfv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glTexParameterfv(*target, *pname, (const GLfloat *)popBuf());
}


//180
static void EXEC_glTexParameteri(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	glTexParameteri(*target, *pname, *param);
}


//181
static void EXEC_glTexParameteriv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glTexParameteriv(*target, *pname, (const GLint *)popBuf());
}


//182
static void EXEC_glTexImage1D(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *internalformat = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLint *border = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glTexImage1D(*target, *level, *internalformat, *width, *border, *format, *type, (const GLvoid *)popBuf());
}


//183
static void EXEC_glTexImage2D(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *internalformat = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLint *border = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLboolean *null = (GLboolean*)commandbuf;  commandbuf += sizeof(GLboolean);
	
	int l =0;
	byte *pixels = popBuf(&l);
	
	//LOG("glTexImage2D: %d/%d, %d %d\n", *width, *height, l, hash(pixels, l));
		
	//if(*null) {
		glTexImage2D(*target, *level, *internalformat, *width, *height, *border, *format, *type, (const GLvoid *)pixels);
	//}
	//else {
	//	LOG("183 no pixels!\n");
	//	glTexImage2D(*target, *level, *internalformat, *width, *height, *border, *format, *type, NULL);
	//}
	
}


//184
static void EXEC_glTexEnvf(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glTexEnvf(*target, *pname, *param);
}


//185
static void EXEC_glTexEnvfv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glTexEnvfv(*target, *pname, (const GLfloat *)popBuf());
}


//186
static void EXEC_glTexEnvi(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glTexEnvi(*target, *pname, *param);
}


//187
static void EXEC_glTexEnviv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glTexEnviv(*target, *pname, (const GLint *)popBuf());
}


//188
static void EXEC_glTexGend(byte *commandbuf)
{
	GLenum *coord = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLdouble *param = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glTexGend(*coord, *pname, *param);
}


//189
static void EXEC_glTexGendv(byte *commandbuf)
{
	GLenum *coord = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glTexGendv(*coord, *pname, (const GLdouble *)popBuf());
}


//190
static void EXEC_glTexGenf(byte *commandbuf)
{
	GLenum *coord = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glTexGenf(*coord, *pname, *param);
}


//191
static void EXEC_glTexGenfv(byte *commandbuf)
{
	GLenum *coord = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glTexGenfv(*coord, *pname, (const GLfloat *)popBuf());
}


//192
static void EXEC_glTexGeni(byte *commandbuf)
{
	GLenum *coord = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glTexGeni(*coord, *pname, *param);
}


//193
static void EXEC_glTexGeniv(byte *commandbuf)
{
	GLenum *coord = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glTexGeniv(*coord, *pname, (const GLint *)popBuf());
}


//194
static void EXEC_glFeedbackBuffer(byte *commandbuf)
{
	GLsizei *size = (GLsizei*)commandbuf;    commandbuf += sizeof(GLsizei);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glFeedbackBuffer(*size, *type, (GLfloat *)popBuf());
}


//195
static void EXEC_glSelectBuffer(byte *commandbuf)
{
	GLsizei *size = (GLsizei*)commandbuf;    commandbuf += sizeof(GLsizei);

	glSelectBuffer(*size, (GLuint *)popBuf());
}


//196
static void EXEC_glRenderMode(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	pushRet(glRenderMode(*mode));
}


//197
static void EXEC_glInitNames(byte *commandbuf)
{

	glInitNames();
}


//198
static void EXEC_glLoadName(byte *commandbuf)
{
	GLuint *name = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	glLoadName(*name);
}


//199
static void EXEC_glPassThrough(byte *commandbuf)
{
	GLfloat *token = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glPassThrough(*token);
}


//200
static void EXEC_glPopName(byte *commandbuf)
{

	glPopName();
}


//201
static void EXEC_glPushName(byte *commandbuf)
{
	GLuint *name = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	glPushName(*name);
}


//202
static void EXEC_glDrawBuffer(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glDrawBuffer(*mode);
}


//203
static void EXEC_glClear(byte *commandbuf)
{
	GLbitfield *mask = (GLbitfield*)commandbuf;  commandbuf += sizeof(GLbitfield);

	glClear(*mask);
}


//204
static void EXEC_glClearAccum(byte *commandbuf)
{
	GLfloat *red = (GLfloat*)commandbuf;     commandbuf += sizeof(GLfloat);
	GLfloat *green = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *blue = (GLfloat*)commandbuf;    commandbuf += sizeof(GLfloat);
	GLfloat *alpha = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glClearAccum(*red, *green, *blue, *alpha);
}


//205
static void EXEC_glClearIndex(byte *commandbuf)
{
	GLfloat *c = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glClearIndex(*c);
}


//206
static void EXEC_glClearColor(byte *commandbuf)
{
	GLclampf *red = (GLclampf*)commandbuf;   commandbuf += sizeof(GLclampf);
	GLclampf *green = (GLclampf*)commandbuf;     commandbuf += sizeof(GLclampf);
	GLclampf *blue = (GLclampf*)commandbuf;  commandbuf += sizeof(GLclampf);
	GLclampf *alpha = (GLclampf*)commandbuf;     commandbuf += sizeof(GLclampf);

	glClearColor(*red, *green, *blue, *alpha);
}


//207
static void EXEC_glClearStencil(byte *commandbuf)
{
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glClearStencil(*s);
}


//208
static void EXEC_glClearDepth(byte *commandbuf)
{
	GLclampd *depth = (GLclampd*)commandbuf;     commandbuf += sizeof(GLclampd);

	glClearDepth(*depth);
}


//209
static void EXEC_glStencilMask(byte *commandbuf)
{
	GLuint *mask = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	glStencilMask(*mask);
}


//210
static void EXEC_glColorMask(byte *commandbuf)
{
	GLboolean *red = (GLboolean*)commandbuf;     commandbuf += sizeof(GLboolean);
	GLboolean *green = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);
	GLboolean *blue = (GLboolean*)commandbuf;    commandbuf += sizeof(GLboolean);
	GLboolean *alpha = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);

	glColorMask(*red, *green, *blue, *alpha);
}


//211
static void EXEC_glDepthMask(byte *commandbuf)
{
	GLboolean *flag = (GLboolean*)commandbuf;    commandbuf += sizeof(GLboolean);

	glDepthMask(*flag);
}


//212
static void EXEC_glIndexMask(byte *commandbuf)
{
	GLuint *mask = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	glIndexMask(*mask);
}


//213
static void EXEC_glAccum(byte *commandbuf)
{
	GLenum *op = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLfloat *value = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glAccum(*op, *value);
}


//214
static void EXEC_glDisable(byte *commandbuf)
{
	GLenum *cap = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	glDisable(*cap);
}


//215
static void EXEC_glEnable(byte *commandbuf)
{
	GLenum *cap = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	glEnable(*cap);
}


//216
static void EXEC_glFinish(byte *commandbuf)
{

	glFinish();
}


//217
static void EXEC_glFlush(byte *commandbuf)
{

	glFlush();
}


//218
static void EXEC_glPopAttrib(byte *commandbuf)
{

	glPopAttrib();
}


//219
static void EXEC_glPushAttrib(byte *commandbuf)
{
	GLbitfield *mask = (GLbitfield*)commandbuf;  commandbuf += sizeof(GLbitfield);

	glPushAttrib(*mask);
}


//220
static void EXEC_glMap1d(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLdouble *u1 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *u2 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLint *stride = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *order = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glMap1d(*target, *u1, *u2, *stride, *order, (const GLdouble *)popBuf());
}


//221
static void EXEC_glMap1f(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLfloat *u1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *u2 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLint *stride = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *order = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glMap1f(*target, *u1, *u2, *stride, *order, (const GLfloat *)popBuf());
}


//222
static void EXEC_glMap2d(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLdouble *u1 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *u2 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLint *ustride = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *uorder = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLdouble *v1 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *v2 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLint *vstride = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *vorder = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	glMap2d(*target, *u1, *u2, *ustride, *uorder, *v1, *v2, *vstride, *vorder, (const GLdouble *)popBuf());
}


//223
static void EXEC_glMap2f(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLfloat *u1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *u2 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLint *ustride = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *uorder = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLfloat *v1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v2 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLint *vstride = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *vorder = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	glMap2f(*target, *u1, *u2, *ustride, *uorder, *v1, *v2, *vstride, *vorder, (const GLfloat *)popBuf());
}


//224
static void EXEC_glMapGrid1d(byte *commandbuf)
{
	GLint *un = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLdouble *u1 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *u2 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);

	glMapGrid1d(*un, *u1, *u2);
}


//225
static void EXEC_glMapGrid1f(byte *commandbuf)
{
	GLint *un = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLfloat *u1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *u2 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	glMapGrid1f(*un, *u1, *u2);
}


//226
static void EXEC_glMapGrid2d(byte *commandbuf)
{
	GLint *un = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLdouble *u1 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *u2 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLint *vn = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLdouble *v1 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *v2 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);

	glMapGrid2d(*un, *u1, *u2, *vn, *v1, *v2);
}


//227
static void EXEC_glMapGrid2f(byte *commandbuf)
{
	GLint *un = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLfloat *u1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *u2 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLint *vn = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLfloat *v1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v2 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	glMapGrid2f(*un, *u1, *u2, *vn, *v1, *v2);
}


//228
static void EXEC_glEvalCoord1d(byte *commandbuf)
{
	GLdouble *u = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glEvalCoord1d(*u);
}


//229
static void EXEC_glEvalCoord1dv(byte *commandbuf)
{

	glEvalCoord1dv((const GLdouble *)popBuf());
}


//230
static void EXEC_glEvalCoord1f(byte *commandbuf)
{
	GLfloat *u = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glEvalCoord1f(*u);
}


//231
static void EXEC_glEvalCoord1fv(byte *commandbuf)
{

	glEvalCoord1fv((const GLfloat *)popBuf());
}


//232
static void EXEC_glEvalCoord2d(byte *commandbuf)
{
	GLdouble *u = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *v = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glEvalCoord2d(*u, *v);
}


//233
static void EXEC_glEvalCoord2dv(byte *commandbuf)
{

	glEvalCoord2dv((const GLdouble *)popBuf());
}


//234
static void EXEC_glEvalCoord2f(byte *commandbuf)
{
	GLfloat *u = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *v = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glEvalCoord2f(*u, *v);
}


//235
static void EXEC_glEvalCoord2fv(byte *commandbuf)
{

	glEvalCoord2fv((const GLfloat *)popBuf());
}


//236
static void EXEC_glEvalMesh1(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLint *i1 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *i2 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	glEvalMesh1(*mode, *i1, *i2);
}


//237
static void EXEC_glEvalPoint1(byte *commandbuf)
{
	GLint *i = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glEvalPoint1(*i);
}


//238
static void EXEC_glEvalMesh2(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLint *i1 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *i2 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *j1 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *j2 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	glEvalMesh2(*mode, *i1, *i2, *j1, *j2);
}


//239
static void EXEC_glEvalPoint2(byte *commandbuf)
{
	GLint *i = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *j = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glEvalPoint2(*i, *j);
}


//240
static void EXEC_glAlphaFunc(byte *commandbuf)
{
	GLenum *func = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLclampf *ref = (GLclampf*)commandbuf;   commandbuf += sizeof(GLclampf);

	glAlphaFunc(*func, *ref);
}


//241
static void EXEC_glBlendFunc(byte *commandbuf)
{
	GLenum *sfactor = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLenum *dfactor = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	glBlendFunc(*sfactor, *dfactor);
}


//242
static void EXEC_glLogicOp(byte *commandbuf)
{
	GLenum *opcode = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glLogicOp(*opcode);
}


//243
static void EXEC_glStencilFunc(byte *commandbuf)
{
	GLenum *func = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLint *ref = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLuint *mask = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	glStencilFunc(*func, *ref, *mask);
}


//244
static void EXEC_glStencilOp(byte *commandbuf)
{
	GLenum *fail = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *zfail = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *zpass = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glStencilOp(*fail, *zfail, *zpass);
}


//245
static void EXEC_glDepthFunc(byte *commandbuf)
{
	GLenum *func = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glDepthFunc(*func);
}


//246
static void EXEC_glPixelZoom(byte *commandbuf)
{
	GLfloat *xfactor = (GLfloat*)commandbuf;     commandbuf += sizeof(GLfloat);
	GLfloat *yfactor = (GLfloat*)commandbuf;     commandbuf += sizeof(GLfloat);

	glPixelZoom(*xfactor, *yfactor);
}


//247
static void EXEC_glPixelTransferf(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glPixelTransferf(*pname, *param);
}


//248
static void EXEC_glPixelTransferi(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glPixelTransferi(*pname, *param);
}


//249
static void EXEC_glPixelStoref(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glPixelStoref(*pname, *param);
}


//250
static void EXEC_glPixelStorei(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glPixelStorei(*pname, *param);
}


//251
static void EXEC_glPixelMapfv(byte *commandbuf)
{
	GLenum *map = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLsizei *mapsize = (GLsizei*)commandbuf;     commandbuf += sizeof(GLsizei);

	glPixelMapfv(*map, *mapsize, (const GLfloat *)popBuf());
}


//252
static void EXEC_glPixelMapuiv(byte *commandbuf)
{
	GLenum *map = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLsizei *mapsize = (GLsizei*)commandbuf;     commandbuf += sizeof(GLsizei);

	glPixelMapuiv(*map, *mapsize, (const GLuint *)popBuf());
}


//253
static void EXEC_glPixelMapusv(byte *commandbuf)
{
	GLenum *map = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLsizei *mapsize = (GLsizei*)commandbuf;     commandbuf += sizeof(GLsizei);

	glPixelMapusv(*map, *mapsize, (const GLushort *)popBuf());
}


//254
static void EXEC_glReadBuffer(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glReadBuffer(*mode);
}


//255
static void EXEC_glCopyPixels(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glCopyPixels(*x, *y, *width, *height, *type);
}


//256
static void EXEC_glReadPixels(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glReadPixels(*x, *y, *width, *height, *format, *type, (GLvoid *)popBuf());
}


//257
static void EXEC_glDrawPixels(byte *commandbuf)
{
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glDrawPixels(*width, *height, *format, *type, (const GLvoid *)popBuf());
}


//258
static void EXEC_glGetBooleanv(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetBooleanv(*pname, (GLboolean *)popBuf());
}


//259
static void EXEC_glGetClipPlane(byte *commandbuf)
{
	GLenum *plane = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetClipPlane(*plane, (GLdouble *)popBuf());
}


//260
static void EXEC_glGetDoublev(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetDoublev(*pname, (GLdouble *)popBuf());
}


//261
static void EXEC_glGetError(byte *commandbuf)
{

	pushRet(glGetError());
}


//262
static void EXEC_glGetFloatv(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetFloatv(*pname, (GLfloat *)popBuf());
}


//263
static void EXEC_glGetIntegerv(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetIntegerv(*pname, (GLint *)popBuf());
}


//264
static void EXEC_glGetLightfv(byte *commandbuf)
{
	GLenum *light = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetLightfv(*light, *pname, (GLfloat *)popBuf());
}


//265
static void EXEC_glGetLightiv(byte *commandbuf)
{
	GLenum *light = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetLightiv(*light, *pname, (GLint *)popBuf());
}


//266
static void EXEC_glGetMapdv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *query = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetMapdv(*target, *query, (GLdouble *)popBuf());
}


//267
static void EXEC_glGetMapfv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *query = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetMapfv(*target, *query, (GLfloat *)popBuf());
}


//268
static void EXEC_glGetMapiv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *query = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetMapiv(*target, *query, (GLint *)popBuf());
}


//269
static void EXEC_glGetMaterialfv(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetMaterialfv(*face, *pname, (GLfloat *)popBuf());
}


//270
static void EXEC_glGetMaterialiv(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetMaterialiv(*face, *pname, (GLint *)popBuf());
}


//271
static void EXEC_glGetPixelMapfv(byte *commandbuf)
{
	GLenum *map = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	glGetPixelMapfv(*map, (GLfloat *)popBuf());
}


//272
static void EXEC_glGetPixelMapuiv(byte *commandbuf)
{
	GLenum *map = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	glGetPixelMapuiv(*map, (GLuint *)popBuf());
}


//273
static void EXEC_glGetPixelMapusv(byte *commandbuf)
{
	GLenum *map = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	glGetPixelMapusv(*map, (GLushort *)popBuf());
}


//274
static void EXEC_glGetPolygonStipple(byte *commandbuf)
{

	glGetPolygonStipple((GLubyte *)popBuf());
}


//275
static void EXEC_glGetString(byte *commandbuf)
{
	GLenum *name = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	pushRet(glGetString(*name));
}


//276
static void EXEC_glGetTexEnvfv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetTexEnvfv(*target, *pname, (GLfloat *)popBuf());
}


//277
static void EXEC_glGetTexEnviv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetTexEnviv(*target, *pname, (GLint *)popBuf());
}


//278
static void EXEC_glGetTexGendv(byte *commandbuf)
{
	GLenum *coord = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetTexGendv(*coord, *pname, (GLdouble *)popBuf());
}


//279
static void EXEC_glGetTexGenfv(byte *commandbuf)
{
	GLenum *coord = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetTexGenfv(*coord, *pname, (GLfloat *)popBuf());
}


//280
static void EXEC_glGetTexGeniv(byte *commandbuf)
{
	GLenum *coord = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetTexGeniv(*coord, *pname, (GLint *)popBuf());
}


//281
static void EXEC_glGetTexImage(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glGetTexImage(*target, *level, *format, *type, (GLvoid *)popBuf());
}


//282
static void EXEC_glGetTexParameterfv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetTexParameterfv(*target, *pname, (GLfloat *)popBuf());
}


//283
static void EXEC_glGetTexParameteriv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetTexParameteriv(*target, *pname, (GLint *)popBuf());
}


//284
static void EXEC_glGetTexLevelParameterfv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetTexLevelParameterfv(*target, *level, *pname, (GLfloat *)popBuf());
}


//285
static void EXEC_glGetTexLevelParameteriv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetTexLevelParameteriv(*target, *level, *pname, (GLint *)popBuf());
}


//286
static void EXEC_glIsEnabled(byte *commandbuf)
{
	GLenum *cap = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	pushRet(glIsEnabled(*cap));
}


//287
static void EXEC_glIsList(byte *commandbuf)
{
	GLuint *list = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	pushRet(glIsList(*list));
}


//288
static void EXEC_glDepthRange(byte *commandbuf)
{
	GLclampd *zNear = (GLclampd*)commandbuf;     commandbuf += sizeof(GLclampd);
	GLclampd *zFar = (GLclampd*)commandbuf;  commandbuf += sizeof(GLclampd);

	glDepthRange(*zNear, *zFar);
}


//289
static void EXEC_glFrustum(byte *commandbuf)
{
	GLdouble *left = (GLdouble*)commandbuf;  commandbuf += sizeof(GLdouble);
	GLdouble *right = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *bottom = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *top = (GLdouble*)commandbuf;   commandbuf += sizeof(GLdouble);
	GLdouble *zNear = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *zFar = (GLdouble*)commandbuf;  commandbuf += sizeof(GLdouble);
	LOG("called glFrustum, panic!!!!\n");
	glFrustum(*left, *right, *bottom, *top, *zNear, *zFar);
}


//290
static void EXEC_glLoadIdentity(byte *commandbuf)
{

	glLoadIdentity();
}


//291
static void EXEC_glLoadMatrixf(byte *commandbuf)
{

	glLoadMatrixf((const GLfloat *)popBuf());
}


//292
static void EXEC_glLoadMatrixd(byte *commandbuf)
{

	glLoadMatrixd((const GLdouble *)popBuf());
}


//293
static void EXEC_glMatrixMode(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	currentMode = *mode;
	glMatrixMode(*mode);
}


//294
static void EXEC_glMultMatrixf(byte *commandbuf)
{

	glMultMatrixf((const GLfloat *)popBuf());
}


//295
static void EXEC_glMultMatrixd(byte *commandbuf)
{

	glMultMatrixd((const GLdouble *)popBuf());
}


//296
static void EXEC_glOrtho(byte *commandbuf)
{
	GLdouble *left = (GLdouble*)commandbuf;  commandbuf += sizeof(GLdouble);
	GLdouble *right = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *bottom = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *top = (GLdouble*)commandbuf;   commandbuf += sizeof(GLdouble);
	GLdouble *zNear = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *zFar = (GLdouble*)commandbuf;  commandbuf += sizeof(GLdouble);

	glOrtho(*left, *right, *bottom, *top, *zNear, *zFar);
}


//297
static void EXEC_glPopMatrix(byte *commandbuf)
{

	glPopMatrix();
}


//298
static void EXEC_glPushMatrix(byte *commandbuf)
{

	glPushMatrix();
}


//299
static void EXEC_glRotated(byte *commandbuf)
{
	GLdouble *angle = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glRotated(*angle, *x, *y, *z);
}


//300
static void EXEC_glRotatef(byte *commandbuf)
{
	GLfloat *angle = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glRotatef(*angle, *x, *y, *z);
}


//301
static void EXEC_glScaled(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glScaled(*x, *y, *z);
}


//302
static void EXEC_glScalef(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glScalef(*x, *y, *z);
}


//303
static void EXEC_glTranslated(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glTranslated(*x, *y, *z);
}


//304
static void EXEC_glTranslatef(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glTranslatef(*x, *y, *z);
}


//305
static void EXEC_glViewport(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	glViewport(*x, *y, *width, *height);
}


//306
static void EXEC_glArrayElement(byte *commandbuf)
{
	GLint *i = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glArrayElement(*i);
}


//307
static void EXEC_glBindTexture(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *texture = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	glBindTexture(*target, *texture);
}


//moved glcolorpointer to be with glVertexPointer etc

//309
static void EXEC_glDisableClientState(byte *commandbuf)
{
	GLenum *array = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glDisableClientState(*array);
}


//310
static void EXEC_glDrawArrays(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLint *first = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	
	//LOG("Starting glDrawArrays(%d, %d, %d)\n", *mode, *first, *count);
	
	glDrawArrays(*mode, *first, *count);
	
	//LOG("ok\n");
}


//311
static void EXEC_glDrawElements(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	
	int l = 0;
	
	const GLvoid * buf = (const GLvoid *)popBuf(&l);
		
	//LOG("About to glDrawElements(%d, %d, %d)\n", l, *count, hash((byte *)buf, l));
	
	glDrawElements(*mode, *count, *type, buf);
	
	//LOG("Done!\n");
}


//312
static void EXEC_glEdgeFlagPointer(byte *commandbuf)
{
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	glEdgeFlagPointer(*stride, (const GLvoid *)popBuf());
}


//313
static void EXEC_glEnableClientState(byte *commandbuf)
{
	GLenum *array = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glEnableClientState(*array);
}


//314
static void EXEC_glIndexPointer(byte *commandbuf)
{
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	glIndexPointer(*type, *stride, (const GLvoid *)popBuf());
}


//315
static void EXEC_glIndexub(byte *commandbuf)
{
	GLubyte *c = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);

	glIndexub(*c);
}


//316
static void EXEC_glIndexubv(byte *commandbuf)
{

	glIndexubv((const GLubyte *)popBuf());
}


//317
static void EXEC_glInterleavedArrays(byte *commandbuf)
{
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLboolean *null = (GLboolean*)commandbuf;    commandbuf += sizeof(GLsizei);
	if(*null)
		glInterleavedArrays(*format, *stride, (char *) NULL);
	else {
		const GLvoid * buf =  (const GLvoid *)popBuf();
		glInterleavedArrays(*format, *stride, buf);
	}
}


//318
static void EXEC_glNormalPointer(byte *commandbuf)
{
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLboolean *null = (GLboolean*)commandbuf;    commandbuf += sizeof(GLsizei);
	
	if(*null){
		glNormalPointer(*type, *stride, (const GLvoid *)NULL);	
	}else{
		glNormalPointer(*type, *stride, (const GLvoid *)popBuf());
	}
}


//319
static void EXEC_glPolygonOffset(byte *commandbuf)
{
	GLfloat *factor = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *units = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glPolygonOffset(*factor, *units);
}









//308
static void EXEC_glColorPointer(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLboolean *null = (GLboolean*)commandbuf;    commandbuf += sizeof(GLsizei);
	if(*null)
		glColorPointer(*size, *type, *stride, (char *) NULL);
	else {
		int i = 0;
		const GLvoid * buf =  (const GLvoid *)popBuf(&i);
		glColorPointer(*size, *type, *stride, buf);
		//LOG("glColorPointer size: %d, %d, %d\n", *size, i, hash((byte *)buf, i));
		//LOG("glColorPointer size: %d, bytes: %d\n", *size, i);
		
		//LOG("EXEC glColorPointer(%d, %s, %d) - %d\n", *size, getGLParamName(*type), *stride, i);
	}
}

//320
static void EXEC_glTexCoordPointer(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLboolean *null = (GLboolean*)commandbuf;    commandbuf += sizeof(GLsizei);
	if(*null)
		glTexCoordPointer(*size, *type, *stride, (char *) NULL);
	else {
		int i = 0;
		const GLvoid * buf =  (const GLvoid *)popBuf(&i);
		glTexCoordPointer(*size, *type, *stride, buf);
		//LOG("glTexCoordPointer size: %d, bytes: %d\n", *size, i);
		
		//LOG("EXEC glTexCoordPointer(%d, %s, %d) - %d\n", *size, getGLParamName(*type), *stride, i);
		
	}
}


//321
static void EXEC_glVertexPointer(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLboolean *null = (GLboolean*)commandbuf;    commandbuf += sizeof(GLsizei);

	if(*null)
		glVertexPointer(*size, *type, *stride, (char *) NULL);
	else {
		int i = 0;
		const GLvoid * buf =  (const GLvoid *)popBuf(&i);
		glVertexPointer(*size, *type, *stride, buf);
		//LOG("glVertexPointer size: %d, bytes: %d\n", *size, i);
		
		//LOG("EXEC glVertexPointer(%d, %s, %d) - %d\n", *size, getGLParamName(*type), *stride, i);
	}
}












//322
static void EXEC_glAreTexturesResident(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	pushRet(glAreTexturesResident(*n, (const GLuint *)popBuf(), (GLboolean *)popBuf()));
}


//323
static void EXEC_glCopyTexImage1D(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLint *border = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	glCopyTexImage1D(*target, *level, *internalformat, *x, *y, *width, *border);
}


//324
static void EXEC_glCopyTexImage2D(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLint *border = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	glCopyTexImage2D(*target, *level, *internalformat, *x, *y, *width, *height, *border);
}


//325
static void EXEC_glCopyTexSubImage1D(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *xoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glCopyTexSubImage1D(*target, *level, *xoffset, *x, *y, *width);
}


//326
static void EXEC_glCopyTexSubImage2D(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *xoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *yoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	glCopyTexSubImage2D(*target, *level, *xoffset, *yoffset, *x, *y, *width, *height);
}


//327
static void EXEC_glDeleteTextures(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glDeleteTextures(*n, (const GLuint *)popBuf());
}


//328
static void EXEC_glGenTextures(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	glGenTextures(*n, (GLuint *)popBuf());
}


//329
static void EXEC_glGetPointerv(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetPointerv(*pname, (GLvoid **)popBuf());
}


//330
static void EXEC_glIsTexture(byte *commandbuf)
{
	GLuint *texture = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	pushRet(glIsTexture(*texture));
}


//331
static void EXEC_glPrioritizeTextures(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glPrioritizeTextures(*n, (const GLuint *)popBuf(), (const GLclampf *)popBuf());
}


//332
static void EXEC_glTexSubImage1D(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *xoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	//GLuint *UNUSED = (GLuint*)commandbuf;	 commandbuf += sizeof(GLuint);

	glTexSubImage1D(*target, *level, *xoffset, *width, *format, *type,  (const GLvoid *)popBuf());
}


//333
static void EXEC_glTexSubImage2D(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *xoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *yoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	//GLuint *UNUSED = (GLuint*)commandbuf;	 commandbuf += sizeof(GLuint);

	glTexSubImage2D(*target, *level, *xoffset, *yoffset, *width, *height, *format, *type,  (const GLvoid *)popBuf());
}


//334
static void EXEC_glPopClientAttrib(byte *commandbuf)
{

	glPopClientAttrib();
}


//335
static void EXEC_glPushClientAttrib(byte *commandbuf)
{
	GLbitfield *mask = (GLbitfield*)commandbuf;  commandbuf += sizeof(GLbitfield);

	glPushClientAttrib(*mask);
}


//336
static void EXEC_glBlendColor(byte *commandbuf)
{
	GLclampf *red = (GLclampf*)commandbuf;   commandbuf += sizeof(GLclampf);
	GLclampf *green = (GLclampf*)commandbuf;     commandbuf += sizeof(GLclampf);
	GLclampf *blue = (GLclampf*)commandbuf;  commandbuf += sizeof(GLclampf);
	GLclampf *alpha = (GLclampf*)commandbuf;     commandbuf += sizeof(GLclampf);

	glBlendColor(*red, *green, *blue, *alpha);
}


//337
static void EXEC_glBlendEquation(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glBlendEquation(*mode);
}


//338
static void EXEC_glDrawRangeElements(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLuint *start = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLuint *end = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glDrawRangeElements(*mode, *start, *end, *count, *type, (const GLvoid *)popBuf());
}


//339
static void EXEC_glColorTable(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glColorTable(*target, *internalformat, *width, *format, *type, (const GLvoid *)popBuf());
}


//340
static void EXEC_glColorTableParameterfv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glColorTableParameterfv(*target, *pname, (const GLfloat *)popBuf());
}


//341
static void EXEC_glColorTableParameteriv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glColorTableParameteriv(*target, *pname, (const GLint *)popBuf());
}


//342
static void EXEC_glCopyColorTable(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glCopyColorTable(*target, *internalformat, *x, *y, *width);
}


//343
static void EXEC_glGetColorTable(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glGetColorTable(*target, *format, *type, (GLvoid *)popBuf());
}


//344
static void EXEC_glGetColorTableParameterfv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetColorTableParameterfv(*target, *pname, (GLfloat *)popBuf());
}


//345
static void EXEC_glGetColorTableParameteriv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetColorTableParameteriv(*target, *pname, (GLint *)popBuf());
}


//346
static void EXEC_glColorSubTable(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *start = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glColorSubTable(*target, *start, *count, *format, *type, (const GLvoid *)popBuf());
}


//347
static void EXEC_glCopyColorSubTable(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *start = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glCopyColorSubTable(*target, *start, *x, *y, *width);
}


//348
static void EXEC_glConvolutionFilter1D(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glConvolutionFilter1D(*target, *internalformat, *width, *format, *type, (const GLvoid *)popBuf());
}


//349
static void EXEC_glConvolutionFilter2D(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glConvolutionFilter2D(*target, *internalformat, *width, *height, *format, *type, (const GLvoid *)popBuf());
}


//350
static void EXEC_glConvolutionParameterf(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *params = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	glConvolutionParameterf(*target, *pname, *params);
}


//351
static void EXEC_glConvolutionParameterfv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glConvolutionParameterfv(*target, *pname, (const GLfloat *)popBuf());
}


//352
static void EXEC_glConvolutionParameteri(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *params = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	glConvolutionParameteri(*target, *pname, *params);
}


//353
static void EXEC_glConvolutionParameteriv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glConvolutionParameteriv(*target, *pname, (const GLint *)popBuf());
}


//354
static void EXEC_glCopyConvolutionFilter1D(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glCopyConvolutionFilter1D(*target, *internalformat, *x, *y, *width);
}


//355
static void EXEC_glCopyConvolutionFilter2D(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	glCopyConvolutionFilter2D(*target, *internalformat, *x, *y, *width, *height);
}


//356
static void EXEC_glGetConvolutionFilter(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glGetConvolutionFilter(*target, *format, *type, (GLvoid *)popBuf());
}


//357
static void EXEC_glGetConvolutionParameterfv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetConvolutionParameterfv(*target, *pname, (GLfloat *)popBuf());
}


//358
static void EXEC_glGetConvolutionParameteriv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetConvolutionParameteriv(*target, *pname, (GLint *)popBuf());
}


//359
static void EXEC_glGetSeparableFilter(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glGetSeparableFilter(*target, *format, *type, (GLvoid *)popBuf(), (GLvoid *)popBuf(), (GLvoid *)popBuf());
}


//360
static void EXEC_glSeparableFilter2D(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glSeparableFilter2D(*target, *internalformat, *width, *height, *format, *type, (const GLvoid *)popBuf(), (const GLvoid *)popBuf());
}


//361
static void EXEC_glGetHistogram(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLboolean *reset = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glGetHistogram(*target, *reset, *format, *type, (GLvoid *)popBuf());
}


//362
static void EXEC_glGetHistogramParameterfv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetHistogramParameterfv(*target, *pname, (GLfloat *)popBuf());
}


//363
static void EXEC_glGetHistogramParameteriv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetHistogramParameteriv(*target, *pname, (GLint *)popBuf());
}


//364
static void EXEC_glGetMinmax(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLboolean *reset = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glGetMinmax(*target, *reset, *format, *type, (GLvoid *)popBuf());
}


//365
static void EXEC_glGetMinmaxParameterfv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetMinmaxParameterfv(*target, *pname, (GLfloat *)popBuf());
}


//366
static void EXEC_glGetMinmaxParameteriv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetMinmaxParameteriv(*target, *pname, (GLint *)popBuf());
}


//367
static void EXEC_glHistogram(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLboolean *sink = (GLboolean*)commandbuf;    commandbuf += sizeof(GLboolean);

	glHistogram(*target, *width, *internalformat, *sink);
}


//368
static void EXEC_glMinmax(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLboolean *sink = (GLboolean*)commandbuf;    commandbuf += sizeof(GLboolean);

	glMinmax(*target, *internalformat, *sink);
}


//369
static void EXEC_glResetHistogram(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glResetHistogram(*target);
}


//370
static void EXEC_glResetMinmax(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glResetMinmax(*target);
}


//371
static void EXEC_glTexImage3D(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *internalformat = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLsizei *depth = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLint *border = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glTexImage3D(*target, *level, *internalformat, *width, *height, *depth, *border, *format, *type, (const GLvoid *)popBuf());
}


//372
static void EXEC_glTexSubImage3D(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *xoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *yoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *zoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLsizei *depth = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	//GLuint *UNUSED = (GLuint*)commandbuf;	 commandbuf += sizeof(GLuint);

	glTexSubImage3D(*target, *level, *xoffset, *yoffset, *zoffset, *width, *height, *depth, *format, *type,  (const GLvoid *)popBuf());
}


//373
static void EXEC_glCopyTexSubImage3D(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *xoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *yoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *zoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	glCopyTexSubImage3D(*target, *level, *xoffset, *yoffset, *zoffset, *x, *y, *width, *height);
}


//374
static void EXEC_glActiveTexture(byte *commandbuf)
{
	GLenum *texture = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	glActiveTexture(*texture);
}


//375
static void EXEC_glClientActiveTexture(byte *commandbuf)
{
	GLenum *texture = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	glClientActiveTexture(*texture);
}


//376
static void EXEC_glMultiTexCoord1d(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glMultiTexCoord1d(*target, *s);
}


//377
static void EXEC_glMultiTexCoord1dv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord1dv(*target, (const GLdouble *)popBuf());
}


//378
static void EXEC_glMultiTexCoord1f(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glMultiTexCoord1f(*target, *s);
}


//379
static void EXEC_glMultiTexCoord1fv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord1fv(*target, (const GLfloat *)popBuf());
}


//380
static void EXEC_glMultiTexCoord1i(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glMultiTexCoord1i(*target, *s);
}


//381
static void EXEC_glMultiTexCoord1iv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord1iv(*target, (const GLint *)popBuf());
}


//382
static void EXEC_glMultiTexCoord1s(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glMultiTexCoord1s(*target, *s);
}


//383
static void EXEC_glMultiTexCoord1sv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord1sv(*target, (const GLshort *)popBuf());
}


//384
static void EXEC_glMultiTexCoord2d(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *t = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glMultiTexCoord2d(*target, *s, *t);
}


//385
static void EXEC_glMultiTexCoord2dv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord2dv(*target, (const GLdouble *)popBuf());
}


//386
static void EXEC_glMultiTexCoord2f(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glMultiTexCoord2f(*target, *s, *t);
}


//387
static void EXEC_glMultiTexCoord2fv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord2fv(*target, (const GLfloat *)popBuf());
}


//388
static void EXEC_glMultiTexCoord2i(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *t = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glMultiTexCoord2i(*target, *s, *t);
}


//389
static void EXEC_glMultiTexCoord2iv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord2iv(*target, (const GLint *)popBuf());
}


//390
static void EXEC_glMultiTexCoord2s(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *t = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glMultiTexCoord2s(*target, *s, *t);
}


//391
static void EXEC_glMultiTexCoord2sv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord2sv(*target, (const GLshort *)popBuf());
}


//392
static void EXEC_glMultiTexCoord3d(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *t = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *r = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glMultiTexCoord3d(*target, *s, *t, *r);
}


//393
static void EXEC_glMultiTexCoord3dv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord3dv(*target, (const GLdouble *)popBuf());
}


//394
static void EXEC_glMultiTexCoord3f(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *r = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glMultiTexCoord3f(*target, *s, *t, *r);
}


//395
static void EXEC_glMultiTexCoord3fv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord3fv(*target, (const GLfloat *)popBuf());
}


//396
static void EXEC_glMultiTexCoord3i(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *t = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *r = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glMultiTexCoord3i(*target, *s, *t, *r);
}


//397
static void EXEC_glMultiTexCoord3iv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord3iv(*target, (const GLint *)popBuf());
}


//398
static void EXEC_glMultiTexCoord3s(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *t = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *r = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glMultiTexCoord3s(*target, *s, *t, *r);
}


//399
static void EXEC_glMultiTexCoord3sv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord3sv(*target, (const GLshort *)popBuf());
}


//400
static void EXEC_glMultiTexCoord4d(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *t = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *r = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *q = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glMultiTexCoord4d(*target, *s, *t, *r, *q);
}


//401
static void EXEC_glMultiTexCoord4dv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord4dv(*target, (const GLdouble *)popBuf());
}


//402
static void EXEC_glMultiTexCoord4f(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *r = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *q = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glMultiTexCoord4f(*target, *s, *t, *r, *q);
}


//403
static void EXEC_glMultiTexCoord4fv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord4fv(*target, (const GLfloat *)popBuf());
}


//404
static void EXEC_glMultiTexCoord4i(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *t = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *r = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *q = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glMultiTexCoord4i(*target, *s, *t, *r, *q);
}


//405
static void EXEC_glMultiTexCoord4iv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord4iv(*target, (const GLint *)popBuf());
}


//406
static void EXEC_glMultiTexCoord4s(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *t = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *r = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *q = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glMultiTexCoord4s(*target, *s, *t, *r, *q);
}


//407
static void EXEC_glMultiTexCoord4sv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord4sv(*target, (const GLshort *)popBuf());
}


//408
static void EXEC_glLoadTransposeMatrixf(byte *commandbuf)
{

	glLoadTransposeMatrixf((const GLfloat *)popBuf());
}


//409
static void EXEC_glLoadTransposeMatrixd(byte *commandbuf)
{

	glLoadTransposeMatrixd((const GLdouble *)popBuf());
}


//410
static void EXEC_glMultTransposeMatrixf(byte *commandbuf)
{

	glMultTransposeMatrixf((const GLfloat *)popBuf());
}


//411
static void EXEC_glMultTransposeMatrixd(byte *commandbuf)
{

	glMultTransposeMatrixd((const GLdouble *)popBuf());
}


//412
static void EXEC_glSampleCoverage(byte *commandbuf)
{
	GLclampf *value = (GLclampf*)commandbuf;     commandbuf += sizeof(GLclampf);
	GLboolean *invert = (GLboolean*)commandbuf;  commandbuf += sizeof(GLboolean);

	glSampleCoverage(*value, *invert);
}


//413
static void EXEC_glCompressedTexImage3D(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLsizei *depth = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLint *border = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLsizei *imageSize = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glCompressedTexImage3D(*target, *level, *internalformat, *width, *height, *depth, *border, *imageSize, (const GLvoid *)popBuf());
}


//414
static void EXEC_glCompressedTexImage2D(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLint *border = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLsizei *imageSize = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glCompressedTexImage2D(*target, *level, *internalformat, *width, *height, *border, *imageSize, (const GLvoid *)popBuf());
}


//415
static void EXEC_glCompressedTexImage1D(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLint *border = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLsizei *imageSize = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glCompressedTexImage1D(*target, *level, *internalformat, *width, *border, *imageSize, (const GLvoid *)popBuf());
}


//416
static void EXEC_glCompressedTexSubImage3D(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *xoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *yoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *zoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLsizei *depth = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *imageSize = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glCompressedTexSubImage3D(*target, *level, *xoffset, *yoffset, *zoffset, *width, *height, *depth, *format, *imageSize, (const GLvoid *)popBuf());
}


//417
static void EXEC_glCompressedTexSubImage2D(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *xoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *yoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *imageSize = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glCompressedTexSubImage2D(*target, *level, *xoffset, *yoffset, *width, *height, *format, *imageSize, (const GLvoid *)popBuf());
}


//418
static void EXEC_glCompressedTexSubImage1D(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *xoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *imageSize = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glCompressedTexSubImage1D(*target, *level, *xoffset, *width, *format, *imageSize, (const GLvoid *)popBuf());
}


//419
static void EXEC_glGetCompressedTexImage(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glGetCompressedTexImage(*target, *level, (GLvoid *)popBuf());
}


//420
static void EXEC_glBlendFuncSeparate(byte *commandbuf)
{
	GLenum *sfactorRGB = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *dfactorRGB = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *sfactorAlpha = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *dfactorAlpha = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glBlendFuncSeparate(*sfactorRGB, *dfactorRGB, *sfactorAlpha, *dfactorAlpha);
}


//421
static void EXEC_glFogCoordf(byte *commandbuf)
{
	GLfloat *coord = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glFogCoordf(*coord);
}


//422
static void EXEC_glFogCoordfv(byte *commandbuf)
{
	glFogCoordfv((const GLfloat *)popBuf());
}


//423
static void EXEC_glFogCoordd(byte *commandbuf)
{
	GLdouble *coord = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glFogCoordd(*coord);
}


//424
static void EXEC_glFogCoorddv(byte *commandbuf)
{

	glFogCoorddv((const GLdouble *)popBuf());
}


//425
static void EXEC_glFogCoordPointer(byte *commandbuf)
{
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	glFogCoordPointer(*type, *stride, (const GLvoid *)popBuf());
}


//426
static void EXEC_glMultiDrawArrays(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *primcount = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glMultiDrawArrays(*mode, (GLint *)popBuf(), (GLsizei *)popBuf(), *primcount);
}


//427
static void EXEC_glMultiDrawElements(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *primcount = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glMultiDrawElements(*mode, (GLsizei *)popBuf(), *type, (const GLvoid **)popBuf(), *primcount);
}


//428
static void EXEC_glPointParameterf(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glPointParameterf(*pname, *param);
}


//429
static void EXEC_glPointParameterfv(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glPointParameterfv(*pname, (GLfloat *)popBuf());
}


//430
static void EXEC_glPointParameteri(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	//TODO: why does this exist?
	#ifndef SYMPHONY
	glPointParameteri(*pname, *param);
	#endif
}


//431
static void EXEC_glPointParameteriv(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	//TODO: why does this exist?
	#ifndef SYMPHONY
	glPointParameteriv(*pname, (GLint *)popBuf());
	#endif
}


//432
static void EXEC_glSecondaryColor3b(byte *commandbuf)
{
	GLbyte *red = (GLbyte*)commandbuf;   commandbuf += sizeof(GLbyte);
	GLbyte *green = (GLbyte*)commandbuf;     commandbuf += sizeof(GLbyte);
	GLbyte *blue = (GLbyte*)commandbuf;  commandbuf += sizeof(GLbyte);

	glSecondaryColor3b(*red, *green, *blue);
}


//433
static void EXEC_glSecondaryColor3bv(byte *commandbuf)
{

	glSecondaryColor3bv((const GLbyte *)popBuf());
}


//434
static void EXEC_glSecondaryColor3d(byte *commandbuf)
{
	GLdouble *red = (GLdouble*)commandbuf;   commandbuf += sizeof(GLdouble);
	GLdouble *green = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *blue = (GLdouble*)commandbuf;  commandbuf += sizeof(GLdouble);

	glSecondaryColor3d(*red, *green, *blue);
}


//435
static void EXEC_glSecondaryColor3dv(byte *commandbuf)
{

	glSecondaryColor3dv((const GLdouble *)popBuf());
}


//436
static void EXEC_glSecondaryColor3f(byte *commandbuf)
{
	GLfloat *red = (GLfloat*)commandbuf;     commandbuf += sizeof(GLfloat);
	GLfloat *green = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *blue = (GLfloat*)commandbuf;    commandbuf += sizeof(GLfloat);

	glSecondaryColor3f(*red, *green, *blue);
}


//437
static void EXEC_glSecondaryColor3fv(byte *commandbuf)
{

	glSecondaryColor3fv((const GLfloat *)popBuf());
}


//438
static void EXEC_glSecondaryColor3i(byte *commandbuf)
{
	GLint *red = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *green = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *blue = (GLint*)commandbuf;    commandbuf += sizeof(GLint);

	glSecondaryColor3i(*red, *green, *blue);
}


//439
static void EXEC_glSecondaryColor3iv(byte *commandbuf)
{

	glSecondaryColor3iv((const GLint *)popBuf());
}


//440
static void EXEC_glSecondaryColor3s(byte *commandbuf)
{
	GLshort *red = (GLshort*)commandbuf;     commandbuf += sizeof(GLshort);
	GLshort *green = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *blue = (GLshort*)commandbuf;    commandbuf += sizeof(GLshort);

	glSecondaryColor3s(*red, *green, *blue);
}


//441
static void EXEC_glSecondaryColor3sv(byte *commandbuf)
{

	glSecondaryColor3sv((const GLshort *)popBuf());
}


//442
static void EXEC_glSecondaryColor3ub(byte *commandbuf)
{
	GLubyte *red = (GLubyte*)commandbuf;     commandbuf += sizeof(GLubyte);
	GLubyte *green = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *blue = (GLubyte*)commandbuf;    commandbuf += sizeof(GLubyte);

	glSecondaryColor3ub(*red, *green, *blue);
}


//443
static void EXEC_glSecondaryColor3ubv(byte *commandbuf)
{

	glSecondaryColor3ubv((const GLubyte *)popBuf());
}


//444
static void EXEC_glSecondaryColor3ui(byte *commandbuf)
{
	GLuint *red = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *green = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLuint *blue = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	glSecondaryColor3ui(*red, *green, *blue);
}


//445
static void EXEC_glSecondaryColor3uiv(byte *commandbuf)
{

	glSecondaryColor3uiv((const GLuint *)popBuf());
}


//446
static void EXEC_glSecondaryColor3us(byte *commandbuf)
{
	GLushort *red = (GLushort*)commandbuf;   commandbuf += sizeof(GLushort);
	GLushort *green = (GLushort*)commandbuf;     commandbuf += sizeof(GLushort);
	GLushort *blue = (GLushort*)commandbuf;  commandbuf += sizeof(GLushort);

	glSecondaryColor3us(*red, *green, *blue);
}


//447
static void EXEC_glSecondaryColor3usv(byte *commandbuf)
{

	glSecondaryColor3usv((const GLushort *)popBuf());
}


//448
static void EXEC_glSecondaryColorPointer(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	glSecondaryColorPointer(*size, *type, *stride, (GLvoid *)popBuf());
}


//449
static void EXEC_glWindowPos2d(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glWindowPos2d(*x, *y);
}


//450
static void EXEC_glWindowPos2dv(byte *commandbuf)
{

	glWindowPos2dv((const GLdouble *)popBuf());
}


//451
static void EXEC_glWindowPos2f(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glWindowPos2f(*x, *y);
}


//452
static void EXEC_glWindowPos2fv(byte *commandbuf)
{

	glWindowPos2fv((const GLfloat *)popBuf());
}


//453
static void EXEC_glWindowPos2i(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glWindowPos2i(*x, *y);
}


//454
static void EXEC_glWindowPos2iv(byte *commandbuf)
{

	glWindowPos2iv((const GLint *)popBuf());
}


//455
static void EXEC_glWindowPos2s(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glWindowPos2s(*x, *y);
}


//456
static void EXEC_glWindowPos2sv(byte *commandbuf)
{

	glWindowPos2sv((const GLshort *)popBuf());
}


//457
static void EXEC_glWindowPos3d(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glWindowPos3d(*x, *y, *z);
}


//458
static void EXEC_glWindowPos3dv(byte *commandbuf)
{

	glWindowPos3dv((const GLdouble *)popBuf());
}


//459
static void EXEC_glWindowPos3f(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glWindowPos3f(*x, *y, *z);
}


//460
static void EXEC_glWindowPos3fv(byte *commandbuf)
{

	glWindowPos3fv((const GLfloat *)popBuf());
}


//461
static void EXEC_glWindowPos3i(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *z = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glWindowPos3i(*x, *y, *z);
}


//462
static void EXEC_glWindowPos3iv(byte *commandbuf)
{

	glWindowPos3iv((const GLint *)popBuf());
}


//463
static void EXEC_glWindowPos3s(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glWindowPos3s(*x, *y, *z);
}


//464
static void EXEC_glWindowPos3sv(byte *commandbuf)
{

	glWindowPos3sv((const GLshort *)popBuf());
}


//465
static void EXEC_glBindBuffer(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *buffer = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	glBindBuffer(*target, *buffer);
}


//466
static void EXEC_glBufferData(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizeiptr *size = (GLsizeiptr*)commandbuf;  commandbuf += sizeof(GLsizeiptr);
	GLenum *usage = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glBufferData(*target, *size, (const GLvoid *)popBuf(), *usage);
}


//467
static void EXEC_glBufferSubData(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLintptr *offset = (GLintptr*)commandbuf;    commandbuf += sizeof(GLintptr);
	GLsizeiptr *size = (GLsizeiptr*)commandbuf;  commandbuf += sizeof(GLsizeiptr);

	glBufferSubData(*target, *offset, *size, (const GLvoid *)popBuf());
}


//468
static void EXEC_glDeleteBuffers(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glDeleteBuffers(*n, (const GLuint *)popBuf());
}


//469
static void EXEC_glGenBuffers(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glGenBuffers(*n, (GLuint *)popBuf());
}


//470
static void EXEC_glGetBufferParameteriv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetBufferParameteriv(*target, *pname, (GLint *)popBuf());
}


//471
static void EXEC_glGetBufferPointerv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetBufferPointerv(*target, *pname, (GLvoid **)popBuf());
}


//472
static void EXEC_glGetBufferSubData(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLintptr *offset = (GLintptr*)commandbuf;    commandbuf += sizeof(GLintptr);
	GLsizeiptr *size = (GLsizeiptr*)commandbuf;  commandbuf += sizeof(GLsizeiptr);

	glGetBufferSubData(*target, *offset, *size, (GLvoid *)popBuf());
}


//473
static void EXEC_glIsBuffer(byte *commandbuf)
{
	GLuint *buffer = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	pushRet(glIsBuffer(*buffer));
}


//474
static void EXEC_glMapBuffer(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *access = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	pushRet((const GLubyte*)glMapBuffer(*target, *access));

}


//475
static void EXEC_glUnmapBuffer(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	pushRet(glUnmapBuffer(*target));
}


//476
static void EXEC_glGenQueries(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glGenQueries(*n, (GLuint *)popBuf());
}


//477
static void EXEC_glDeleteQueries(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glDeleteQueries(*n, (const GLuint *)popBuf());
}


//478
static void EXEC_glIsQuery(byte *commandbuf)
{
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	pushRet(glIsQuery(*id));
}


//479
static void EXEC_glBeginQuery(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	glBeginQuery(*target, *id);
}


//480
static void EXEC_glEndQuery(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glEndQuery(*target);
}


//481
static void EXEC_glGetQueryiv(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetQueryiv(*target, *pname, (GLint *)popBuf());
}


//482
static void EXEC_glGetQueryObjectiv(byte *commandbuf)
{
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetQueryObjectiv(*id, *pname, (GLint *)popBuf());
}


//483
static void EXEC_glGetQueryObjectuiv(byte *commandbuf)
{
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetQueryObjectuiv(*id, *pname, (GLuint *)popBuf());
}


//484
static void EXEC_glBlendEquationSeparate(byte *commandbuf)
{
	GLenum *modeRGB = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLenum *modeA = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glBlendEquationSeparate(*modeRGB, *modeA);
}


//485
static void EXEC_glDrawBuffers(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glDrawBuffers(*n, (const GLenum *)popBuf());
}


//486
static void EXEC_glStencilFuncSeparate(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *func = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLint *ref = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLuint *mask = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	glStencilFuncSeparate(*face, *func, *ref, *mask);
}


//487
static void EXEC_glStencilOpSeparate(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *sfail = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *zfail = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *zpass = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glStencilOpSeparate(*face, *sfail, *zfail, *zpass);
}


//488
static void EXEC_glStencilMaskSeparate(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLuint *mask = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	glStencilMaskSeparate(*face, *mask);
}


//489
static void EXEC_glAttachShader(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *shader = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	glAttachShader(*program, *shader);
}


//490
static void EXEC_glBindAttribLocation(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glBindAttribLocation(*program, *index, (const GLchar *)popBuf());
}


//491
static void EXEC_glCompileShader(byte *commandbuf)
{
	GLuint *shader = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	glCompileShader(*shader);
}


//492
static void EXEC_glCreateProgram(byte *commandbuf)
{

	pushRet(glCreateProgram());
}


//493
static void EXEC_glCreateShader(byte *commandbuf)
{
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	pushRet(glCreateShader(*type));
}


//494
static void EXEC_glDeleteProgram(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	glDeleteProgram(*program);
}


//495
static void EXEC_glDeleteShader(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	glDeleteShader(*program);
}


//496
static void EXEC_glDetachShader(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *shader = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	glDetachShader(*program, *shader);
}


//497
static void EXEC_glDisableVertexAttribArray(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glDisableVertexAttribArray(*index);
}


//498
static void EXEC_glEnableVertexAttribArray(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glEnableVertexAttribArray(*index);
}


//499
static void EXEC_glGetActiveAttrib(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLsizei  *bufSize = (GLsizei *)commandbuf;   commandbuf += sizeof(GLsizei );

	glGetActiveAttrib(*program, *index, *bufSize, (GLsizei *)popBuf(), (GLint *)popBuf(), (GLenum *)popBuf(), (GLchar *)popBuf());
}


//500
static void EXEC_glGetActiveUniform(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLsizei *bufSize = (GLsizei*)commandbuf;     commandbuf += sizeof(GLsizei);

	glGetActiveUniform(*program, *index, *bufSize, (GLsizei *)popBuf(), (GLint *)popBuf(), (GLenum *)popBuf(), (GLchar *)popBuf());
}


//501
static void EXEC_glGetAttachedShaders(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLsizei *maxCount = (GLsizei*)commandbuf;    commandbuf += sizeof(GLsizei);

	glGetAttachedShaders(*program, *maxCount, (GLsizei *)popBuf(), (GLuint *)popBuf());
}


//502
static void EXEC_glGetAttribLocation(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	pushRet(glGetAttribLocation(*program, (const GLchar *)popBuf()));
}


//503
static void EXEC_glGetProgramiv(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetProgramiv(*program, *pname, (GLint *)popBuf());
}


//504
static void EXEC_glGetProgramInfoLog(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLsizei *bufSize = (GLsizei*)commandbuf;     commandbuf += sizeof(GLsizei);

	glGetProgramInfoLog(*program, *bufSize, (GLsizei *)popBuf(), (GLchar *)popBuf());
}


//505
static void EXEC_glGetShaderiv(byte *commandbuf)
{
	GLuint *shader = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint   *params = (GLint *)popBuf();
	glGetShaderiv(*shader, *pname, params);
	pushRet(*params);
}


//506
static void EXEC_glGetShaderInfoLog(byte *commandbuf)
{
	GLuint  *shader  = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLsizei *bufSize = (GLsizei*)commandbuf; commandbuf += sizeof(GLsizei);
	GLint   *length  = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLchar  *infolog = (GLchar *)popBuf();
	if(*length == -1) {
		length = NULL;
	}

	glGetShaderInfoLog(*shader, *bufSize, length, infolog);
	pushRet(infolog);
}


//507
static void EXEC_glGetShaderSource(byte *commandbuf)
{
	GLuint *shader = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLsizei *bufSize = (GLsizei*)commandbuf; commandbuf += sizeof(GLsizei);
	GLint   *length  = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLchar  *source = (GLchar *)popBuf();
	if(*length == -1) {
		length = NULL;
	}

	glGetShaderSource(*shader, *bufSize, length, source);
}


//508
static void EXEC_glGetUniformLocation(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	pushRet(glGetUniformLocation(*program, (const GLchar *)popBuf()));
}


//509
static void EXEC_glGetUniformfv(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);

	glGetUniformfv(*program, *location, (GLfloat *)popBuf());
}


//510
static void EXEC_glGetUniformiv(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);

	glGetUniformiv(*program, *location, (GLint *)popBuf());
}


//511
static void EXEC_glGetVertexAttribdv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetVertexAttribdv(*index, *pname, (GLdouble *)popBuf());
}


//512
static void EXEC_glGetVertexAttribfv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetVertexAttribfv(*index, *pname, (GLfloat *)popBuf());
}


//513
static void EXEC_glGetVertexAttribiv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetVertexAttribiv(*index, *pname, (GLint *)popBuf());
}


//514
static void EXEC_glGetVertexAttribPointerv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetVertexAttribPointerv(*index, *pname, (GLvoid **)popBuf());
}


//515
static void EXEC_glIsProgram(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	pushRet(glIsProgram(*program));
}


//516
static void EXEC_glIsShader(byte *commandbuf)
{
	GLuint *shader = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	pushRet(glIsShader(*shader));
}


//517
static void EXEC_glLinkProgram(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	glLinkProgram(*program);
}


//518
static void EXEC_glShaderSource(byte *commandbuf)
{
	GLuint *shader = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	const GLchar * string = (const GLchar *)popBuf();
	GLint   *length = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	if(*length == -1) {
		length = NULL;
	}

	glShaderSource(*shader, *count, &string, length);
}


//519
static void EXEC_glUseProgram(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	glUseProgram(*program);
}


//520
static void EXEC_glUniform1f(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLfloat *v0 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	glUniform1f(*location, *v0);
}


//521
static void EXEC_glUniform2f(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLfloat *v0 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	glUniform2f(*location, *v0, *v1);
}


//522
static void EXEC_glUniform3f(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLfloat *v0 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v2 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	glUniform3f(*location, *v0, *v1, *v2);
}


//523
static void EXEC_glUniform4f(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLfloat *v0 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v2 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v3 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	glUniform4f(*location, *v0, *v1, *v2, *v3);
}


//524
static void EXEC_glUniform1i(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLint *v0 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	glUniform1i(*location, *v0);
}


//525
static void EXEC_glUniform2i(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLint *v0 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v1 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	glUniform2i(*location, *v0, *v1);
}


//526
static void EXEC_glUniform3i(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLint *v0 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v1 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v2 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	glUniform3i(*location, *v0, *v1, *v2);
}


//527
static void EXEC_glUniform4i(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLint *v0 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v1 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v2 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v3 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	glUniform4i(*location, *v0, *v1, *v2, *v3);
}


//528
static void EXEC_glUniform1fv(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glUniform1fv(*location, *count, (const GLfloat *)popBuf());
}


//529
static void EXEC_glUniform2fv(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glUniform2fv(*location, *count, (const GLfloat *)popBuf());
}


//530
static void EXEC_glUniform3fv(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glUniform3fv(*location, *count, (const GLfloat *)popBuf());
}


//531
static void EXEC_glUniform4fv(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glUniform4fv(*location, *count, (const GLfloat *)popBuf());
}


//532
static void EXEC_glUniform1iv(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glUniform1iv(*location, *count, (const GLint *)popBuf());
}


//533
static void EXEC_glUniform2iv(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glUniform2iv(*location, *count, (const GLint *)popBuf());
}


//534
static void EXEC_glUniform3iv(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glUniform3iv(*location, *count, (const GLint *)popBuf());
}


//535
static void EXEC_glUniform4iv(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glUniform4iv(*location, *count, (const GLint *)popBuf());
}


//536
static void EXEC_glUniformMatrix2fv(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLboolean *transpose = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);

	glUniformMatrix2fv(*location, *count, *transpose, (const GLfloat *)popBuf());
}


//537
static void EXEC_glUniformMatrix3fv(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLboolean *transpose = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);

	glUniformMatrix3fv(*location, *count, *transpose, (const GLfloat *)popBuf());
}


//538
static void EXEC_glUniformMatrix4fv(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLboolean *transpose = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);

	glUniformMatrix4fv(*location, *count, *transpose, (const GLfloat *)popBuf());
}


//539
static void EXEC_glValidateProgram(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	glValidateProgram(*program);
}


//540
static void EXEC_glVertexAttrib1d(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glVertexAttrib1d(*index, *x);
}


//541
static void EXEC_glVertexAttrib1dv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib1dv(*index, (const GLdouble *)popBuf());
}


//542
static void EXEC_glVertexAttrib1f(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glVertexAttrib1f(*index, *x);
}


//543
static void EXEC_glVertexAttrib1fv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib1fv(*index, (const GLfloat *)popBuf());
}


//544
static void EXEC_glVertexAttrib1s(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glVertexAttrib1s(*index, *x);
}


//545
static void EXEC_glVertexAttrib1sv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib1sv(*index, (const GLshort *)popBuf());
}


//546
static void EXEC_glVertexAttrib2d(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glVertexAttrib2d(*index, *x, *y);
}


//547
static void EXEC_glVertexAttrib2dv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib2dv(*index, (const GLdouble *)popBuf());
}


//548
static void EXEC_glVertexAttrib2f(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glVertexAttrib2f(*index, *x, *y);
}


//549
static void EXEC_glVertexAttrib2fv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib2fv(*index, (const GLfloat *)popBuf());
}


//550
static void EXEC_glVertexAttrib2s(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glVertexAttrib2s(*index, *x, *y);
}


//551
static void EXEC_glVertexAttrib2sv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib2sv(*index, (const GLshort *)popBuf());
}


//552
static void EXEC_glVertexAttrib3d(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glVertexAttrib3d(*index, *x, *y, *z);
}


//553
static void EXEC_glVertexAttrib3dv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib3dv(*index, (const GLdouble *)popBuf());
}


//554
static void EXEC_glVertexAttrib3f(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glVertexAttrib3f(*index, *x, *y, *z);
}


//555
static void EXEC_glVertexAttrib3fv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib3fv(*index, (const GLfloat *)popBuf());
}


//556
static void EXEC_glVertexAttrib3s(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glVertexAttrib3s(*index, *x, *y, *z);
}


//557
static void EXEC_glVertexAttrib3sv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib3sv(*index, (const GLshort *)popBuf());
}


//558
static void EXEC_glVertexAttrib4Nbv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4Nbv(*index, (const GLbyte *)popBuf());
}


//559
static void EXEC_glVertexAttrib4Niv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4Niv(*index, (const GLint *)popBuf());
}


//560
static void EXEC_glVertexAttrib4Nsv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4Nsv(*index, (const GLshort *)popBuf());
}


//561
static void EXEC_glVertexAttrib4Nub(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLubyte *x = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *y = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *z = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *w = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);

	glVertexAttrib4Nub(*index, *x, *y, *z, *w);
}


//562
static void EXEC_glVertexAttrib4Nubv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4Nubv(*index, (const GLubyte *)popBuf());
}


//563
static void EXEC_glVertexAttrib4Nuiv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4Nuiv(*index, (const GLuint *)popBuf());
}


//564
static void EXEC_glVertexAttrib4Nusv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4Nusv(*index, (const GLushort *)popBuf());
}


//565
static void EXEC_glVertexAttrib4bv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4bv(*index, (const GLbyte *)popBuf());
}


//566
static void EXEC_glVertexAttrib4d(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *w = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glVertexAttrib4d(*index, *x, *y, *z, *w);
}


//567
static void EXEC_glVertexAttrib4dv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4dv(*index, (const GLdouble *)popBuf());
}


//568
static void EXEC_glVertexAttrib4f(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *w = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glVertexAttrib4f(*index, *x, *y, *z, *w);
}


//569
static void EXEC_glVertexAttrib4fv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4fv(*index, (const GLfloat *)popBuf());
}


//570
static void EXEC_glVertexAttrib4iv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4iv(*index, (const GLint *)popBuf());
}


//571
static void EXEC_glVertexAttrib4s(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *w = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glVertexAttrib4s(*index, *x, *y, *z, *w);
}


//572
static void EXEC_glVertexAttrib4sv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4sv(*index, (const GLshort *)popBuf());
}


//573
static void EXEC_glVertexAttrib4ubv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4ubv(*index, (const GLubyte *)popBuf());
}


//574
static void EXEC_glVertexAttrib4uiv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4uiv(*index, (const GLuint *)popBuf());
}


//575
static void EXEC_glVertexAttrib4usv(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4usv(*index, (const GLushort *)popBuf());
}


//576
static void EXEC_glVertexAttribPointer(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLboolean *normalized = (GLboolean*)commandbuf;  commandbuf += sizeof(GLboolean);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	glVertexAttribPointer(*index, *size, *type, *normalized, *stride, (const GLvoid *)popBuf());
}


//577
static void EXEC_glUniformMatrix2x3fv(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLboolean *transpose = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);

	//TODO: why does this exist?
	#ifndef SYMPHONY
	glUniformMatrix2x3fv(*location, *count, *transpose, (const GLfloat *)popBuf());
	#endif
}


//578
static void EXEC_glUniformMatrix3x2fv(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLboolean *transpose = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);

	#ifndef SYMPHONY
	glUniformMatrix3x2fv(*location, *count, *transpose, (const GLfloat *)popBuf());
	#endif
}


//579
static void EXEC_glUniformMatrix2x4fv(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLboolean *transpose = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);

	#ifndef SYMPHONY
	glUniformMatrix2x4fv(*location, *count, *transpose, (const GLfloat *)popBuf());
	#endif
}


//580
static void EXEC_glUniformMatrix4x2fv(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLboolean *transpose = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);

	#ifndef SYMPHONY
	glUniformMatrix4x2fv(*location, *count, *transpose, (const GLfloat *)popBuf());
	#endif
}


//581
static void EXEC_glUniformMatrix3x4fv(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLboolean *transpose = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);

	#ifndef SYMPHONY
	glUniformMatrix3x4fv(*location, *count, *transpose, (const GLfloat *)popBuf());
	#endif
}


//582
static void EXEC_glUniformMatrix4x3fv(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLboolean *transpose = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);

	#ifndef SYMPHONY
	glUniformMatrix4x3fv(*location, *count, *transpose, (const GLfloat *)popBuf());
	#endif
}


//374
static void EXEC_glActiveTextureARB(byte *commandbuf)
{
	GLenum *texture = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	glActiveTextureARB(*texture);
}


//375
static void EXEC_glClientActiveTextureARB(byte *commandbuf)
{
	GLenum *texture = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	glClientActiveTextureARB(*texture);
}


//376
static void EXEC_glMultiTexCoord1dARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glMultiTexCoord1dARB(*target, *s);
}


//377
static void EXEC_glMultiTexCoord1dvARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord1dvARB(*target, (const GLdouble *)popBuf());
}


//378
static void EXEC_glMultiTexCoord1fARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glMultiTexCoord1fARB(*target, *s);
}


//379
static void EXEC_glMultiTexCoord1fvARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord1fvARB(*target, (const GLfloat *)popBuf());
}


//380
static void EXEC_glMultiTexCoord1iARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glMultiTexCoord1iARB(*target, *s);
}


//381
static void EXEC_glMultiTexCoord1ivARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord1ivARB(*target, (const GLint *)popBuf());
}


//382
static void EXEC_glMultiTexCoord1sARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glMultiTexCoord1sARB(*target, *s);
}


//383
static void EXEC_glMultiTexCoord1svARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord1svARB(*target, (const GLshort *)popBuf());
}


//384
static void EXEC_glMultiTexCoord2dARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *t = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glMultiTexCoord2dARB(*target, *s, *t);
}


//385
static void EXEC_glMultiTexCoord2dvARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord2dvARB(*target, (const GLdouble *)popBuf());
}


//386
static void EXEC_glMultiTexCoord2fARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glMultiTexCoord2fARB(*target, *s, *t);
}


//387
static void EXEC_glMultiTexCoord2fvARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord2fvARB(*target, (const GLfloat *)popBuf());
}


//388
static void EXEC_glMultiTexCoord2iARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *t = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glMultiTexCoord2iARB(*target, *s, *t);
}


//389
static void EXEC_glMultiTexCoord2ivARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord2ivARB(*target, (const GLint *)popBuf());
}


//390
static void EXEC_glMultiTexCoord2sARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *t = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glMultiTexCoord2sARB(*target, *s, *t);
}


//391
static void EXEC_glMultiTexCoord2svARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord2svARB(*target, (const GLshort *)popBuf());
}


//392
static void EXEC_glMultiTexCoord3dARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *t = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *r = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glMultiTexCoord3dARB(*target, *s, *t, *r);
}


//393
static void EXEC_glMultiTexCoord3dvARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord3dvARB(*target, (const GLdouble *)popBuf());
}


//394
static void EXEC_glMultiTexCoord3fARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *r = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glMultiTexCoord3fARB(*target, *s, *t, *r);
}


//395
static void EXEC_glMultiTexCoord3fvARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord3fvARB(*target, (const GLfloat *)popBuf());
}


//396
static void EXEC_glMultiTexCoord3iARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *t = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *r = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glMultiTexCoord3iARB(*target, *s, *t, *r);
}


//397
static void EXEC_glMultiTexCoord3ivARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord3ivARB(*target, (const GLint *)popBuf());
}


//398
static void EXEC_glMultiTexCoord3sARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *t = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *r = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glMultiTexCoord3sARB(*target, *s, *t, *r);
}


//399
static void EXEC_glMultiTexCoord3svARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord3svARB(*target, (const GLshort *)popBuf());
}


//400
static void EXEC_glMultiTexCoord4dARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *t = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *r = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *q = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glMultiTexCoord4dARB(*target, *s, *t, *r, *q);
}


//401
static void EXEC_glMultiTexCoord4dvARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord4dvARB(*target, (const GLdouble *)popBuf());
}


//402
static void EXEC_glMultiTexCoord4fARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *r = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *q = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glMultiTexCoord4fARB(*target, *s, *t, *r, *q);
}


//403
static void EXEC_glMultiTexCoord4fvARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord4fvARB(*target, (const GLfloat *)popBuf());
}


//404
static void EXEC_glMultiTexCoord4iARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *t = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *r = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *q = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glMultiTexCoord4iARB(*target, *s, *t, *r, *q);
}


//405
static void EXEC_glMultiTexCoord4ivARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord4ivARB(*target, (const GLint *)popBuf());
}


//406
static void EXEC_glMultiTexCoord4sARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *t = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *r = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *q = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glMultiTexCoord4sARB(*target, *s, *t, *r, *q);
}


//407
static void EXEC_glMultiTexCoord4svARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glMultiTexCoord4svARB(*target, (const GLshort *)popBuf());
}


//617
static void EXEC_glLoadTransposeMatrixfARB(byte *commandbuf)
{

	glLoadTransposeMatrixfARB((GLfloat *)popBuf());
}


//618
static void EXEC_glLoadTransposeMatrixdARB(byte *commandbuf)
{

	glLoadTransposeMatrixdARB((GLdouble *)popBuf());
}


//619
static void EXEC_glMultTransposeMatrixfARB(byte *commandbuf)
{

	glMultTransposeMatrixfARB((GLfloat *)popBuf());
}


//620
static void EXEC_glMultTransposeMatrixdARB(byte *commandbuf)
{

	glMultTransposeMatrixdARB((GLdouble *)popBuf());
}


//621
static void EXEC_glSampleCoverageARB(byte *commandbuf)
{
	GLclampf *value = (GLclampf*)commandbuf;     commandbuf += sizeof(GLclampf);
	GLboolean *invert = (GLboolean*)commandbuf;  commandbuf += sizeof(GLboolean);

	glSampleCoverageARB(*value, *invert);
}


//622
static void EXEC_glCompressedTexImage3DARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLsizei *depth = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLint *border = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLsizei *imageSize = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glCompressedTexImage3DARB(*target, *level, *internalformat, *width, *height, *depth, *border, *imageSize, (const GLvoid *)popBuf());
}


//623
static void EXEC_glCompressedTexImage2DARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLint *border = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLsizei *imageSize = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glCompressedTexImage2DARB(*target, *level, *internalformat, *width, *height, *border, *imageSize, (const GLvoid *)popBuf());
}


//624
static void EXEC_glCompressedTexImage1DARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLint *border = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLsizei *imageSize = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glCompressedTexImage1DARB(*target, *level, *internalformat, *width, *border, *imageSize, (const GLvoid *)popBuf());
}


//625
static void EXEC_glCompressedTexSubImage3DARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *xoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *yoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *zoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLsizei *depth = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *imageSize = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glCompressedTexSubImage3DARB(*target, *level, *xoffset, *yoffset, *zoffset, *width, *height, *depth, *format, *imageSize, (const GLvoid *)popBuf());
}


//626
static void EXEC_glCompressedTexSubImage2DARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *xoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *yoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *imageSize = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glCompressedTexSubImage2DARB(*target, *level, *xoffset, *yoffset, *width, *height, *format, *imageSize, (const GLvoid *)popBuf());
}


//627
static void EXEC_glCompressedTexSubImage1DARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *xoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *imageSize = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glCompressedTexSubImage1DARB(*target, *level, *xoffset, *width, *format, *imageSize, (const GLvoid *)popBuf());
}


//628
static void EXEC_glGetCompressedTexImageARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glGetCompressedTexImageARB(*target, *level, (GLvoid *)popBuf());
}


//629
static void EXEC_glPointParameterfARB(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glPointParameterfARB(*pname, *param);
}


//630
static void EXEC_glPointParameterfvARB(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glPointParameterfvARB(*pname, (GLfloat *)popBuf());
}


//631
static void EXEC_glWeightbvARB(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);

	glWeightbvARB(*size, (GLbyte *)popBuf());
}


//632
static void EXEC_glWeightsvARB(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);

	glWeightsvARB(*size, (GLshort *)popBuf());
}


//633
static void EXEC_glWeightivARB(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);

	glWeightivARB(*size, (GLint *)popBuf());
}


//634
static void EXEC_glWeightfvARB(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);

	glWeightfvARB(*size, (GLfloat *)popBuf());
}


//635
static void EXEC_glWeightdvARB(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);

	glWeightdvARB(*size, (GLdouble *)popBuf());
}


//636
static void EXEC_glWeightubvARB(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);

	glWeightubvARB(*size, (GLubyte *)popBuf());
}


//637
static void EXEC_glWeightusvARB(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);

	glWeightusvARB(*size, (GLushort *)popBuf());
}


//638
static void EXEC_glWeightuivARB(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);

	glWeightuivARB(*size, (GLuint *)popBuf());
}


//639
static void EXEC_glWeightPointerARB(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	glWeightPointerARB(*size, *type, *stride, (GLvoid *)popBuf());
}


//640
static void EXEC_glVertexBlendARB(byte *commandbuf)
{
	GLint *count = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glVertexBlendARB(*count);
}


//641
static void EXEC_glCurrentPaletteMatrixARB(byte *commandbuf)
{
	GLint *index = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glCurrentPaletteMatrixARB(*index);
}


//642
static void EXEC_glMatrixIndexubvARB(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);

	glMatrixIndexubvARB(*size, (GLubyte *)popBuf());
}


//643
static void EXEC_glMatrixIndexusvARB(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);

	glMatrixIndexusvARB(*size, (GLushort *)popBuf());
}


//644
static void EXEC_glMatrixIndexuivARB(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);

	glMatrixIndexuivARB(*size, (GLuint *)popBuf());
}


//645
static void EXEC_glMatrixIndexPointerARB(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	glMatrixIndexPointerARB(*size, *type, *stride, (GLvoid *)popBuf());
}


//646
static void EXEC_glWindowPos2dARB(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glWindowPos2dARB(*x, *y);
}


//647
static void EXEC_glWindowPos2fARB(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glWindowPos2fARB(*x, *y);
}


//648
static void EXEC_glWindowPos2iARB(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glWindowPos2iARB(*x, *y);
}


//649
static void EXEC_glWindowPos2sARB(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glWindowPos2sARB(*x, *y);
}


//650
static void EXEC_glWindowPos2dvARB(byte *commandbuf)
{

	glWindowPos2dvARB((const GLdouble *)popBuf());
}


//651
static void EXEC_glWindowPos2fvARB(byte *commandbuf)
{

	glWindowPos2fvARB((const GLfloat *)popBuf());
}


//652
static void EXEC_glWindowPos2ivARB(byte *commandbuf)
{

	glWindowPos2ivARB((const GLint *)popBuf());
}


//653
static void EXEC_glWindowPos2svARB(byte *commandbuf)
{

	glWindowPos2svARB((const GLshort *)popBuf());
}


//654
static void EXEC_glWindowPos3dARB(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glWindowPos3dARB(*x, *y, *z);
}


//655
static void EXEC_glWindowPos3fARB(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glWindowPos3fARB(*x, *y, *z);
}


//656
static void EXEC_glWindowPos3iARB(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *z = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glWindowPos3iARB(*x, *y, *z);
}


//657
static void EXEC_glWindowPos3sARB(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glWindowPos3sARB(*x, *y, *z);
}


//658
static void EXEC_glWindowPos3dvARB(byte *commandbuf)
{

	glWindowPos3dvARB((const GLdouble *)popBuf());
}


//659
static void EXEC_glWindowPos3fvARB(byte *commandbuf)
{

	glWindowPos3fvARB((const GLfloat *)popBuf());
}


//660
static void EXEC_glWindowPos3ivARB(byte *commandbuf)
{

	glWindowPos3ivARB((const GLint *)popBuf());
}


//661
static void EXEC_glWindowPos3svARB(byte *commandbuf)
{

	glWindowPos3svARB((const GLshort *)popBuf());
}


//662
static void EXEC_glGetVertexAttribdvARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetVertexAttribdvARB(*index, *pname, (GLdouble *)popBuf());
}


//663
static void EXEC_glGetVertexAttribfvARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetVertexAttribfvARB(*index, *pname, (GLfloat *)popBuf());
}


//664
static void EXEC_glGetVertexAttribivARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetVertexAttribivARB(*index, *pname, (GLint *)popBuf());
}


//665
static void EXEC_glVertexAttrib1dARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glVertexAttrib1dARB(*index, *x);
}


//666
static void EXEC_glVertexAttrib1dvARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib1dvARB(*index, (const GLdouble *)popBuf());
}


//667
static void EXEC_glVertexAttrib1fARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glVertexAttrib1fARB(*index, *x);
}


//668
static void EXEC_glVertexAttrib1fvARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib1fvARB(*index, (const GLfloat *)popBuf());
}


//669
static void EXEC_glVertexAttrib1sARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glVertexAttrib1sARB(*index, *x);
}


//670
static void EXEC_glVertexAttrib1svARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib1svARB(*index, (const GLshort *)popBuf());
}


//671
static void EXEC_glVertexAttrib2dARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glVertexAttrib2dARB(*index, *x, *y);
}


//672
static void EXEC_glVertexAttrib2dvARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib2dvARB(*index, (const GLdouble *)popBuf());
}


//673
static void EXEC_glVertexAttrib2fARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glVertexAttrib2fARB(*index, *x, *y);
}


//674
static void EXEC_glVertexAttrib2fvARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib2fvARB(*index, (const GLfloat *)popBuf());
}


//675
static void EXEC_glVertexAttrib2sARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glVertexAttrib2sARB(*index, *x, *y);
}


//676
static void EXEC_glVertexAttrib2svARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib2svARB(*index, (const GLshort *)popBuf());
}


//677
static void EXEC_glVertexAttrib3dARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glVertexAttrib3dARB(*index, *x, *y, *z);
}


//678
static void EXEC_glVertexAttrib3dvARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib3dvARB(*index, (const GLdouble *)popBuf());
}


//679
static void EXEC_glVertexAttrib3fARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glVertexAttrib3fARB(*index, *x, *y, *z);
}


//680
static void EXEC_glVertexAttrib3fvARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib3fvARB(*index, (const GLfloat *)popBuf());
}


//681
static void EXEC_glVertexAttrib3sARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glVertexAttrib3sARB(*index, *x, *y, *z);
}


//682
static void EXEC_glVertexAttrib3svARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib3svARB(*index, (const GLshort *)popBuf());
}


//683
static void EXEC_glVertexAttrib4dARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *w = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glVertexAttrib4dARB(*index, *x, *y, *z, *w);
}


//684
static void EXEC_glVertexAttrib4dvARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4dvARB(*index, (const GLdouble *)popBuf());
}


//685
static void EXEC_glVertexAttrib4fARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *w = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glVertexAttrib4fARB(*index, *x, *y, *z, *w);
}


//686
static void EXEC_glVertexAttrib4fvARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4fvARB(*index, (const GLfloat *)popBuf());
}


//687
static void EXEC_glVertexAttrib4sARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *w = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glVertexAttrib4sARB(*index, *x, *y, *z, *w);
}


//688
static void EXEC_glVertexAttrib4svARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4svARB(*index, (const GLshort *)popBuf());
}


//689
static void EXEC_glVertexAttrib4NubARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLubyte *x = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *y = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *z = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *w = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);

	glVertexAttrib4NubARB(*index, *x, *y, *z, *w);
}


//690
static void EXEC_glVertexAttrib4NubvARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4NubvARB(*index, (const GLubyte *)popBuf());
}


//691
static void EXEC_glVertexAttrib4bvARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4bvARB(*index, (const GLbyte *)popBuf());
}


//692
static void EXEC_glVertexAttrib4ivARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4ivARB(*index, (const GLint *)popBuf());
}


//693
static void EXEC_glVertexAttrib4ubvARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4ubvARB(*index, (const GLubyte *)popBuf());
}


//694
static void EXEC_glVertexAttrib4usvARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4usvARB(*index, (const GLushort *)popBuf());
}


//695
static void EXEC_glVertexAttrib4uivARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4uivARB(*index, (const GLuint *)popBuf());
}


//696
static void EXEC_glVertexAttrib4NbvARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4NbvARB(*index, (const GLbyte *)popBuf());
}


//697
static void EXEC_glVertexAttrib4NsvARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4NsvARB(*index, (const GLshort *)popBuf());
}


//698
static void EXEC_glVertexAttrib4NivARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4NivARB(*index, (const GLint *)popBuf());
}


//699
static void EXEC_glVertexAttrib4NusvARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4NusvARB(*index, (const GLushort *)popBuf());
}


//700
static void EXEC_glVertexAttrib4NuivARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4NuivARB(*index, (const GLuint *)popBuf());
}


//701
static void EXEC_glVertexAttribPointerARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLboolean *normalized = (GLboolean*)commandbuf;  commandbuf += sizeof(GLboolean);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	glVertexAttribPointerARB(*index, *size, *type, *normalized, *stride, (const GLvoid *)popBuf());
}


//702
static void EXEC_glEnableVertexAttribArrayARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glEnableVertexAttribArrayARB(*index);
}


//703
static void EXEC_glDisableVertexAttribArrayARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glDisableVertexAttribArrayARB(*index);
}


//704
static void EXEC_glProgramStringARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *len = (GLsizei*)commandbuf;     commandbuf += sizeof(GLsizei);

	glProgramStringARB(*target, *format, *len, (const GLvoid *)popBuf());
}


//705
static void EXEC_glBindProgramARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	glBindProgramARB(*target, *program);
}


//706
static void EXEC_glDeleteProgramsARB(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glDeleteProgramsARB(*n, (const GLuint *)popBuf());
}


//707
static void EXEC_glGenProgramsARB(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glGenProgramsARB(*n, (GLuint *)popBuf());
}


//708
static void EXEC_glIsProgramARB(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	pushRet(glIsProgramARB(*program));
}


//709
static void EXEC_glProgramEnvParameter4dARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *w = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glProgramEnvParameter4dARB(*target, *index, *x, *y, *z, *w);
}


//710
static void EXEC_glProgramEnvParameter4dvARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glProgramEnvParameter4dvARB(*target, *index, (const GLdouble *)popBuf());
}


//711
static void EXEC_glProgramEnvParameter4fARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *w = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glProgramEnvParameter4fARB(*target, *index, *x, *y, *z, *w);
}


//712
static void EXEC_glProgramEnvParameter4fvARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glProgramEnvParameter4fvARB(*target, *index, (const GLfloat *)popBuf());
}


//713
static void EXEC_glProgramLocalParameter4dARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *w = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glProgramLocalParameter4dARB(*target, *index, *x, *y, *z, *w);
}


//714
static void EXEC_glProgramLocalParameter4dvARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glProgramLocalParameter4dvARB(*target, *index, (const GLdouble *)popBuf());
}


//715
static void EXEC_glProgramLocalParameter4fARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *w = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glProgramLocalParameter4fARB(*target, *index, *x, *y, *z, *w);
}


//716
static void EXEC_glProgramLocalParameter4fvARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glProgramLocalParameter4fvARB(*target, *index, (const GLfloat *)popBuf());
}


//717
static void EXEC_glGetProgramEnvParameterdvARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glGetProgramEnvParameterdvARB(*target, *index, (GLdouble *)popBuf());
}


//718
static void EXEC_glGetProgramEnvParameterfvARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glGetProgramEnvParameterfvARB(*target, *index, (GLfloat *)popBuf());
}


//719
static void EXEC_glGetProgramLocalParameterdvARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glGetProgramLocalParameterdvARB(*target, *index, (GLdouble *)popBuf());
}


//720
static void EXEC_glGetProgramLocalParameterfvARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glGetProgramLocalParameterfvARB(*target, *index, (GLfloat *)popBuf());
}


//721
static void EXEC_glGetProgramivARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetProgramivARB(*target, *pname, (GLint *)popBuf());
}


//722
static void EXEC_glGetProgramStringARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetProgramStringARB(*target, *pname, (GLvoid *)popBuf());
}


//723
static void EXEC_glGetVertexAttribPointervARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetVertexAttribPointervARB(*index, *pname, (GLvoid **)popBuf());
}


//724
static void EXEC_glBindBufferARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *buffer = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	glBindBufferARB(*target, *buffer);
}


//725
static void EXEC_glBufferDataARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizeiptrARB *size = (GLsizeiptrARB*)commandbuf;    commandbuf += sizeof(GLsizeiptrARB);
	GLenum *usage = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glBufferDataARB(*target, *size, (const GLvoid *)popBuf(), *usage);
}


//726
static void EXEC_glBufferSubDataARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLintptrARB *offset = (GLintptrARB*)commandbuf;  commandbuf += sizeof(GLintptrARB);
	GLsizeiptrARB *size = (GLsizeiptrARB*)commandbuf;    commandbuf += sizeof(GLsizeiptrARB);

	glBufferSubDataARB(*target, *offset, *size, (const GLvoid *)popBuf());
}


//727
static void EXEC_glDeleteBuffersARB(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glDeleteBuffersARB(*n, (const GLuint *)popBuf());
}


//728
static void EXEC_glGenBuffersARB(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glGenBuffersARB(*n, (GLuint *)popBuf());
}


//729
static void EXEC_glGetBufferParameterivARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetBufferParameterivARB(*target, *pname, (GLint *)popBuf());
}


//730
static void EXEC_glGetBufferPointervARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetBufferPointervARB(*target, *pname, (GLvoid **)popBuf());
}


//731
static void EXEC_glGetBufferSubDataARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLintptrARB *offset = (GLintptrARB*)commandbuf;  commandbuf += sizeof(GLintptrARB);
	GLsizeiptrARB *size = (GLsizeiptrARB*)commandbuf;    commandbuf += sizeof(GLsizeiptrARB);

	glGetBufferSubDataARB(*target, *offset, *size, (GLvoid *)popBuf());
}


//732
static void EXEC_glIsBufferARB(byte *commandbuf)
{
	GLuint *buffer = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	pushRet(glIsBufferARB(*buffer));
}


//733
static void EXEC_glMapBufferARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *access = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	pushRet((const GLubyte*) glMapBufferARB(*target, *access));
}


//734
static void EXEC_glUnmapBufferARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	pushRet(glUnmapBufferARB(*target));
}


//735
static void EXEC_glGenQueriesARB(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glGenQueriesARB(*n, (GLuint *)popBuf());
}


//736
static void EXEC_glDeleteQueriesARB(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glDeleteQueriesARB(*n, (const GLuint *)popBuf());
}


//737
static void EXEC_glIsQueryARB(byte *commandbuf)
{
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	pushRet(glIsQueryARB(*id));
}


//738
static void EXEC_glBeginQueryARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	glBeginQueryARB(*target, *id);
}


//739
static void EXEC_glEndQueryARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glEndQueryARB(*target);
}


//740
static void EXEC_glGetQueryivARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetQueryivARB(*target, *pname, (GLint *)popBuf());
}


//741
static void EXEC_glGetQueryObjectivARB(byte *commandbuf)
{
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetQueryObjectivARB(*id, *pname, (GLint *)popBuf());
}


//742
static void EXEC_glGetQueryObjectuivARB(byte *commandbuf)
{
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetQueryObjectuivARB(*id, *pname, (GLuint *)popBuf());
}


//743
static void EXEC_glDeleteObjectARB(byte *commandbuf)
{
	GLhandleARB *obj = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);

	glDeleteObjectARB(*obj);
}


//744
static void EXEC_glGetHandleARB(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	pushRet(glGetHandleARB(*pname));
}


//745
static void EXEC_glDetachObjectARB(byte *commandbuf)
{
	GLhandleARB *containerObj = (GLhandleARB*)commandbuf;    commandbuf += sizeof(GLhandleARB);
	GLhandleARB *attachedObj = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);

	glDetachObjectARB(*containerObj, *attachedObj);
}


//746
static void EXEC_glCreateShaderObjectARB(byte *commandbuf)
{
	GLenum *shaderType = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	pushRet(glCreateShaderObjectARB(*shaderType));
}


//747
static void EXEC_glShaderSourceARB(byte *commandbuf)
{
	GLuint *shader = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	const GLchar * string = (const GLchar *)popBuf();
	GLint   *length = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	if(*length == -1) {
		length = NULL;
	}

	glShaderSourceARB(*shader, *count,  &string, length);
}


//748
static void EXEC_glCompileShaderARB(byte *commandbuf)
{
	GLhandleARB *shader = (GLhandleARB*)commandbuf;  commandbuf += sizeof(GLhandleARB);

	glCompileShaderARB(*shader);
}


//749
static void EXEC_glCreateProgramObjectARB(byte *commandbuf)
{

	pushRet(glCreateProgramObjectARB());
}


//750
static void EXEC_glAttachObjectARB(byte *commandbuf)
{
	GLhandleARB *containerObj = (GLhandleARB*)commandbuf;    commandbuf += sizeof(GLhandleARB);
	GLhandleARB *obj = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);

	glAttachObjectARB(*containerObj, *obj);
}


//751
static void EXEC_glLinkProgramARB(byte *commandbuf)
{
	GLhandleARB *program = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);

	glLinkProgramARB(*program);
}


//752
static void EXEC_glUseProgramObjectARB(byte *commandbuf)
{
	GLhandleARB *program = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);

	glUseProgramObjectARB(*program);
}


//753
static void EXEC_glValidateProgramARB(byte *commandbuf)
{
	GLhandleARB *program = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);

	glValidateProgramARB(*program);
}


//754
static void EXEC_glUniform1fARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLfloat *v0 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	glUniform1fARB(*location, *v0);
}


//755
static void EXEC_glUniform2fARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLfloat *v0 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	glUniform2fARB(*location, *v0, *v1);
}


//756
static void EXEC_glUniform3fARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLfloat *v0 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v2 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	glUniform3fARB(*location, *v0, *v1, *v2);
}


//757
static void EXEC_glUniform4fARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLfloat *v0 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v2 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v3 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	glUniform4fARB(*location, *v0, *v1, *v2, *v3);
}


//758
static void EXEC_glUniform1iARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLint *v0 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	glUniform1iARB(*location, *v0);
}


//759
static void EXEC_glUniform2iARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLint *v0 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v1 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	glUniform2iARB(*location, *v0, *v1);
}


//760
static void EXEC_glUniform3iARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLint *v0 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v1 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v2 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	glUniform3iARB(*location, *v0, *v1, *v2);
}


//761
static void EXEC_glUniform4iARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLint *v0 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v1 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v2 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v3 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	glUniform4iARB(*location, *v0, *v1, *v2, *v3);
}


//762
static void EXEC_glUniform1fvARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glUniform1fvARB(*location, *count, (const GLfloat *)popBuf());
}


//763
static void EXEC_glUniform2fvARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glUniform2fvARB(*location, *count, (const GLfloat *)popBuf());
}


//764
static void EXEC_glUniform3fvARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glUniform3fvARB(*location, *count, (const GLfloat *)popBuf());
}


//765
static void EXEC_glUniform4fvARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glUniform4fvARB(*location, *count, (const GLfloat *)popBuf());
}


//766
static void EXEC_glUniform1ivARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glUniform1ivARB(*location, *count, (const GLint *)popBuf());
}


//767
static void EXEC_glUniform2ivARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glUniform2ivARB(*location, *count, (const GLint *)popBuf());
}


//768
static void EXEC_glUniform3ivARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glUniform3ivARB(*location, *count, (const GLint *)popBuf());
}


//769
static void EXEC_glUniform4ivARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glUniform4ivARB(*location, *count, (const GLint *)popBuf());
}


//770
static void EXEC_glUniformMatrix2fvARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLboolean *transpose = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);

	glUniformMatrix2fvARB(*location, *count, *transpose, (const GLfloat *)popBuf());
}


//771
static void EXEC_glUniformMatrix3fvARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLboolean *transpose = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);

	glUniformMatrix3fvARB(*location, *count, *transpose, (const GLfloat *)popBuf());
}


//772
static void EXEC_glUniformMatrix4fvARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLboolean *transpose = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);

	glUniformMatrix4fvARB(*location, *count, *transpose, (const GLfloat *)popBuf());
}


//773
static void EXEC_glGetObjectParameterfvARB(byte *commandbuf)
{
	GLhandleARB *obj = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetObjectParameterfvARB(*obj, *pname, (GLfloat *)popBuf());
}


//774
static void EXEC_glGetObjectParameterivARB(byte *commandbuf)
{
	GLhandleARB *obj = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetObjectParameterivARB(*obj, *pname, (GLint *)popBuf());
}


//775
static void EXEC_glGetInfoLogARB(byte *commandbuf)
{
	GLhandleARB *obj = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);
	GLsizei *maxLength = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *length = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	glGetInfoLogARB(*obj, *maxLength, length, (GLcharARB *)popBuf());
}


//776
static void EXEC_glGetAttachedObjectsARB(byte *commandbuf)
{
	GLhandleARB *containerObj = (GLhandleARB*)commandbuf;    commandbuf += sizeof(GLhandleARB);
	GLsizei *maxLength = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glGetAttachedObjectsARB(*containerObj, *maxLength, (GLsizei *)popBuf(), (GLhandleARB *)popBuf());
}


//777
static void EXEC_glGetUniformLocationARB(byte *commandbuf)
{
	GLhandleARB *program = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);
	int num = glGetUniformLocationARB(*program, (const GLcharARB *)popBuf());
	pushRet(num);
}


//778
static void EXEC_glGetActiveUniformARB(byte *commandbuf)
{
	GLhandleARB *program = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLsizei *bufSize = (GLsizei*)commandbuf;     commandbuf += sizeof(GLsizei);

	glGetActiveUniformARB(*program, *index, *bufSize, (GLsizei *)popBuf(), (GLint *)popBuf(), (GLenum *)popBuf(), (GLcharARB *)popBuf());
}


//779
static void EXEC_glGetUniformfvARB(byte *commandbuf)
{
	GLhandleARB *program = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);

	glGetUniformfvARB(*program, *location, (GLfloat *)popBuf());
}


//780
static void EXEC_glGetUniformivARB(byte *commandbuf)
{
	GLhandleARB *program = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);

	glGetUniformivARB(*program, *location, (GLint *)popBuf());
}


//781
static void EXEC_glGetShaderSourceARB(byte *commandbuf)
{
	GLuint *shader = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLsizei *bufSize = (GLsizei*)commandbuf;     commandbuf += sizeof(GLsizei);
	GLint   *length  = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLchar  *source = (GLchar *)popBuf();
	if(*length == -1) {
		length = NULL;
	}

	glGetShaderSourceARB(*shader, *bufSize, length, source);
}


//782
static void EXEC_glBindAttribLocationARB(byte *commandbuf)
{
	GLhandleARB *program = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glBindAttribLocationARB(*program, *index, (const GLcharARB *)popBuf());
}


//783
static void EXEC_glGetActiveAttribARB(byte *commandbuf)
{
	GLhandleARB *program = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLsizei *bufSize = (GLsizei*)commandbuf;     commandbuf += sizeof(GLsizei);

	glGetActiveAttribARB(*program, *index, *bufSize, (GLsizei *)popBuf(), (GLint *)popBuf(), (GLenum *)popBuf(), (GLcharARB *)popBuf());
}


//784
static void EXEC_glGetAttribLocationARB(byte *commandbuf)
{
	GLhandleARB *program = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);

	pushRet(glGetAttribLocationARB(*program, (const GLcharARB *)popBuf()));
}


//785
static void EXEC_glDrawBuffersARB(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glDrawBuffersARB(*n, (const GLenum *)popBuf());
}


//786
static void EXEC_glBlendColorEXT(byte *commandbuf)
{
	GLclampf *red = (GLclampf*)commandbuf;   commandbuf += sizeof(GLclampf);
	GLclampf *green = (GLclampf*)commandbuf;     commandbuf += sizeof(GLclampf);
	GLclampf *blue = (GLclampf*)commandbuf;  commandbuf += sizeof(GLclampf);
	GLclampf *alpha = (GLclampf*)commandbuf;     commandbuf += sizeof(GLclampf);

	glBlendColorEXT(*red, *green, *blue, *alpha);
}


//787
static void EXEC_glPolygonOffsetEXT(byte *commandbuf)
{
	GLfloat *factor = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *bias = (GLfloat*)commandbuf;    commandbuf += sizeof(GLfloat);

	glPolygonOffsetEXT(*factor, *bias);
}


//788
static void EXEC_glTexImage3DEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLsizei *depth = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLint *border = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glTexImage3DEXT(*target, *level, *internalformat, *width, *height, *depth, *border, *format, *type, (const GLvoid *)popBuf());
}


//789
static void EXEC_glTexSubImage3DEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *xoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *yoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *zoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLsizei *depth = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	//GLuint *UNUSED = (GLuint*)commandbuf;	 commandbuf += sizeof(GLuint);

	glTexSubImage3DEXT(*target, *level, *xoffset, *yoffset, *zoffset, *width, *height, *depth, *format, *type,  (const GLvoid *)popBuf());
}


//790
static void EXEC_glGetTexFilterFuncSGIS(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *filter = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glGetTexFilterFuncSGIS(*target, *filter, (GLfloat *)popBuf());
}


//791
static void EXEC_glTexFilterFuncSGIS(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *filter = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glTexFilterFuncSGIS(*target, *filter, *n, (const GLfloat *)popBuf());
}


//792
static void EXEC_glTexSubImage1DEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *xoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	//GLuint *UNUSED = (GLuint*)commandbuf;	 commandbuf += sizeof(GLuint);

	glTexSubImage1DEXT(*target, *level, *xoffset, *width, *format, *type,  (const GLvoid *)popBuf());
}


//793
static void EXEC_glTexSubImage2DEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *xoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *yoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	//GLuint *UNUSED = (GLuint*)commandbuf;	 commandbuf += sizeof(GLuint);

	glTexSubImage2DEXT(*target, *level, *xoffset, *yoffset, *width, *height, *format, *type,  (const GLvoid *)popBuf());
}


//794
static void EXEC_glCopyTexImage1DEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLint *border = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	glCopyTexImage1DEXT(*target, *level, *internalformat, *x, *y, *width, *border);
}


//795
static void EXEC_glCopyTexImage2DEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLint *border = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	glCopyTexImage2DEXT(*target, *level, *internalformat, *x, *y, *width, *height, *border);
}


//796
static void EXEC_glCopyTexSubImage1DEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *xoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glCopyTexSubImage1DEXT(*target, *level, *xoffset, *x, *y, *width);
}


//797
static void EXEC_glCopyTexSubImage2DEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *xoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *yoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	glCopyTexSubImage2DEXT(*target, *level, *xoffset, *yoffset, *x, *y, *width, *height);
}


//798
static void EXEC_glCopyTexSubImage3DEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *xoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *yoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *zoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	glCopyTexSubImage3DEXT(*target, *level, *xoffset, *yoffset, *zoffset, *x, *y, *width, *height);
}


//799
static void EXEC_glGetHistogramEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLboolean *reset = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glGetHistogramEXT(*target, *reset, *format, *type, (GLvoid *)popBuf());
}


//800
static void EXEC_glGetHistogramParameterfvEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetHistogramParameterfvEXT(*target, *pname, (GLfloat *)popBuf());
}


//801
static void EXEC_glGetHistogramParameterivEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetHistogramParameterivEXT(*target, *pname, (GLint *)popBuf());
}


//802
static void EXEC_glGetMinmaxEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLboolean *reset = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glGetMinmaxEXT(*target, *reset, *format, *type, (GLvoid *)popBuf());
}


//803
static void EXEC_glGetMinmaxParameterfvEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetMinmaxParameterfvEXT(*target, *pname, (GLfloat *)popBuf());
}


//804
static void EXEC_glGetMinmaxParameterivEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetMinmaxParameterivEXT(*target, *pname, (GLint *)popBuf());
}


//805
static void EXEC_glHistogramEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLboolean *sink = (GLboolean*)commandbuf;    commandbuf += sizeof(GLboolean);

	glHistogramEXT(*target, *width, *internalformat, *sink);
}


//806
static void EXEC_glMinmaxEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLboolean *sink = (GLboolean*)commandbuf;    commandbuf += sizeof(GLboolean);

	glMinmaxEXT(*target, *internalformat, *sink);
}


//807
static void EXEC_glResetHistogramEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glResetHistogramEXT(*target);
}


//808
static void EXEC_glResetMinmaxEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glResetMinmaxEXT(*target);
}


//809
static void EXEC_glConvolutionFilter1DEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glConvolutionFilter1DEXT(*target, *internalformat, *width, *format, *type, (const GLvoid *)popBuf());
}


//810
static void EXEC_glConvolutionFilter2DEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glConvolutionFilter2DEXT(*target, *internalformat, *width, *height, *format, *type, (const GLvoid *)popBuf());
}


//811
static void EXEC_glConvolutionParameterfEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *params = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	glConvolutionParameterfEXT(*target, *pname, *params);
}


//812
static void EXEC_glConvolutionParameterfvEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glConvolutionParameterfvEXT(*target, *pname, (const GLfloat *)popBuf());
}


//813
static void EXEC_glConvolutionParameteriEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *params = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	glConvolutionParameteriEXT(*target, *pname, *params);
}


//814
static void EXEC_glConvolutionParameterivEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glConvolutionParameterivEXT(*target, *pname, (const GLint *)popBuf());
}


//815
static void EXEC_glCopyConvolutionFilter1DEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glCopyConvolutionFilter1DEXT(*target, *internalformat, *x, *y, *width);
}


//816
static void EXEC_glCopyConvolutionFilter2DEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	glCopyConvolutionFilter2DEXT(*target, *internalformat, *x, *y, *width, *height);
}


//817
static void EXEC_glGetConvolutionFilterEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glGetConvolutionFilterEXT(*target, *format, *type, (GLvoid *)popBuf());
}


//818
static void EXEC_glGetConvolutionParameterfvEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetConvolutionParameterfvEXT(*target, *pname, (GLfloat *)popBuf());
}


//819
static void EXEC_glGetConvolutionParameterivEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetConvolutionParameterivEXT(*target, *pname, (GLint *)popBuf());
}


//820
static void EXEC_glGetSeparableFilterEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glGetSeparableFilterEXT(*target, *format, *type, (GLvoid *)popBuf(), (GLvoid *)popBuf(), (GLvoid *)popBuf());
}


//821
static void EXEC_glSeparableFilter2DEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glSeparableFilter2DEXT(*target, *internalformat, *width, *height, *format, *type, (const GLvoid *)popBuf(), (const GLvoid *)popBuf());
}


//822
static void EXEC_glColorTableSGI(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glColorTableSGI(*target, *internalformat, *width, *format, *type, (const GLvoid *)popBuf());
}


//823
static void EXEC_glColorTableParameterfvSGI(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glColorTableParameterfvSGI(*target, *pname, (const GLfloat *)popBuf());
}


//824
static void EXEC_glColorTableParameterivSGI(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glColorTableParameterivSGI(*target, *pname, (const GLint *)popBuf());
}


//825
static void EXEC_glCopyColorTableSGI(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glCopyColorTableSGI(*target, *internalformat, *x, *y, *width);
}


//826
static void EXEC_glGetColorTableSGI(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glGetColorTableSGI(*target, *format, *type, (GLvoid *)popBuf());
}


//827
static void EXEC_glGetColorTableParameterfvSGI(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetColorTableParameterfvSGI(*target, *pname, (GLfloat *)popBuf());
}


//828
static void EXEC_glGetColorTableParameterivSGI(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetColorTableParameterivSGI(*target, *pname, (GLint *)popBuf());
}


//829
static void EXEC_glPixelTexGenParameteriSGIS(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	//glPixelTexGenParameteriSGIS(*pname, *param);
}


//830
static void EXEC_glPixelTexGenParameterivSGIS(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	//glPixelTexGenParameterivSGIS(*pname, (const GLint *)popBuf());
}


//831
static void EXEC_glPixelTexGenParameterfSGIS(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	//glPixelTexGenParameterfSGIS(*pname, *param);
}


//832
static void EXEC_glPixelTexGenParameterfvSGIS(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	//glPixelTexGenParameterfvSGIS(*pname, (const GLfloat *)popBuf());
}


//833
static void EXEC_glGetPixelTexGenParameterivSGIS(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	//glGetPixelTexGenParameterivSGIS(*pname, (GLint *)popBuf());
}


//834
static void EXEC_glGetPixelTexGenParameterfvSGIS(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	//glGetPixelTexGenParameterfvSGIS(*pname, (GLfloat *)popBuf());
}


//835
static void EXEC_glTexImage4DSGIS(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLsizei *depth = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *size4d = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLint *border = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	//glTexImage4DSGIS(*target, *level, *internalformat, *width, *height, *depth, *size4d, *border, *format, *type, (const GLvoid *)popBuf());
}


//836
static void EXEC_glTexSubImage4DSGIS(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *level = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *xoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *yoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *zoffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *woffset = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLsizei *depth = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *size4d = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	//GLuint *UNUSED = (GLuint*)commandbuf;	 commandbuf += sizeof(GLuint);

	//glTexSubImage4DSGIS(*target, *level, *xoffset, *yoffset, *zoffset, *woffset, *width, *height, *depth, *size4d, *format, *type,  (const GLvoid *)popBuf());
}


//837
static void EXEC_glAreTexturesResidentEXT(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	pushRet(glAreTexturesResidentEXT(*n, (const GLuint *)popBuf(), (GLboolean *)popBuf()));
}


//838
static void EXEC_glBindTextureEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *texture = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	glBindTextureEXT(*target, *texture);
}


//839
static void EXEC_glDeleteTexturesEXT(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glDeleteTexturesEXT(*n, (const GLuint *)popBuf());
}


//840
static void EXEC_glGenTexturesEXT(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glGenTexturesEXT(*n, (GLuint *)popBuf());
}


//841
static void EXEC_glIsTextureEXT(byte *commandbuf)
{
	GLuint *texture = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	pushRet(glIsTextureEXT(*texture));
}


//842
static void EXEC_glPrioritizeTexturesEXT(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glPrioritizeTexturesEXT(*n, (const GLuint *)popBuf(), (const GLclampf *)popBuf());
}


//843
static void EXEC_glDetailTexFuncSGIS(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glDetailTexFuncSGIS(*target, *n, (const GLfloat *)popBuf());
}


//844
static void EXEC_glGetDetailTexFuncSGIS(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glGetDetailTexFuncSGIS(*target, (GLfloat *)popBuf());
}


//845
static void EXEC_glSharpenTexFuncSGIS(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glSharpenTexFuncSGIS(*target, *n, (const GLfloat *)popBuf());
}


//846
static void EXEC_glGetSharpenTexFuncSGIS(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glGetSharpenTexFuncSGIS(*target, (GLfloat *)popBuf());
}


//847
static void EXEC_glSampleMaskSGIS(byte *commandbuf)
{
	GLclampf *value = (GLclampf*)commandbuf;     commandbuf += sizeof(GLclampf);
	GLboolean *invert = (GLboolean*)commandbuf;  commandbuf += sizeof(GLboolean);

	glSampleMaskSGIS(*value, *invert);
}


//848
static void EXEC_glSamplePatternSGIS(byte *commandbuf)
{
	GLenum *pattern = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	glSamplePatternSGIS(*pattern);
}


//849
static void EXEC_glArrayElementEXT(byte *commandbuf)
{
	GLint *i = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glArrayElementEXT(*i);
}


//850
static void EXEC_glColorPointerEXT(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glColorPointerEXT(*size, *type, *stride, *count, (const GLvoid *)popBuf());
}


//851
static void EXEC_glDrawArraysEXT(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLint *first = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glDrawArraysEXT(*mode, *first, *count);
}


//852
static void EXEC_glEdgeFlagPointerEXT(byte *commandbuf)
{
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glEdgeFlagPointerEXT(*stride, *count, (const GLboolean *)popBuf());
}



//853
static void EXEC_glGetPointervEXT(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	LOG("Warning: Called glGetPointervEXT, using glGetPointerv instead\n");
	
	glGetPointerv(*pname, (GLvoid **)popBuf());
}




//854
static void EXEC_glIndexPointerEXT(byte *commandbuf)
{
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glIndexPointerEXT(*type, *stride, *count, (const GLvoid *)popBuf());
}


//855
static void EXEC_glNormalPointerEXT(byte *commandbuf)
{
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glNormalPointerEXT(*type, *stride, *count, (const GLvoid *)popBuf());
}


//856
static void EXEC_glTexCoordPointerEXT(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glTexCoordPointerEXT(*size, *type, *stride, *count, (const GLvoid *)popBuf());
}


//857
static void EXEC_glVertexPointerEXT(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glVertexPointerEXT(*size, *type, *stride, *count, (const GLvoid *)popBuf());
}


//858
static void EXEC_glBlendEquationEXT(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glBlendEquationEXT(*mode);
}


//859
static void EXEC_glSpriteParameterfSGIX(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glSpriteParameterfSGIX(*pname, *param);
}


//860
static void EXEC_glSpriteParameterfvSGIX(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glSpriteParameterfvSGIX(*pname, (GLfloat *)popBuf());
}


//861
static void EXEC_glSpriteParameteriSGIX(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glSpriteParameteriSGIX(*pname, *param);
}


//862
static void EXEC_glSpriteParameterivSGIX(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glSpriteParameterivSGIX(*pname, (GLint *)popBuf());
}


//863
static void EXEC_glPointParameterfEXT(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glPointParameterfEXT(*pname, *param);
}


//864
static void EXEC_glPointParameterfvEXT(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glPointParameterfvEXT(*pname, (GLfloat *)popBuf());
}


//865
static void EXEC_glGetInstrumentsSGIX(byte *commandbuf)
{

	//pushRet(glGetInstrumentsSGIX());
}


//866
static void EXEC_glInstrumentsBufferSGIX(byte *commandbuf)
{
	GLsizei *size = (GLsizei*)commandbuf;    commandbuf += sizeof(GLsizei);

	//glInstrumentsBufferSGIX(*size, (GLint *)popBuf());
}


//867
static void EXEC_glPollInstrumentsSGIX(byte *commandbuf)
{

	//pushRet(glPollInstrumentsSGIX((GLint *)popBuf()));
}


//868
static void EXEC_glReadInstrumentsSGIX(byte *commandbuf)
{
	GLint *marker = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	//glReadInstrumentsSGIX(*marker);
}


//869
static void EXEC_glStartInstrumentsSGIX(byte *commandbuf)
{

	//glStartInstrumentsSGIX();
}


//870
static void EXEC_glStopInstrumentsSGIX(byte *commandbuf)
{
	GLint *marker = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	//glStopInstrumentsSGIX(*marker);
}


//871
static void EXEC_glFrameZoomSGIX(byte *commandbuf)
{
	GLint *factor = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	//glFrameZoomSGIX(*factor);
}


//872
static void EXEC_glTagSampleBufferSGIX(byte *commandbuf)
{

	//glTagSampleBufferSGIX();
}


//873
static void EXEC_glReferencePlaneSGIX(byte *commandbuf)
{

	//glReferencePlaneSGIX((const GLdouble *)popBuf());
}


//874
static void EXEC_glFlushRasterSGIX(byte *commandbuf)
{

	//glFlushRasterSGIX();
}


//875
static void EXEC_glFogFuncSGIS(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glFogFuncSGIS(*n, (const GLfloat *)popBuf());
}


//876
static void EXEC_glGetFogFuncSGIS(byte *commandbuf)
{

	glGetFogFuncSGIS((GLfloat *)popBuf());
}


//877
static void EXEC_glImageTransformParameteriHP(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glImageTransformParameteriHP(*target, *pname, *param);
}


//878
static void EXEC_glImageTransformParameterfHP(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glImageTransformParameterfHP(*target, *pname, *param);
}


//879
static void EXEC_glImageTransformParameterivHP(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glImageTransformParameterivHP(*target, *pname, (const GLint *)popBuf());
}


//880
static void EXEC_glImageTransformParameterfvHP(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glImageTransformParameterfvHP(*target, *pname, (const GLfloat *)popBuf());
}


//881
static void EXEC_glGetImageTransformParameterivHP(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetImageTransformParameterivHP(*target, *pname, (GLint *)popBuf());
}


//882
static void EXEC_glGetImageTransformParameterfvHP(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetImageTransformParameterfvHP(*target, *pname, (GLfloat *)popBuf());
}


//883
static void EXEC_glColorSubTableEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *start = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glColorSubTableEXT(*target, *start, *count, *format, *type, (const GLvoid *)popBuf());
}


//884
static void EXEC_glCopyColorSubTableEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *start = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glCopyColorSubTableEXT(*target, *start, *x, *y, *width);
}


//885
static void EXEC_glHintPGI(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *mode = (GLint*)commandbuf;    commandbuf += sizeof(GLint);

	//glHintPGI(*target, *mode);
}


//886
static void EXEC_glColorTableEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glColorTableEXT(*target, *internalformat, *width, *format, *type, (const GLvoid *)popBuf());
}


//887
static void EXEC_glGetColorTableEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *format = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glGetColorTableEXT(*target, *format, *type, (GLvoid *)popBuf());
}


//888
static void EXEC_glGetColorTableParameterivEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetColorTableParameterivEXT(*target, *pname, (GLint *)popBuf());
}


//889
static void EXEC_glGetColorTableParameterfvEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetColorTableParameterfvEXT(*target, *pname, (GLfloat *)popBuf());
}


//890
static void EXEC_glGetListParameterfvSGIX(byte *commandbuf)
{
	GLuint *list = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	//glGetListParameterfvSGIX(*list, *pname, (GLfloat *)popBuf());
}


//891
static void EXEC_glGetListParameterivSGIX(byte *commandbuf)
{
	GLuint *list = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	//glGetListParameterivSGIX(*list, *pname, (GLint *)popBuf());
}


//892
static void EXEC_glListParameterfSGIX(byte *commandbuf)
{
	GLuint *list = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	//glListParameterfSGIX(*list, *pname, *param);
}


//893
static void EXEC_glListParameterfvSGIX(byte *commandbuf)
{
	GLuint *list = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	//glListParameterfvSGIX(*list, *pname, (const GLfloat *)popBuf());
}


//894
static void EXEC_glListParameteriSGIX(byte *commandbuf)
{
	GLuint *list = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	//glListParameteriSGIX(*list, *pname, *param);
}


//895
static void EXEC_glListParameterivSGIX(byte *commandbuf)
{
	GLuint *list = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	//glListParameterivSGIX(*list, *pname, (const GLint *)popBuf());
}


//896
static void EXEC_glIndexMaterialEXT(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glIndexMaterialEXT(*face, *mode);
}


//897
static void EXEC_glIndexFuncEXT(byte *commandbuf)
{
	GLenum *func = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLclampf *ref = (GLclampf*)commandbuf;   commandbuf += sizeof(GLclampf);

	glIndexFuncEXT(*func, *ref);
}


//898
static void EXEC_glLockArraysEXT(byte *commandbuf)
{
	GLint *first = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glLockArraysEXT(*first, *count);
}


//899
static void EXEC_glUnlockArraysEXT(byte *commandbuf)
{

	glUnlockArraysEXT();
}


//900
static void EXEC_glCullParameterdvEXT(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glCullParameterdvEXT(*pname, (GLdouble *)popBuf());
}


//901
static void EXEC_glCullParameterfvEXT(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glCullParameterfvEXT(*pname, (GLfloat *)popBuf());
}


//902
static void EXEC_glFragmentColorMaterialSGIX(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glFragmentColorMaterialSGIX(*face, *mode);
}


//903
static void EXEC_glFragmentLightfSGIX(byte *commandbuf)
{
	GLenum *light = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glFragmentLightfSGIX(*light, *pname, *param);
}


//904
static void EXEC_glFragmentLightfvSGIX(byte *commandbuf)
{
	GLenum *light = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glFragmentLightfvSGIX(*light, *pname, (GLfloat *)popBuf());
}


//905
static void EXEC_glFragmentLightiSGIX(byte *commandbuf)
{
	GLenum *light = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glFragmentLightiSGIX(*light, *pname, *param);
}


//906
static void EXEC_glFragmentLightivSGIX(byte *commandbuf)
{
	GLenum *light = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glFragmentLightivSGIX(*light, *pname, (GLint *)popBuf());
}


//907
static void EXEC_glFragmentLightModelfSGIX(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glFragmentLightModelfSGIX(*pname, *param);
}


//908
static void EXEC_glFragmentLightModelfvSGIX(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glFragmentLightModelfvSGIX(*pname, (GLfloat *)popBuf());
}


//909
static void EXEC_glFragmentLightModeliSGIX(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glFragmentLightModeliSGIX(*pname, *param);
}


//910
static void EXEC_glFragmentLightModelivSGIX(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glFragmentLightModelivSGIX(*pname, (GLint *)popBuf());
}


//911
static void EXEC_glFragmentMaterialfSGIX(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glFragmentMaterialfSGIX(*face, *pname, *param);
}


//912
static void EXEC_glFragmentMaterialfvSGIX(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glFragmentMaterialfvSGIX(*face, *pname, (const GLfloat *)popBuf());
}


//913
static void EXEC_glFragmentMaterialiSGIX(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glFragmentMaterialiSGIX(*face, *pname, *param);
}


//914
static void EXEC_glFragmentMaterialivSGIX(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glFragmentMaterialivSGIX(*face, *pname, (const GLint *)popBuf());
}


//915
static void EXEC_glGetFragmentLightfvSGIX(byte *commandbuf)
{
	GLenum *light = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetFragmentLightfvSGIX(*light, *pname, (GLfloat *)popBuf());
}


//916
static void EXEC_glGetFragmentLightivSGIX(byte *commandbuf)
{
	GLenum *light = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetFragmentLightivSGIX(*light, *pname, (GLint *)popBuf());
}


//917
static void EXEC_glGetFragmentMaterialfvSGIX(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetFragmentMaterialfvSGIX(*face, *pname, (GLfloat *)popBuf());
}


//918
static void EXEC_glGetFragmentMaterialivSGIX(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetFragmentMaterialivSGIX(*face, *pname, (GLint *)popBuf());
}


//919
static void EXEC_glLightEnviSGIX(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	//glLightEnviSGIX(*pname, *param);
}


//920
static void EXEC_glDrawRangeElementsEXT(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLuint *start = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLuint *end = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glDrawRangeElementsEXT(*mode, *start, *end, *count, *type, (const GLvoid *)popBuf());
}


//921
static void EXEC_glApplyTextureEXT(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glApplyTextureEXT(*mode);
}


//922
static void EXEC_glTextureLightEXT(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glTextureLightEXT(*pname);
}


//923
static void EXEC_glTextureMaterialEXT(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glTextureMaterialEXT(*face, *mode);
}


//924
static void EXEC_glAsyncMarkerSGIX(byte *commandbuf)
{
	GLuint *marker = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	glAsyncMarkerSGIX(*marker);
}


//925
static void EXEC_glFinishAsyncSGIX(byte *commandbuf)
{

	pushRet(glFinishAsyncSGIX((GLuint *)popBuf()));
}


//926
static void EXEC_glPollAsyncSGIX(byte *commandbuf)
{

	pushRet(glPollAsyncSGIX((GLuint *)popBuf()));
}


//927
static void EXEC_glGenAsyncMarkersSGIX(byte *commandbuf)
{
	GLsizei *range = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	pushRet(glGenAsyncMarkersSGIX(*range));
}


//928
static void EXEC_glDeleteAsyncMarkersSGIX(byte *commandbuf)
{
	GLuint *marker = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLsizei *range = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glDeleteAsyncMarkersSGIX(*marker, *range);
}


//929
static void EXEC_glIsAsyncMarkerSGIX(byte *commandbuf)
{
	GLuint *marker = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	pushRet(glIsAsyncMarkerSGIX(*marker));
}


//930
static void EXEC_glVertexPointervINTEL(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glVertexPointervINTEL(*size, *type, (const GLvoid **)popBuf());
}


//931
static void EXEC_glNormalPointervINTEL(byte *commandbuf)
{
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glNormalPointervINTEL(*type, (const GLvoid **)popBuf());
}


//932
static void EXEC_glColorPointervINTEL(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glColorPointervINTEL(*size, *type, (const GLvoid **)popBuf());
}


//933
static void EXEC_glTexCoordPointervINTEL(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glTexCoordPointervINTEL(*size, *type, (const GLvoid **)popBuf());
}


//934
static void EXEC_glPixelTransformParameteriEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glPixelTransformParameteriEXT(*target, *pname, *param);
}


//935
static void EXEC_glPixelTransformParameterfEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glPixelTransformParameterfEXT(*target, *pname, *param);
}


//936
static void EXEC_glPixelTransformParameterivEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glPixelTransformParameterivEXT(*target, *pname, (const GLint *)popBuf());
}


//937
static void EXEC_glPixelTransformParameterfvEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glPixelTransformParameterfvEXT(*target, *pname, (const GLfloat *)popBuf());
}


//938
static void EXEC_glSecondaryColor3bEXT(byte *commandbuf)
{
	GLbyte *red = (GLbyte*)commandbuf;   commandbuf += sizeof(GLbyte);
	GLbyte *green = (GLbyte*)commandbuf;     commandbuf += sizeof(GLbyte);
	GLbyte *blue = (GLbyte*)commandbuf;  commandbuf += sizeof(GLbyte);

	glSecondaryColor3bEXT(*red, *green, *blue);
}


//939
static void EXEC_glSecondaryColor3bvEXT(byte *commandbuf)
{

	glSecondaryColor3bvEXT((const GLbyte *)popBuf());
}


//940
static void EXEC_glSecondaryColor3dEXT(byte *commandbuf)
{
	GLdouble *red = (GLdouble*)commandbuf;   commandbuf += sizeof(GLdouble);
	GLdouble *green = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *blue = (GLdouble*)commandbuf;  commandbuf += sizeof(GLdouble);

	glSecondaryColor3dEXT(*red, *green, *blue);
}


//941
static void EXEC_glSecondaryColor3dvEXT(byte *commandbuf)
{

	glSecondaryColor3dvEXT((const GLdouble *)popBuf());
}


//942
static void EXEC_glSecondaryColor3fEXT(byte *commandbuf)
{
	GLfloat *red = (GLfloat*)commandbuf;     commandbuf += sizeof(GLfloat);
	GLfloat *green = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *blue = (GLfloat*)commandbuf;    commandbuf += sizeof(GLfloat);

	glSecondaryColor3fEXT(*red, *green, *blue);
}


//943
static void EXEC_glSecondaryColor3fvEXT(byte *commandbuf)
{

	glSecondaryColor3fvEXT((const GLfloat *)popBuf());
}


//944
static void EXEC_glSecondaryColor3iEXT(byte *commandbuf)
{
	GLint *red = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *green = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *blue = (GLint*)commandbuf;    commandbuf += sizeof(GLint);

	glSecondaryColor3iEXT(*red, *green, *blue);
}


//945
static void EXEC_glSecondaryColor3ivEXT(byte *commandbuf)
{

	glSecondaryColor3ivEXT((const GLint *)popBuf());
}


//946
static void EXEC_glSecondaryColor3sEXT(byte *commandbuf)
{
	GLshort *red = (GLshort*)commandbuf;     commandbuf += sizeof(GLshort);
	GLshort *green = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *blue = (GLshort*)commandbuf;    commandbuf += sizeof(GLshort);

	glSecondaryColor3sEXT(*red, *green, *blue);
}


//947
static void EXEC_glSecondaryColor3svEXT(byte *commandbuf)
{

	glSecondaryColor3svEXT((const GLshort *)popBuf());
}


//948
static void EXEC_glSecondaryColor3ubEXT(byte *commandbuf)
{
	GLubyte *red = (GLubyte*)commandbuf;     commandbuf += sizeof(GLubyte);
	GLubyte *green = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *blue = (GLubyte*)commandbuf;    commandbuf += sizeof(GLubyte);

	glSecondaryColor3ubEXT(*red, *green, *blue);
}


//949
static void EXEC_glSecondaryColor3ubvEXT(byte *commandbuf)
{

	glSecondaryColor3ubvEXT((const GLubyte *)popBuf());
}


//950
static void EXEC_glSecondaryColor3uiEXT(byte *commandbuf)
{
	GLuint *red = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *green = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLuint *blue = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	glSecondaryColor3uiEXT(*red, *green, *blue);
}


//951
static void EXEC_glSecondaryColor3uivEXT(byte *commandbuf)
{

	glSecondaryColor3uivEXT((const GLuint *)popBuf());
}


//952
static void EXEC_glSecondaryColor3usEXT(byte *commandbuf)
{
	GLushort *red = (GLushort*)commandbuf;   commandbuf += sizeof(GLushort);
	GLushort *green = (GLushort*)commandbuf;     commandbuf += sizeof(GLushort);
	GLushort *blue = (GLushort*)commandbuf;  commandbuf += sizeof(GLushort);

	glSecondaryColor3usEXT(*red, *green, *blue);
}


//953
static void EXEC_glSecondaryColor3usvEXT(byte *commandbuf)
{

	glSecondaryColor3usvEXT((const GLushort *)popBuf());
}


//954
static void EXEC_glSecondaryColorPointerEXT(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	glSecondaryColorPointerEXT(*size, *type, *stride, (GLvoid *)popBuf());
}


//955
static void EXEC_glTextureNormalEXT(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glTextureNormalEXT(*mode);
}


//956
static void EXEC_glMultiDrawArraysEXT(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *primcount = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glMultiDrawArraysEXT(*mode, (GLint *)popBuf(), (GLsizei *)popBuf(), *primcount);
}


//957
static void EXEC_glMultiDrawElementsEXT(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *primcount = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glMultiDrawElementsEXT(*mode, (GLsizei *)popBuf(), *type, (const GLvoid **)popBuf(), *primcount);
}


//958
static void EXEC_glFogCoordfEXT(byte *commandbuf)
{
	GLfloat *coord = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glFogCoordfEXT(*coord);
}


//959
static void EXEC_glFogCoordfvEXT(byte *commandbuf)
{

	glFogCoordfvEXT((const GLfloat *)popBuf());
}


//960
static void EXEC_glFogCoorddEXT(byte *commandbuf)
{
	GLdouble *coord = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glFogCoorddEXT(*coord);
}


//961
static void EXEC_glFogCoorddvEXT(byte *commandbuf)
{

	glFogCoorddvEXT((const GLdouble *)popBuf());
}


//962
static void EXEC_glFogCoordPointerEXT(byte *commandbuf)
{
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	glFogCoordPointerEXT(*type, *stride, (const GLvoid *)popBuf());
}


//963
static void EXEC_glTangent3bEXT(byte *commandbuf)
{
	GLbyte *tx = (GLbyte*)commandbuf;    commandbuf += sizeof(GLbyte);
	GLbyte *ty = (GLbyte*)commandbuf;    commandbuf += sizeof(GLbyte);
	GLbyte *tz = (GLbyte*)commandbuf;    commandbuf += sizeof(GLbyte);

	//glTangent3bEXT(*tx, *ty, *tz);
}


//964
static void EXEC_glTangent3bvEXT(byte *commandbuf)
{

	//glTangent3bvEXT((const GLbyte *)popBuf());
}


//965
static void EXEC_glTangent3dEXT(byte *commandbuf)
{
	GLdouble *tx = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *ty = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *tz = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);

	//glTangent3dEXT(*tx, *ty, *tz);
}


//966
static void EXEC_glTangent3dvEXT(byte *commandbuf)
{

	//glTangent3dvEXT((const GLdouble *)popBuf());
}


//967
static void EXEC_glTangent3fEXT(byte *commandbuf)
{
	GLfloat *tx = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *ty = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *tz = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	//glTangent3fEXT(*tx, *ty, *tz);
}


//968
static void EXEC_glTangent3fvEXT(byte *commandbuf)
{

	//glTangent3fvEXT((const GLfloat *)popBuf());
}


//969
static void EXEC_glTangent3iEXT(byte *commandbuf)
{
	GLint *tx = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *ty = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *tz = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	//glTangent3iEXT(*tx, *ty, *tz);
}


//970
static void EXEC_glTangent3ivEXT(byte *commandbuf)
{

	//glTangent3ivEXT((const GLint *)popBuf());
}


//971
static void EXEC_glTangent3sEXT(byte *commandbuf)
{
	GLshort *tx = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);
	GLshort *ty = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);
	GLshort *tz = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);

	//glTangent3sEXT(*tx, *ty, *tz);
}


//972
static void EXEC_glTangent3svEXT(byte *commandbuf)
{

	//glTangent3svEXT((const GLshort *)popBuf());
}


//973
static void EXEC_glBinormal3bEXT(byte *commandbuf)
{
	GLbyte *bx = (GLbyte*)commandbuf;    commandbuf += sizeof(GLbyte);
	GLbyte *by = (GLbyte*)commandbuf;    commandbuf += sizeof(GLbyte);
	GLbyte *bz = (GLbyte*)commandbuf;    commandbuf += sizeof(GLbyte);

	//glBinormal3bEXT(*bx, *by, *bz);
}


//974
static void EXEC_glBinormal3bvEXT(byte *commandbuf)
{

	//glBinormal3bvEXT((const GLbyte *)popBuf());
}


//975
static void EXEC_glBinormal3dEXT(byte *commandbuf)
{
	GLdouble *bx = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *by = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *bz = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);

	//glBinormal3dEXT(*bx, *by, *bz);
}


//976
static void EXEC_glBinormal3dvEXT(byte *commandbuf)
{

	//glBinormal3dvEXT((const GLdouble *)popBuf());
}


//977
static void EXEC_glBinormal3fEXT(byte *commandbuf)
{
	GLfloat *bx = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *by = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *bz = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	//glBinormal3fEXT(*bx, *by, *bz);
}


//978
static void EXEC_glBinormal3fvEXT(byte *commandbuf)
{

	//glBinormal3fvEXT((const GLfloat *)popBuf());
}


//979
static void EXEC_glBinormal3iEXT(byte *commandbuf)
{
	GLint *bx = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *by = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *bz = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	//glBinormal3iEXT(*bx, *by, *bz);
}


//980
static void EXEC_glBinormal3ivEXT(byte *commandbuf)
{

	//glBinormal3ivEXT((const GLint *)popBuf());
}


//981
static void EXEC_glBinormal3sEXT(byte *commandbuf)
{
	GLshort *bx = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);
	GLshort *by = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);
	GLshort *bz = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);

	//glBinormal3sEXT(*bx, *by, *bz);
}


//982
static void EXEC_glBinormal3svEXT(byte *commandbuf)
{

	//glBinormal3svEXT((const GLshort *)popBuf());
}


//983
static void EXEC_glTangentPointerEXT(byte *commandbuf)
{
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	//glTangentPointerEXT(*type, *stride, (const GLvoid *)popBuf());
}


//984
static void EXEC_glBinormalPointerEXT(byte *commandbuf)
{
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	//glBinormalPointerEXT(*type, *stride, (const GLvoid *)popBuf());
}


//985
static void EXEC_glPixelTexGenSGIX(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glPixelTexGenSGIX(*mode);
}


//986
static void EXEC_glFinishTextureSUNX(byte *commandbuf)
{

	glFinishTextureSUNX();
}


//987
static void EXEC_glGlobalAlphaFactorbSUN(byte *commandbuf)
{
	GLbyte *factor = (GLbyte*)commandbuf;    commandbuf += sizeof(GLbyte);

	glGlobalAlphaFactorbSUN(*factor);
}


//988
static void EXEC_glGlobalAlphaFactorsSUN(byte *commandbuf)
{
	GLshort *factor = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);

	glGlobalAlphaFactorsSUN(*factor);
}


//989
static void EXEC_glGlobalAlphaFactoriSUN(byte *commandbuf)
{
	GLint *factor = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	glGlobalAlphaFactoriSUN(*factor);
}


//990
static void EXEC_glGlobalAlphaFactorfSUN(byte *commandbuf)
{
	GLfloat *factor = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	glGlobalAlphaFactorfSUN(*factor);
}


//991
static void EXEC_glGlobalAlphaFactordSUN(byte *commandbuf)
{
	GLdouble *factor = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);

	glGlobalAlphaFactordSUN(*factor);
}


//992
static void EXEC_glGlobalAlphaFactorubSUN(byte *commandbuf)
{
	GLubyte *factor = (GLubyte*)commandbuf;  commandbuf += sizeof(GLubyte);

	glGlobalAlphaFactorubSUN(*factor);
}


//993
static void EXEC_glGlobalAlphaFactorusSUN(byte *commandbuf)
{
	GLushort *factor = (GLushort*)commandbuf;    commandbuf += sizeof(GLushort);

	glGlobalAlphaFactorusSUN(*factor);
}


//994
static void EXEC_glGlobalAlphaFactoruiSUN(byte *commandbuf)
{
	GLuint *factor = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	glGlobalAlphaFactoruiSUN(*factor);
}


//995
static void EXEC_glReplacementCodeuiSUN(byte *commandbuf)
{
	GLuint *code = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	glReplacementCodeuiSUN(*code);
}


//996
static void EXEC_glReplacementCodeusSUN(byte *commandbuf)
{
	GLushort *code = (GLushort*)commandbuf;  commandbuf += sizeof(GLushort);

	glReplacementCodeusSUN(*code);
}


//997
static void EXEC_glReplacementCodeubSUN(byte *commandbuf)
{
	GLubyte *code = (GLubyte*)commandbuf;    commandbuf += sizeof(GLubyte);

	glReplacementCodeubSUN(*code);
}


//998
static void EXEC_glReplacementCodeuivSUN(byte *commandbuf)
{

	glReplacementCodeuivSUN((const GLuint *)popBuf());
}


//999
static void EXEC_glReplacementCodeusvSUN(byte *commandbuf)
{

	glReplacementCodeusvSUN((const GLushort *)popBuf());
}


//1000
static void EXEC_glReplacementCodeubvSUN(byte *commandbuf)
{

	glReplacementCodeubvSUN((const GLubyte *)popBuf());
}


//1001
static void EXEC_glReplacementCodePointerSUN(byte *commandbuf)
{
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	glReplacementCodePointerSUN(*type, *stride, (const GLvoid *)popBuf());
}


//1002
static void EXEC_glColor4ubVertex2fSUN(byte *commandbuf)
{
	GLubyte *r = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *g = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *b = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *a = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glColor4ubVertex2fSUN(*r, *g, *b, *a, *x, *y);
}


//1003
static void EXEC_glColor4ubVertex2fvSUN(byte *commandbuf)
{

	glColor4ubVertex2fvSUN((const GLubyte *)popBuf(), (const GLfloat *)popBuf());
}


//1004
static void EXEC_glColor4ubVertex3fSUN(byte *commandbuf)
{
	GLubyte *r = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *g = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *b = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *a = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glColor4ubVertex3fSUN(*r, *g, *b, *a, *x, *y, *z);
}


//1005
static void EXEC_glColor4ubVertex3fvSUN(byte *commandbuf)
{

	glColor4ubVertex3fvSUN((const GLubyte *)popBuf(), (const GLfloat *)popBuf());
}


//1006
static void EXEC_glColor3fVertex3fSUN(byte *commandbuf)
{
	GLfloat *r = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *g = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *b = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glColor3fVertex3fSUN(*r, *g, *b, *x, *y, *z);
}


//1007
static void EXEC_glColor3fVertex3fvSUN(byte *commandbuf)
{

	glColor3fVertex3fvSUN((const GLfloat *)popBuf(), (const GLfloat *)popBuf());
}


//1008
static void EXEC_glNormal3fVertex3fSUN(byte *commandbuf)
{
	GLfloat *nx = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *ny = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *nz = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glNormal3fVertex3fSUN(*nx, *ny, *nz, *x, *y, *z);
}


//1009
static void EXEC_glNormal3fVertex3fvSUN(byte *commandbuf)
{

	glNormal3fVertex3fvSUN((const GLfloat *)popBuf(), (const GLfloat *)popBuf());
}


//1010
static void EXEC_glColor4fNormal3fVertex3fSUN(byte *commandbuf)
{
	GLfloat *r = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *g = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *b = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *a = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *nx = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *ny = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *nz = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glColor4fNormal3fVertex3fSUN(*r, *g, *b, *a, *nx, *ny, *nz, *x, *y, *z);
}


//1011
static void EXEC_glColor4fNormal3fVertex3fvSUN(byte *commandbuf)
{

	glColor4fNormal3fVertex3fvSUN((const GLfloat *)popBuf(), (const GLfloat *)popBuf(), (const GLfloat *)popBuf());
}


//1012
static void EXEC_glTexCoord2fVertex3fSUN(byte *commandbuf)
{
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glTexCoord2fVertex3fSUN(*s, *t, *x, *y, *z);
}


//1013
static void EXEC_glTexCoord2fVertex3fvSUN(byte *commandbuf)
{

	glTexCoord2fVertex3fvSUN((const GLfloat *)popBuf(), (const GLfloat *)popBuf());
}


//1014
static void EXEC_glTexCoord4fVertex4fSUN(byte *commandbuf)
{
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *p = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *q = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *w = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glTexCoord4fVertex4fSUN(*s, *t, *p, *q, *x, *y, *z, *w);
}


//1015
static void EXEC_glTexCoord4fVertex4fvSUN(byte *commandbuf)
{

	glTexCoord4fVertex4fvSUN((const GLfloat *)popBuf(), (const GLfloat *)popBuf());
}


//1016
static void EXEC_glTexCoord2fColor4ubVertex3fSUN(byte *commandbuf)
{
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLubyte *r = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *g = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *b = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *a = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glTexCoord2fColor4ubVertex3fSUN(*s, *t, *r, *g, *b, *a, *x, *y, *z);
}


//1017
static void EXEC_glTexCoord2fColor4ubVertex3fvSUN(byte *commandbuf)
{

	glTexCoord2fColor4ubVertex3fvSUN((const GLfloat *)popBuf(), (const GLubyte *)popBuf(), (const GLfloat *)popBuf());
}


//1018
static void EXEC_glTexCoord2fColor3fVertex3fSUN(byte *commandbuf)
{
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *r = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *g = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *b = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glTexCoord2fColor3fVertex3fSUN(*s, *t, *r, *g, *b, *x, *y, *z);
}


//1019
static void EXEC_glTexCoord2fColor3fVertex3fvSUN(byte *commandbuf)
{

	glTexCoord2fColor3fVertex3fvSUN((const GLfloat *)popBuf(), (const GLfloat *)popBuf(), (const GLfloat *)popBuf());
}


//1020
static void EXEC_glTexCoord2fNormal3fVertex3fSUN(byte *commandbuf)
{
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *nx = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *ny = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *nz = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glTexCoord2fNormal3fVertex3fSUN(*s, *t, *nx, *ny, *nz, *x, *y, *z);
}


//1021
static void EXEC_glTexCoord2fNormal3fVertex3fvSUN(byte *commandbuf)
{

	glTexCoord2fNormal3fVertex3fvSUN((const GLfloat *)popBuf(), (const GLfloat *)popBuf(), (const GLfloat *)popBuf());
}


//1022
static void EXEC_glTexCoord2fColor4fNormal3fVertex3fSUN(byte *commandbuf)
{
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *r = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *g = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *b = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *a = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *nx = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *ny = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *nz = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glTexCoord2fColor4fNormal3fVertex3fSUN(*s, *t, *r, *g, *b, *a, *nx, *ny, *nz, *x, *y, *z);
}


//1023
static void EXEC_glTexCoord2fColor4fNormal3fVertex3fvSUN(byte *commandbuf)
{

	glTexCoord2fColor4fNormal3fVertex3fvSUN((const GLfloat *)popBuf(), (const GLfloat *)popBuf(), (const GLfloat *)popBuf(), (const GLfloat *)popBuf());
}


//1024
static void EXEC_glTexCoord4fColor4fNormal3fVertex4fSUN(byte *commandbuf)
{
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *p = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *q = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *r = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *g = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *b = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *a = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *nx = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *ny = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *nz = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *w = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glTexCoord4fColor4fNormal3fVertex4fSUN(*s, *t, *p, *q, *r, *g, *b, *a, *nx, *ny, *nz, *x, *y, *z, *w);
}


//1025
static void EXEC_glTexCoord4fColor4fNormal3fVertex4fvSUN(byte *commandbuf)
{

	glTexCoord4fColor4fNormal3fVertex4fvSUN((const GLfloat *)popBuf(), (const GLfloat *)popBuf(), (const GLfloat *)popBuf(), (const GLfloat *)popBuf());
}


//1026
static void EXEC_glReplacementCodeuiVertex3fSUN(byte *commandbuf)
{
	GLuint *rc = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glReplacementCodeuiVertex3fSUN(*rc, *x, *y, *z);
}


//1027
static void EXEC_glReplacementCodeuiVertex3fvSUN(byte *commandbuf)
{

	glReplacementCodeuiVertex3fvSUN((const GLuint *)popBuf(), (const GLfloat *)popBuf());
}


//1028
static void EXEC_glReplacementCodeuiColor4ubVertex3fSUN(byte *commandbuf)
{
	GLuint *rc = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLubyte *r = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *g = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *b = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *a = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glReplacementCodeuiColor4ubVertex3fSUN(*rc, *r, *g, *b, *a, *x, *y, *z);
}


//1029
static void EXEC_glReplacementCodeuiColor4ubVertex3fvSUN(byte *commandbuf)
{

	glReplacementCodeuiColor4ubVertex3fvSUN((const GLuint *)popBuf(), (const GLubyte *)popBuf(), (const GLfloat *)popBuf());
}


//1030
static void EXEC_glReplacementCodeuiColor3fVertex3fSUN(byte *commandbuf)
{
	GLuint *rc = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLfloat *r = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *g = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *b = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glReplacementCodeuiColor3fVertex3fSUN(*rc, *r, *g, *b, *x, *y, *z);
}


//1031
static void EXEC_glReplacementCodeuiColor3fVertex3fvSUN(byte *commandbuf)
{

	glReplacementCodeuiColor3fVertex3fvSUN((const GLuint *)popBuf(), (const GLfloat *)popBuf(), (const GLfloat *)popBuf());
}


//1032
static void EXEC_glReplacementCodeuiNormal3fVertex3fSUN(byte *commandbuf)
{
	GLuint *rc = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLfloat *nx = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *ny = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *nz = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glReplacementCodeuiNormal3fVertex3fSUN(*rc, *nx, *ny, *nz, *x, *y, *z);
}


//1033
static void EXEC_glReplacementCodeuiNormal3fVertex3fvSUN(byte *commandbuf)
{

	glReplacementCodeuiNormal3fVertex3fvSUN((const GLuint *)popBuf(), (const GLfloat *)popBuf(), (const GLfloat *)popBuf());
}


//1034
static void EXEC_glReplacementCodeuiColor4fNormal3fVertex3fSUN(byte *commandbuf)
{
	GLuint *rc = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLfloat *r = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *g = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *b = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *a = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *nx = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *ny = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *nz = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glReplacementCodeuiColor4fNormal3fVertex3fSUN(*rc, *r, *g, *b, *a, *nx, *ny, *nz, *x, *y, *z);
}


//1035
static void EXEC_glReplacementCodeuiColor4fNormal3fVertex3fvSUN(byte *commandbuf)
{

	glReplacementCodeuiColor4fNormal3fVertex3fvSUN((const GLuint *)popBuf(), (const GLfloat *)popBuf(), (const GLfloat *)popBuf(), (const GLfloat *)popBuf());
}


//1036
static void EXEC_glReplacementCodeuiTexCoord2fVertex3fSUN(byte *commandbuf)
{
	GLuint *rc = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glReplacementCodeuiTexCoord2fVertex3fSUN(*rc, *s, *t, *x, *y, *z);
}


//1037
static void EXEC_glReplacementCodeuiTexCoord2fVertex3fvSUN(byte *commandbuf)
{

	glReplacementCodeuiTexCoord2fVertex3fvSUN((const GLuint *)popBuf(), (const GLfloat *)popBuf(), (const GLfloat *)popBuf());
}


//1038
static void EXEC_glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN(byte *commandbuf)
{
	GLuint *rc = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *nx = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *ny = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *nz = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN(*rc, *s, *t, *nx, *ny, *nz, *x, *y, *z);
}


//1039
static void EXEC_glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN(byte *commandbuf)
{

	glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN((const GLuint *)popBuf(), (const GLfloat *)popBuf(), (const GLfloat *)popBuf(), (const GLfloat *)popBuf());
}


//1040
static void EXEC_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN(byte *commandbuf)
{
	GLuint *rc = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *r = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *g = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *b = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *a = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *nx = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *ny = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *nz = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN(*rc, *s, *t, *r, *g, *b, *a, *nx, *ny, *nz, *x, *y, *z);
}


//1041
static void EXEC_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN(byte *commandbuf)
{

	glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN((const GLuint *)popBuf(), (const GLfloat *)popBuf(), (const GLfloat *)popBuf(), (const GLfloat *)popBuf(), (const GLfloat *)popBuf());
}


//1042
static void EXEC_glBlendFuncSeparateEXT(byte *commandbuf)
{
	GLenum *sfactorRGB = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *dfactorRGB = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *sfactorAlpha = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *dfactorAlpha = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glBlendFuncSeparateEXT(*sfactorRGB, *dfactorRGB, *sfactorAlpha, *dfactorAlpha);
}


//1043
static void EXEC_glVertexWeightfEXT(byte *commandbuf)
{
	GLfloat *weight = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	glVertexWeightfEXT(*weight);
}


//1044
static void EXEC_glVertexWeightfvEXT(byte *commandbuf)
{

	glVertexWeightfvEXT((GLfloat *)popBuf());
}


//1045
static void EXEC_glVertexWeightPointerEXT(byte *commandbuf)
{
	GLsizei *size = (GLsizei*)commandbuf;    commandbuf += sizeof(GLsizei);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	glVertexWeightPointerEXT(*size, *type, *stride, (GLvoid *)popBuf());
}


//1046
static void EXEC_glFlushVertexArrayRangeNV(byte *commandbuf)
{

	glFlushVertexArrayRangeNV();
}


//1047
static void EXEC_glVertexArrayRangeNV(byte *commandbuf)
{
	GLsizei *length = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	glVertexArrayRangeNV(*length, (GLvoid *)popBuf());
}


//1048
static void EXEC_glCombinerParameterfvNV(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glCombinerParameterfvNV(*pname, (const GLfloat *)popBuf());
}


//1049
static void EXEC_glCombinerParameterfNV(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glCombinerParameterfNV(*pname, *param);
}


//1050
static void EXEC_glCombinerParameterivNV(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glCombinerParameterivNV(*pname, (const GLint *)popBuf());
}


//1051
static void EXEC_glCombinerParameteriNV(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glCombinerParameteriNV(*pname, *param);
}


//1052
static void EXEC_glCombinerInputNV(byte *commandbuf)
{
	GLenum *stage = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *portion = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLenum *variable = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *input = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *mapping = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLenum *componentUsage = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glCombinerInputNV(*stage, *portion, *variable, *input, *mapping, *componentUsage);
}


//1053
static void EXEC_glCombinerOutputNV(byte *commandbuf)
{
	GLenum *stage = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *portion = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLenum *abOutput = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *cdOutput = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *sumOutput = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *scale = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *bias = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLboolean *abDotProduct = (GLboolean*)commandbuf;    commandbuf += sizeof(GLboolean);
	GLboolean *cdDotProduct = (GLboolean*)commandbuf;    commandbuf += sizeof(GLboolean);
	GLboolean *muxSum = (GLboolean*)commandbuf;  commandbuf += sizeof(GLboolean);

	glCombinerOutputNV(*stage, *portion, *abOutput, *cdOutput, *sumOutput, *scale, *bias, *abDotProduct, *cdDotProduct, *muxSum);
}


//1054
static void EXEC_glFinalCombinerInputNV(byte *commandbuf)
{
	GLenum *variable = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *input = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *mapping = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLenum *componentUsage = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glFinalCombinerInputNV(*variable, *input, *mapping, *componentUsage);
}


//1055
static void EXEC_glGetCombinerInputParameterfvNV(byte *commandbuf)
{
	GLenum *stage = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *portion = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLenum *variable = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetCombinerInputParameterfvNV(*stage, *portion, *variable, *pname, (GLfloat *)popBuf());
}


//1056
static void EXEC_glGetCombinerInputParameterivNV(byte *commandbuf)
{
	GLenum *stage = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *portion = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLenum *variable = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetCombinerInputParameterivNV(*stage, *portion, *variable, *pname, (GLint *)popBuf());
}


//1057
static void EXEC_glGetCombinerOutputParameterfvNV(byte *commandbuf)
{
	GLenum *stage = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *portion = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetCombinerOutputParameterfvNV(*stage, *portion, *pname, (GLfloat *)popBuf());
}


//1058
static void EXEC_glGetCombinerOutputParameterivNV(byte *commandbuf)
{
	GLenum *stage = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *portion = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetCombinerOutputParameterivNV(*stage, *portion, *pname, (GLint *)popBuf());
}


//1059
static void EXEC_glGetFinalCombinerInputParameterfvNV(byte *commandbuf)
{
	GLenum *variable = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetFinalCombinerInputParameterfvNV(*variable, *pname, (GLfloat *)popBuf());
}


//1060
static void EXEC_glGetFinalCombinerInputParameterivNV(byte *commandbuf)
{
	GLenum *variable = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetFinalCombinerInputParameterivNV(*variable, *pname, (GLint *)popBuf());
}


//1061
static void EXEC_glResizeBuffersMESA(byte *commandbuf)
{

	glResizeBuffersMESA();
}


//1062
static void EXEC_glWindowPos2dMESA(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glWindowPos2dMESA(*x, *y);
}


//1063
static void EXEC_glWindowPos2dvMESA(byte *commandbuf)
{

	glWindowPos2dvMESA((const GLdouble *)popBuf());
}


//1064
static void EXEC_glWindowPos2fMESA(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glWindowPos2fMESA(*x, *y);
}


//1065
static void EXEC_glWindowPos2fvMESA(byte *commandbuf)
{

	glWindowPos2fvMESA((const GLfloat *)popBuf());
}


//1066
static void EXEC_glWindowPos2iMESA(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glWindowPos2iMESA(*x, *y);
}


//1067
static void EXEC_glWindowPos2ivMESA(byte *commandbuf)
{

	glWindowPos2ivMESA((const GLint *)popBuf());
}


//1068
static void EXEC_glWindowPos2sMESA(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glWindowPos2sMESA(*x, *y);
}


//1069
static void EXEC_glWindowPos2svMESA(byte *commandbuf)
{

	glWindowPos2svMESA((const GLshort *)popBuf());
}


//1070
static void EXEC_glWindowPos3dMESA(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glWindowPos3dMESA(*x, *y, *z);
}


//1071
static void EXEC_glWindowPos3dvMESA(byte *commandbuf)
{

	glWindowPos3dvMESA((const GLdouble *)popBuf());
}


//1072
static void EXEC_glWindowPos3fMESA(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glWindowPos3fMESA(*x, *y, *z);
}


//1073
static void EXEC_glWindowPos3fvMESA(byte *commandbuf)
{

	glWindowPos3fvMESA((const GLfloat *)popBuf());
}


//1074
static void EXEC_glWindowPos3iMESA(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *z = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glWindowPos3iMESA(*x, *y, *z);
}


//1075
static void EXEC_glWindowPos3ivMESA(byte *commandbuf)
{

	glWindowPos3ivMESA((const GLint *)popBuf());
}


//1076
static void EXEC_glWindowPos3sMESA(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glWindowPos3sMESA(*x, *y, *z);
}


//1077
static void EXEC_glWindowPos3svMESA(byte *commandbuf)
{

	glWindowPos3svMESA((const GLshort *)popBuf());
}


//1078
static void EXEC_glWindowPos4dMESA(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *w = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glWindowPos4dMESA(*x, *y, *z, *w);
}


//1079
static void EXEC_glWindowPos4dvMESA(byte *commandbuf)
{

	glWindowPos4dvMESA((const GLdouble *)popBuf());
}


//1080
static void EXEC_glWindowPos4fMESA(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *w = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glWindowPos4fMESA(*x, *y, *z, *w);
}


//1081
static void EXEC_glWindowPos4fvMESA(byte *commandbuf)
{

	glWindowPos4fvMESA((const GLfloat *)popBuf());
}


//1082
static void EXEC_glWindowPos4iMESA(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *z = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *w = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glWindowPos4iMESA(*x, *y, *z, *w);
}


//1083
static void EXEC_glWindowPos4ivMESA(byte *commandbuf)
{

	glWindowPos4ivMESA((const GLint *)popBuf());
}


//1084
static void EXEC_glWindowPos4sMESA(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *w = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glWindowPos4sMESA(*x, *y, *z, *w);
}


//1085
static void EXEC_glWindowPos4svMESA(byte *commandbuf)
{

	glWindowPos4svMESA((const GLshort *)popBuf());
}


//1086
static void EXEC_glMultiModeDrawArraysIBM(byte *commandbuf)
{
	GLsizei *primcount = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLint *modestride = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	glMultiModeDrawArraysIBM((const GLenum *)popBuf(), (const GLint *)popBuf(), (const GLsizei *)popBuf(), *primcount, *modestride);
}


//1087
static void EXEC_glMultiModeDrawElementsIBM(byte *commandbuf)
{
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *primcount = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLint *modestride = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	glMultiModeDrawElementsIBM((const GLenum *)popBuf(), (const GLsizei *)popBuf(), *type, (const GLvoid * const *)popBuf(), *primcount, *modestride);
}


//1088
static void EXEC_glColorPointerListIBM(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLint *stride = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *ptrstride = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glColorPointerListIBM(*size, *type, *stride, (const GLvoid **)popBuf(), *ptrstride);
}


//1089
static void EXEC_glSecondaryColorPointerListIBM(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLint *stride = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *ptrstride = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glSecondaryColorPointerListIBM(*size, *type, *stride, (const GLvoid **)popBuf(), *ptrstride);
}


//1090
static void EXEC_glEdgeFlagPointerListIBM(byte *commandbuf)
{
	GLint *stride = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *ptrstride = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glEdgeFlagPointerListIBM(*stride, (const GLboolean **)popBuf(), *ptrstride);
}


//1091
static void EXEC_glFogCoordPointerListIBM(byte *commandbuf)
{
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLint *stride = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *ptrstride = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glFogCoordPointerListIBM(*type, *stride, (const GLvoid **)popBuf(), *ptrstride);
}


//1092
static void EXEC_glIndexPointerListIBM(byte *commandbuf)
{
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLint *stride = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *ptrstride = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glIndexPointerListIBM(*type, *stride, (const GLvoid **)popBuf(), *ptrstride);
}


//1093
static void EXEC_glNormalPointerListIBM(byte *commandbuf)
{
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLint *stride = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *ptrstride = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glNormalPointerListIBM(*type, *stride, (const GLvoid **)popBuf(), *ptrstride);
}


//1094
static void EXEC_glTexCoordPointerListIBM(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLint *stride = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *ptrstride = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glTexCoordPointerListIBM(*size, *type, *stride, (const GLvoid **)popBuf(), *ptrstride);
}


//1095
static void EXEC_glVertexPointerListIBM(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLint *stride = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *ptrstride = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glVertexPointerListIBM(*size, *type, *stride, (const GLvoid **)popBuf(), *ptrstride);
}


//1096
static void EXEC_glTbufferMask3DFX(byte *commandbuf)
{
	GLuint *mask = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	glTbufferMask3DFX(*mask);
}


//1097
static void EXEC_glSampleMaskEXT(byte *commandbuf)
{
	GLclampf *value = (GLclampf*)commandbuf;     commandbuf += sizeof(GLclampf);
	GLboolean *invert = (GLboolean*)commandbuf;  commandbuf += sizeof(GLboolean);

	glSampleMaskEXT(*value, *invert);
}


//1098
static void EXEC_glSamplePatternEXT(byte *commandbuf)
{
	GLenum *pattern = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	glSamplePatternEXT(*pattern);
}


//1099
static void EXEC_glTextureColorMaskSGIS(byte *commandbuf)
{
	GLboolean *red = (GLboolean*)commandbuf;     commandbuf += sizeof(GLboolean);
	GLboolean *green = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);
	GLboolean *blue = (GLboolean*)commandbuf;    commandbuf += sizeof(GLboolean);
	GLboolean *alpha = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);

	//glTextureColorMaskSGIS(*red, *green, *blue, *alpha);
}


//1100
static void EXEC_glDeleteFencesNV(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glDeleteFencesNV(*n, (const GLuint *)popBuf());
}


//1101
static void EXEC_glGenFencesNV(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glGenFencesNV(*n, (GLuint *)popBuf());
}


//1102
static void EXEC_glIsFenceNV(byte *commandbuf)
{
	GLuint *fence = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	pushRet(glIsFenceNV(*fence));
}


//1103
static void EXEC_glTestFenceNV(byte *commandbuf)
{
	GLuint *fence = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	pushRet(glTestFenceNV(*fence));
}


//1104
static void EXEC_glGetFenceivNV(byte *commandbuf)
{
	GLuint *fence = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetFenceivNV(*fence, *pname, (GLint *)popBuf());
}


//1105
static void EXEC_glFinishFenceNV(byte *commandbuf)
{
	GLuint *fence = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glFinishFenceNV(*fence);
}


//1106
static void EXEC_glSetFenceNV(byte *commandbuf)
{
	GLuint *fence = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLenum *condition = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glSetFenceNV(*fence, *condition);
}


//1107
static void EXEC_glMapControlPointsNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *ustride = (GLsizei*)commandbuf;     commandbuf += sizeof(GLsizei);
	GLsizei *vstride = (GLsizei*)commandbuf;     commandbuf += sizeof(GLsizei);
	GLint *uorder = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *vorder = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLboolean *packed = (GLboolean*)commandbuf;  commandbuf += sizeof(GLboolean);

	glMapControlPointsNV(*target, *index, *type, *ustride, *vstride, *uorder, *vorder, *packed, (const GLvoid *)popBuf());
}


//1108
static void EXEC_glMapParameterivNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glMapParameterivNV(*target, *pname, (const GLint *)popBuf());
}


//1109
static void EXEC_glMapParameterfvNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glMapParameterfvNV(*target, *pname, (const GLfloat *)popBuf());
}


//1110
static void EXEC_glGetMapControlPointsNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *ustride = (GLsizei*)commandbuf;     commandbuf += sizeof(GLsizei);
	GLsizei *vstride = (GLsizei*)commandbuf;     commandbuf += sizeof(GLsizei);
	GLboolean *packed = (GLboolean*)commandbuf;  commandbuf += sizeof(GLboolean);

	glGetMapControlPointsNV(*target, *index, *type, *ustride, *vstride, *packed, (GLvoid *)popBuf());
}


//1111
static void EXEC_glGetMapParameterivNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetMapParameterivNV(*target, *pname, (GLint *)popBuf());
}


//1112
static void EXEC_glGetMapParameterfvNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetMapParameterfvNV(*target, *pname, (GLfloat *)popBuf());
}


//1113
static void EXEC_glGetMapAttribParameterivNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetMapAttribParameterivNV(*target, *index, *pname, (GLint *)popBuf());
}


//1114
static void EXEC_glGetMapAttribParameterfvNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetMapAttribParameterfvNV(*target, *index, *pname, (GLfloat *)popBuf());
}


//1115
static void EXEC_glEvalMapsNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glEvalMapsNV(*target, *mode);
}


//1116
static void EXEC_glCombinerStageParameterfvNV(byte *commandbuf)
{
	GLenum *stage = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glCombinerStageParameterfvNV(*stage, *pname, (const GLfloat *)popBuf());
}


//1117
static void EXEC_glGetCombinerStageParameterfvNV(byte *commandbuf)
{
	GLenum *stage = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetCombinerStageParameterfvNV(*stage, *pname, (GLfloat *)popBuf());
}


//1118
static void EXEC_glAreProgramsResidentNV(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	pushRet(glAreProgramsResidentNV(*n, (const GLuint *)popBuf(), (GLboolean *)popBuf()));
}


//1119
static void EXEC_glBindProgramNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	glBindProgramNV(*target, *program);
}


//1120
static void EXEC_glDeleteProgramsNV(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glDeleteProgramsNV(*n, (const GLuint *)popBuf());
}


//1121
static void EXEC_glExecuteProgramNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	glExecuteProgramNV(*target, *id, (const GLfloat *)popBuf());
}


//1122
static void EXEC_glGenProgramsNV(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glGenProgramsNV(*n, (GLuint *)popBuf());
}


//1123
static void EXEC_glGetProgramParameterdvNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetProgramParameterdvNV(*target, *index, *pname, (GLdouble *)popBuf());
}


//1124
static void EXEC_glGetProgramParameterfvNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetProgramParameterfvNV(*target, *index, *pname, (GLfloat *)popBuf());
}


//1125
static void EXEC_glGetProgramivNV(byte *commandbuf)
{
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetProgramivNV(*id, *pname, (GLint *)popBuf());
}


//1126
static void EXEC_glGetProgramStringNV(byte *commandbuf)
{
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetProgramStringNV(*id, *pname, (GLubyte *)popBuf());
}


//1127
static void EXEC_glGetTrackMatrixivNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *address = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetTrackMatrixivNV(*target, *address, *pname, (GLint *)popBuf());
}


//1128
static void EXEC_glGetVertexAttribdvNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetVertexAttribdvNV(*index, *pname, (GLdouble *)popBuf());
}


//1129
static void EXEC_glGetVertexAttribfvNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetVertexAttribfvNV(*index, *pname, (GLfloat *)popBuf());
}


//1130
static void EXEC_glGetVertexAttribivNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetVertexAttribivNV(*index, *pname, (GLint *)popBuf());
}


//1131
static void EXEC_glGetVertexAttribPointervNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glGetVertexAttribPointervNV(*index, *pname, (GLvoid **)popBuf());
}


//1132
static void EXEC_glIsProgramNV(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	pushRet(glIsProgramNV(*program));
}


//1133
static void EXEC_glLoadProgramNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLsizei *len = (GLsizei*)commandbuf;     commandbuf += sizeof(GLsizei);

	glLoadProgramNV(*target, *id, *len, (const GLubyte *)popBuf());
}


//1134
static void EXEC_glProgramParameter4dNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *w = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glProgramParameter4dNV(*target, *index, *x, *y, *z, *w);
}


//1135
static void EXEC_glProgramParameter4dvNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glProgramParameter4dvNV(*target, *index, (const GLdouble *)popBuf());
}


//1136
static void EXEC_glProgramParameter4fNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *w = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glProgramParameter4fNV(*target, *index, *x, *y, *z, *w);
}


//1137
static void EXEC_glProgramParameter4fvNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glProgramParameter4fvNV(*target, *index, (const GLfloat *)popBuf());
}


//1138
static void EXEC_glProgramParameters4dvNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLuint *num = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	glProgramParameters4dvNV(*target, *index, *num, (const GLdouble *)popBuf());
}


//1139
static void EXEC_glProgramParameters4fvNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLuint *num = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	glProgramParameters4fvNV(*target, *index, *num, (const GLfloat *)popBuf());
}


//1140
static void EXEC_glRequestResidentProgramsNV(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glRequestResidentProgramsNV(*n, (GLuint *)popBuf());
}


//1141
static void EXEC_glTrackMatrixNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *address = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLenum *matrix = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *transform = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glTrackMatrixNV(*target, *address, *matrix, *transform);
}


//1142
static void EXEC_glVertexAttribPointerNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	glVertexAttribPointerNV(*index, *size, *type, *stride, (const GLvoid *)popBuf());
}


//1143
static void EXEC_glVertexAttrib1sNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glVertexAttrib1sNV(*index, *x);
}


//1144
static void EXEC_glVertexAttrib1svNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib1svNV(*index, (const GLshort *)popBuf());
}


//1145
static void EXEC_glVertexAttrib2sNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glVertexAttrib2sNV(*index, *x, *y);
}


//1146
static void EXEC_glVertexAttrib2svNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib2svNV(*index, (const GLshort *)popBuf());
}


//1147
static void EXEC_glVertexAttrib3sNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glVertexAttrib3sNV(*index, *x, *y, *z);
}


//1148
static void EXEC_glVertexAttrib3svNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib3svNV(*index, (const GLshort *)popBuf());
}


//1149
static void EXEC_glVertexAttrib4sNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *w = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	glVertexAttrib4sNV(*index, *x, *y, *z, *w);
}


//1150
static void EXEC_glVertexAttrib4svNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4svNV(*index, (const GLshort *)popBuf());
}


//1151
static void EXEC_glVertexAttrib1fNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glVertexAttrib1fNV(*index, *x);
}


//1152
static void EXEC_glVertexAttrib1fvNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib1fvNV(*index, (const GLfloat *)popBuf());
}


//1153
static void EXEC_glVertexAttrib2fNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glVertexAttrib2fNV(*index, *x, *y);
}


//1154
static void EXEC_glVertexAttrib2fvNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib2fvNV(*index, (const GLfloat *)popBuf());
}


//1155
static void EXEC_glVertexAttrib3fNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glVertexAttrib3fNV(*index, *x, *y, *z);
}


//1156
static void EXEC_glVertexAttrib3fvNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib3fvNV(*index, (const GLfloat *)popBuf());
}


//1157
static void EXEC_glVertexAttrib4fNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *w = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glVertexAttrib4fNV(*index, *x, *y, *z, *w);
}


//1158
static void EXEC_glVertexAttrib4fvNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4fvNV(*index, (const GLfloat *)popBuf());
}


//1159
static void EXEC_glVertexAttrib1dNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glVertexAttrib1dNV(*index, *x);
}


//1160
static void EXEC_glVertexAttrib1dvNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib1dvNV(*index, (const GLdouble *)popBuf());
}


//1161
static void EXEC_glVertexAttrib2dNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glVertexAttrib2dNV(*index, *x, *y);
}


//1162
static void EXEC_glVertexAttrib2dvNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib2dvNV(*index, (const GLdouble *)popBuf());
}


//1163
static void EXEC_glVertexAttrib3dNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glVertexAttrib3dNV(*index, *x, *y, *z);
}


//1164
static void EXEC_glVertexAttrib3dvNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib3dvNV(*index, (const GLdouble *)popBuf());
}


//1165
static void EXEC_glVertexAttrib4dNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *w = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glVertexAttrib4dNV(*index, *x, *y, *z, *w);
}


//1166
static void EXEC_glVertexAttrib4dvNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4dvNV(*index, (const GLdouble *)popBuf());
}


//1167
static void EXEC_glVertexAttrib4ubNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLubyte *x = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *y = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *z = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *w = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);

	glVertexAttrib4ubNV(*index, *x, *y, *z, *w);
}


//1168
static void EXEC_glVertexAttrib4ubvNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	glVertexAttrib4ubvNV(*index, (const GLubyte *)popBuf());
}


//1169
static void EXEC_glVertexAttribs1svNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glVertexAttribs1svNV(*index, *n, (const GLshort *)popBuf());
}


//1170
static void EXEC_glVertexAttribs2svNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glVertexAttribs2svNV(*index, *n, (const GLshort *)popBuf());
}


//1171
static void EXEC_glVertexAttribs3svNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glVertexAttribs3svNV(*index, *n, (const GLshort *)popBuf());
}


//1172
static void EXEC_glVertexAttribs4svNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glVertexAttribs4svNV(*index, *n, (const GLshort *)popBuf());
}


//1173
static void EXEC_glVertexAttribs1fvNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glVertexAttribs1fvNV(*index, *n, (const GLfloat *)popBuf());
}


//1174
static void EXEC_glVertexAttribs2fvNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glVertexAttribs2fvNV(*index, *n, (const GLfloat *)popBuf());
}


//1175
static void EXEC_glVertexAttribs3fvNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glVertexAttribs3fvNV(*index, *n, (const GLfloat *)popBuf());
}


//1176
static void EXEC_glVertexAttribs4fvNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glVertexAttribs4fvNV(*index, *n, (const GLfloat *)popBuf());
}


//1177
static void EXEC_glVertexAttribs1dvNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glVertexAttribs1dvNV(*index, *n, (const GLdouble *)popBuf());
}


//1178
static void EXEC_glVertexAttribs2dvNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glVertexAttribs2dvNV(*index, *n, (const GLdouble *)popBuf());
}


//1179
static void EXEC_glVertexAttribs3dvNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glVertexAttribs3dvNV(*index, *n, (const GLdouble *)popBuf());
}


//1180
static void EXEC_glVertexAttribs4dvNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glVertexAttribs4dvNV(*index, *n, (const GLdouble *)popBuf());
}


//1181
static void EXEC_glVertexAttribs4ubvNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glVertexAttribs4ubvNV(*index, *n, (const GLubyte *)popBuf());
}


//1182
static void EXEC_glGenFragmentShadersATI(byte *commandbuf)
{
	GLuint *range = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	pushRet(glGenFragmentShadersATI(*range));
}


//1183
static void EXEC_glBindFragmentShaderATI(byte *commandbuf)
{
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	glBindFragmentShaderATI(*id);
}


//1184
static void EXEC_glDeleteFragmentShaderATI(byte *commandbuf)
{
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	glDeleteFragmentShaderATI(*id);
}


//1185
static void EXEC_glBeginFragmentShaderATI(byte *commandbuf)
{

	glBeginFragmentShaderATI();
}


//1186
static void EXEC_glEndFragmentShaderATI(byte *commandbuf)
{

	glEndFragmentShaderATI();
}


//1187
static void EXEC_glPassTexCoordATI(byte *commandbuf)
{
	GLuint *dst = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *coord = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLenum *swizzle = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	glPassTexCoordATI(*dst, *coord, *swizzle);
}


//1188
static void EXEC_glSampleMapATI(byte *commandbuf)
{
	GLuint *dst = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *interp = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLenum *swizzle = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	glSampleMapATI(*dst, *interp, *swizzle);
}


//1189
static void EXEC_glColorFragmentOp1ATI(byte *commandbuf)
{
	GLenum *op = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *dst = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *dstMask = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *dstMod = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLuint *arg1 = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLuint *arg1Rep = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *arg1Mod = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	glColorFragmentOp1ATI(*op, *dst, *dstMask, *dstMod, *arg1, *arg1Rep, *arg1Mod);
}


//1190
static void EXEC_glColorFragmentOp2ATI(byte *commandbuf)
{
	GLenum *op = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *dst = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *dstMask = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *dstMod = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLuint *arg1 = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLuint *arg1Rep = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *arg1Mod = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *arg2 = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLuint *arg2Rep = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *arg2Mod = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	glColorFragmentOp2ATI(*op, *dst, *dstMask, *dstMod, *arg1, *arg1Rep, *arg1Mod, *arg2, *arg2Rep, *arg2Mod);
}


//1191
static void EXEC_glColorFragmentOp3ATI(byte *commandbuf)
{
	GLenum *op = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *dst = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *dstMask = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *dstMod = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLuint *arg1 = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLuint *arg1Rep = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *arg1Mod = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *arg2 = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLuint *arg2Rep = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *arg2Mod = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *arg3 = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLuint *arg3Rep = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *arg3Mod = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	glColorFragmentOp3ATI(*op, *dst, *dstMask, *dstMod, *arg1, *arg1Rep, *arg1Mod, *arg2, *arg2Rep, *arg2Mod, *arg3, *arg3Rep, *arg3Mod);
}


//1192
static void EXEC_glAlphaFragmentOp1ATI(byte *commandbuf)
{
	GLenum *op = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *dst = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *dstMod = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLuint *arg1 = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLuint *arg1Rep = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *arg1Mod = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	glAlphaFragmentOp1ATI(*op, *dst, *dstMod, *arg1, *arg1Rep, *arg1Mod);
}


//1193
static void EXEC_glAlphaFragmentOp2ATI(byte *commandbuf)
{
	GLenum *op = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *dst = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *dstMod = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLuint *arg1 = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLuint *arg1Rep = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *arg1Mod = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *arg2 = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLuint *arg2Rep = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *arg2Mod = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	glAlphaFragmentOp2ATI(*op, *dst, *dstMod, *arg1, *arg1Rep, *arg1Mod, *arg2, *arg2Rep, *arg2Mod);
}


//1194
static void EXEC_glAlphaFragmentOp3ATI(byte *commandbuf)
{
	GLenum *op = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *dst = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *dstMod = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLuint *arg1 = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLuint *arg1Rep = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *arg1Mod = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *arg2 = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLuint *arg2Rep = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *arg2Mod = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *arg3 = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLuint *arg3Rep = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *arg3Mod = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	glAlphaFragmentOp3ATI(*op, *dst, *dstMod, *arg1, *arg1Rep, *arg1Mod, *arg2, *arg2Rep, *arg2Mod, *arg3, *arg3Rep, *arg3Mod);
}


//1195
static void EXEC_glSetFragmentShaderConstantATI(byte *commandbuf)
{
	GLuint *dst = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	glSetFragmentShaderConstantATI(*dst, (const GLfloat *)popBuf());
}


//1196
static void EXEC_glDrawMeshArraysSUN(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLint *first = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	//glDrawMeshArraysSUN(*mode, *first, *count, *width);
}


//1197
static void EXEC_glPointParameteriNV(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	glPointParameteriNV(*pname, *param);
}


//1198
static void EXEC_glPointParameterivNV(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glPointParameterivNV(*pname, (const GLint *)popBuf());
}


//1199
static void EXEC_glActiveStencilFaceEXT(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	glActiveStencilFaceEXT(*face);
}


//1200
static void EXEC_glDrawBuffersATI(byte *commandbuf)
{
	GLsizei *n = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	glDrawBuffersATI(*n, (const GLenum *)popBuf());
}


//1201
static void EXEC_glProgramNamedParameter4fNV(byte *commandbuf)
{
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLsizei *len = (GLsizei*)commandbuf;     commandbuf += sizeof(GLsizei);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *w = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	glProgramNamedParameter4fNV(*id, *len, (const GLubyte *)popBuf(), *x, *y, *z, *w);
}


//1202
static void EXEC_glProgramNamedParameter4dNV(byte *commandbuf)
{
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLsizei *len = (GLsizei*)commandbuf;     commandbuf += sizeof(GLsizei);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *w = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	glProgramNamedParameter4dNV(*id, *len, (const GLubyte *)popBuf(), *x, *y, *z, *w);
}


//1203
static void EXEC_glProgramNamedParameter4fvNV(byte *commandbuf)
{
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLsizei *len = (GLsizei*)commandbuf;     commandbuf += sizeof(GLsizei);

	glProgramNamedParameter4fvNV(*id, *len, (const GLubyte *)popBuf(), (const GLfloat *)popBuf());
}


//1204
static void EXEC_glProgramNamedParameter4dvNV(byte *commandbuf)
{
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLsizei *len = (GLsizei*)commandbuf;     commandbuf += sizeof(GLsizei);

	glProgramNamedParameter4dvNV(*id, *len, (const GLubyte *)popBuf(), (const GLdouble *)popBuf());
}


//1205
static void EXEC_glGetProgramNamedParameterfvNV(byte *commandbuf)
{
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLsizei *len = (GLsizei*)commandbuf;     commandbuf += sizeof(GLsizei);

	glGetProgramNamedParameterfvNV(*id, *len, (const GLubyte *)popBuf(), (GLfloat *)popBuf());
}


//1206
static void EXEC_glGetProgramNamedParameterdvNV(byte *commandbuf)
{
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLsizei *len = (GLsizei*)commandbuf;     commandbuf += sizeof(GLsizei);

	glGetProgramNamedParameterdvNV(*id, *len, (const GLubyte *)popBuf(), (GLdouble *)popBuf());
}


//1207
static void EXEC_glDepthBoundsEXT(byte *commandbuf)
{
	GLclampd *zmin = (GLclampd*)commandbuf;  commandbuf += sizeof(GLclampd);
	GLclampd *zmax = (GLclampd*)commandbuf;  commandbuf += sizeof(GLclampd);

	glDepthBoundsEXT(*zmin, *zmax);
}


//1208
static void EXEC_glBlendEquationSeparateEXT(byte *commandbuf)
{
	GLenum *modeRGB = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLenum *modeA = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glBlendEquationSeparateEXT(*modeRGB, *modeA);
}


//1209
static void EXEC_glBlitFramebufferEXT(byte *commandbuf)
{
	GLint *srcX0 = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *srcY0 = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *srcX1 = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *srcY1 = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *dstX0 = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *dstY0 = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *dstX1 = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *dstY1 = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLbitfield *mask = (GLbitfield*)commandbuf;  commandbuf += sizeof(GLbitfield);
	GLenum *filter = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	glBlitFramebufferEXT(*srcX0, *srcY0, *srcX1, *srcY1, *dstX0, *dstY0, *dstX1, *dstY1, *mask, *filter);
}


//1210
static void EXEC_glBlendEquationSeparateATI(byte *commandbuf)
{
	GLenum *modeRGB = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLenum *modeA = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	//glBlendEquationSeparateATI(*modeRGB, *modeA);
}


//1211
static void EXEC_glStencilOpSeparateATI(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *sfail = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *zfail = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *zpass = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	glStencilOpSeparateATI(*face, *sfail, *zfail, *zpass);
}


//1212
static void EXEC_glStencilFuncSeparateATI(byte *commandbuf)
{
	GLenum *frontfunc = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *backfunc = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLint *ref = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLuint *mask = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	glStencilFuncSeparateATI(*frontfunc, *backfunc, *ref, *mask);
}


//1213
static void EXEC_glProgramEnvParameters4fvEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	#ifndef SYMPHONY
	glProgramEnvParameters4fvEXT(*target, *index, *count, (const GLfloat *)popBuf());
	#endif
}


//1214
static void EXEC_glProgramLocalParameters4fvEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	#ifndef SYMPHONY
	glProgramLocalParameters4fvEXT(*target, *index, *count, (const GLfloat *)popBuf());
	#endif
}


//1215
static void EXEC_glGetQueryObjecti64vEXT(byte *commandbuf)
{
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	#ifndef SYMPHONY
	glGetQueryObjecti64vEXT(*id, *pname, (GLint64EXT *)popBuf());
	#endif
}


//1216
static void EXEC_glGetQueryObjectui64vEXT(byte *commandbuf)
{
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	#ifndef SYMPHONY
	glGetQueryObjectui64vEXT(*id, *pname, (GLuint64EXT *)popBuf());
	#endif
}


//1217
static void EXEC_glBlendFuncSeparateINGR(byte *commandbuf)
{
	GLenum *sfactorRGB = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *dfactorRGB = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *sfactorAlpha = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *dfactorAlpha = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	//glBlendFuncSeparateINGR(*sfactorRGB, *dfactorRGB, *sfactorAlpha, *dfactorAlpha);
}


//1218
static void EXEC_glCreateDebugObjectMESA(byte *commandbuf)
{

	//pushRet(glCreateDebugObjectMESA());
}


//1219
static void EXEC_glClearDebugLogMESA(byte *commandbuf)
{
	GLhandleARB *obj = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);
	GLenum *logType = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLenum *shaderType = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	//glClearDebugLogMESA(*obj, *logType, *shaderType);
}


//1220
static void EXEC_glGetDebugLogMESA(byte *commandbuf)
{
	GLhandleARB *obj = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);
	GLenum *logType = (GLenum*)commandbuf;      commandbuf += sizeof(GLenum);
	GLenum *shaderType = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *maxLength = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *length = (GLsizei*)commandbuf;     commandbuf += sizeof(GLsizei);

	//glGetDebugLogMESA(*obj, *logType, *shaderType, *maxLength, length, (GLcharARB *)popBuf());
}


//1221
static void EXEC_glGetDebugLogLengthMESA(byte *commandbuf)
{
	GLhandleARB *obj = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);
	GLenum *logType = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLenum *shaderType = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	//pushRet(glGetDebugLogLengthMESA(*obj, *logType, *shaderType));
}


//1222
static void EXEC_glPointParameterfSGIS(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	//glPointParameterfSGIS(*pname, *param);
}


//1223
static void EXEC_glPointParameterfvSGIS(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	//glPointParameterfvSGIS(*pname, (const GLfloat *)popBuf());
}


//1224
static void EXEC_glIglooInterfaceSGIX(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	//glIglooInterfaceSGIX(*pname, (const GLvoid *)popBuf());
}


//1225
static void EXEC_glDeformationMap3dSGIX(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLdouble *u1 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *u2 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLint *ustride = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *uorder = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLdouble *v1 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *v2 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLint *vstride = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *vorder = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLdouble *w1 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *w2 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLint *wstride = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *worder = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	//glDeformationMap3dSGIX(*target, *u1, *u2, *ustride, *uorder, *v1, *v2, *vstride, *vorder, *w1, *w2, *wstride, *worder, (const GLdouble *)popBuf());
}


//1226
static void EXEC_glDeformationMap3fSGIX(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLfloat *u1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *u2 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLint *ustride = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *uorder = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLfloat *v1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v2 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLint *vstride = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *vorder = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLfloat *w1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *w2 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLint *wstride = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *worder = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	//glDeformationMap3fSGIX(*target, *u1, *u2, *ustride, *uorder, *v1, *v2, *vstride, *vorder, *w1, *w2, *wstride, *worder, (const GLfloat *)popBuf());
}


//1227
static void EXEC_glDeformSGIX(byte *commandbuf)
{
	GLenum *mask = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	//glDeformSGIX(*mask);
}


//1228
static void EXEC_glLoadIdentityDeformationMapSGIX(byte *commandbuf)
{
	GLenum *mask = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	//glLoadIdentityDeformationMapSGIX(*mask);
}


/********************************************************
	GLU Intercepts
********************************************************/

//1501
static void EXEC_gluBeginCurve(byte *commandbuf)
{
	LOG("Called unimplemted stub gluBeginCurve!\n");
	// (GLUnurbs* nurb)
}


//1502
static void EXEC_gluBeginPolygon(byte *commandbuf)
{
	LOG("Called unimplemted stub gluBeginPolygon!\n");
	//(GLUtesselator* tess)
}


//1503
static void EXEC_gluBeginSurface(byte *commandbuf)
{
	LOG("Called unimplemted stub gluBeginSurface!\n");
	// (GLUnurbs* nurb)
}


//1504
static void EXEC_gluBeginTrim(byte *commandbuf)
{
	LOG("Called unimplemted stub gluBeginTrim!\n");
	// (GLUnurbs* nurb)
}


//1505
static void EXEC_gluBuild1DMipmapLevels(byte *commandbuf)
{
	LOG("Called unimplemted stub gluBuild1DMipmapLevels !\n");
	// (GLenum target, GLint internalFormat, GLsizei width, GLenum format, GLenum type, GLint level, GLint base, GLint max, const void *data)
	//returns GLint
}


//1506
static void EXEC_gluBuild1DMipmaps(byte *commandbuf)
{
	LOG("Called unimplemted stub gluBuild1DMipmaps!\n");
	//(GLenum target, GLint internalFormat, GLsizei width, GLenum format, GLenum type, const void *data)
	//returns GLint
}


//1507
static void EXEC_gluBuild2DMipmapLevels(byte *commandbuf)
{
	LOG("Called unimplemted stub gluBuild2DMipmapLevels!\n");
	//(GLenum target, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, GLint level, GLint base, GLint max, const void *data)
	//returns GLint
}


//1508
static void EXEC_gluBuild2DMipmaps(byte *commandbuf)
{
	LOG("Called unimplemted stub gluBuild2DMipmaps!\n");
	//(GLenum target, GLint internalFormat, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *data)
	//returns GLint
}


//1509
static void EXEC_gluBuild3DMipmapLevels(byte *commandbuf)
{
	LOG("Called unimplemted stub gluBuild3DMipmapLevels!\n");
	//(GLenum target, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLint level, GLint base, GLint max, const void *data)
	//returns GLint
}


//1510
static void EXEC_gluBuild3DMipmaps(byte *commandbuf)
{
	//(GLenum target, GLint internalFormat, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const void *data)
	LOG("Called unimplemted stub gluBuild3DMipmaps!\n");
	//returns GLint
}


//1511
static void EXEC_gluCheckExtension(byte *commandbuf)
{
	LOG("Called unimplemted stub gluCheckExtension!\n");
	//(const GLubyte *extName, const GLubyte *extString)
	//returns GLboolean
}


//1512
static void EXEC_gluCylinder(byte *commandbuf)
{
	LOG("Called unimplemted stub gluCylinder!\n");
	//(GLUquadric* quad, GLdouble base, GLdouble top, GLdouble height, GLint slices, GLint stacks)
}


//1513
static void EXEC_gluDeleteNurbsRenderer(byte *commandbuf)
{
	LOG("Called unimplemted stub gluDeleteNurbsRenderer!\n");
	//(GLUnurbs* nurb)
}


//1514
static void EXEC_gluDeleteQuadric(byte *commandbuf)
{
	LOG("Called unimplemted stub gluDeleteQuadric!\n");
	//(GLUquadric* quad)
}


//1515
static void EXEC_gluDeleteTess(byte *commandbuf)
{
	LOG("Called unimplemted stub gluDeleteTess!\n");
	//(GLUtesselator* tess)
}


//1516
static void EXEC_gluDisk(byte *commandbuf)
{
	LOG("Called unimplemted stub gluDisk!\n");
	//(GLUquadric* quad, GLdouble inner, GLdouble outer, GLint slices, GLint loops)
}


//1517
static void EXEC_gluEndCurve(byte *commandbuf)
{
	LOG("Called unimplemted stub gluEndCurve!\n");
	//(GLUnurbs* nurb)
}


//1518
static void EXEC_gluEndPolygon(byte *commandbuf)
{
	LOG("Called unimplemted stub gluEndPolygon!\n");
	//(GLUtesselator* tess)
}


//1519
static void EXEC_gluEndSurface(byte *commandbuf)
{
	LOG("Called unimplemted stub gluEndSurface!\n");
	//(GLUnurbs* nurb)
}


//1520
static void EXEC_gluEndTrim(byte *commandbuf)
{
	LOG("Called unimplemted stub gluEndTrim!\n");
	//(GLUnurbs* nurb)
}


//1521
static void EXEC_gluErrorString(byte *commandbuf)
{
	LOG("Called unimplemted stub gluErrorString!\n");
	//(GLenum error)
	//returns const GLubyte *
}


//1522
static void EXEC_gluGetNurbsProperty(byte *commandbuf)
{
	LOG("Called unimplemted stub gluGetNurbsProperty!\n");
	//(GLUnurbs* nurb, GLenum property, GLfloat* data)
}


//1523
static void EXEC_gluGetString(byte *commandbuf)
{
	LOG("Called unimplemted stub gluGetString!\n");
	//(GLenum name)
	//returns const GLubyte *
}


//1524
static void EXEC_gluGetTessProperty(byte *commandbuf)
{
	LOG("Called unimplemted stub gluGetTessProperty!\n");
	//(GLUtesselator* tess, GLenum which, GLdouble* data)
}


//1525
static void EXEC_gluLoadSamplingMatrices(byte *commandbuf)
{
	LOG("Called unimplemted stub gluLoadSamplingMatrices!\n");
	//(GLUnurbs* nurb, const GLfloat *model, const GLfloat *perspective, const GLint *view)
}


//1526
static void EXEC_gluLookAt(byte *commandbuf)
{
	LOG("Called untested stub gluLookAt!\n");
   /* GLdouble *eyeX = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *eyeX = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *eyeX = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *eyeX = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *eyeX = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *eyeX = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *eyeX = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *eyeX = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
*/
    //glDeformationMap3fSGIX(*target, *u1, *u2, *ustride, *uorder, *v1, *v2, *vstride, *vorder, *w1, *w2, *wstride, *worder, (const GLfloat *)popBuf());
	//(GLdouble eyeX, GLdouble eyeY, GLdouble eyeZ, GLdouble centerX, GLdouble centerY, GLdouble centerZ, GLdouble upX, GLdouble upY, GLdouble upZ)
}


//1527
static void EXEC_gluNewNurbsRenderer(byte *commandbuf)
{
	LOG("Called unimplemted stub gluNewNurbsRenderer!\n");
	//returns GLUnurbs*
}


//1528
static void EXEC_gluNewQuadric(byte *commandbuf)
{
	LOG("Called unimplemted stub gluNewQuadric!\n");
	//returns GLUquadric*
}


//1529
static void EXEC_gluNewTess(byte *commandbuf)
{
	LOG("Called unimplemted stub gluNewTess!\n");
	//returns GLUtesselator*
}


//1530
static void EXEC_gluNextContour(byte *commandbuf)
{
	LOG("Called unimplemted stub gluNextContour!\n");
	//(GLUtesselator* tess, GLenum type)
}


//1531
static void EXEC_gluNurbsCallback(byte *commandbuf)
{
	LOG("Called unimplemted stub gluNurbsCallback!\n");
	//(GLUnurbs* nurb, GLenum which, _GLUfuncptr CallBackFunc)
}


//1532
static void EXEC_gluNurbsCallbackData(byte *commandbuf)
{
	LOG("Called unimplemted stub gluNurbsCallbackData!\n");
	//(GLUnurbs* nurb, GLvoid* userData)
}


//1533
static void EXEC_gluNurbsCallbackDataEXT(byte *commandbuf)
{
	LOG("Called unimplemted stub gluNurbsCallbackDataEXT!\n");
	//(GLUnurbs* nurb, GLvoid* userData)
}


//1534
static void EXEC_gluNurbsCurve(byte *commandbuf)
{
	LOG("Called unimplemted stub gluNurbsCurve!\n");
	//(GLUnurbs* nurb, GLint knotCount, GLfloat *knots, GLint stride, GLfloat *control, GLint order, GLenum type)
}


//1535
static void EXEC_gluNurbsProperty(byte *commandbuf)
{
	LOG("Called unimplemted stub gluNurbsProperty!\n");
	//(GLUnurbs* nurb, GLenum property, GLfloat value)
}


//1536
static void EXEC_gluNurbsSurface(byte *commandbuf)
{
	LOG("Called unimplemted stub gluNurbsSurface!\n");
	//(GLUnurbs* nurb, GLint sKnotCount, GLfloat* sKnots, GLint tKnotCount, GLfloat* tKnots, GLint sStride, GLint tStride, GLfloat* control, GLint sOrder, GLint tOrder, GLenum type)
}


//1537
static void EXEC_gluOrtho2D(byte *commandbuf)
{
	LOG("Called unimplemted stub gluOrtho2D!\n");
	//(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top)
}


//1538
static void EXEC_gluPartialDisk(byte *commandbuf)
{
	LOG("Called unimplemted stub gluPartialDisk!\n");
	//(GLUquadric* quad, GLdouble inner, GLdouble outer, GLint slices, GLint loops, GLdouble start, GLdouble sweep)
}


//1539
static void EXEC_gluPerspective(byte *commandbuf)
{
	GLdouble *fovy = (GLdouble*)commandbuf;  commandbuf += sizeof(GLdouble);
	GLdouble *aspect = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *zNear = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *zFar = (GLdouble*)commandbuf;  commandbuf += sizeof(GLdouble);
	gluPerspective(*fovy, *aspect, *zNear, *zFar);
}


//1540
static void EXEC_gluPickMatrix(byte *commandbuf)
{
	LOG("Called unimplemted stub gluPickMatrix!\n");
	//(GLdouble x, GLdouble y, GLdouble delX, GLdouble delY, GLint *viewport)
}


//1541
static void EXEC_gluProject(byte *commandbuf)
{
	LOG("Called unimplemted stub gluProject!\n");
	//(GLdouble objX, GLdouble objY, GLdouble objZ, const GLdouble *model, const GLdouble *proj, const GLint *view, GLdouble* winX, GLdouble* winY, GLdouble* winZ)
	//returns GLint
}


//1542
static void EXEC_gluPwlCurve(byte *commandbuf)
{
	LOG("Called unimplemted stub gluPwlCurve!\n");
	//(GLUnurbs* nurb, GLint count, GLfloat* data, GLint stride, GLenum type)
}


//1543
static void EXEC_gluQuadricCallback(byte *commandbuf)
{
	LOG("Called unimplemted stub gluQuadricCallback!\n");
	//(GLUquadric* quad, GLenum which, _GLUfuncptr CallBackFunc)
}


//1544
static void EXEC_gluQuadricDrawStyle(byte *commandbuf)
{
	LOG("Called unimplemted stub gluQuadricDrawStyle!\n");
	//(GLUquadric* quad, GLenum draw)
}


//1545
static void EXEC_gluQuadricNormals(byte *commandbuf)
{
	LOG("Called unimplemted stub gluQuadricNormals!\n");
	//(GLUquadric* quad, GLenum normal)
}


//1546
static void EXEC_gluQuadricOrientation(byte *commandbuf)
{
	LOG("Called unimplemted stub gluQuadricOrientation!\n");
	//(GLUquadric* quad, GLenum orientation)
}


//1547
static void EXEC_gluQuadricTexture(byte *commandbuf)
{
	LOG("Called unimplemted stub gluQuadricTexture!\n");
	//(GLUquadric* quad, GLboolean texture)
}


//1548
static void EXEC_glugluScaleImage(byte *commandbuf)
{
	LOG("Called unimplemted stub gluScaleImage!\n");
	//(GLenum format, GLsizei wIn, GLsizei hIn, GLenum typeIn, const void *dataIn, GLsizei wOut, GLsizei hOut, GLenum typeOut, GLvoid* dataOut)
	//returns Glint
}


//1549
static void EXEC_gluSphere(byte *commandbuf)
{
	LOG("Called unimplemted stub gluSphere!\n");
	//(GLUquadric* quad, GLdouble radius, GLint slices, GLint stacks)
}


//1550
static void EXEC_gluTessBeginContour(byte *commandbuf)
{
	LOG("Called unimplemted stub gluTessBeginContour!\n");
	//(GLUtesselator* tess)
}


//1551
static void EXEC_gluTessBeginPolygon(byte *commandbuf)
{
	LOG("Called unimplemted stub gluTessBeginPolygon!\n");
	//(GLUtesselator* tess, GLvoid* data)
}


//1552
static void EXEC_gluTessCallback(byte *commandbuf)
{
	LOG("Called unimplemted stub gluTessCallback!\n");
	//(GLUtesselator* tess, GLenum which, _GLUfuncptr CallBackFunc)
}


//1553
static void EXEC_gluTessEndContour(byte *commandbuf)
{
	LOG("Called unimplemted stub gluTessEndContour!\n");
	//(GLUtesselator* tess)
}


//1554
static void EXEC_gluTessEndPolygon(byte *commandbuf)
{
	LOG("Called unimplemted stub gluTessEndPolygon!\n");
	//(GLUtesselator* tess)
}


//1555
static void EXEC_gluTessNormal(byte *commandbuf)
{
	LOG("Called unimplemted stub gluTessNormal!\n");
	//(GLUtesselator* tess, GLdouble valueX, GLdouble valueY, GLdouble valueZ)
}


//1556
static void EXEC_gluTessProperty(byte *commandbuf)
{
	LOG("Called unimplemted stub gluTessProperty!\n");
	//(GLUtesselator* tess, GLenum which, GLdouble data)
}


//1557
static void EXEC_gluTessVertex(byte *commandbuf)
{
	LOG("Called unimplemted stub gluTessVertex!\n");
	//(GLUtesselator* tess, GLdouble *location, GLvoid* data)
}


//1558
static void EXEC_gluUnProject(byte *commandbuf)
{
	LOG("Called unimplemted stub gluUnProject!\n");
	//(GLdouble winX, GLdouble winY, GLdouble winZ, const GLdouble *model, const GLdouble *proj, const GLint *view, GLdouble* objX, GLdouble* objY, GLdouble* objZ)
	//returns glint
}


//1559
static void EXEC_gluUnProject4(byte *commandbuf)
{
	LOG("Called unimplemted stub gluUnProject4!\n");
	//(GLdouble winX, GLdouble winY, GLdouble winZ, GLdouble clipW, const GLdouble *model, const GLdouble *proj, const GLint *view, GLdouble nearVal, GLdouble farVal, GLdouble* objX, GLdouble* objY, GLdouble* objZ, GLdouble* objW)
	//returns glint
}


/********************************************************
	GLX Intercepts
********************************************************/

//1601
static void EXEC_glXChooseVisual(byte *commandbuf)
{
	LOG("Called unimplemted stub glXChooseVisual!\n");
	//( Display *dpy, int screen, int *attribList )
	//returns XVisualInfo*
}


//1602
static void EXEC_glXCreateContext(byte *commandbuf)
{
	LOG("Called unimplemted stub glXCreateContext!\n");
	//( Display *dpy, XVisualInfo *vis, GLXContext shareList, Bool direct )
	//returns GLXContext
}


//1603
static void EXEC_glXDestroyContext(byte *commandbuf)
{
	LOG("Called unimplemted stub glXDestroyContext!\n");
	//( Display *dpy, GLXContext ctx )
}


//1604
static void EXEC_glXMakeCurrent(byte *commandbuf)
{
	LOG("Called unimplemted stub glXMakeCurrent!\n");
	//( Display *dpy, GLXDrawable drawable, GLXContext ctx)
	//returns Bool
}


//1605
static void EXEC_glXCopyContext(byte *commandbuf)
{
	LOG("Called unimplemted stub glXCopyContext!\n");
	//( Display *dpy, GLXContext src, GLXContext dst, unsigned long mask )
}


//1606
static void EXEC_glXSwapBuffers(byte *commandbuf)
{
	LOG("Called unimplemted stub glXSwapBuffers!\n");
	//( Display *dpy, GLXDrawable drawable )
}


//1607
static void EXEC_glXCreateGLXPixmap(byte *commandbuf)
{
	LOG("Called unimplemted stub glXCreateGLXPixmap!\n");
	//( Display *dpy, XVisualInfo *visual, Pixmap pixmap )
	//returns GLXPixmap
}


//1608
static void EXEC_glXDestroyGLXPixmap(byte *commandbuf)
{
	LOG("Called unimplemted stub glXDestroyGLXPixmap!\n");
	//( Display *dpy, GLXPixmap pixmap )
}


//1609
static void EXEC_glXQueryExtension(byte *commandbuf)
{
	LOG("Called unimplemted stub glXQueryExtension!\n");
	//( Display *dpy, int *errorb, int *event )
	//returns bool
}


//1610
static void EXEC_glXQueryVersion(byte *commandbuf)
{
	LOG("Called unimplemted stub glXQueryVersion!\n");
	//( Display *dpy, int *maj, int *min )
	//returns bool
}


//1611
static void EXEC_glXIsDirect(byte *commandbuf)
{
	LOG("Called unimplemted stub glXIsDirect!\n");
	//( Display *dpy, GLXContext ctx )
	//returns bool
}


//1612
static void EXEC_glXGetConfig(byte *commandbuf)
{
	LOG("Called unimplemted stub glXGetConfig!\n");
	//( Display *dpy, XVisualInfo *visual, int attrib, int *value )
	//returns int
}


//1613
static void EXEC_glXGetCurrentContext(byte *commandbuf)
{
	LOG("Called unimplemted stub glXGetCurrentContext!\n");
	//returns GLXContext
}


//1614
static void EXEC_glXGetCurrentDrawable(byte *commandbuf)
{
	LOG("Called unimplemted stub glXGetCurrentDrawable!\n");
	//returns GLXDrawable
}


//1615
static void EXEC_glXWaitGL(byte *commandbuf)
{
	LOG("Called unimplemted stub glXWaitGL!\n");
}


//1616
static void EXEC_glXWaitX(byte *commandbuf)
{
	LOG("Called unimplemted stub glXWaitX!\n");
}


//1617
static void EXEC_glXUseXFont(byte *commandbuf)
{
	LOG("Called unimplemted stub glXUseXFont!\n");
	//( Font font, int first, int count, int list )
}


//GLX 1.1 and later
//1618
static void EXEC_glXQueryExtensionsString(byte *commandbuf)
{
	LOG("Called unimplemted stub glXQueryExtensionsString!\n");
	//( Display *dpy, int screen )
	//returns const char *
}


//1619
static void EXEC_glXQueryServerString(byte *commandbuf)
{
	LOG("Called unimplemted stub glXQueryServerString!\n");
	//( Display *dpy, int screen, int name )
	//returns const char *
}


//1620
static void EXEC_glXGetClientString(byte *commandbuf)
{
	LOG("Called unimplemted stub glXGetClientString!\n");
	//( Display *dpy, int name )
	//returns const char *
}


// GLX 1.2 and later
//1621
static void EXEC_glXGetCurrentDisplay(byte *commandbuf)
{
	LOG("Called unimplemted stub glXGetCurrentDisplay!\n");
	//returns Display *
}


// GLX 1.3 and later
//1622
static void EXEC_glXChooseFBConfig(byte *commandbuf)
{
	LOG("Called unimplemted stub glXChooseFBConfig!\n");
	//( Display *dpy, int screen, const int *attribList, int *nitems )
	//returns GLXFBConfig *
}


//1623
static void EXEC_glXGetFBConfigAttrib(byte *commandbuf)
{
	LOG("Called unimplemted stub glXGetFBConfigAttrib!\n");
	//( Display *dpy, GLXFBConfig config, int attribute, int *value )
	//returns int
}


//1624
static void EXEC_glXGetFBConfigs(byte *commandbuf)
{
	LOG("Called unimplemted stub glXGetFBConfigs!\n");
	//( Display *dpy, int screen, int *nelements )
	//returns GLXFBConfig *
}


//1625
static void EXEC_glXGetVisualFromFBConfig(byte *commandbuf)
{
	LOG("Called unimplemted stub glXGetVisualFromFBConfig!\n");
	//( Display *dpy, GLXFBConfig config )
	//returns XVisualInfo *
}


//1626
static void EXEC_glXCreateWindow(byte *commandbuf)
{
	LOG("Called unimplemted stub glXCreateWindow!\n");
	//( Display *dpy, GLXFBConfig config, Window win, const int *attribList )
	//returns  GLXWindow
}


//1627
static void EXEC_glXDestroyWindow(byte *commandbuf)
{
	LOG("Called unimplemted stub glXDestroyWindow!\n");
	//( Display *dpy, GLXWindow window )
}


//1628
static void EXEC_glXCreatePixmap(byte *commandbuf)
{
	LOG("Called unimplemted stub glXCreatePixmap!\n");
	//( Display *dpy, GLXFBConfig config, Pixmap pixmap, const int *attribList )
	//returns GLXPixmap
}


//1629
static void EXEC_glXDestroyPixmap(byte *commandbuf)
{
	LOG("Called unimplemted stub glXDestroyPixmap!\n");
	//( Display *dpy, GLXPixmap pixmap )
}


//1630
static void EXEC_glXCreatePbuffer(byte *commandbuf)
{
	LOG("Called unimplemted stub glXCreatePbuffer!\n");
	//( Display *dpy, GLXFBConfig config, const int *attribList )
	//returns GLXPbuffer
}


//1631
static void EXEC_glXDestroyPbuffer(byte *commandbuf)
{
	LOG("Called unimplemted stub glXDestroyPbuffer!\n");
	//( Display *dpy, GLXPbuffer pbuf )
}


//1632
static void EXEC_glXQueryDrawable(byte *commandbuf)
{
	LOG("Called unimplemted stub glXQueryDrawable!\n");
	//( Display *dpy, GLXDrawable draw, int attribute, unsigned int *value )
}


//1633
static void EXEC_glXCreateNewContext(byte *commandbuf)
{
	LOG("Called unimplemted stub glXCreateNewContext!\n");
	//( Display *dpy, GLXFBConfig config, int renderType, GLXContext shareList, Bool direct )
	//returns GLXContext
}


//1634
static void EXEC_glXMakeContextCurrent(byte *commandbuf)
{
	LOG("Called unimplemted stub glXMakeContextCurrent!\n");
	//( Display *dpy, GLXDrawable draw, GLXDrawable read, GLXContext ctx )
	//returns Bool
}


//1635
static void EXEC_glXGetCurrentReadDrawable(byte *commandbuf)
{
	LOG("Called unimplemted stub glXGetCurrentReadDrawable!\n");
	//returns GLXDrawable
}


//1636
static void EXEC_glXQueryContext(byte *commandbuf)
{
	LOG("Called unimplemted stub glXQueryContext!\n");
	//( Display *dpy, GLXContext ctx, int attribute, int *value )
	//returns int
}


//1637
static void EXEC_glXSelectEvent(byte *commandbuf)
{
	LOG("Called unimplemted stub glXSelectEvent!\n");
	//( Display *dpy, GLXDrawable drawable, unsigned long mask )
}


//1638
static void EXEC_glXGetSelectedEvent(byte *commandbuf)
{
	LOG("Called unimplemted stub glXGetSelectedEvent!\n");
	//( Display *dpy, GLXDrawable drawable, unsigned long *mask )
}


//1639
static void EXEC_glXGetProcAddressARB(byte *commandbuf)
{
	LOG("Called unimplemted stub glXGetProcAddressARB!\n");
	//(const GLubyte *)
	//returns __GLXextFuncPtr
}


//1640
static void EXEC_glXGetProcAddress(byte *commandbuf)
{
	LOG("Called unimplemted stub glXGetProcAddress!\n");
	//(const GLubyte *procname)
}


//1641
static void EXEC_glXAllocateMemoryNV(byte *commandbuf)
{
	LOG("Called unimplemted stub glXAllocateMemoryNV!\n");
	//(GLsizei size, GLfloat readfreq, GLfloat writefreq, GLfloat priority)
}


//1642
static void EXEC_glXFreeMemoryNV(byte *commandbuf)
{
	LOG("Called unimplemted stub glXFreeMemoryNV!\n");
	//(GLvoid *pointer)
}


//1643
static void EXEC_glXAllocateMemoryMESA(byte *commandbuf)
{
	LOG("Called unimplemted stub glXAllocateMemoryMESA!\n");
	//(Display *dpy, int scrn, size_t size, float readfreq, float writefreq, float priority)
}


//1644
static void EXEC_glXFreeMemoryMESA(byte *commandbuf)
{
	LOG("Called unimplemted stub glXFreeMemoryMESA!\n");
	//(Display *dpy, int scrn, void *pointer)
}


//1645
static void EXEC_glXGetMemoryOffsetMESA(byte *commandbuf)
{
	LOG("Called unimplemted stub glXGetMemoryOffsetMESA!\n");
	//(Display *dpy, int scrn, const void *pointer)
	//returns GLuint
}


//1646
static void EXEC_glXBindTexImageARB(byte *commandbuf)
{
	LOG("Called unimplemted stub glXBindTexImageARB!\n");
	//(Display *dpy, GLXPbuffer pbuffer, int buffer)
	//returns Bool
}


//1647
static void EXEC_glXReleaseTexImageARB(byte *commandbuf)
{
	LOG("Called unimplemted stub glXReleaseTexImageARB!\n");
	//(Display *dpy, GLXPbuffer pbuffer, int buffer)
	//returns Bool
}


//1648
static void EXEC_glXDrawableAttribARB(byte *commandbuf)
{
	LOG("Called unimplemted stub glXDrawableAttribARB!\n");
	//(Display *dpy, GLXDrawable draw, const int *attribList)
	//returns Bool
}


//1649
static void EXEC_glXGetFrameUsageMESA(byte *commandbuf)
{
	LOG("Called unimplemted stub glXGetFrameUsageMESA!\n");
	//(Display *dpy, GLXDrawable drawable, float *usage)
	//returns int
}


//1650
static void EXEC_glXBeginFrameTrackingMESA(byte *commandbuf)
{
	LOG("Called unimplemted stub glXBeginFrameTrackingMESA!\n");
	//(Display *dpy, GLXDrawable drawable)
	//returns int
}


//1651
static void EXEC_glXEndFrameTrackingMESA(byte *commandbuf)
{
	LOG("Called unimplemted stub glXEndFrameTrackingMESA!\n");
	//(Display *dpy, GLXDrawable drawable)
	//returns int
}


//1652
static void EXEC_glXQueryFrameTrackingMESA(byte *commandbuf)
{
	LOG("Called unimplemted stub glXQueryFrameTrackingMESA!\n");
	//(Display *dpy, GLXDrawable drawable, int64_t *swapCount, int64_t *missedFrames, float *lastMissedUsage)
	//returns int
}


//1653
static void EXEC_glXSwapIntervalMESA(byte *commandbuf)
{
	LOG("Called unimplemted stub glXSwapIntervalMESA!\n");
	//(unsigned int interval)
	//returns int
}


//1654
static void EXEC_glXGetSwapIntervalMESA(byte *commandbuf)
{
	LOG("Called unimplemted stub glXGetSwapIntervalMESA!\n");
	//returns int
}


//1655
static void EXEC_glXBindTexImageEXT(byte *commandbuf)
{
	LOG("Called unimplemted stub glXBindTexImageEXT!\n");
	//(Display *dpy, GLXDrawable drawable, int buffer, const int *attrib_list)
}


//1656
static void EXEC_glXReleaseTexImageEXT(byte *commandbuf)
{
	LOG("Called unimplemted stub glXReleaseTexImageEXT!\n");
	//(Display *dpy, GLXDrawable drawable, int buffer)
}


/*********************************************************
	Method Pointers
*********************************************************/

bool ExecModule::init()
{

	LOG("Loading ExecModule\n");

	for(int i=0;i<1700;i++) {
		mFunctions[i] = NULL;
	}

	//GL functions
	mFunctions[0] = EXEC_glNewList;
	mFunctions[1] = EXEC_glEndList;
	mFunctions[2] = EXEC_glCallList;
	mFunctions[3] = EXEC_glCallLists;
	mFunctions[4] = EXEC_glDeleteLists;
	mFunctions[5] = EXEC_glGenLists;
	mFunctions[6] = EXEC_glListBase;
	mFunctions[7] = EXEC_glBegin;
	mFunctions[8] = EXEC_glBitmap;
	mFunctions[9] = EXEC_glColor3b;
	mFunctions[10] = EXEC_glColor3bv;
	mFunctions[11] = EXEC_glColor3d;
	mFunctions[12] = EXEC_glColor3dv;
	mFunctions[13] = EXEC_glColor3f;
	mFunctions[14] = EXEC_glColor3fv;
	mFunctions[15] = EXEC_glColor3i;
	mFunctions[16] = EXEC_glColor3iv;
	mFunctions[17] = EXEC_glColor3s;
	mFunctions[18] = EXEC_glColor3sv;
	mFunctions[19] = EXEC_glColor3ub;
	mFunctions[20] = EXEC_glColor3ubv;
	mFunctions[21] = EXEC_glColor3ui;
	mFunctions[22] = EXEC_glColor3uiv;
	mFunctions[23] = EXEC_glColor3us;
	mFunctions[24] = EXEC_glColor3usv;
	mFunctions[25] = EXEC_glColor4b;
	mFunctions[26] = EXEC_glColor4bv;
	mFunctions[27] = EXEC_glColor4d;
	mFunctions[28] = EXEC_glColor4dv;
	mFunctions[29] = EXEC_glColor4f;
	mFunctions[30] = EXEC_glColor4fv;
	mFunctions[31] = EXEC_glColor4i;
	mFunctions[32] = EXEC_glColor4iv;
	mFunctions[33] = EXEC_glColor4s;
	mFunctions[34] = EXEC_glColor4sv;
	mFunctions[35] = EXEC_glColor4ub;
	mFunctions[36] = EXEC_glColor4ubv;
	mFunctions[37] = EXEC_glColor4ui;
	mFunctions[38] = EXEC_glColor4uiv;
	mFunctions[39] = EXEC_glColor4us;
	mFunctions[40] = EXEC_glColor4usv;
	mFunctions[41] = EXEC_glEdgeFlag;
	mFunctions[42] = EXEC_glEdgeFlagv;
	mFunctions[43] = EXEC_glEnd;
	mFunctions[44] = EXEC_glIndexd;
	mFunctions[45] = EXEC_glIndexdv;
	mFunctions[46] = EXEC_glIndexf;
	mFunctions[47] = EXEC_glIndexfv;
	mFunctions[48] = EXEC_glIndexi;
	mFunctions[49] = EXEC_glIndexiv;
	mFunctions[50] = EXEC_glIndexs;
	mFunctions[51] = EXEC_glIndexsv;
	mFunctions[52] = EXEC_glNormal3b;
	mFunctions[53] = EXEC_glNormal3bv;
	mFunctions[54] = EXEC_glNormal3d;
	mFunctions[55] = EXEC_glNormal3dv;
	mFunctions[56] = EXEC_glNormal3f;
	mFunctions[57] = EXEC_glNormal3fv;
	mFunctions[58] = EXEC_glNormal3i;
	mFunctions[59] = EXEC_glNormal3iv;
	mFunctions[60] = EXEC_glNormal3s;
	mFunctions[61] = EXEC_glNormal3sv;
	mFunctions[62] = EXEC_glRasterPos2d;
	mFunctions[63] = EXEC_glRasterPos2dv;
	mFunctions[64] = EXEC_glRasterPos2f;
	mFunctions[65] = EXEC_glRasterPos2fv;
	mFunctions[66] = EXEC_glRasterPos2i;
	mFunctions[67] = EXEC_glRasterPos2iv;
	mFunctions[68] = EXEC_glRasterPos2s;
	mFunctions[69] = EXEC_glRasterPos2sv;
	mFunctions[70] = EXEC_glRasterPos3d;
	mFunctions[71] = EXEC_glRasterPos3dv;
	mFunctions[72] = EXEC_glRasterPos3f;
	mFunctions[73] = EXEC_glRasterPos3fv;
	mFunctions[74] = EXEC_glRasterPos3i;
	mFunctions[75] = EXEC_glRasterPos3iv;
	mFunctions[76] = EXEC_glRasterPos3s;
	mFunctions[77] = EXEC_glRasterPos3sv;
	mFunctions[78] = EXEC_glRasterPos4d;
	mFunctions[79] = EXEC_glRasterPos4dv;
	mFunctions[80] = EXEC_glRasterPos4f;
	mFunctions[81] = EXEC_glRasterPos4fv;
	mFunctions[82] = EXEC_glRasterPos4i;
	mFunctions[83] = EXEC_glRasterPos4iv;
	mFunctions[84] = EXEC_glRasterPos4s;
	mFunctions[85] = EXEC_glRasterPos4sv;
	mFunctions[86] = EXEC_glRectd;
	mFunctions[87] = EXEC_glRectdv;
	mFunctions[88] = EXEC_glRectf;
	mFunctions[89] = EXEC_glRectfv;
	mFunctions[90] = EXEC_glRecti;
	mFunctions[91] = EXEC_glRectiv;
	mFunctions[92] = EXEC_glRects;
	mFunctions[93] = EXEC_glRectsv;
	mFunctions[94] = EXEC_glTexCoord1d;
	mFunctions[95] = EXEC_glTexCoord1dv;
	mFunctions[96] = EXEC_glTexCoord1f;
	mFunctions[97] = EXEC_glTexCoord1fv;
	mFunctions[98] = EXEC_glTexCoord1i;
	mFunctions[99] = EXEC_glTexCoord1iv;
	mFunctions[100] = EXEC_glTexCoord1s;
	mFunctions[101] = EXEC_glTexCoord1sv;
	mFunctions[102] = EXEC_glTexCoord2d;
	mFunctions[103] = EXEC_glTexCoord2dv;
	mFunctions[104] = EXEC_glTexCoord2f;
	mFunctions[105] = EXEC_glTexCoord2fv;
	mFunctions[106] = EXEC_glTexCoord2i;
	mFunctions[107] = EXEC_glTexCoord2iv;
	mFunctions[108] = EXEC_glTexCoord2s;
	mFunctions[109] = EXEC_glTexCoord2sv;
	mFunctions[110] = EXEC_glTexCoord3d;
	mFunctions[111] = EXEC_glTexCoord3dv;
	mFunctions[112] = EXEC_glTexCoord3f;
	mFunctions[113] = EXEC_glTexCoord3fv;
	mFunctions[114] = EXEC_glTexCoord3i;
	mFunctions[115] = EXEC_glTexCoord3iv;
	mFunctions[116] = EXEC_glTexCoord3s;
	mFunctions[117] = EXEC_glTexCoord3sv;
	mFunctions[118] = EXEC_glTexCoord4d;
	mFunctions[119] = EXEC_glTexCoord4dv;
	mFunctions[120] = EXEC_glTexCoord4f;
	mFunctions[121] = EXEC_glTexCoord4fv;
	mFunctions[122] = EXEC_glTexCoord4i;
	mFunctions[123] = EXEC_glTexCoord4iv;
	mFunctions[124] = EXEC_glTexCoord4s;
	mFunctions[125] = EXEC_glTexCoord4sv;
	mFunctions[126] = EXEC_glVertex2d;
	mFunctions[127] = EXEC_glVertex2dv;
	mFunctions[128] = EXEC_glVertex2f;
	mFunctions[129] = EXEC_glVertex2fv;
	mFunctions[130] = EXEC_glVertex2i;
	mFunctions[131] = EXEC_glVertex2iv;
	mFunctions[132] = EXEC_glVertex2s;
	mFunctions[133] = EXEC_glVertex2sv;
	mFunctions[134] = EXEC_glVertex3d;
	mFunctions[135] = EXEC_glVertex3dv;
	mFunctions[136] = EXEC_glVertex3f;
	mFunctions[137] = EXEC_glVertex3fv;
	mFunctions[138] = EXEC_glVertex3i;
	mFunctions[139] = EXEC_glVertex3iv;
	mFunctions[140] = EXEC_glVertex3s;
	mFunctions[141] = EXEC_glVertex3sv;
	mFunctions[142] = EXEC_glVertex4d;
	mFunctions[143] = EXEC_glVertex4dv;
	mFunctions[144] = EXEC_glVertex4f;
	mFunctions[145] = EXEC_glVertex4fv;
	mFunctions[146] = EXEC_glVertex4i;
	mFunctions[147] = EXEC_glVertex4iv;
	mFunctions[148] = EXEC_glVertex4s;
	mFunctions[149] = EXEC_glVertex4sv;
	mFunctions[150] = EXEC_glClipPlane;
	mFunctions[151] = EXEC_glColorMaterial;
	mFunctions[152] = EXEC_glCullFace;
	mFunctions[153] = EXEC_glFogf;
	mFunctions[154] = EXEC_glFogfv;
	mFunctions[155] = EXEC_glFogi;
	mFunctions[156] = EXEC_glFogiv;
	mFunctions[157] = EXEC_glFrontFace;
	mFunctions[158] = EXEC_glHint;
	mFunctions[159] = EXEC_glLightf;
	mFunctions[160] = EXEC_glLightfv;
	mFunctions[161] = EXEC_glLighti;
	mFunctions[162] = EXEC_glLightiv;
	mFunctions[163] = EXEC_glLightModelf;
	mFunctions[164] = EXEC_glLightModelfv;
	mFunctions[165] = EXEC_glLightModeli;
	mFunctions[166] = EXEC_glLightModeliv;
	mFunctions[167] = EXEC_glLineStipple;
	mFunctions[168] = EXEC_glLineWidth;
	mFunctions[169] = EXEC_glMaterialf;
	mFunctions[170] = EXEC_glMaterialfv;
	mFunctions[171] = EXEC_glMateriali;
	mFunctions[172] = EXEC_glMaterialiv;
	mFunctions[173] = EXEC_glPointSize;
	mFunctions[174] = EXEC_glPolygonMode;
	mFunctions[175] = EXEC_glPolygonStipple;
	mFunctions[176] = EXEC_glScissor;
	mFunctions[177] = EXEC_glShadeModel;
	mFunctions[178] = EXEC_glTexParameterf;
	mFunctions[179] = EXEC_glTexParameterfv;
	mFunctions[180] = EXEC_glTexParameteri;
	mFunctions[181] = EXEC_glTexParameteriv;
	mFunctions[182] = EXEC_glTexImage1D;
	mFunctions[183] = EXEC_glTexImage2D;
	mFunctions[184] = EXEC_glTexEnvf;
	mFunctions[185] = EXEC_glTexEnvfv;
	mFunctions[186] = EXEC_glTexEnvi;
	mFunctions[187] = EXEC_glTexEnviv;
	mFunctions[188] = EXEC_glTexGend;
	mFunctions[189] = EXEC_glTexGendv;
	mFunctions[190] = EXEC_glTexGenf;
	mFunctions[191] = EXEC_glTexGenfv;
	mFunctions[192] = EXEC_glTexGeni;
	mFunctions[193] = EXEC_glTexGeniv;
	mFunctions[194] = EXEC_glFeedbackBuffer;
	mFunctions[195] = EXEC_glSelectBuffer;
	mFunctions[196] = EXEC_glRenderMode;
	mFunctions[197] = EXEC_glInitNames;
	mFunctions[198] = EXEC_glLoadName;
	mFunctions[199] = EXEC_glPassThrough;
	mFunctions[200] = EXEC_glPopName;
	mFunctions[201] = EXEC_glPushName;
	mFunctions[202] = EXEC_glDrawBuffer;
	mFunctions[203] = EXEC_glClear;
	mFunctions[204] = EXEC_glClearAccum;
	mFunctions[205] = EXEC_glClearIndex;
	mFunctions[206] = EXEC_glClearColor;
	mFunctions[207] = EXEC_glClearStencil;
	mFunctions[208] = EXEC_glClearDepth;
	mFunctions[209] = EXEC_glStencilMask;
	mFunctions[210] = EXEC_glColorMask;
	mFunctions[211] = EXEC_glDepthMask;
	mFunctions[212] = EXEC_glIndexMask;
	mFunctions[213] = EXEC_glAccum;
	mFunctions[214] = EXEC_glDisable;
	mFunctions[215] = EXEC_glEnable;
	mFunctions[216] = EXEC_glFinish;
	mFunctions[217] = EXEC_glFlush;
	mFunctions[218] = EXEC_glPopAttrib;
	mFunctions[219] = EXEC_glPushAttrib;
	mFunctions[220] = EXEC_glMap1d;
	mFunctions[221] = EXEC_glMap1f;
	mFunctions[222] = EXEC_glMap2d;
	mFunctions[223] = EXEC_glMap2f;
	mFunctions[224] = EXEC_glMapGrid1d;
	mFunctions[225] = EXEC_glMapGrid1f;
	mFunctions[226] = EXEC_glMapGrid2d;
	mFunctions[227] = EXEC_glMapGrid2f;
	mFunctions[228] = EXEC_glEvalCoord1d;
	mFunctions[229] = EXEC_glEvalCoord1dv;
	mFunctions[230] = EXEC_glEvalCoord1f;
	mFunctions[231] = EXEC_glEvalCoord1fv;
	mFunctions[232] = EXEC_glEvalCoord2d;
	mFunctions[233] = EXEC_glEvalCoord2dv;
	mFunctions[234] = EXEC_glEvalCoord2f;
	mFunctions[235] = EXEC_glEvalCoord2fv;
	mFunctions[236] = EXEC_glEvalMesh1;
	mFunctions[237] = EXEC_glEvalPoint1;
	mFunctions[238] = EXEC_glEvalMesh2;
	mFunctions[239] = EXEC_glEvalPoint2;
	mFunctions[240] = EXEC_glAlphaFunc;
	mFunctions[241] = EXEC_glBlendFunc;
	mFunctions[242] = EXEC_glLogicOp;
	mFunctions[243] = EXEC_glStencilFunc;
	mFunctions[244] = EXEC_glStencilOp;
	mFunctions[245] = EXEC_glDepthFunc;
	mFunctions[246] = EXEC_glPixelZoom;
	mFunctions[247] = EXEC_glPixelTransferf;
	mFunctions[248] = EXEC_glPixelTransferi;
	mFunctions[249] = EXEC_glPixelStoref;
	mFunctions[250] = EXEC_glPixelStorei;
	mFunctions[251] = EXEC_glPixelMapfv;
	mFunctions[252] = EXEC_glPixelMapuiv;
	mFunctions[253] = EXEC_glPixelMapusv;
	mFunctions[254] = EXEC_glReadBuffer;
	mFunctions[255] = EXEC_glCopyPixels;
	mFunctions[256] = EXEC_glReadPixels;
	mFunctions[257] = EXEC_glDrawPixels;
	mFunctions[258] = EXEC_glGetBooleanv;
	mFunctions[259] = EXEC_glGetClipPlane;
	mFunctions[260] = EXEC_glGetDoublev;
	mFunctions[261] = EXEC_glGetError;
	mFunctions[262] = EXEC_glGetFloatv;
	mFunctions[263] = EXEC_glGetIntegerv;
	mFunctions[264] = EXEC_glGetLightfv;
	mFunctions[265] = EXEC_glGetLightiv;
	mFunctions[266] = EXEC_glGetMapdv;
	mFunctions[267] = EXEC_glGetMapfv;
	mFunctions[268] = EXEC_glGetMapiv;
	mFunctions[269] = EXEC_glGetMaterialfv;
	mFunctions[270] = EXEC_glGetMaterialiv;
	mFunctions[271] = EXEC_glGetPixelMapfv;
	mFunctions[272] = EXEC_glGetPixelMapuiv;
	mFunctions[273] = EXEC_glGetPixelMapusv;
	mFunctions[274] = EXEC_glGetPolygonStipple;
	mFunctions[275] = EXEC_glGetString;
	mFunctions[276] = EXEC_glGetTexEnvfv;
	mFunctions[277] = EXEC_glGetTexEnviv;
	mFunctions[278] = EXEC_glGetTexGendv;
	mFunctions[279] = EXEC_glGetTexGenfv;
	mFunctions[280] = EXEC_glGetTexGeniv;
	mFunctions[281] = EXEC_glGetTexImage;
	mFunctions[282] = EXEC_glGetTexParameterfv;
	mFunctions[283] = EXEC_glGetTexParameteriv;
	mFunctions[284] = EXEC_glGetTexLevelParameterfv;
	mFunctions[285] = EXEC_glGetTexLevelParameteriv;
	mFunctions[286] = EXEC_glIsEnabled;
	mFunctions[287] = EXEC_glIsList;
	mFunctions[288] = EXEC_glDepthRange;
	mFunctions[289] = EXEC_glFrustum;
	mFunctions[290] = EXEC_glLoadIdentity;
	mFunctions[291] = EXEC_glLoadMatrixf;
	mFunctions[292] = EXEC_glLoadMatrixd;
	mFunctions[293] = EXEC_glMatrixMode;
	mFunctions[294] = EXEC_glMultMatrixf;
	mFunctions[295] = EXEC_glMultMatrixd;
	mFunctions[296] = EXEC_glOrtho;
	mFunctions[297] = EXEC_glPopMatrix;
	mFunctions[298] = EXEC_glPushMatrix;
	mFunctions[299] = EXEC_glRotated;
	mFunctions[300] = EXEC_glRotatef;
	mFunctions[301] = EXEC_glScaled;
	mFunctions[302] = EXEC_glScalef;
	mFunctions[303] = EXEC_glTranslated;
	mFunctions[304] = EXEC_glTranslatef;
	mFunctions[305] = EXEC_glViewport;
	mFunctions[306] = EXEC_glArrayElement;
	mFunctions[308] = EXEC_glColorPointer;
	mFunctions[309] = EXEC_glDisableClientState;
	mFunctions[310] = EXEC_glDrawArrays;
	mFunctions[311] = EXEC_glDrawElements;
	mFunctions[312] = EXEC_glEdgeFlagPointer;
	mFunctions[313] = EXEC_glEnableClientState;
	mFunctions[329] = EXEC_glGetPointerv;
	mFunctions[314] = EXEC_glIndexPointer;
	mFunctions[317] = EXEC_glInterleavedArrays;
	mFunctions[318] = EXEC_glNormalPointer;
	mFunctions[320] = EXEC_glTexCoordPointer;
	mFunctions[321] = EXEC_glVertexPointer;
	mFunctions[319] = EXEC_glPolygonOffset;
	mFunctions[323] = EXEC_glCopyTexImage1D;
	mFunctions[324] = EXEC_glCopyTexImage2D;
	mFunctions[325] = EXEC_glCopyTexSubImage1D;
	mFunctions[326] = EXEC_glCopyTexSubImage2D;
	mFunctions[332] = EXEC_glTexSubImage1D;
	mFunctions[333] = EXEC_glTexSubImage2D;
	mFunctions[322] = EXEC_glAreTexturesResident;
	mFunctions[307] = EXEC_glBindTexture;
	mFunctions[327] = EXEC_glDeleteTextures;
	mFunctions[328] = EXEC_glGenTextures;
	mFunctions[330] = EXEC_glIsTexture;
	mFunctions[331] = EXEC_glPrioritizeTextures;
	mFunctions[315] = EXEC_glIndexub;
	mFunctions[316] = EXEC_glIndexubv;
	mFunctions[334] = EXEC_glPopClientAttrib;
	mFunctions[335] = EXEC_glPushClientAttrib;
	mFunctions[336] = EXEC_glBlendColor;
	mFunctions[337] = EXEC_glBlendEquation;
	mFunctions[338] = EXEC_glDrawRangeElements;
	mFunctions[339] = EXEC_glColorTable;
	mFunctions[340] = EXEC_glColorTableParameterfv;
	mFunctions[341] = EXEC_glColorTableParameteriv;
	mFunctions[342] = EXEC_glCopyColorTable;
	mFunctions[343] = EXEC_glGetColorTable;
	mFunctions[344] = EXEC_glGetColorTableParameterfv;
	mFunctions[345] = EXEC_glGetColorTableParameteriv;
	mFunctions[346] = EXEC_glColorSubTable;
	mFunctions[347] = EXEC_glCopyColorSubTable;
	mFunctions[348] = EXEC_glConvolutionFilter1D;
	mFunctions[349] = EXEC_glConvolutionFilter2D;
	mFunctions[350] = EXEC_glConvolutionParameterf;
	mFunctions[351] = EXEC_glConvolutionParameterfv;
	mFunctions[352] = EXEC_glConvolutionParameteri;
	mFunctions[353] = EXEC_glConvolutionParameteriv;
	mFunctions[354] = EXEC_glCopyConvolutionFilter1D;
	mFunctions[355] = EXEC_glCopyConvolutionFilter2D;
	mFunctions[356] = EXEC_glGetConvolutionFilter;
	mFunctions[357] = EXEC_glGetConvolutionParameterfv;
	mFunctions[358] = EXEC_glGetConvolutionParameteriv;
	mFunctions[359] = EXEC_glGetSeparableFilter;
	mFunctions[360] = EXEC_glSeparableFilter2D;
	mFunctions[361] = EXEC_glGetHistogram;
	mFunctions[362] = EXEC_glGetHistogramParameterfv;
	mFunctions[363] = EXEC_glGetHistogramParameteriv;
	mFunctions[364] = EXEC_glGetMinmax;
	mFunctions[365] = EXEC_glGetMinmaxParameterfv;
	mFunctions[366] = EXEC_glGetMinmaxParameteriv;
	mFunctions[367] = EXEC_glHistogram;
	mFunctions[368] = EXEC_glMinmax;
	mFunctions[369] = EXEC_glResetHistogram;
	mFunctions[370] = EXEC_glResetMinmax;
	mFunctions[371] = EXEC_glTexImage3D;
	mFunctions[372] = EXEC_glTexSubImage3D;
	mFunctions[373] = EXEC_glCopyTexSubImage3D;
	mFunctions[374] = EXEC_glActiveTexture;
	mFunctions[375] = EXEC_glClientActiveTexture;
	mFunctions[376] = EXEC_glMultiTexCoord1d;
	mFunctions[377] = EXEC_glMultiTexCoord1dv;
	mFunctions[378] = EXEC_glMultiTexCoord1f;
	mFunctions[379] = EXEC_glMultiTexCoord1fv;
	mFunctions[380] = EXEC_glMultiTexCoord1i;
	mFunctions[381] = EXEC_glMultiTexCoord1iv;
	mFunctions[382] = EXEC_glMultiTexCoord1s;
	mFunctions[383] = EXEC_glMultiTexCoord1sv;
	mFunctions[384] = EXEC_glMultiTexCoord2d;
	mFunctions[385] = EXEC_glMultiTexCoord2dv;
	mFunctions[386] = EXEC_glMultiTexCoord2f;
	mFunctions[387] = EXEC_glMultiTexCoord2fv;
	mFunctions[388] = EXEC_glMultiTexCoord2i;
	mFunctions[389] = EXEC_glMultiTexCoord2iv;
	mFunctions[390] = EXEC_glMultiTexCoord2s;
	mFunctions[391] = EXEC_glMultiTexCoord2sv;
	mFunctions[392] = EXEC_glMultiTexCoord3d;
	mFunctions[393] = EXEC_glMultiTexCoord3dv;
	mFunctions[394] = EXEC_glMultiTexCoord3f;
	mFunctions[395] = EXEC_glMultiTexCoord3fv;
	mFunctions[396] = EXEC_glMultiTexCoord3i;
	mFunctions[397] = EXEC_glMultiTexCoord3iv;
	mFunctions[398] = EXEC_glMultiTexCoord3s;
	mFunctions[399] = EXEC_glMultiTexCoord3sv;
	mFunctions[400] = EXEC_glMultiTexCoord4d;
	mFunctions[401] = EXEC_glMultiTexCoord4dv;
	mFunctions[402] = EXEC_glMultiTexCoord4f;
	mFunctions[403] = EXEC_glMultiTexCoord4fv;
	mFunctions[404] = EXEC_glMultiTexCoord4i;
	mFunctions[405] = EXEC_glMultiTexCoord4iv;
	mFunctions[406] = EXEC_glMultiTexCoord4s;
	mFunctions[407] = EXEC_glMultiTexCoord4sv;
	mFunctions[408] = EXEC_glLoadTransposeMatrixf;
	mFunctions[409] = EXEC_glLoadTransposeMatrixd;
	mFunctions[410] = EXEC_glMultTransposeMatrixf;
	mFunctions[411] = EXEC_glMultTransposeMatrixd;
	mFunctions[412] = EXEC_glSampleCoverage;
	mFunctions[413] = EXEC_glCompressedTexImage3D;
	mFunctions[414] = EXEC_glCompressedTexImage2D;
	mFunctions[415] = EXEC_glCompressedTexImage1D;
	mFunctions[416] = EXEC_glCompressedTexSubImage3D;
	mFunctions[417] = EXEC_glCompressedTexSubImage2D;
	mFunctions[418] = EXEC_glCompressedTexSubImage1D;
	mFunctions[419] = EXEC_glGetCompressedTexImage;
	mFunctions[420] = EXEC_glBlendFuncSeparate;
	mFunctions[421] = EXEC_glFogCoordf;
	mFunctions[422] = EXEC_glFogCoordfv;
	mFunctions[423] = EXEC_glFogCoordd;
	mFunctions[424] = EXEC_glFogCoorddv;
	mFunctions[425] = EXEC_glFogCoordPointer;
	mFunctions[426] = EXEC_glMultiDrawArrays;
	mFunctions[427] = EXEC_glMultiDrawElements;
	mFunctions[428] = EXEC_glPointParameterf;
	mFunctions[429] = EXEC_glPointParameterfv;
	mFunctions[430] = EXEC_glPointParameteri;
	mFunctions[431] = EXEC_glPointParameteriv;
	mFunctions[432] = EXEC_glSecondaryColor3b;
	mFunctions[433] = EXEC_glSecondaryColor3bv;
	mFunctions[434] = EXEC_glSecondaryColor3d;
	mFunctions[435] = EXEC_glSecondaryColor3dv;
	mFunctions[436] = EXEC_glSecondaryColor3f;
	mFunctions[437] = EXEC_glSecondaryColor3fv;
	mFunctions[438] = EXEC_glSecondaryColor3i;
	mFunctions[439] = EXEC_glSecondaryColor3iv;
	mFunctions[440] = EXEC_glSecondaryColor3s;
	mFunctions[441] = EXEC_glSecondaryColor3sv;
	mFunctions[442] = EXEC_glSecondaryColor3ub;
	mFunctions[443] = EXEC_glSecondaryColor3ubv;
	mFunctions[444] = EXEC_glSecondaryColor3ui;
	mFunctions[445] = EXEC_glSecondaryColor3uiv;
	mFunctions[446] = EXEC_glSecondaryColor3us;
	mFunctions[447] = EXEC_glSecondaryColor3usv;
	mFunctions[448] = EXEC_glSecondaryColorPointer;
	mFunctions[449] = EXEC_glWindowPos2d;
	mFunctions[450] = EXEC_glWindowPos2dv;
	mFunctions[451] = EXEC_glWindowPos2f;
	mFunctions[452] = EXEC_glWindowPos2fv;
	mFunctions[453] = EXEC_glWindowPos2i;
	mFunctions[454] = EXEC_glWindowPos2iv;
	mFunctions[455] = EXEC_glWindowPos2s;
	mFunctions[456] = EXEC_glWindowPos2sv;
	mFunctions[457] = EXEC_glWindowPos3d;
	mFunctions[458] = EXEC_glWindowPos3dv;
	mFunctions[459] = EXEC_glWindowPos3f;
	mFunctions[460] = EXEC_glWindowPos3fv;
	mFunctions[461] = EXEC_glWindowPos3i;
	mFunctions[462] = EXEC_glWindowPos3iv;
	mFunctions[463] = EXEC_glWindowPos3s;
	mFunctions[464] = EXEC_glWindowPos3sv;
	mFunctions[465] = EXEC_glBindBuffer;
	mFunctions[466] = EXEC_glBufferData;
	mFunctions[467] = EXEC_glBufferSubData;
	mFunctions[468] = EXEC_glDeleteBuffers;
	mFunctions[469] = EXEC_glGenBuffers;
	mFunctions[470] = EXEC_glGetBufferParameteriv;
	mFunctions[471] = EXEC_glGetBufferPointerv;
	mFunctions[472] = EXEC_glGetBufferSubData;
	mFunctions[473] = EXEC_glIsBuffer;
	mFunctions[474] = EXEC_glMapBuffer;
	mFunctions[475] = EXEC_glUnmapBuffer;
	mFunctions[476] = EXEC_glGenQueries;
	mFunctions[477] = EXEC_glDeleteQueries;
	mFunctions[478] = EXEC_glIsQuery;
	mFunctions[479] = EXEC_glBeginQuery;
	mFunctions[480] = EXEC_glEndQuery;
	mFunctions[481] = EXEC_glGetQueryiv;
	mFunctions[482] = EXEC_glGetQueryObjectiv;
	mFunctions[483] = EXEC_glGetQueryObjectuiv;
	mFunctions[484] = EXEC_glBlendEquationSeparate;
	mFunctions[485] = EXEC_glDrawBuffers;
	mFunctions[486] = EXEC_glStencilFuncSeparate;
	mFunctions[487] = EXEC_glStencilOpSeparate;
	mFunctions[488] = EXEC_glStencilMaskSeparate;
	mFunctions[489] = EXEC_glAttachShader;
	mFunctions[490] = EXEC_glBindAttribLocation;
	mFunctions[491] = EXEC_glCompileShader;
	mFunctions[492] = EXEC_glCreateProgram;
	mFunctions[493] = EXEC_glCreateShader;
	mFunctions[494] = EXEC_glDeleteProgram;
	mFunctions[495] = EXEC_glDeleteShader;
	mFunctions[496] = EXEC_glDetachShader;
	mFunctions[497] = EXEC_glDisableVertexAttribArray;
	mFunctions[498] = EXEC_glEnableVertexAttribArray;
	mFunctions[499] = EXEC_glGetActiveAttrib;
	mFunctions[500] = EXEC_glGetActiveUniform;
	mFunctions[501] = EXEC_glGetAttachedShaders;
	mFunctions[502] = EXEC_glGetAttribLocation;
	mFunctions[503] = EXEC_glGetProgramiv;
	mFunctions[504] = EXEC_glGetProgramInfoLog;
	mFunctions[505] = EXEC_glGetShaderiv;
	mFunctions[506] = EXEC_glGetShaderInfoLog;
	mFunctions[507] = EXEC_glGetShaderSource;
	mFunctions[508] = EXEC_glGetUniformLocation;
	mFunctions[509] = EXEC_glGetUniformfv;
	mFunctions[510] = EXEC_glGetUniformiv;
	mFunctions[511] = EXEC_glGetVertexAttribdv;
	mFunctions[512] = EXEC_glGetVertexAttribfv;
	mFunctions[513] = EXEC_glGetVertexAttribiv;
	mFunctions[514] = EXEC_glGetVertexAttribPointerv;
	mFunctions[515] = EXEC_glIsProgram;
	mFunctions[516] = EXEC_glIsShader;
	mFunctions[517] = EXEC_glLinkProgram;
	mFunctions[518] = EXEC_glShaderSource;
	mFunctions[519] = EXEC_glUseProgram;
	mFunctions[520] = EXEC_glUniform1f;
	mFunctions[521] = EXEC_glUniform2f;
	mFunctions[522] = EXEC_glUniform3f;
	mFunctions[523] = EXEC_glUniform4f;
	mFunctions[524] = EXEC_glUniform1i;
	mFunctions[525] = EXEC_glUniform2i;
	mFunctions[526] = EXEC_glUniform3i;
	mFunctions[527] = EXEC_glUniform4i;
	mFunctions[528] = EXEC_glUniform1fv;
	mFunctions[529] = EXEC_glUniform2fv;
	mFunctions[530] = EXEC_glUniform3fv;
	mFunctions[531] = EXEC_glUniform4fv;
	mFunctions[532] = EXEC_glUniform1iv;
	mFunctions[533] = EXEC_glUniform2iv;
	mFunctions[534] = EXEC_glUniform3iv;
	mFunctions[535] = EXEC_glUniform4iv;
	mFunctions[536] = EXEC_glUniformMatrix2fv;
	mFunctions[537] = EXEC_glUniformMatrix3fv;
	mFunctions[538] = EXEC_glUniformMatrix4fv;
	mFunctions[539] = EXEC_glValidateProgram;
	mFunctions[540] = EXEC_glVertexAttrib1d;
	mFunctions[541] = EXEC_glVertexAttrib1dv;
	mFunctions[542] = EXEC_glVertexAttrib1f;
	mFunctions[543] = EXEC_glVertexAttrib1fv;
	mFunctions[544] = EXEC_glVertexAttrib1s;
	mFunctions[545] = EXEC_glVertexAttrib1sv;
	mFunctions[546] = EXEC_glVertexAttrib2d;
	mFunctions[547] = EXEC_glVertexAttrib2dv;
	mFunctions[548] = EXEC_glVertexAttrib2f;
	mFunctions[549] = EXEC_glVertexAttrib2fv;
	mFunctions[550] = EXEC_glVertexAttrib2s;
	mFunctions[551] = EXEC_glVertexAttrib2sv;
	mFunctions[552] = EXEC_glVertexAttrib3d;
	mFunctions[553] = EXEC_glVertexAttrib3dv;
	mFunctions[554] = EXEC_glVertexAttrib3f;
	mFunctions[555] = EXEC_glVertexAttrib3fv;
	mFunctions[556] = EXEC_glVertexAttrib3s;
	mFunctions[557] = EXEC_glVertexAttrib3sv;
	mFunctions[558] = EXEC_glVertexAttrib4Nbv;
	mFunctions[559] = EXEC_glVertexAttrib4Niv;
	mFunctions[560] = EXEC_glVertexAttrib4Nsv;
	mFunctions[561] = EXEC_glVertexAttrib4Nub;
	mFunctions[562] = EXEC_glVertexAttrib4Nubv;
	mFunctions[563] = EXEC_glVertexAttrib4Nuiv;
	mFunctions[564] = EXEC_glVertexAttrib4Nusv;
	mFunctions[565] = EXEC_glVertexAttrib4bv;
	mFunctions[566] = EXEC_glVertexAttrib4d;
	mFunctions[567] = EXEC_glVertexAttrib4dv;
	mFunctions[568] = EXEC_glVertexAttrib4f;
	mFunctions[569] = EXEC_glVertexAttrib4fv;
	mFunctions[570] = EXEC_glVertexAttrib4iv;
	mFunctions[571] = EXEC_glVertexAttrib4s;
	mFunctions[572] = EXEC_glVertexAttrib4sv;
	mFunctions[573] = EXEC_glVertexAttrib4ubv;
	mFunctions[574] = EXEC_glVertexAttrib4uiv;
	mFunctions[575] = EXEC_glVertexAttrib4usv;
	mFunctions[576] = EXEC_glVertexAttribPointer;
	mFunctions[577] = EXEC_glUniformMatrix2x3fv;
	mFunctions[578] = EXEC_glUniformMatrix3x2fv;
	mFunctions[579] = EXEC_glUniformMatrix2x4fv;
	mFunctions[580] = EXEC_glUniformMatrix4x2fv;
	mFunctions[581] = EXEC_glUniformMatrix3x4fv;
	mFunctions[582] = EXEC_glUniformMatrix4x3fv;
	mFunctions[374] = EXEC_glActiveTextureARB;
	mFunctions[375] = EXEC_glClientActiveTextureARB;
	mFunctions[376] = EXEC_glMultiTexCoord1dARB;
	mFunctions[377] = EXEC_glMultiTexCoord1dvARB;
	mFunctions[378] = EXEC_glMultiTexCoord1fARB;
	mFunctions[379] = EXEC_glMultiTexCoord1fvARB;
	mFunctions[380] = EXEC_glMultiTexCoord1iARB;
	mFunctions[381] = EXEC_glMultiTexCoord1ivARB;
	mFunctions[382] = EXEC_glMultiTexCoord1sARB;
	mFunctions[383] = EXEC_glMultiTexCoord1svARB;
	mFunctions[384] = EXEC_glMultiTexCoord2dARB;
	mFunctions[385] = EXEC_glMultiTexCoord2dvARB;
	mFunctions[386] = EXEC_glMultiTexCoord2fARB;
	mFunctions[387] = EXEC_glMultiTexCoord2fvARB;
	mFunctions[388] = EXEC_glMultiTexCoord2iARB;
	mFunctions[389] = EXEC_glMultiTexCoord2ivARB;
	mFunctions[390] = EXEC_glMultiTexCoord2sARB;
	mFunctions[391] = EXEC_glMultiTexCoord2svARB;
	mFunctions[392] = EXEC_glMultiTexCoord3dARB;
	mFunctions[393] = EXEC_glMultiTexCoord3dvARB;
	mFunctions[394] = EXEC_glMultiTexCoord3fARB;
	mFunctions[395] = EXEC_glMultiTexCoord3fvARB;
	mFunctions[396] = EXEC_glMultiTexCoord3iARB;
	mFunctions[397] = EXEC_glMultiTexCoord3ivARB;
	mFunctions[398] = EXEC_glMultiTexCoord3sARB;
	mFunctions[399] = EXEC_glMultiTexCoord3svARB;
	mFunctions[400] = EXEC_glMultiTexCoord4dARB;
	mFunctions[401] = EXEC_glMultiTexCoord4dvARB;
	mFunctions[402] = EXEC_glMultiTexCoord4fARB;
	mFunctions[403] = EXEC_glMultiTexCoord4fvARB;
	mFunctions[404] = EXEC_glMultiTexCoord4iARB;
	mFunctions[405] = EXEC_glMultiTexCoord4ivARB;
	mFunctions[406] = EXEC_glMultiTexCoord4sARB;
	mFunctions[407] = EXEC_glMultiTexCoord4svARB;
	mFunctions[617] = EXEC_glLoadTransposeMatrixfARB;
	mFunctions[618] = EXEC_glLoadTransposeMatrixdARB;
	mFunctions[619] = EXEC_glMultTransposeMatrixfARB;
	mFunctions[620] = EXEC_glMultTransposeMatrixdARB;
	mFunctions[621] = EXEC_glSampleCoverageARB;
	mFunctions[622] = EXEC_glCompressedTexImage3DARB;
	mFunctions[623] = EXEC_glCompressedTexImage2DARB;
	mFunctions[624] = EXEC_glCompressedTexImage1DARB;
	mFunctions[625] = EXEC_glCompressedTexSubImage3DARB;
	mFunctions[626] = EXEC_glCompressedTexSubImage2DARB;
	mFunctions[627] = EXEC_glCompressedTexSubImage1DARB;
	mFunctions[628] = EXEC_glGetCompressedTexImageARB;
	mFunctions[629] = EXEC_glPointParameterfARB;
	mFunctions[630] = EXEC_glPointParameterfvARB;
	mFunctions[631] = EXEC_glWeightbvARB;
	mFunctions[632] = EXEC_glWeightsvARB;
	mFunctions[633] = EXEC_glWeightivARB;
	mFunctions[634] = EXEC_glWeightfvARB;
	mFunctions[635] = EXEC_glWeightdvARB;
	mFunctions[636] = EXEC_glWeightubvARB;
	mFunctions[637] = EXEC_glWeightusvARB;
	mFunctions[638] = EXEC_glWeightuivARB;
	mFunctions[639] = EXEC_glWeightPointerARB;
	mFunctions[640] = EXEC_glVertexBlendARB;
	mFunctions[641] = EXEC_glCurrentPaletteMatrixARB;
	mFunctions[642] = EXEC_glMatrixIndexubvARB;
	mFunctions[643] = EXEC_glMatrixIndexusvARB;
	mFunctions[644] = EXEC_glMatrixIndexuivARB;
	mFunctions[645] = EXEC_glMatrixIndexPointerARB;
	mFunctions[646] = EXEC_glWindowPos2dARB;
	mFunctions[647] = EXEC_glWindowPos2fARB;
	mFunctions[648] = EXEC_glWindowPos2iARB;
	mFunctions[649] = EXEC_glWindowPos2sARB;
	mFunctions[650] = EXEC_glWindowPos2dvARB;
	mFunctions[651] = EXEC_glWindowPos2fvARB;
	mFunctions[652] = EXEC_glWindowPos2ivARB;
	mFunctions[653] = EXEC_glWindowPos2svARB;
	mFunctions[654] = EXEC_glWindowPos3dARB;
	mFunctions[655] = EXEC_glWindowPos3fARB;
	mFunctions[656] = EXEC_glWindowPos3iARB;
	mFunctions[657] = EXEC_glWindowPos3sARB;
	mFunctions[658] = EXEC_glWindowPos3dvARB;
	mFunctions[659] = EXEC_glWindowPos3fvARB;
	mFunctions[660] = EXEC_glWindowPos3ivARB;
	mFunctions[661] = EXEC_glWindowPos3svARB;
	mFunctions[662] = EXEC_glGetVertexAttribdvARB;
	mFunctions[663] = EXEC_glGetVertexAttribfvARB;
	mFunctions[664] = EXEC_glGetVertexAttribivARB;
	mFunctions[665] = EXEC_glVertexAttrib1dARB;
	mFunctions[666] = EXEC_glVertexAttrib1dvARB;
	mFunctions[667] = EXEC_glVertexAttrib1fARB;
	mFunctions[668] = EXEC_glVertexAttrib1fvARB;
	mFunctions[669] = EXEC_glVertexAttrib1sARB;
	mFunctions[670] = EXEC_glVertexAttrib1svARB;
	mFunctions[671] = EXEC_glVertexAttrib2dARB;
	mFunctions[672] = EXEC_glVertexAttrib2dvARB;
	mFunctions[673] = EXEC_glVertexAttrib2fARB;
	mFunctions[674] = EXEC_glVertexAttrib2fvARB;
	mFunctions[675] = EXEC_glVertexAttrib2sARB;
	mFunctions[676] = EXEC_glVertexAttrib2svARB;
	mFunctions[677] = EXEC_glVertexAttrib3dARB;
	mFunctions[678] = EXEC_glVertexAttrib3dvARB;
	mFunctions[679] = EXEC_glVertexAttrib3fARB;
	mFunctions[680] = EXEC_glVertexAttrib3fvARB;
	mFunctions[681] = EXEC_glVertexAttrib3sARB;
	mFunctions[682] = EXEC_glVertexAttrib3svARB;
	mFunctions[683] = EXEC_glVertexAttrib4dARB;
	mFunctions[684] = EXEC_glVertexAttrib4dvARB;
	mFunctions[685] = EXEC_glVertexAttrib4fARB;
	mFunctions[686] = EXEC_glVertexAttrib4fvARB;
	mFunctions[687] = EXEC_glVertexAttrib4sARB;
	mFunctions[688] = EXEC_glVertexAttrib4svARB;
	mFunctions[689] = EXEC_glVertexAttrib4NubARB;
	mFunctions[690] = EXEC_glVertexAttrib4NubvARB;
	mFunctions[691] = EXEC_glVertexAttrib4bvARB;
	mFunctions[692] = EXEC_glVertexAttrib4ivARB;
	mFunctions[693] = EXEC_glVertexAttrib4ubvARB;
	mFunctions[694] = EXEC_glVertexAttrib4usvARB;
	mFunctions[695] = EXEC_glVertexAttrib4uivARB;
	mFunctions[696] = EXEC_glVertexAttrib4NbvARB;
	mFunctions[697] = EXEC_glVertexAttrib4NsvARB;
	mFunctions[698] = EXEC_glVertexAttrib4NivARB;
	mFunctions[699] = EXEC_glVertexAttrib4NusvARB;
	mFunctions[700] = EXEC_glVertexAttrib4NuivARB;
	mFunctions[701] = EXEC_glVertexAttribPointerARB;
	mFunctions[702] = EXEC_glEnableVertexAttribArrayARB;
	mFunctions[703] = EXEC_glDisableVertexAttribArrayARB;
	mFunctions[704] = EXEC_glProgramStringARB;
	mFunctions[705] = EXEC_glBindProgramARB;
	mFunctions[706] = EXEC_glDeleteProgramsARB;
	mFunctions[707] = EXEC_glGenProgramsARB;
	mFunctions[708] = EXEC_glIsProgramARB;
	mFunctions[709] = EXEC_glProgramEnvParameter4dARB;
	mFunctions[710] = EXEC_glProgramEnvParameter4dvARB;
	mFunctions[711] = EXEC_glProgramEnvParameter4fARB;
	mFunctions[712] = EXEC_glProgramEnvParameter4fvARB;
	mFunctions[713] = EXEC_glProgramLocalParameter4dARB;
	mFunctions[714] = EXEC_glProgramLocalParameter4dvARB;
	mFunctions[715] = EXEC_glProgramLocalParameter4fARB;
	mFunctions[716] = EXEC_glProgramLocalParameter4fvARB;
	mFunctions[717] = EXEC_glGetProgramEnvParameterdvARB;
	mFunctions[718] = EXEC_glGetProgramEnvParameterfvARB;
	mFunctions[719] = EXEC_glGetProgramLocalParameterdvARB;
	mFunctions[720] = EXEC_glGetProgramLocalParameterfvARB;
	mFunctions[721] = EXEC_glGetProgramivARB;
	mFunctions[722] = EXEC_glGetProgramStringARB;
	mFunctions[723] = EXEC_glGetVertexAttribPointervARB;
	mFunctions[724] = EXEC_glBindBufferARB;
	mFunctions[725] = EXEC_glBufferDataARB;
	mFunctions[726] = EXEC_glBufferSubDataARB;
	mFunctions[727] = EXEC_glDeleteBuffersARB;
	mFunctions[728] = EXEC_glGenBuffersARB;
	mFunctions[729] = EXEC_glGetBufferParameterivARB;
	mFunctions[730] = EXEC_glGetBufferPointervARB;
	mFunctions[731] = EXEC_glGetBufferSubDataARB;
	mFunctions[732] = EXEC_glIsBufferARB;
	mFunctions[733] = EXEC_glMapBufferARB;
	mFunctions[734] = EXEC_glUnmapBufferARB;
	mFunctions[735] = EXEC_glGenQueriesARB;
	mFunctions[736] = EXEC_glDeleteQueriesARB;
	mFunctions[737] = EXEC_glIsQueryARB;
	mFunctions[738] = EXEC_glBeginQueryARB;
	mFunctions[739] = EXEC_glEndQueryARB;
	mFunctions[740] = EXEC_glGetQueryivARB;
	mFunctions[741] = EXEC_glGetQueryObjectivARB;
	mFunctions[742] = EXEC_glGetQueryObjectuivARB;
	mFunctions[743] = EXEC_glDeleteObjectARB;
	mFunctions[744] = EXEC_glGetHandleARB;
	mFunctions[745] = EXEC_glDetachObjectARB;
	mFunctions[746] = EXEC_glCreateShaderObjectARB;
	mFunctions[747] = EXEC_glShaderSourceARB;
	mFunctions[748] = EXEC_glCompileShaderARB;
	mFunctions[749] = EXEC_glCreateProgramObjectARB;
	mFunctions[750] = EXEC_glAttachObjectARB;
	mFunctions[751] = EXEC_glLinkProgramARB;
	mFunctions[752] = EXEC_glUseProgramObjectARB;
	mFunctions[753] = EXEC_glValidateProgramARB;
	mFunctions[754] = EXEC_glUniform1fARB;
	mFunctions[755] = EXEC_glUniform2fARB;
	mFunctions[756] = EXEC_glUniform3fARB;
	mFunctions[757] = EXEC_glUniform4fARB;
	mFunctions[758] = EXEC_glUniform1iARB;
	mFunctions[759] = EXEC_glUniform2iARB;
	mFunctions[760] = EXEC_glUniform3iARB;
	mFunctions[761] = EXEC_glUniform4iARB;
	mFunctions[762] = EXEC_glUniform1fvARB;
	mFunctions[763] = EXEC_glUniform2fvARB;
	mFunctions[764] = EXEC_glUniform3fvARB;
	mFunctions[765] = EXEC_glUniform4fvARB;
	mFunctions[766] = EXEC_glUniform1ivARB;
	mFunctions[767] = EXEC_glUniform2ivARB;
	mFunctions[768] = EXEC_glUniform3ivARB;
	mFunctions[769] = EXEC_glUniform4ivARB;
	mFunctions[770] = EXEC_glUniformMatrix2fvARB;
	mFunctions[771] = EXEC_glUniformMatrix3fvARB;
	mFunctions[772] = EXEC_glUniformMatrix4fvARB;
	mFunctions[773] = EXEC_glGetObjectParameterfvARB;
	mFunctions[774] = EXEC_glGetObjectParameterivARB;
	mFunctions[775] = EXEC_glGetInfoLogARB;
	mFunctions[776] = EXEC_glGetAttachedObjectsARB;
	mFunctions[777] = EXEC_glGetUniformLocationARB;
	mFunctions[778] = EXEC_glGetActiveUniformARB;
	mFunctions[779] = EXEC_glGetUniformfvARB;
	mFunctions[780] = EXEC_glGetUniformivARB;
	mFunctions[781] = EXEC_glGetShaderSourceARB;
	mFunctions[782] = EXEC_glBindAttribLocationARB;
	mFunctions[783] = EXEC_glGetActiveAttribARB;
	mFunctions[784] = EXEC_glGetAttribLocationARB;
	mFunctions[785] = EXEC_glDrawBuffersARB;
	mFunctions[786] = EXEC_glBlendColorEXT;
	mFunctions[787] = EXEC_glPolygonOffsetEXT;
	mFunctions[788] = EXEC_glTexImage3DEXT;
	mFunctions[789] = EXEC_glTexSubImage3DEXT;
	mFunctions[790] = EXEC_glGetTexFilterFuncSGIS;
	mFunctions[791] = EXEC_glTexFilterFuncSGIS;
	mFunctions[792] = EXEC_glTexSubImage1DEXT;
	mFunctions[793] = EXEC_glTexSubImage2DEXT;
	mFunctions[794] = EXEC_glCopyTexImage1DEXT;
	mFunctions[795] = EXEC_glCopyTexImage2DEXT;
	mFunctions[796] = EXEC_glCopyTexSubImage1DEXT;
	mFunctions[797] = EXEC_glCopyTexSubImage2DEXT;
	mFunctions[798] = EXEC_glCopyTexSubImage3DEXT;
	mFunctions[799] = EXEC_glGetHistogramEXT;
	mFunctions[800] = EXEC_glGetHistogramParameterfvEXT;
	mFunctions[801] = EXEC_glGetHistogramParameterivEXT;
	mFunctions[802] = EXEC_glGetMinmaxEXT;
	mFunctions[803] = EXEC_glGetMinmaxParameterfvEXT;
	mFunctions[804] = EXEC_glGetMinmaxParameterivEXT;
	mFunctions[805] = EXEC_glHistogramEXT;
	mFunctions[806] = EXEC_glMinmaxEXT;
	mFunctions[807] = EXEC_glResetHistogramEXT;
	mFunctions[808] = EXEC_glResetMinmaxEXT;
	mFunctions[809] = EXEC_glConvolutionFilter1DEXT;
	mFunctions[810] = EXEC_glConvolutionFilter2DEXT;
	mFunctions[811] = EXEC_glConvolutionParameterfEXT;
	mFunctions[812] = EXEC_glConvolutionParameterfvEXT;
	mFunctions[813] = EXEC_glConvolutionParameteriEXT;
	mFunctions[814] = EXEC_glConvolutionParameterivEXT;
	mFunctions[815] = EXEC_glCopyConvolutionFilter1DEXT;
	mFunctions[816] = EXEC_glCopyConvolutionFilter2DEXT;
	mFunctions[817] = EXEC_glGetConvolutionFilterEXT;
	mFunctions[818] = EXEC_glGetConvolutionParameterfvEXT;
	mFunctions[819] = EXEC_glGetConvolutionParameterivEXT;
	mFunctions[820] = EXEC_glGetSeparableFilterEXT;
	mFunctions[821] = EXEC_glSeparableFilter2DEXT;
	mFunctions[822] = EXEC_glColorTableSGI;
	mFunctions[823] = EXEC_glColorTableParameterfvSGI;
	mFunctions[824] = EXEC_glColorTableParameterivSGI;
	mFunctions[825] = EXEC_glCopyColorTableSGI;
	mFunctions[826] = EXEC_glGetColorTableSGI;
	mFunctions[827] = EXEC_glGetColorTableParameterfvSGI;
	mFunctions[828] = EXEC_glGetColorTableParameterivSGI;
	mFunctions[829] = EXEC_glPixelTexGenParameteriSGIS;
	mFunctions[830] = EXEC_glPixelTexGenParameterivSGIS;
	mFunctions[831] = EXEC_glPixelTexGenParameterfSGIS;
	mFunctions[832] = EXEC_glPixelTexGenParameterfvSGIS;
	mFunctions[833] = EXEC_glGetPixelTexGenParameterivSGIS;
	mFunctions[834] = EXEC_glGetPixelTexGenParameterfvSGIS;
	mFunctions[835] = EXEC_glTexImage4DSGIS;
	mFunctions[836] = EXEC_glTexSubImage4DSGIS;
	mFunctions[837] = EXEC_glAreTexturesResidentEXT;
	mFunctions[838] = EXEC_glBindTextureEXT;
	mFunctions[839] = EXEC_glDeleteTexturesEXT;
	mFunctions[840] = EXEC_glGenTexturesEXT;
	mFunctions[841] = EXEC_glIsTextureEXT;
	mFunctions[842] = EXEC_glPrioritizeTexturesEXT;
	mFunctions[843] = EXEC_glDetailTexFuncSGIS;
	mFunctions[844] = EXEC_glGetDetailTexFuncSGIS;
	mFunctions[845] = EXEC_glSharpenTexFuncSGIS;
	mFunctions[846] = EXEC_glGetSharpenTexFuncSGIS;
	mFunctions[847] = EXEC_glSampleMaskSGIS;
	mFunctions[848] = EXEC_glSamplePatternSGIS;
	mFunctions[849] = EXEC_glArrayElementEXT;
	mFunctions[850] = EXEC_glColorPointerEXT;
	mFunctions[851] = EXEC_glDrawArraysEXT;
	mFunctions[852] = EXEC_glEdgeFlagPointerEXT;
	mFunctions[853] = EXEC_glGetPointervEXT;
	mFunctions[854] = EXEC_glIndexPointerEXT;
	mFunctions[855] = EXEC_glNormalPointerEXT;
	mFunctions[856] = EXEC_glTexCoordPointerEXT;
	mFunctions[857] = EXEC_glVertexPointerEXT;
	mFunctions[858] = EXEC_glBlendEquationEXT;
	mFunctions[859] = EXEC_glSpriteParameterfSGIX;
	mFunctions[860] = EXEC_glSpriteParameterfvSGIX;
	mFunctions[861] = EXEC_glSpriteParameteriSGIX;
	mFunctions[862] = EXEC_glSpriteParameterivSGIX;
	mFunctions[863] = EXEC_glPointParameterfEXT;
	mFunctions[864] = EXEC_glPointParameterfvEXT;
	mFunctions[865] = EXEC_glGetInstrumentsSGIX;
	mFunctions[866] = EXEC_glInstrumentsBufferSGIX;
	mFunctions[867] = EXEC_glPollInstrumentsSGIX;
	mFunctions[868] = EXEC_glReadInstrumentsSGIX;
	mFunctions[869] = EXEC_glStartInstrumentsSGIX;
	mFunctions[870] = EXEC_glStopInstrumentsSGIX;
	mFunctions[871] = EXEC_glFrameZoomSGIX;
	mFunctions[872] = EXEC_glTagSampleBufferSGIX;
	mFunctions[873] = EXEC_glReferencePlaneSGIX;
	mFunctions[874] = EXEC_glFlushRasterSGIX;
	mFunctions[875] = EXEC_glFogFuncSGIS;
	mFunctions[876] = EXEC_glGetFogFuncSGIS;
	mFunctions[877] = EXEC_glImageTransformParameteriHP;
	mFunctions[878] = EXEC_glImageTransformParameterfHP;
	mFunctions[879] = EXEC_glImageTransformParameterivHP;
	mFunctions[880] = EXEC_glImageTransformParameterfvHP;
	mFunctions[881] = EXEC_glGetImageTransformParameterivHP;
	mFunctions[882] = EXEC_glGetImageTransformParameterfvHP;
	mFunctions[883] = EXEC_glColorSubTableEXT;
	mFunctions[884] = EXEC_glCopyColorSubTableEXT;
	mFunctions[885] = EXEC_glHintPGI;
	mFunctions[886] = EXEC_glColorTableEXT;
	mFunctions[887] = EXEC_glGetColorTableEXT;
	mFunctions[888] = EXEC_glGetColorTableParameterivEXT;
	mFunctions[889] = EXEC_glGetColorTableParameterfvEXT;
	mFunctions[890] = EXEC_glGetListParameterfvSGIX;
	mFunctions[891] = EXEC_glGetListParameterivSGIX;
	mFunctions[892] = EXEC_glListParameterfSGIX;
	mFunctions[893] = EXEC_glListParameterfvSGIX;
	mFunctions[894] = EXEC_glListParameteriSGIX;
	mFunctions[895] = EXEC_glListParameterivSGIX;
	mFunctions[896] = EXEC_glIndexMaterialEXT;
	mFunctions[897] = EXEC_glIndexFuncEXT;
	mFunctions[898] = EXEC_glLockArraysEXT;
	mFunctions[899] = EXEC_glUnlockArraysEXT;
	mFunctions[900] = EXEC_glCullParameterdvEXT;
	mFunctions[901] = EXEC_glCullParameterfvEXT;
	mFunctions[902] = EXEC_glFragmentColorMaterialSGIX;
	mFunctions[903] = EXEC_glFragmentLightfSGIX;
	mFunctions[904] = EXEC_glFragmentLightfvSGIX;
	mFunctions[905] = EXEC_glFragmentLightiSGIX;
	mFunctions[906] = EXEC_glFragmentLightivSGIX;
	mFunctions[907] = EXEC_glFragmentLightModelfSGIX;
	mFunctions[908] = EXEC_glFragmentLightModelfvSGIX;
	mFunctions[909] = EXEC_glFragmentLightModeliSGIX;
	mFunctions[910] = EXEC_glFragmentLightModelivSGIX;
	mFunctions[911] = EXEC_glFragmentMaterialfSGIX;
	mFunctions[912] = EXEC_glFragmentMaterialfvSGIX;
	mFunctions[913] = EXEC_glFragmentMaterialiSGIX;
	mFunctions[914] = EXEC_glFragmentMaterialivSGIX;
	mFunctions[915] = EXEC_glGetFragmentLightfvSGIX;
	mFunctions[916] = EXEC_glGetFragmentLightivSGIX;
	mFunctions[917] = EXEC_glGetFragmentMaterialfvSGIX;
	mFunctions[918] = EXEC_glGetFragmentMaterialivSGIX;
	mFunctions[919] = EXEC_glLightEnviSGIX;
	mFunctions[920] = EXEC_glDrawRangeElementsEXT;
	mFunctions[921] = EXEC_glApplyTextureEXT;
	mFunctions[922] = EXEC_glTextureLightEXT;
	mFunctions[923] = EXEC_glTextureMaterialEXT;
	mFunctions[924] = EXEC_glAsyncMarkerSGIX;
	mFunctions[925] = EXEC_glFinishAsyncSGIX;
	mFunctions[926] = EXEC_glPollAsyncSGIX;
	mFunctions[927] = EXEC_glGenAsyncMarkersSGIX;
	mFunctions[928] = EXEC_glDeleteAsyncMarkersSGIX;
	mFunctions[929] = EXEC_glIsAsyncMarkerSGIX;
	mFunctions[930] = EXEC_glVertexPointervINTEL;
	mFunctions[931] = EXEC_glNormalPointervINTEL;
	mFunctions[932] = EXEC_glColorPointervINTEL;
	mFunctions[933] = EXEC_glTexCoordPointervINTEL;
	mFunctions[934] = EXEC_glPixelTransformParameteriEXT;
	mFunctions[935] = EXEC_glPixelTransformParameterfEXT;
	mFunctions[936] = EXEC_glPixelTransformParameterivEXT;
	mFunctions[937] = EXEC_glPixelTransformParameterfvEXT;
	mFunctions[938] = EXEC_glSecondaryColor3bEXT;
	mFunctions[939] = EXEC_glSecondaryColor3bvEXT;
	mFunctions[940] = EXEC_glSecondaryColor3dEXT;
	mFunctions[941] = EXEC_glSecondaryColor3dvEXT;
	mFunctions[942] = EXEC_glSecondaryColor3fEXT;
	mFunctions[943] = EXEC_glSecondaryColor3fvEXT;
	mFunctions[944] = EXEC_glSecondaryColor3iEXT;
	mFunctions[945] = EXEC_glSecondaryColor3ivEXT;
	mFunctions[946] = EXEC_glSecondaryColor3sEXT;
	mFunctions[947] = EXEC_glSecondaryColor3svEXT;
	mFunctions[948] = EXEC_glSecondaryColor3ubEXT;
	mFunctions[949] = EXEC_glSecondaryColor3ubvEXT;
	mFunctions[950] = EXEC_glSecondaryColor3uiEXT;
	mFunctions[951] = EXEC_glSecondaryColor3uivEXT;
	mFunctions[952] = EXEC_glSecondaryColor3usEXT;
	mFunctions[953] = EXEC_glSecondaryColor3usvEXT;
	mFunctions[954] = EXEC_glSecondaryColorPointerEXT;
	mFunctions[955] = EXEC_glTextureNormalEXT;
	mFunctions[956] = EXEC_glMultiDrawArraysEXT;
	mFunctions[957] = EXEC_glMultiDrawElementsEXT;
	mFunctions[958] = EXEC_glFogCoordfEXT;
	mFunctions[959] = EXEC_glFogCoordfvEXT;
	mFunctions[960] = EXEC_glFogCoorddEXT;
	mFunctions[961] = EXEC_glFogCoorddvEXT;
	mFunctions[962] = EXEC_glFogCoordPointerEXT;
	mFunctions[963] = EXEC_glTangent3bEXT;
	mFunctions[964] = EXEC_glTangent3bvEXT;
	mFunctions[965] = EXEC_glTangent3dEXT;
	mFunctions[966] = EXEC_glTangent3dvEXT;
	mFunctions[967] = EXEC_glTangent3fEXT;
	mFunctions[968] = EXEC_glTangent3fvEXT;
	mFunctions[969] = EXEC_glTangent3iEXT;
	mFunctions[970] = EXEC_glTangent3ivEXT;
	mFunctions[971] = EXEC_glTangent3sEXT;
	mFunctions[972] = EXEC_glTangent3svEXT;
	mFunctions[973] = EXEC_glBinormal3bEXT;
	mFunctions[974] = EXEC_glBinormal3bvEXT;
	mFunctions[975] = EXEC_glBinormal3dEXT;
	mFunctions[976] = EXEC_glBinormal3dvEXT;
	mFunctions[977] = EXEC_glBinormal3fEXT;
	mFunctions[978] = EXEC_glBinormal3fvEXT;
	mFunctions[979] = EXEC_glBinormal3iEXT;
	mFunctions[980] = EXEC_glBinormal3ivEXT;
	mFunctions[981] = EXEC_glBinormal3sEXT;
	mFunctions[982] = EXEC_glBinormal3svEXT;
	mFunctions[983] = EXEC_glTangentPointerEXT;
	mFunctions[984] = EXEC_glBinormalPointerEXT;
	mFunctions[985] = EXEC_glPixelTexGenSGIX;
	mFunctions[986] = EXEC_glFinishTextureSUNX;
	mFunctions[987] = EXEC_glGlobalAlphaFactorbSUN;
	mFunctions[988] = EXEC_glGlobalAlphaFactorsSUN;
	mFunctions[989] = EXEC_glGlobalAlphaFactoriSUN;
	mFunctions[990] = EXEC_glGlobalAlphaFactorfSUN;
	mFunctions[991] = EXEC_glGlobalAlphaFactordSUN;
	mFunctions[992] = EXEC_glGlobalAlphaFactorubSUN;
	mFunctions[993] = EXEC_glGlobalAlphaFactorusSUN;
	mFunctions[994] = EXEC_glGlobalAlphaFactoruiSUN;
	mFunctions[995] = EXEC_glReplacementCodeuiSUN;
	mFunctions[996] = EXEC_glReplacementCodeusSUN;
	mFunctions[997] = EXEC_glReplacementCodeubSUN;
	mFunctions[998] = EXEC_glReplacementCodeuivSUN;
	mFunctions[999] = EXEC_glReplacementCodeusvSUN;
	mFunctions[1000] = EXEC_glReplacementCodeubvSUN;
	mFunctions[1001] = EXEC_glReplacementCodePointerSUN;
	mFunctions[1002] = EXEC_glColor4ubVertex2fSUN;
	mFunctions[1003] = EXEC_glColor4ubVertex2fvSUN;
	mFunctions[1004] = EXEC_glColor4ubVertex3fSUN;
	mFunctions[1005] = EXEC_glColor4ubVertex3fvSUN;
	mFunctions[1006] = EXEC_glColor3fVertex3fSUN;
	mFunctions[1007] = EXEC_glColor3fVertex3fvSUN;
	mFunctions[1008] = EXEC_glNormal3fVertex3fSUN;
	mFunctions[1009] = EXEC_glNormal3fVertex3fvSUN;
	mFunctions[1010] = EXEC_glColor4fNormal3fVertex3fSUN;
	mFunctions[1011] = EXEC_glColor4fNormal3fVertex3fvSUN;
	mFunctions[1012] = EXEC_glTexCoord2fVertex3fSUN;
	mFunctions[1013] = EXEC_glTexCoord2fVertex3fvSUN;
	mFunctions[1014] = EXEC_glTexCoord4fVertex4fSUN;
	mFunctions[1015] = EXEC_glTexCoord4fVertex4fvSUN;
	mFunctions[1016] = EXEC_glTexCoord2fColor4ubVertex3fSUN;
	mFunctions[1017] = EXEC_glTexCoord2fColor4ubVertex3fvSUN;
	mFunctions[1018] = EXEC_glTexCoord2fColor3fVertex3fSUN;
	mFunctions[1019] = EXEC_glTexCoord2fColor3fVertex3fvSUN;
	mFunctions[1020] = EXEC_glTexCoord2fNormal3fVertex3fSUN;
	mFunctions[1021] = EXEC_glTexCoord2fNormal3fVertex3fvSUN;
	mFunctions[1022] = EXEC_glTexCoord2fColor4fNormal3fVertex3fSUN;
	mFunctions[1023] = EXEC_glTexCoord2fColor4fNormal3fVertex3fvSUN;
	mFunctions[1024] = EXEC_glTexCoord4fColor4fNormal3fVertex4fSUN;
	mFunctions[1025] = EXEC_glTexCoord4fColor4fNormal3fVertex4fvSUN;
	mFunctions[1026] = EXEC_glReplacementCodeuiVertex3fSUN;
	mFunctions[1027] = EXEC_glReplacementCodeuiVertex3fvSUN;
	mFunctions[1028] = EXEC_glReplacementCodeuiColor4ubVertex3fSUN;
	mFunctions[1029] = EXEC_glReplacementCodeuiColor4ubVertex3fvSUN;
	mFunctions[1030] = EXEC_glReplacementCodeuiColor3fVertex3fSUN;
	mFunctions[1031] = EXEC_glReplacementCodeuiColor3fVertex3fvSUN;
	mFunctions[1032] = EXEC_glReplacementCodeuiNormal3fVertex3fSUN;
	mFunctions[1033] = EXEC_glReplacementCodeuiNormal3fVertex3fvSUN;
	mFunctions[1034] = EXEC_glReplacementCodeuiColor4fNormal3fVertex3fSUN;
	mFunctions[1035] = EXEC_glReplacementCodeuiColor4fNormal3fVertex3fvSUN;
	mFunctions[1036] = EXEC_glReplacementCodeuiTexCoord2fVertex3fSUN;
	mFunctions[1037] = EXEC_glReplacementCodeuiTexCoord2fVertex3fvSUN;
	mFunctions[1038] = EXEC_glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN;
	mFunctions[1039] = EXEC_glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN;
	mFunctions[1040] = EXEC_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN;
	mFunctions[1041] = EXEC_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN;
	mFunctions[1042] = EXEC_glBlendFuncSeparateEXT;
	mFunctions[1043] = EXEC_glVertexWeightfEXT;
	mFunctions[1044] = EXEC_glVertexWeightfvEXT;
	mFunctions[1045] = EXEC_glVertexWeightPointerEXT;
	mFunctions[1046] = EXEC_glFlushVertexArrayRangeNV;
	mFunctions[1047] = EXEC_glVertexArrayRangeNV;
	mFunctions[1048] = EXEC_glCombinerParameterfvNV;
	mFunctions[1049] = EXEC_glCombinerParameterfNV;
	mFunctions[1050] = EXEC_glCombinerParameterivNV;
	mFunctions[1051] = EXEC_glCombinerParameteriNV;
	mFunctions[1052] = EXEC_glCombinerInputNV;
	mFunctions[1053] = EXEC_glCombinerOutputNV;
	mFunctions[1054] = EXEC_glFinalCombinerInputNV;
	mFunctions[1055] = EXEC_glGetCombinerInputParameterfvNV;
	mFunctions[1056] = EXEC_glGetCombinerInputParameterivNV;
	mFunctions[1057] = EXEC_glGetCombinerOutputParameterfvNV;
	mFunctions[1058] = EXEC_glGetCombinerOutputParameterivNV;
	mFunctions[1059] = EXEC_glGetFinalCombinerInputParameterfvNV;
	mFunctions[1060] = EXEC_glGetFinalCombinerInputParameterivNV;
	mFunctions[1061] = EXEC_glResizeBuffersMESA;
	mFunctions[1062] = EXEC_glWindowPos2dMESA;
	mFunctions[1063] = EXEC_glWindowPos2dvMESA;
	mFunctions[1064] = EXEC_glWindowPos2fMESA;
	mFunctions[1065] = EXEC_glWindowPos2fvMESA;
	mFunctions[1066] = EXEC_glWindowPos2iMESA;
	mFunctions[1067] = EXEC_glWindowPos2ivMESA;
	mFunctions[1068] = EXEC_glWindowPos2sMESA;
	mFunctions[1069] = EXEC_glWindowPos2svMESA;
	mFunctions[1070] = EXEC_glWindowPos3dMESA;
	mFunctions[1071] = EXEC_glWindowPos3dvMESA;
	mFunctions[1072] = EXEC_glWindowPos3fMESA;
	mFunctions[1073] = EXEC_glWindowPos3fvMESA;
	mFunctions[1074] = EXEC_glWindowPos3iMESA;
	mFunctions[1075] = EXEC_glWindowPos3ivMESA;
	mFunctions[1076] = EXEC_glWindowPos3sMESA;
	mFunctions[1077] = EXEC_glWindowPos3svMESA;
	mFunctions[1078] = EXEC_glWindowPos4dMESA;
	mFunctions[1079] = EXEC_glWindowPos4dvMESA;
	mFunctions[1080] = EXEC_glWindowPos4fMESA;
	mFunctions[1081] = EXEC_glWindowPos4fvMESA;
	mFunctions[1082] = EXEC_glWindowPos4iMESA;
	mFunctions[1083] = EXEC_glWindowPos4ivMESA;
	mFunctions[1084] = EXEC_glWindowPos4sMESA;
	mFunctions[1085] = EXEC_glWindowPos4svMESA;
	mFunctions[1086] = EXEC_glMultiModeDrawArraysIBM;
	mFunctions[1087] = EXEC_glMultiModeDrawElementsIBM;
	mFunctions[1088] = EXEC_glColorPointerListIBM;
	mFunctions[1089] = EXEC_glSecondaryColorPointerListIBM;
	mFunctions[1090] = EXEC_glEdgeFlagPointerListIBM;
	mFunctions[1091] = EXEC_glFogCoordPointerListIBM;
	mFunctions[1092] = EXEC_glIndexPointerListIBM;
	mFunctions[1093] = EXEC_glNormalPointerListIBM;
	mFunctions[1094] = EXEC_glTexCoordPointerListIBM;
	mFunctions[1095] = EXEC_glVertexPointerListIBM;
	mFunctions[1096] = EXEC_glTbufferMask3DFX;
	mFunctions[1097] = EXEC_glSampleMaskEXT;
	mFunctions[1098] = EXEC_glSamplePatternEXT;
	mFunctions[1099] = EXEC_glTextureColorMaskSGIS;
	mFunctions[1100] = EXEC_glDeleteFencesNV;
	mFunctions[1101] = EXEC_glGenFencesNV;
	mFunctions[1102] = EXEC_glIsFenceNV;
	mFunctions[1103] = EXEC_glTestFenceNV;
	mFunctions[1104] = EXEC_glGetFenceivNV;
	mFunctions[1105] = EXEC_glFinishFenceNV;
	mFunctions[1106] = EXEC_glSetFenceNV;
	mFunctions[1107] = EXEC_glMapControlPointsNV;
	mFunctions[1108] = EXEC_glMapParameterivNV;
	mFunctions[1109] = EXEC_glMapParameterfvNV;
	mFunctions[1110] = EXEC_glGetMapControlPointsNV;
	mFunctions[1111] = EXEC_glGetMapParameterivNV;
	mFunctions[1112] = EXEC_glGetMapParameterfvNV;
	mFunctions[1113] = EXEC_glGetMapAttribParameterivNV;
	mFunctions[1114] = EXEC_glGetMapAttribParameterfvNV;
	mFunctions[1115] = EXEC_glEvalMapsNV;
	mFunctions[1116] = EXEC_glCombinerStageParameterfvNV;
	mFunctions[1117] = EXEC_glGetCombinerStageParameterfvNV;
	mFunctions[1118] = EXEC_glAreProgramsResidentNV;
	mFunctions[1119] = EXEC_glBindProgramNV;
	mFunctions[1120] = EXEC_glDeleteProgramsNV;
	mFunctions[1121] = EXEC_glExecuteProgramNV;
	mFunctions[1122] = EXEC_glGenProgramsNV;
	mFunctions[1123] = EXEC_glGetProgramParameterdvNV;
	mFunctions[1124] = EXEC_glGetProgramParameterfvNV;
	mFunctions[1125] = EXEC_glGetProgramivNV;
	mFunctions[1126] = EXEC_glGetProgramStringNV;
	mFunctions[1127] = EXEC_glGetTrackMatrixivNV;
	mFunctions[1128] = EXEC_glGetVertexAttribdvNV;
	mFunctions[1129] = EXEC_glGetVertexAttribfvNV;
	mFunctions[1130] = EXEC_glGetVertexAttribivNV;
	mFunctions[1131] = EXEC_glGetVertexAttribPointervNV;
	mFunctions[1132] = EXEC_glIsProgramNV;
	mFunctions[1133] = EXEC_glLoadProgramNV;
	mFunctions[1134] = EXEC_glProgramParameter4dNV;
	mFunctions[1135] = EXEC_glProgramParameter4dvNV;
	mFunctions[1136] = EXEC_glProgramParameter4fNV;
	mFunctions[1137] = EXEC_glProgramParameter4fvNV;
	mFunctions[1138] = EXEC_glProgramParameters4dvNV;
	mFunctions[1139] = EXEC_glProgramParameters4fvNV;
	mFunctions[1140] = EXEC_glRequestResidentProgramsNV;
	mFunctions[1141] = EXEC_glTrackMatrixNV;
	mFunctions[1142] = EXEC_glVertexAttribPointerNV;
	mFunctions[1143] = EXEC_glVertexAttrib1sNV;
	mFunctions[1144] = EXEC_glVertexAttrib1svNV;
	mFunctions[1145] = EXEC_glVertexAttrib2sNV;
	mFunctions[1146] = EXEC_glVertexAttrib2svNV;
	mFunctions[1147] = EXEC_glVertexAttrib3sNV;
	mFunctions[1148] = EXEC_glVertexAttrib3svNV;
	mFunctions[1149] = EXEC_glVertexAttrib4sNV;
	mFunctions[1150] = EXEC_glVertexAttrib4svNV;
	mFunctions[1151] = EXEC_glVertexAttrib1fNV;
	mFunctions[1152] = EXEC_glVertexAttrib1fvNV;
	mFunctions[1153] = EXEC_glVertexAttrib2fNV;
	mFunctions[1154] = EXEC_glVertexAttrib2fvNV;
	mFunctions[1155] = EXEC_glVertexAttrib3fNV;
	mFunctions[1156] = EXEC_glVertexAttrib3fvNV;
	mFunctions[1157] = EXEC_glVertexAttrib4fNV;
	mFunctions[1158] = EXEC_glVertexAttrib4fvNV;
	mFunctions[1159] = EXEC_glVertexAttrib1dNV;
	mFunctions[1160] = EXEC_glVertexAttrib1dvNV;
	mFunctions[1161] = EXEC_glVertexAttrib2dNV;
	mFunctions[1162] = EXEC_glVertexAttrib2dvNV;
	mFunctions[1163] = EXEC_glVertexAttrib3dNV;
	mFunctions[1164] = EXEC_glVertexAttrib3dvNV;
	mFunctions[1165] = EXEC_glVertexAttrib4dNV;
	mFunctions[1166] = EXEC_glVertexAttrib4dvNV;
	mFunctions[1167] = EXEC_glVertexAttrib4ubNV;
	mFunctions[1168] = EXEC_glVertexAttrib4ubvNV;
	mFunctions[1169] = EXEC_glVertexAttribs1svNV;
	mFunctions[1170] = EXEC_glVertexAttribs2svNV;
	mFunctions[1171] = EXEC_glVertexAttribs3svNV;
	mFunctions[1172] = EXEC_glVertexAttribs4svNV;
	mFunctions[1173] = EXEC_glVertexAttribs1fvNV;
	mFunctions[1174] = EXEC_glVertexAttribs2fvNV;
	mFunctions[1175] = EXEC_glVertexAttribs3fvNV;
	mFunctions[1176] = EXEC_glVertexAttribs4fvNV;
	mFunctions[1177] = EXEC_glVertexAttribs1dvNV;
	mFunctions[1178] = EXEC_glVertexAttribs2dvNV;
	mFunctions[1179] = EXEC_glVertexAttribs3dvNV;
	mFunctions[1180] = EXEC_glVertexAttribs4dvNV;
	mFunctions[1181] = EXEC_glVertexAttribs4ubvNV;
	mFunctions[1182] = EXEC_glGenFragmentShadersATI;
	mFunctions[1183] = EXEC_glBindFragmentShaderATI;
	mFunctions[1184] = EXEC_glDeleteFragmentShaderATI;
	mFunctions[1185] = EXEC_glBeginFragmentShaderATI;
	mFunctions[1186] = EXEC_glEndFragmentShaderATI;
	mFunctions[1187] = EXEC_glPassTexCoordATI;
	mFunctions[1188] = EXEC_glSampleMapATI;
	mFunctions[1189] = EXEC_glColorFragmentOp1ATI;
	mFunctions[1190] = EXEC_glColorFragmentOp2ATI;
	mFunctions[1191] = EXEC_glColorFragmentOp3ATI;
	mFunctions[1192] = EXEC_glAlphaFragmentOp1ATI;
	mFunctions[1193] = EXEC_glAlphaFragmentOp2ATI;
	mFunctions[1194] = EXEC_glAlphaFragmentOp3ATI;
	mFunctions[1195] = EXEC_glSetFragmentShaderConstantATI;
	mFunctions[1196] = EXEC_glDrawMeshArraysSUN;
	mFunctions[1197] = EXEC_glPointParameteriNV;
	mFunctions[1198] = EXEC_glPointParameterivNV;
	mFunctions[1199] = EXEC_glActiveStencilFaceEXT;
	mFunctions[1200] = EXEC_glDrawBuffersATI;
	mFunctions[1201] = EXEC_glProgramNamedParameter4fNV;
	mFunctions[1202] = EXEC_glProgramNamedParameter4dNV;
	mFunctions[1203] = EXEC_glProgramNamedParameter4fvNV;
	mFunctions[1204] = EXEC_glProgramNamedParameter4dvNV;
	mFunctions[1205] = EXEC_glGetProgramNamedParameterfvNV;
	mFunctions[1206] = EXEC_glGetProgramNamedParameterdvNV;
	mFunctions[1207] = EXEC_glDepthBoundsEXT;
	mFunctions[1208] = EXEC_glBlendEquationSeparateEXT;
	mFunctions[1209] = EXEC_glBlitFramebufferEXT;
	mFunctions[1210] = EXEC_glBlendEquationSeparateATI;
	mFunctions[1211] = EXEC_glStencilOpSeparateATI;
	mFunctions[1212] = EXEC_glStencilFuncSeparateATI;
	mFunctions[1213] = EXEC_glProgramEnvParameters4fvEXT;
	mFunctions[1214] = EXEC_glProgramLocalParameters4fvEXT;
	mFunctions[1215] = EXEC_glGetQueryObjecti64vEXT;
	mFunctions[1216] = EXEC_glGetQueryObjectui64vEXT;
	mFunctions[1217] = EXEC_glBlendFuncSeparateINGR;
	mFunctions[1218] = EXEC_glCreateDebugObjectMESA;
	mFunctions[1219] = EXEC_glClearDebugLogMESA;
	mFunctions[1220] = EXEC_glGetDebugLogMESA;
	mFunctions[1221] = EXEC_glGetDebugLogLengthMESA;
	mFunctions[1222] = EXEC_glPointParameterfSGIS;
	mFunctions[1223] = EXEC_glPointParameterfvSGIS;
	mFunctions[1224] = EXEC_glIglooInterfaceSGIX;
	mFunctions[1225] = EXEC_glDeformationMap3dSGIX;
	mFunctions[1226] = EXEC_glDeformationMap3fSGIX;
	mFunctions[1227] = EXEC_glDeformSGIX;
	mFunctions[1228] = EXEC_glLoadIdentityDeformationMapSGIX;

	//CGL functions
	mFunctions[1499] = EXEC_CGLSwapBuffers;

	//GLU functions
	mFunctions[1501] = EXEC_gluBeginCurve;
	mFunctions[1502] = EXEC_gluBeginPolygon;
	mFunctions[1503] = EXEC_gluBeginSurface;
	mFunctions[1504] = EXEC_gluBeginTrim;
	mFunctions[1505] = EXEC_gluBuild1DMipmapLevels;
	mFunctions[1506] = EXEC_gluBuild1DMipmaps;
	mFunctions[1507] = EXEC_gluBuild2DMipmapLevels;
	mFunctions[1508] = EXEC_gluBuild2DMipmaps;
	mFunctions[1509] = EXEC_gluBuild3DMipmapLevels;
	mFunctions[1510] = EXEC_gluBuild3DMipmaps;
	mFunctions[1511] = EXEC_gluCheckExtension;
	mFunctions[1512] = EXEC_gluCylinder;
	mFunctions[1513] = EXEC_gluDeleteNurbsRenderer;
	mFunctions[1514] = EXEC_gluDeleteQuadric;
	mFunctions[1515] = EXEC_gluDeleteTess;
	mFunctions[1516] = EXEC_gluDisk;
	mFunctions[1517] = EXEC_gluEndCurve;
	mFunctions[1518] = EXEC_gluEndPolygon;
	mFunctions[1519] = EXEC_gluEndSurface;
	mFunctions[1520] = EXEC_gluEndTrim;
	mFunctions[1521] = EXEC_gluErrorString;
	mFunctions[1522] = EXEC_gluGetNurbsProperty;
	mFunctions[1523] = EXEC_gluGetString;
	mFunctions[1524] = EXEC_gluGetTessProperty;
	mFunctions[1525] = EXEC_gluLoadSamplingMatrices;
	mFunctions[1526] = EXEC_gluLookAt;
	mFunctions[1527] = EXEC_gluNewNurbsRenderer;
	mFunctions[1528] = EXEC_gluNewQuadric;
	mFunctions[1529] = EXEC_gluNewTess;
	mFunctions[1530] = EXEC_gluNextContour;
	mFunctions[1531] = EXEC_gluNurbsCallback;
	mFunctions[1532] = EXEC_gluNurbsCallbackData;
	mFunctions[1533] = EXEC_gluNurbsCallbackDataEXT;
	mFunctions[1534] = EXEC_gluNurbsCurve;
	mFunctions[1535] = EXEC_gluNurbsProperty;
	mFunctions[1536] = EXEC_gluNurbsSurface;
	mFunctions[1537] = EXEC_gluOrtho2D;
	mFunctions[1538] = EXEC_gluPartialDisk;
	mFunctions[1539] = EXEC_gluPerspective;
	mFunctions[1540] = EXEC_gluPickMatrix;
	mFunctions[1541] = EXEC_gluProject;
	mFunctions[1542] = EXEC_gluPwlCurve;
	mFunctions[1543] = EXEC_gluQuadricCallback;
	mFunctions[1544] = EXEC_gluQuadricDrawStyle;
	mFunctions[1545] = EXEC_gluQuadricNormals;
	mFunctions[1546] = EXEC_gluQuadricOrientation;
	mFunctions[1547] = EXEC_gluQuadricTexture;
	mFunctions[1548] = EXEC_glugluScaleImage;
	mFunctions[1549] = EXEC_gluSphere;
	mFunctions[1550] = EXEC_gluTessBeginContour;
	mFunctions[1551] = EXEC_gluTessBeginPolygon;
	mFunctions[1552] = EXEC_gluTessCallback;
	mFunctions[1553] = EXEC_gluTessEndContour;
	mFunctions[1554] = EXEC_gluTessEndPolygon;
	mFunctions[1555] = EXEC_gluTessNormal;
	mFunctions[1556] = EXEC_gluTessProperty;
	mFunctions[1557] = EXEC_gluTessVertex;
	mFunctions[1558] = EXEC_gluUnProject;
	mFunctions[1559] = EXEC_gluUnProject4;

	//GLX functions
	mFunctions[1601] = EXEC_glXChooseVisual;
	mFunctions[1602] = EXEC_glXCreateContext;
	mFunctions[1603] = EXEC_glXDestroyContext;
	mFunctions[1604] = EXEC_glXMakeCurrent;
	mFunctions[1605] = EXEC_glXCopyContext;
	mFunctions[1606] = EXEC_glXSwapBuffers;
	mFunctions[1607] = EXEC_glXCreateGLXPixmap;
	mFunctions[1608] = EXEC_glXDestroyGLXPixmap;
	mFunctions[1609] = EXEC_glXQueryExtension;
	mFunctions[1610] = EXEC_glXQueryVersion;
	mFunctions[1611] = EXEC_glXIsDirect;
	mFunctions[1612] = EXEC_glXGetConfig;
	mFunctions[1613] = EXEC_glXGetCurrentContext;
	mFunctions[1614] = EXEC_glXGetCurrentDrawable;
	mFunctions[1615] = EXEC_glXWaitGL;
	mFunctions[1616] = EXEC_glXWaitX;
	mFunctions[1617] = EXEC_glXUseXFont;
	mFunctions[1618] = EXEC_glXQueryExtensionsString;
	mFunctions[1619] = EXEC_glXQueryServerString;
	mFunctions[1620] = EXEC_glXGetClientString;
	mFunctions[1621] = EXEC_glXGetCurrentDisplay;
	mFunctions[1622] = EXEC_glXChooseFBConfig;
	mFunctions[1623] = EXEC_glXGetFBConfigAttrib;
	mFunctions[1624] = EXEC_glXGetFBConfigs;
	mFunctions[1625] = EXEC_glXGetVisualFromFBConfig;
	mFunctions[1626] = EXEC_glXCreateWindow;
	mFunctions[1627] = EXEC_glXDestroyWindow;
	mFunctions[1628] = EXEC_glXCreatePixmap;
	mFunctions[1629] = EXEC_glXDestroyPixmap;
	mFunctions[1630] = EXEC_glXCreatePbuffer;
	mFunctions[1631] = EXEC_glXDestroyPbuffer;
	mFunctions[1632] = EXEC_glXQueryDrawable;
	mFunctions[1633] = EXEC_glXCreateNewContext;
	mFunctions[1634] = EXEC_glXMakeContextCurrent;
	mFunctions[1635] = EXEC_glXGetCurrentReadDrawable;
	mFunctions[1636] = EXEC_glXQueryContext;
	mFunctions[1637] = EXEC_glXSelectEvent;
	mFunctions[1638] = EXEC_glXGetSelectedEvent;
	mFunctions[1639] = EXEC_glXGetProcAddressARB;
	mFunctions[1640] = EXEC_glXGetProcAddress;
	mFunctions[1641] = EXEC_glXAllocateMemoryNV;
	mFunctions[1642] = EXEC_glXFreeMemoryNV;
	mFunctions[1643] = EXEC_glXAllocateMemoryMESA;
	mFunctions[1644] = EXEC_glXFreeMemoryMESA;
	mFunctions[1645] = EXEC_glXGetMemoryOffsetMESA;
	mFunctions[1646] = EXEC_glXBindTexImageARB;
	mFunctions[1647] = EXEC_glXReleaseTexImageARB;
	mFunctions[1648] = EXEC_glXDrawableAttribARB;
	mFunctions[1649] = EXEC_glXGetFrameUsageMESA;
	mFunctions[1650] = EXEC_glXBeginFrameTrackingMESA;
	mFunctions[1651] = EXEC_glXEndFrameTrackingMESA;
	mFunctions[1652] = EXEC_glXQueryFrameTrackingMESA;
	mFunctions[1653] = EXEC_glXSwapIntervalMESA;
	mFunctions[1654] = EXEC_glXGetSwapIntervalMESA;
	mFunctions[1655] = EXEC_glXBindTexImageEXT;
	mFunctions[1656] = EXEC_glXReleaseTexImageEXT;

	LOG("Loaded!\n");

	return true;
}

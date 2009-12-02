#include "main.h"

#include <GL/gl.h>

extern App *theApp;


/**************************************
	Interception globals
**************************************/
#define MAX_INSTRUCTIONS 100000

Instruction mInstructions[MAX_INSTRUCTIONS];
int iInstructionCount = 0;
int iCurrentBuffer = 0;

Instruction *mCurrentInstruction = NULL;
byte *mCurrentArgs = NULL;



/**************************************
	Interception module stuff
**************************************/
AppModule::AppModule(string command){
	init(command);
}

bool AppModule::init(string command){
	
	return true;
}

bool AppModule::process(list<Instruction> &list){
	for(int i=0;i<iInstructionCount;i++){
		list.push_back(mInstructions[i]);
	}
	
	iInstructionCount = 0;
	
	return true;
}

bool AppModule::sync(){
	return true;
}

/**************************************
	Interception global funcs
**************************************/
void pushOp(uint16_t opID){

	if(iInstructionCount >= MAX_INSTRUCTIONS){
		LOG("Out of instruction space (%d)!\n", iInstructionCount);
		
		
		//force the frame
		if(!theApp->tick()){
			exit(1);
		}
		iInstructionCount = 0;
		exit(1);
	}

	mCurrentInstruction = &mInstructions[iInstructionCount++];
	iCurrentBuffer = 0;
	
	mCurrentInstruction->id = opID;
	mCurrentArgs = mCurrentInstruction->args;
	
	for(int i=0;i<3;i++){
		mCurrentInstruction->buffers[i].buffer = NULL;
		mCurrentInstruction->buffers[i].len = 0;
	}
}

void pushBuf(const void *buffer, int len, bool needReply = false){
	int saved = len;
	if(iCurrentBuffer >= 3){
		LOG("Out of buffer space!\n");
		return;
	}
	
	byte *copy = (byte *)buffer;
	
	//If we don't need a reply, this buffer cannot be relied upon (probably on
	//the stack), so we need to copy it
	//If we *do* need a reply, we're going to block, so it's OK not to copy
	if(!needReply){
		copy = new byte[len];
		if(buffer == NULL)
			len = 0;
		memcpy(copy, buffer, len);
		len = saved;
	}

	//hrm. How many times can we use the word 'buffer' in one line?
	InstructionBuffer *buf = &mCurrentInstruction->buffers[iCurrentBuffer];
	if(buffer == NULL)
		buf->buffer = NULL;
	else
		buf->buffer = copy;
	buf->len = len;
	buf->needClear = !needReply;
	buf->needReply = needReply;
	//LOG("PushBuf %d, size %d (return = %d)\n", mCurrentInstruction->id, len, needReply);
}

void waitForReturn(){
	//This is annoying. We need to force a frame end here so that the other end
	//of the pipeline can get back to us with whatever it is they're going to
	//do with the return buffer or value
	
	//force the frame
	if(!theApp->tick()){
		exit(1);
	}

}

#define PUSHPARAM(type) \
	void pushParam(type data){ \
		memcpy((void *)mCurrentArgs, (const void *)&data, sizeof(type)); \
		mCurrentArgs += sizeof(type); \
	}

PUSHPARAM(GLfloat);
PUSHPARAM(GLuint);
PUSHPARAM(GLint);
//PUSHPARAM(GLenum);
//PUSHPARAM(GLsizei);
PUSHPARAM(GLbyte);
//PUSHPARAM(GLboolean);
PUSHPARAM(GLdouble);

int getTypeSize(GLenum type)
{
	switch (type) 
	{
	case GL_BYTE: 		return sizeof(GLbyte);
	case GL_UNSIGNED_BYTE:  return sizeof(GLubyte);
	case GL_SHORT: 		return sizeof(GLshort);
	case GL_UNSIGNED_SHORT: return sizeof(GLushort);
	case GL_INT: 		return sizeof(GLint);
	case GL_UNSIGNED_INT: 	return sizeof(GLuint);
	case GL_FLOAT: 		return sizeof(GLfloat);
	case GL_DOUBLE:		return sizeof(GLdouble);
	}
}

size_t getLightParamSize(GLenum type)
{
	switch (type) 
	{
	case GL_AMBIENT: 			return 4;
	case GL_DIFFUSE:  			return 4;
	case GL_SPECULAR: 			return 4;
	case GL_POSITION: 			return 4;
	case GL_SPOT_DIRECTION: 		return 3;
	case GL_SPOT_EXPONENT: 			return 1;
	case GL_SPOT_CUTOFF: 			return 1;
	case GL_CONSTANT_ATTENUATION:		return 1;
	case GL_LINEAR_ATTENUATION:		return 1;
	case GL_QUADRATIC_ATTENUATION:		return 1;

	case GL_LIGHT_MODEL_AMBIENT:		return 4;
	case GL_LIGHT_MODEL_COLOR_CONTROL:	return 1;
	case GL_LIGHT_MODEL_LOCAL_VIEWER:	return 1;
	case GL_LIGHT_MODEL_TWO_SIDE:		return 1;

	case GL_EMISSION:			return 4;
	case GL_SHININESS:			return 1;
	case GL_COLOR_INDEXES:			return 3;
	}
}


/**************************************
			SDL intercepts
**************************************/
//Pointer to SDL_INIT
static int (*_SDL_Init)(unsigned int flags) = NULL;
//Pointer to SDL_SetVideoMode
static SDL_Surface* (*_SDL_SetVideoMode)(int ,int ,int ,unsigned int) = NULL;

bool bHasMinimized = false;

extern "C" int SDL_Init(unsigned int flags){

	if (_SDL_Init == NULL) {
		_SDL_Init = (int (*)(unsigned int)) dlsym(RTLD_NEXT, "SDL_Init");
	}
		
	if(!_SDL_Init){
		printf("Couldn't find SDL_Init: %s\n", dlerror());
		exit(0);
	}
	
	int r = (*_SDL_Init)(flags);
	
	//Set up our internals
	if(!theApp){
		theApp = new App();
		theApp->run_shared();
	}
		
	return r;
}

extern "C" SDL_Surface* SDL_SetVideoMode(int width,int height,int bpp,unsigned int  videoFlags ){
	
	if (_SDL_SetVideoMode == NULL) {
		_SDL_SetVideoMode = (SDL_Surface* (*)(int,int,int,unsigned int)) dlsym(RTLD_NEXT, "SDL_SetVideoMode");
	}
	if(!_SDL_SetVideoMode){
		printf("Couldn't find SDL_SetVideoMode: %s\n", dlerror());
		exit(0);
	}
	return (*_SDL_SetVideoMode)(1,1, bpp, videoFlags );
}

extern "C" void SDL_GL_SwapBuffers( ){

	if(!bHasMinimized){
	  if (SDL_WM_IconifyWindow()==0)
		LOG("Could not minimize Window\n");
	  bHasMinimized = true;
	}
	
	pushOp(1499); //Swap buffers

	if(!theApp->tick()){
		exit(1);
	}
}



/**************************************
	Interception exports
**************************************/
//0
extern "C" void glNewList(GLuint list, GLenum mode){
	pushOp(0);
	pushParam(list);
	pushParam(mode);
}

//1
extern "C" void glEndList(){
	pushOp(1);
}

//2
extern "C" void glCallList(GLuint list){
	pushOp(2);
	pushParam(list);
}

//3
extern "C" void glCallLists(GLsizei n, GLenum type, const GLvoid * lists){
	pushOp(3);
	pushParam(n);
	pushParam(type);
	pushBuf(lists, sizeof(const GLuint) * n);
}

//4
extern "C" void glDeleteLists(GLuint list, GLsizei range){
	pushOp(4);
	pushParam(list);
	pushParam(range);
}

//5
extern "C" GLuint glGenLists(GLsizei range){
	pushOp(5);
	pushParam(range);

	GLuint ret;
	pushBuf(&ret, sizeof(GLuint), true);
	waitForReturn();

	return ret;
}

//6
extern "C" void glListBase(GLuint base){
	pushOp(6);
	pushParam(base);
}

//7
extern "C" void glBegin(GLenum mode){
	pushOp(7);
	pushParam(mode);
}

//8
extern "C" void glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig, GLfloat xmove, GLfloat ymove, const GLubyte * bitmap){
	LOG("Called untested stub Bitmap!\n");
	pushOp(8);
	pushParam(width);
	pushParam(height);
	pushParam(xorig);
	pushParam(yorig);
	pushParam(xmove);
	pushParam(ymove);
	pushBuf(bitmap, sizeof(GLubyte) * width * height);
}

//9
extern "C" void glColor3b(GLbyte red, GLbyte green, GLbyte blue){
	pushOp(9);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//10
extern "C" void glColor3bv(const GLbyte * v){
	pushOp(10);
	pushBuf(v, sizeof(const GLbyte) * 3);
}

//11
extern "C" void glColor3d(GLdouble red, GLdouble green, GLdouble blue){
	pushOp(11);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//12
extern "C" void glColor3dv(const GLdouble * v){
	pushOp(12);
	pushBuf(v, sizeof(const GLdouble) * 3);
}

//13
extern "C" void glColor3f(GLfloat red, GLfloat green, GLfloat blue){
	pushOp(13);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//14
extern "C" void glColor3fv(const GLfloat * v){
	pushOp(14);
	pushBuf(v, sizeof(const GLfloat) * 3);
}

//15
extern "C" void glColor3i(GLint red, GLint green, GLint blue){
	pushOp(15);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//16
extern "C" void glColor3iv(const GLint * v){
	pushOp(16);
	pushBuf(v, sizeof(const GLint) * 3);
}

//17
extern "C" void glColor3s(GLshort red, GLshort green, GLshort blue){
	pushOp(17);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//18
extern "C" void glColor3sv(const GLshort * v){
	pushOp(18);
	pushBuf(v, sizeof(const GLshort) * 3);
}

//19
extern "C" void glColor3ub(GLubyte red, GLubyte green, GLubyte blue){
	pushOp(19);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//20
extern "C" void glColor3ubv(const GLubyte * v){
	pushOp(20);
	pushBuf(v, sizeof(const GLubyte) * 3);
}

//21
extern "C" void glColor3ui(GLuint red, GLuint green, GLuint blue){
	pushOp(21);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//22
extern "C" void glColor3uiv(const GLuint * v){
	pushOp(22);
	pushBuf(v, sizeof(const GLuint) * 3);
}

//23
extern "C" void glColor3us(GLushort red, GLushort green, GLushort blue){
	pushOp(23);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//24
extern "C" void glColor3usv(const GLushort * v){
	pushOp(24);
	pushBuf(v, sizeof(const GLushort) * 3);
}

//25
extern "C" void glColor4b(GLbyte red, GLbyte green, GLbyte blue, GLbyte alpha){
	pushOp(25);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
	pushParam(alpha);
}

//26
extern "C" void glColor4bv(const GLbyte * v){
	pushOp(26);
	pushBuf(v, sizeof(const GLbyte) * 4);
}

//27
extern "C" void glColor4d(GLdouble red, GLdouble green, GLdouble blue, GLdouble alpha){
	pushOp(27);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
	pushParam(alpha);
}

//28
extern "C" void glColor4dv(const GLdouble * v){
	pushOp(28);
	pushBuf(v, sizeof(const GLdouble) * 4);
}

//29
extern "C" void glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha){
	pushOp(29);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
	pushParam(alpha);
}

//30
extern "C" void glColor4fv(const GLfloat * v){
	pushOp(30);
	pushBuf(v, sizeof(const GLfloat) * 4);
}

//31
extern "C" void glColor4i(GLint red, GLint green, GLint blue, GLint alpha){
	pushOp(31);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
	pushParam(alpha);
}

//32
extern "C" void glColor4iv(const GLint * v){
	pushOp(32);
	pushBuf(v, sizeof(const GLint) * 4);
}

//33
extern "C" void glColor4s(GLshort red, GLshort green, GLshort blue, GLshort alpha){
	pushOp(33);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
	pushParam(alpha);
}

//34
extern "C" void glColor4sv(const GLshort * v){
	pushOp(34);
	pushBuf(v, sizeof(const GLshort) * 4);
}

//35
extern "C" void glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha){
	pushOp(35);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
	pushParam(alpha);
}

//36
extern "C" void glColor4ubv(const GLubyte * v){
	pushOp(36);
	pushBuf(v, sizeof(const GLubyte) * 4);
}

//37
extern "C" void glColor4ui(GLuint red, GLuint green, GLuint blue, GLuint alpha){
	pushOp(37);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
	pushParam(alpha);
}

//38
extern "C" void glColor4uiv(const GLuint * v){
	pushOp(38);
	pushBuf(v, sizeof(const GLuint) * 4);
}

//39
extern "C" void glColor4us(GLushort red, GLushort green, GLushort blue, GLushort alpha){
	pushOp(39);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
	pushParam(alpha);
}

//40
extern "C" void glColor4usv(const GLushort * v){
	pushOp(40);
	pushBuf(v, sizeof(const GLushort) * 4);
}

//41
extern "C" void glEdgeFlag(GLboolean flag){
	pushOp(41);
	pushParam(flag);
}

//42
extern "C" void glEdgeFlagv(const GLboolean * flag){
	pushOp(42);
	pushBuf(flag, sizeof(const GLboolean) * 1);
}

//43
extern "C" void glEnd(){
	pushOp(43);
}

//44
extern "C" void glIndexd(GLdouble c){
	pushOp(44);
	pushParam(c);
}

//45
extern "C" void glIndexdv(const GLdouble * c){
	pushOp(45);
	pushBuf(c, sizeof(const GLdouble) * 1);
}

//46
extern "C" void glIndexf(GLfloat c){
	pushOp(46);
	pushParam(c);
}

//47
extern "C" void glIndexfv(const GLfloat * c){
	pushOp(47);
	pushBuf(c, sizeof(const GLfloat) * 1);
}

//48
extern "C" void glIndexi(GLint c){
	pushOp(48);
	pushParam(c);
}

//49
extern "C" void glIndexiv(const GLint * c){
	pushOp(49);
	pushBuf(c, sizeof(const GLint) * 1);
}

//50
extern "C" void glIndexs(GLshort c){
	pushOp(50);
	pushParam(c);
}

//51
extern "C" void glIndexsv(const GLshort * c){
	pushOp(51);
	pushBuf(c, sizeof(const GLshort) * 1);
}

//52
extern "C" void glNormal3b(GLbyte nx, GLbyte ny, GLbyte nz){
	pushOp(52);
	pushParam(nx);
	pushParam(ny);
	pushParam(nz);
}

//53
extern "C" void glNormal3bv(const GLbyte * v){
	pushOp(53);
	pushBuf(v, sizeof(const GLbyte) * 3);
}

//54
extern "C" void glNormal3d(GLdouble nx, GLdouble ny, GLdouble nz){
	pushOp(54);
	pushParam(nx);
	pushParam(ny);
	pushParam(nz);
}

//55
extern "C" void glNormal3dv(const GLdouble * v){
	pushOp(55);
	pushBuf(v, sizeof(const GLdouble) * 3);
}

//56
extern "C" void glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz){
	pushOp(56);
	pushParam(nx);
	pushParam(ny);
	pushParam(nz);
}

//57
extern "C" void glNormal3fv(const GLfloat * v){
	pushOp(57);
	pushBuf(v, sizeof(const GLfloat) * 3);
}

//58
extern "C" void glNormal3i(GLint nx, GLint ny, GLint nz){
	pushOp(58);
	pushParam(nx);
	pushParam(ny);
	pushParam(nz);
}

//59
extern "C" void glNormal3iv(const GLint * v){
	pushOp(59);
	pushBuf(v, sizeof(const GLint) * 3);
}

//60
extern "C" void glNormal3s(GLshort nx, GLshort ny, GLshort nz){
	pushOp(60);
	pushParam(nx);
	pushParam(ny);
	pushParam(nz);
}

//61
extern "C" void glNormal3sv(const GLshort * v){
	pushOp(61);
	pushBuf(v, sizeof(const GLshort) * 3);
}

//62
extern "C" void glRasterPos2d(GLdouble x, GLdouble y){
	pushOp(62);
	pushParam(x);
	pushParam(y);
}

//63
extern "C" void glRasterPos2dv(const GLdouble * v){
	pushOp(63);
	pushBuf(v, sizeof(const GLdouble) * 2);
}

//64
extern "C" void glRasterPos2f(GLfloat x, GLfloat y){
	pushOp(64);
	pushParam(x);
	pushParam(y);
}

//65
extern "C" void glRasterPos2fv(const GLfloat * v){
	pushOp(65);
	pushBuf(v, sizeof(const GLfloat) * 2);
}

//66
extern "C" void glRasterPos2i(GLint x, GLint y){
	pushOp(66);
	pushParam(x);
	pushParam(y);
}

//67
extern "C" void glRasterPos2iv(const GLint * v){
	pushOp(67);
	pushBuf(v, sizeof(const GLint) * 2);
}

//68
extern "C" void glRasterPos2s(GLshort x, GLshort y){
	pushOp(68);
	pushParam(x);
	pushParam(y);
}

//69
extern "C" void glRasterPos2sv(const GLshort * v){
	pushOp(69);
	pushBuf(v, sizeof(const GLshort) * 2);
}

//70
extern "C" void glRasterPos3d(GLdouble x, GLdouble y, GLdouble z){
	pushOp(70);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//71
extern "C" void glRasterPos3dv(const GLdouble * v){
	pushOp(71);
	pushBuf(v, sizeof(const GLdouble) * 3);
}

//72
extern "C" void glRasterPos3f(GLfloat x, GLfloat y, GLfloat z){
	pushOp(72);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//73
extern "C" void glRasterPos3fv(const GLfloat * v){
	pushOp(73);
	pushBuf(v, sizeof(const GLfloat) * 3);
}

//74
extern "C" void glRasterPos3i(GLint x, GLint y, GLint z){
	pushOp(74);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//75
extern "C" void glRasterPos3iv(const GLint * v){
	pushOp(75);
	pushBuf(v, sizeof(const GLint) * 3);
}

//76
extern "C" void glRasterPos3s(GLshort x, GLshort y, GLshort z){
	pushOp(76);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//77
extern "C" void glRasterPos3sv(const GLshort * v){
	pushOp(77);
	pushBuf(v, sizeof(const GLshort) * 3);
}

//78
extern "C" void glRasterPos4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w){
	pushOp(78);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//79
extern "C" void glRasterPos4dv(const GLdouble * v){
	pushOp(79);
	pushBuf(v, sizeof(const GLdouble) * 4);
}

//80
extern "C" void glRasterPos4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w){
	pushOp(80);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//81
extern "C" void glRasterPos4fv(const GLfloat * v){
	pushOp(81);
	pushBuf(v, sizeof(const GLfloat) * 4);
}

//82
extern "C" void glRasterPos4i(GLint x, GLint y, GLint z, GLint w){
	pushOp(82);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//83
extern "C" void glRasterPos4iv(const GLint * v){
	pushOp(83);
	pushBuf(v, sizeof(const GLint) * 4);
}

//84
extern "C" void glRasterPos4s(GLshort x, GLshort y, GLshort z, GLshort w){
	pushOp(84);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//85
extern "C" void glRasterPos4sv(const GLshort * v){
	pushOp(85);
	pushBuf(v, sizeof(const GLshort) * 4);
}

//86
extern "C" void glRectd(GLdouble x1, GLdouble y1, GLdouble x2, GLdouble y2){
	pushOp(86);
	pushParam(x1);
	pushParam(y1);
	pushParam(x2);
	pushParam(y2);
}

//87
extern "C" void glRectdv(const GLdouble * v1, const GLdouble * v2){
	pushOp(87);
	pushBuf(v1, sizeof(const GLdouble) * 2);
	pushBuf(v2, sizeof(const GLdouble) * 2);
}

//88
extern "C" void glRectf(GLfloat x1, GLfloat y1, GLfloat x2, GLfloat y2){
	pushOp(88);
	pushParam(x1);
	pushParam(y1);
	pushParam(x2);
	pushParam(y2);
}

//89
extern "C" void glRectfv(const GLfloat * v1, const GLfloat * v2){
	pushOp(89);
	pushBuf(v1, sizeof(const GLfloat) * 2);
	pushBuf(v2, sizeof(const GLfloat) * 2);
}

//90
extern "C" void glRecti(GLint x1, GLint y1, GLint x2, GLint y2){
	pushOp(90);
	pushParam(x1);
	pushParam(y1);
	pushParam(x2);
	pushParam(y2);
}

//91
extern "C" void glRectiv(const GLint * v1, const GLint * v2){
	pushOp(91);
	pushBuf(v1, sizeof(const GLint) * 2);
	pushBuf(v2, sizeof(const GLint) * 2);
}

//92
extern "C" void glRects(GLshort x1, GLshort y1, GLshort x2, GLshort y2){
	pushOp(92);
	pushParam(x1);
	pushParam(y1);
	pushParam(x2);
	pushParam(y2);
}

//93
extern "C" void glRectsv(const GLshort * v1, const GLshort * v2){
	pushOp(93);
	pushBuf(v1, sizeof(const GLshort) * 2);
	pushBuf(v2, sizeof(const GLshort) * 2);
}

//94
extern "C" void glTexCoord1d(GLdouble s){
	pushOp(94);
	pushParam(s);
}

//95
extern "C" void glTexCoord1dv(const GLdouble * v){
	pushOp(95);
	pushBuf(v, sizeof(const GLdouble) * 1);
}

//96
extern "C" void glTexCoord1f(GLfloat s){
	pushOp(96);
	pushParam(s);
}

//97
extern "C" void glTexCoord1fv(const GLfloat * v){
	pushOp(97);
	pushBuf(v, sizeof(const GLfloat) * 1);
}

//98
extern "C" void glTexCoord1i(GLint s){
	pushOp(98);
	pushParam(s);
}

//99
extern "C" void glTexCoord1iv(const GLint * v){
	pushOp(99);
	pushBuf(v, sizeof(const GLint) * 1);
}

//100
extern "C" void glTexCoord1s(GLshort s){
	pushOp(100);
	pushParam(s);
}

//101
extern "C" void glTexCoord1sv(const GLshort * v){
	pushOp(101);
	pushBuf(v, sizeof(const GLshort) * 1);
}

//102
extern "C" void glTexCoord2d(GLdouble s, GLdouble t){
	pushOp(102);
	pushParam(s);
	pushParam(t);
}

//103
extern "C" void glTexCoord2dv(const GLdouble * v){
	pushOp(103);
	pushBuf(v, sizeof(const GLdouble) * 2);
}

//104
extern "C" void glTexCoord2f(GLfloat s, GLfloat t){
	pushOp(104);
	pushParam(s);
	pushParam(t);
}

//105
extern "C" void glTexCoord2fv(const GLfloat * v){
	pushOp(105);
	pushBuf(v, sizeof(const GLfloat) * 2);
}

//106
extern "C" void glTexCoord2i(GLint s, GLint t){
	pushOp(106);
	pushParam(s);
	pushParam(t);
}

//107
extern "C" void glTexCoord2iv(const GLint * v){
	pushOp(107);
	pushBuf(v, sizeof(const GLint) * 2);
}

//108
extern "C" void glTexCoord2s(GLshort s, GLshort t){
	pushOp(108);
	pushParam(s);
	pushParam(t);
}

//109
extern "C" void glTexCoord2sv(const GLshort * v){
	pushOp(109);
	pushBuf(v, sizeof(const GLshort) * 2);
}

//110
extern "C" void glTexCoord3d(GLdouble s, GLdouble t, GLdouble r){
	pushOp(110);
	pushParam(s);
	pushParam(t);
	pushParam(r);
}

//111
extern "C" void glTexCoord3dv(const GLdouble * v){
	pushOp(111);
	pushBuf(v, sizeof(const GLdouble) * 3);
}

//112
extern "C" void glTexCoord3f(GLfloat s, GLfloat t, GLfloat r){
	pushOp(112);
	pushParam(s);
	pushParam(t);
	pushParam(r);
}

//113
extern "C" void glTexCoord3fv(const GLfloat * v){
	pushOp(113);
	pushBuf(v, sizeof(const GLfloat) * 3);
}

//114
extern "C" void glTexCoord3i(GLint s, GLint t, GLint r){
	pushOp(114);
	pushParam(s);
	pushParam(t);
	pushParam(r);
}

//115
extern "C" void glTexCoord3iv(const GLint * v){
	pushOp(115);
	pushBuf(v, sizeof(const GLint) * 3);
}

//116
extern "C" void glTexCoord3s(GLshort s, GLshort t, GLshort r){
	pushOp(116);
	pushParam(s);
	pushParam(t);
	pushParam(r);
}

//117
extern "C" void glTexCoord3sv(const GLshort * v){
	pushOp(117);
	pushBuf(v, sizeof(const GLshort) * 3);
}

//118
extern "C" void glTexCoord4d(GLdouble s, GLdouble t, GLdouble r, GLdouble q){
	pushOp(118);
	pushParam(s);
	pushParam(t);
	pushParam(r);
	pushParam(q);
}

//119
extern "C" void glTexCoord4dv(const GLdouble * v){
	pushOp(119);
	pushBuf(v, sizeof(const GLdouble) * 4);
}

//120
extern "C" void glTexCoord4f(GLfloat s, GLfloat t, GLfloat r, GLfloat q){
	pushOp(120);
	pushParam(s);
	pushParam(t);
	pushParam(r);
	pushParam(q);
}

//121
extern "C" void glTexCoord4fv(const GLfloat * v){
	pushOp(121);
	pushBuf(v, sizeof(const GLfloat) * 4);
}

//122
extern "C" void glTexCoord4i(GLint s, GLint t, GLint r, GLint q){
	pushOp(122);
	pushParam(s);
	pushParam(t);
	pushParam(r);
	pushParam(q);
}

//123
extern "C" void glTexCoord4iv(const GLint * v){
	pushOp(123);
	pushBuf(v, sizeof(const GLint) * 4);
}

//124
extern "C" void glTexCoord4s(GLshort s, GLshort t, GLshort r, GLshort q){
	pushOp(124);
	pushParam(s);
	pushParam(t);
	pushParam(r);
	pushParam(q);
}

//125
extern "C" void glTexCoord4sv(const GLshort * v){
	pushOp(125);
	pushBuf(v, sizeof(const GLshort) * 4);
}

//126
extern "C" void glVertex2d(GLdouble x, GLdouble y){
	pushOp(126);
	pushParam(x);
	pushParam(y);
}

//127
extern "C" void glVertex2dv(const GLdouble * v){
	pushOp(127);
	pushBuf(v, sizeof(const GLdouble) * 2);
}

//128
extern "C" void glVertex2f(GLfloat x, GLfloat y){
	pushOp(128);
	pushParam(x);
	pushParam(y);
}

//129
extern "C" void glVertex2fv(const GLfloat * v){
	pushOp(129);
	pushBuf(v, sizeof(const GLfloat) * 2);
}

//130
extern "C" void glVertex2i(GLint x, GLint y){
	pushOp(130);
	pushParam(x);
	pushParam(y);
}

//131
extern "C" void glVertex2iv(const GLint * v){
	pushOp(131);
	pushBuf(v, sizeof(const GLint) * 2);
}

//132
extern "C" void glVertex2s(GLshort x, GLshort y){
	pushOp(132);
	pushParam(x);
	pushParam(y);
}

//133
extern "C" void glVertex2sv(const GLshort * v){
	pushOp(133);
	pushBuf(v, sizeof(const GLshort) * 2);
}

//134
extern "C" void glVertex3d(GLdouble x, GLdouble y, GLdouble z){
	pushOp(134);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//135
extern "C" void glVertex3dv(const GLdouble * v){
	pushOp(135);
	pushBuf(v, sizeof(const GLdouble) * 3);
}

//136
extern "C" void glVertex3f(GLfloat x, GLfloat y, GLfloat z){
	pushOp(136);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//137
extern "C" void glVertex3fv(const GLfloat * v){
	pushOp(137);
	pushBuf(v, sizeof(const GLfloat) * 3);
}

//138
extern "C" void glVertex3i(GLint x, GLint y, GLint z){
	pushOp(138);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//139
extern "C" void glVertex3iv(const GLint * v){
	pushOp(139);
	pushBuf(v, sizeof(const GLint) * 3);
}

//140
extern "C" void glVertex3s(GLshort x, GLshort y, GLshort z){
	pushOp(140);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//141
extern "C" void glVertex3sv(const GLshort * v){
	pushOp(141);
	pushBuf(v, sizeof(const GLshort) * 3);
}

//142
extern "C" void glVertex4d(GLdouble x, GLdouble y, GLdouble z, GLdouble w){
	pushOp(142);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//143
extern "C" void glVertex4dv(const GLdouble * v){
	pushOp(143);
	pushBuf(v, sizeof(const GLdouble) * 4);
}

//144
extern "C" void glVertex4f(GLfloat x, GLfloat y, GLfloat z, GLfloat w){
	pushOp(144);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//145
extern "C" void glVertex4fv(const GLfloat * v){
	pushOp(145);
	pushBuf(v, sizeof(const GLfloat) * 4);
}

//146
extern "C" void glVertex4i(GLint x, GLint y, GLint z, GLint w){
	pushOp(146);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//147
extern "C" void glVertex4iv(const GLint * v){
	pushOp(147);
	pushBuf(v, sizeof(const GLint) * 4);
}

//148
extern "C" void glVertex4s(GLshort x, GLshort y, GLshort z, GLshort w){
	pushOp(148);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//149
extern "C" void glVertex4sv(const GLshort * v){
	pushOp(149);
	pushBuf(v, sizeof(const GLshort) * 4);
}

//150
extern "C" void glClipPlane(GLenum plane, const GLdouble * equation){
	pushOp(150);
	pushParam(plane);
	pushBuf(equation, sizeof(const GLdouble) * 4);
}

//151
extern "C" void glColorMaterial(GLenum face, GLenum mode){
	pushOp(151);
	pushParam(face);
	pushParam(mode);
}

//152
extern "C" void glCullFace(GLenum mode){
	pushOp(152);
	pushParam(mode);
}

//153
extern "C" void glFogf(GLenum pname, GLfloat param){
	pushOp(153);
	pushParam(pname);
	pushParam(param);
}

//154
extern "C" void glFogfv(GLenum pname, const GLfloat * params){
	pushOp(154);
	pushParam(pname);
	int size = sizeof(const GLfloat);
	if (pname == GL_FOG_COLOR) //if its GL_FOG_COLOR
		size *= 4; //Colour takes an array of 4 values, the rest are just 1 value
	pushBuf(params, size); 
}

//155
extern "C" void glFogi(GLenum pname, GLint param){
	pushOp(155);
	pushParam(pname);
	pushParam(param);
}

//156
extern "C" void glFogiv(GLenum pname, const GLint * params){
	LOG("Called untested stub Fogiv!\n");
        pushOp(156);
	pushParam(pname);
	int size = sizeof(const GLint);
	if(pname == GL_FOG_COLOR)
		size *= 4;  
		pushBuf(params, size);  
         
}

//157
extern "C" void glFrontFace(GLenum mode){
	pushOp(157);
	pushParam(mode);
}

//158
extern "C" void glHint(GLenum target, GLenum mode){
	pushOp(158);
	pushParam(target);
	pushParam(mode);
}

//159
extern "C" void glLightf(GLenum light, GLenum pname, GLfloat param){
	pushOp(159);
	pushParam(light);
	pushParam(pname);
	pushParam(param);
}

//160
extern "C" void glLightfv(GLenum light, GLenum pname, const GLfloat * params){
	LOG("Called untested stub Lightfv!\n");
	pushOp(160);
	pushParam(light);
	pushParam(pname);
	pushBuf(params, sizeof(const GLfloat) * getLightParamSize(pname));
	
}

//161
extern "C" void glLighti(GLenum light, GLenum pname, GLint param){
	pushOp(161);
	pushParam(light);
	pushParam(pname);
	pushParam(param);
}

//162
extern "C" void glLightiv(GLenum light, GLenum pname, const GLint * params){
	LOG("Called untested stub Lightiv!\n");
	pushOp(162);
	pushParam(light);
	pushParam(pname);
	pushBuf(params, sizeof(const GLint) * getLightParamSize(pname));
}

//163
extern "C" void glLightModelf(GLenum pname, GLfloat param){
	pushOp(163);
	pushParam(pname);
	pushParam(param);
}

//164
extern "C" void glLightModelfv(GLenum pname, const GLfloat * params){
	LOG("Called untested stub LightModelfv!\n");
	pushOp(164);
	pushParam(pname);
	pushBuf(params, sizeof(const GLfloat) * getLightParamSize(pname));
}

//165
extern "C" void glLightModeli(GLenum pname, GLint param){
	pushOp(165);
	pushParam(pname);
	pushParam(param);
}

//166
extern "C" void glLightModeliv(GLenum pname, const GLint * params){
	LOG("Called untested stub LightModeliv!\n");
	pushOp(166);
	pushParam(pname);
	pushBuf(params, sizeof(const GLint) * getLightParamSize(pname));
}

//167
extern "C" void glLineStipple(GLint factor, GLushort pattern){
	pushOp(167);
	pushParam(factor);
	pushParam(pattern);
}

//168
extern "C" void glLineWidth(GLfloat width){
	pushOp(168);
	pushParam(width);
}

//169
extern "C" void glMaterialf(GLenum face, GLenum pname, GLfloat param){
	pushOp(169);
	pushParam(face);
	pushParam(pname);
	pushParam(param);
}

//170
extern "C" void glMaterialfv(GLenum face, GLenum pname, const GLfloat * params){
	LOG("Called untested stub Materialfv!\n");
	pushOp(170);
	pushParam(face);
	pushParam(pname);
	pushBuf(params, sizeof(GLfloat) * getLightParamSize(pname));

}

//171
extern "C" void glMateriali(GLenum face, GLenum pname, GLint param){
	pushOp(171);
	pushParam(face);
	pushParam(pname);
	pushParam(param);
}

//172
extern "C" void glMaterialiv(GLenum face, GLenum pname, const GLint * params){
	LOG("Called untested stub Materialiv!\n");
	pushOp(172);
	pushParam(face);
	pushParam(pname);
	pushBuf(params, sizeof(GLint) * getLightParamSize(pname));
}

//173
extern "C" void glPointSize(GLfloat size){
	pushOp(173);
	pushParam(size);
}

//174
extern "C" void glPolygonMode(GLenum face, GLenum mode){
	pushOp(174);
	pushParam(face);
	pushParam(mode);
}

//175
extern "C" void glPolygonStipple(const GLubyte * mask){
	LOG("Called untested stub PolygonStipple!\n");
	pushBuf(mask, sizeof(GLubyte) * 32 * 32);	//stipple = 32 x 32
}

//176
extern "C" void glScissor(GLint x, GLint y, GLsizei width, GLsizei height){
	pushOp(176);
	pushParam(x);
	pushParam(y);
	pushParam(width);
	pushParam(height);
}

//177
extern "C" void glShadeModel(GLenum mode){
	pushOp(177);
	pushParam(mode);	
}

//178
extern "C" void glTexParameterf(GLenum target, GLenum pname, GLfloat param){
	pushOp(178);
	pushParam(target);
	pushParam(pname);
	pushParam(param);
}

//179
extern "C" void glTexParameterfv(GLenum target, GLenum pname, const GLfloat * params){
	LOG("Called unimplemted stub TexParameterfv!\n");
}

//180
extern "C" void glTexParameteri(GLenum target, GLenum pname, GLint param){
	pushOp(180);
	pushParam(target);
	pushParam(pname);
	pushParam(param);
}

//181
extern "C" void glTexParameteriv(GLenum target, GLenum pname, const GLint * params){
	LOG("Called unimplemted stub TexParameteriv!\n");
}

//182
extern "C" void glTexImage1D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLint border, GLenum format, GLenum type, const GLvoid * pixels){
	LOG("Called untested stub TexImage1D!\n");
	pushOp(182);
	pushParam(target);
	pushParam(level);
	pushParam(internalformat);
	pushParam(width);
	pushParam(border);
	pushParam(format);
	pushParam(type);
    	int bpp = 1;
    
    if(format == GL_BGR || format == GL_RGB) bpp = 3;
    else if(format == GL_RGBA || format == GL_BGRA) bpp = 4;

	pushBuf(pixels, getTypeSize(type) * width * bpp);

}

//183
extern "C" void glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid * pixels){
    pushOp(183);
    pushParam(target);
    pushParam(level);
    pushParam(internalformat);
    pushParam(width);
    pushParam(height);
    pushParam(border);
    pushParam(format);
    pushParam(type);
    
    int bpp = 1;
    
    if(format == GL_BGR || format == GL_RGB) bpp = 3;
    else if(format == GL_RGBA || format == GL_BGRA) bpp = 4;
    
    if(pixels)
	    pushBuf(pixels, width * height * bpp);
}

//184
extern "C" void glTexEnvf(GLenum target, GLenum pname, GLfloat param){
	pushOp(184);
	pushParam(target);
	pushParam(pname);
	pushParam(param);
}

//185
extern "C" void glTexEnvfv(GLenum target, GLenum pname, const GLfloat * params){
	LOG("Called unimplemted stub TexEnvfv!\n");
	//1 float or 4 floats, huge list
}

//186
extern "C" void glTexEnvi(GLenum target, GLenum pname, GLint param){
	pushOp(186);
	pushParam(target);
	pushParam(pname);
	pushParam(param);
}

//187
extern "C" void glTexEnviv(GLenum target, GLenum pname, const GLint * params){
	LOG("Called unimplemted stub TexEnviv!\n");
}

//188
extern "C" void glTexGend(GLenum coord, GLenum pname, GLdouble param){
	pushOp(188);
	pushParam(coord);
	pushParam(pname);
	pushParam(param);
}

//189
extern "C" void glTexGendv(GLenum coord, GLenum pname, const GLdouble * params){
	LOG("Called unimplemted stub TexGendv!\n");
}

//190
extern "C" void glTexGenf(GLenum coord, GLenum pname, GLfloat param){
	pushOp(190);
	pushParam(coord);
	pushParam(pname);
	pushParam(param);
}

//191
extern "C" void glTexGenfv(GLenum coord, GLenum pname, const GLfloat * params){
	pushOp(191);
	pushParam(coord);
	pushParam(pname);
	int size = sizeof(const GLfloat);
	if (pname == 9473 || pname == 9474) //GL_EYE_PLANE or GL_OBJECT_PLANE
		size *= 4; 
	pushBuf(params, size); 
}

//192
extern "C" void glTexGeni(GLenum coord, GLenum pname, GLint param){
	pushOp(192);
	pushParam(coord);
	pushParam(pname);
	pushParam(param);
}

//193
extern "C" void glTexGeniv(GLenum coord, GLenum pname, const GLint * params){
	LOG("Called unimplemted stub TexGeniv!\n");
}

//194
extern "C" void glFeedbackBuffer(GLsizei size, GLenum type, GLfloat * buffer){
	LOG("Called untested stub FeedbackBuffer!\n");
	pushOp(194);
	pushParam(size);
	pushParam(type);
	pushBuf(buffer, sizeof(GLfloat) * size);
}

//195
extern "C" void glSelectBuffer(GLsizei size, GLuint * buffer){
	LOG("Called untested stub SelectBuffer!\n");
	pushParam(size);
	pushBuf(buffer, sizeof(GLuint) * size);
}

//196
extern "C" GLint glRenderMode(GLenum mode){
	pushOp(196);
	pushParam(mode);

	GLint ret;
	pushBuf(&ret, sizeof(GLint), true);
	waitForReturn();

	return ret;
}

//197
extern "C" void glInitNames(){
	pushOp(197);
}

//198
extern "C" void glLoadName(GLuint name){
	pushOp(198);
	pushParam(name);
}

//199
extern "C" void glPassThrough(GLfloat token){
	pushOp(199);
	pushParam(token);
}

//200
extern "C" void glPopName(){
	pushOp(200);
}

//201
extern "C" void glPushName(GLuint name){
	pushOp(201);
	pushParam(name);
}

//202
extern "C" void glDrawBuffer(GLenum mode){
	pushOp(202);
	pushParam(mode);
}

//203
extern "C" void glClear(GLbitfield mask){
	pushOp(203);
	pushParam(mask);
}

//204
extern "C" void glClearAccum(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha){
	pushOp(204);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
	pushParam(alpha);
}

//205
extern "C" void glClearIndex(GLfloat c){
	pushOp(205);
	pushParam(c);
}

//206
extern "C" void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha){
	pushOp(206);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
	pushParam(alpha);
}

//207
extern "C" void glClearStencil(GLint s){
	pushOp(207);
	pushParam(s);
}

//208
extern "C" void glClearDepth(GLclampd depth){
	pushOp(208);
	pushParam(depth);
}

//209
extern "C" void glStencilMask(GLuint mask){
	pushOp(209);
	pushParam(mask);
}

//210
extern "C" void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha){
	pushOp(210);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
	pushParam(alpha);
}

//211
extern "C" void glDepthMask(GLboolean flag){
	pushOp(211);
	pushParam(flag);
}

//212
extern "C" void glIndexMask(GLuint mask){
	pushOp(212);
	pushParam(mask);
}

//213
extern "C" void glAccum(GLenum op, GLfloat value){
	pushOp(213);
	pushParam(op);
	pushParam(value);
}

//214
extern "C" void glDisable(GLenum cap){
	pushOp(214);
	pushParam(cap);
}

//215
extern "C" void glEnable(GLenum cap){
	pushOp(215);
	pushParam(cap);
}

//216
extern "C" void glFinish(){
	pushOp(216);
}

//217
extern "C" void glFlush(){
	pushOp(217);
}

//218
extern "C" void glPopAttrib(){
	pushOp(218);
}

//219
extern "C" void glPushAttrib(GLbitfield mask){
	pushOp(219);
	pushParam(mask);
}

//220
extern "C" void glMap1d(GLenum target, GLdouble u1, GLdouble u2, GLint stride, GLint order, const GLdouble * points){
	LOG("Called untested stub Map1d!\n");
	pushOp(220);
	pushParam(target);
	pushParam(u1);
	pushParam(u2);
	pushParam(stride);
	pushParam(order);
	pushBuf(points, sizeof(GLdouble) * (order + stride));
}

//221
extern "C" void glMap1f(GLenum target, GLfloat u1, GLfloat u2, GLint stride, GLint order, const GLfloat * points){
	LOG("Called untested stub Map1f!\n");
	pushOp(221);
	pushParam(target);
	pushParam(u1);
	pushParam(u2);
	pushParam(stride);
	pushParam(order);
	pushBuf(points, sizeof(GLfloat) * (order + stride));
}

//222
extern "C" void glMap2d(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, const GLdouble * points){
	LOG("Called untested stub Map2d!\n");
	pushOp(222);
	pushParam(target);
	pushParam(u1);
	pushParam(u2);
	pushParam(ustride);
	pushParam(uorder);
	pushParam(v1);
	pushParam(v2);
	pushParam(vstride);
	pushParam(vorder);
	pushBuf(points, sizeof(GLdouble) * (uorder + ustride) * (vorder + vstride));
}

//223
extern "C" void glMap2f(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, const GLfloat * points){
	LOG("Called untested stub Map2f!\n");
	pushOp(223);
	pushParam(target);
	pushParam(u1);
	pushParam(u2);
	pushParam(ustride);
	pushParam(uorder);
	pushParam(v1);
	pushParam(v2);
	pushParam(vstride);
	pushParam(vorder);
	pushBuf(points, sizeof(GLfloat) * (uorder + ustride) * (vorder + vstride));
}

//224
extern "C" void glMapGrid1d(GLint un, GLdouble u1, GLdouble u2){
	pushOp(224);
	pushParam(un);
	pushParam(u1);
	pushParam(u2);
}

//225
extern "C" void glMapGrid1f(GLint un, GLfloat u1, GLfloat u2){
	pushOp(225);
	pushParam(un);
	pushParam(u1);
	pushParam(u2);
}

//226
extern "C" void glMapGrid2d(GLint un, GLdouble u1, GLdouble u2, GLint vn, GLdouble v1, GLdouble v2){
	pushOp(226);
	pushParam(un);
	pushParam(u1);
	pushParam(u2);
	pushParam(vn);
	pushParam(v1);
	pushParam(v2);
}

//227
extern "C" void glMapGrid2f(GLint un, GLfloat u1, GLfloat u2, GLint vn, GLfloat v1, GLfloat v2){
	pushOp(227);
	pushParam(un);
	pushParam(u1);
	pushParam(u2);
	pushParam(vn);
	pushParam(v1);
	pushParam(v2);
}

//228
extern "C" void glEvalCoord1d(GLdouble u){
	pushOp(228);
	pushParam(u);
}

//229
extern "C" void glEvalCoord1dv(const GLdouble * u){
	pushOp(229);
	pushBuf(u, sizeof(const GLdouble) * 1);
}

//230
extern "C" void glEvalCoord1f(GLfloat u){
	pushOp(230);
	pushParam(u);
}

//231
extern "C" void glEvalCoord1fv(const GLfloat * u){
	pushOp(231);
	pushBuf(u, sizeof(const GLfloat) * 1);
}

//232
extern "C" void glEvalCoord2d(GLdouble u, GLdouble v){
	pushOp(232);
	pushParam(u);
	pushParam(v);
}

//233
extern "C" void glEvalCoord2dv(const GLdouble * u){
	pushOp(233);
	pushBuf(u, sizeof(const GLdouble) * 2);
}

//234
extern "C" void glEvalCoord2f(GLfloat u, GLfloat v){
	pushOp(234);
	pushParam(u);
	pushParam(v);
}

//235
extern "C" void glEvalCoord2fv(const GLfloat * u){
	pushOp(235);
	pushBuf(u, sizeof(const GLfloat) * 2);
}

//236
extern "C" void glEvalMesh1(GLenum mode, GLint i1, GLint i2){
	pushOp(236);
	pushParam(mode);
	pushParam(i1);
	pushParam(i2);
}

//237
extern "C" void glEvalPoint1(GLint i){
	pushOp(237);
	pushParam(i);
}

//238
extern "C" void glEvalMesh2(GLenum mode, GLint i1, GLint i2, GLint j1, GLint j2){
	pushOp(238);
	pushParam(mode);
	pushParam(i1);
	pushParam(i2);
	pushParam(j1);
	pushParam(j2);
}

//239
extern "C" void glEvalPoint2(GLint i, GLint j){
	pushOp(239);
	pushParam(i);
	pushParam(j);
}

//240
extern "C" void glAlphaFunc(GLenum func, GLclampf ref){
	pushOp(240);
	pushParam(func);
	pushParam(ref);
}

//241
extern "C" void glBlendFunc(GLenum sfactor, GLenum dfactor){
	pushOp(241);
	pushParam(sfactor);
	pushParam(dfactor);
}

//242
extern "C" void glLogicOp(GLenum opcode){
	pushOp(242);
	pushParam(opcode);
}

//243
extern "C" void glStencilFunc(GLenum func, GLint ref, GLuint mask){
	pushOp(243);
	pushParam(func);
	pushParam(ref);
	pushParam(mask);
}

//244
extern "C" void glStencilOp(GLenum fail, GLenum zfail, GLenum zpass){
	pushOp(244);
	pushParam(fail);
	pushParam(zfail);
	pushParam(zpass);
}

//245
extern "C" void glDepthFunc(GLenum func){
	pushOp(245);
	pushParam(func);
}

//246
extern "C" void glPixelZoom(GLfloat xfactor, GLfloat yfactor){
	pushOp(246);
	pushParam(xfactor);
	pushParam(yfactor);
}

//247
extern "C" void glPixelTransferf(GLenum pname, GLfloat param){
	pushOp(247);
	pushParam(pname);
	pushParam(param);
}

//248
extern "C" void glPixelTransferi(GLenum pname, GLint param){
	pushOp(248);
	pushParam(pname);
	pushParam(param);
}

//249
extern "C" void glPixelStoref(GLenum pname, GLfloat param){
	pushOp(249);
	pushParam(pname);
	pushParam(param);
}

//250
extern "C" void glPixelStorei(GLenum pname, GLint param){
	pushOp(250);
	pushParam(pname);
	pushParam(param);
}

//251
extern "C" void glPixelMapfv(GLenum map, GLsizei mapsize, const GLfloat * values){
	pushOp(251);
	pushParam(map);
	pushParam(mapsize);
	pushBuf(values, sizeof(const GLfloat) * mapsize);
}

//252
extern "C" void glPixelMapuiv(GLenum map, GLsizei mapsize, const GLuint * values){
	pushOp(252);
	pushParam(map);
	pushParam(mapsize);
	pushBuf(values, sizeof(const GLuint) * mapsize);
}

//253
extern "C" void glPixelMapusv(GLenum map, GLsizei mapsize, const GLushort * values){
	pushOp(253);
	pushParam(map);
	pushParam(mapsize);
	pushBuf(values, sizeof(const GLushort) * mapsize);
}

//254
extern "C" void glReadBuffer(GLenum mode){
	pushOp(254);
	pushParam(mode);
}

//255
extern "C" void glCopyPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum type){
	pushOp(255);
	pushParam(x);
	pushParam(y);
	pushParam(width);
	pushParam(height);
	pushParam(type);
}

//256
extern "C" void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid * pixels){
	pushOp(256);
	pushParam(x);
	pushParam(y);
	pushParam(width);
	pushParam(height);
	pushParam(format);
	pushParam(type);
	
	
    
    int bpp = 1;
    
    if(format == GL_BGR || format == GL_RGB) bpp = 3;
    else if(format == GL_RGBA || format == GL_BGRA) bpp = 4;
	
	pushBuf(pixels, width * height * bpp, true);
	
	waitForReturn();
}

//257
extern "C" void glDrawPixels(GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels){
	LOG("Called untested stub DrawPixels!\n");
	pushOp(257);
	pushParam(width);
	pushParam(height);
	pushParam(format);
	//more types need defining in my method above
	pushParam(type);
	pushBuf(pixels, getTypeSize(type) * width * height);
}

//258
extern "C" void glGetBooleanv(GLenum pname, GLboolean * params){
	LOG("Called unimplemted stub GetBooleanv!\n");
	//huge number of constants
}

//259
extern "C" void glGetClipPlane(GLenum plane, GLdouble * equation){
	pushOp(259);
	pushParam(plane);
	pushBuf(equation, sizeof(GLdouble) * 4);
}

//260
/*
extern "C" void glGetDoublev(GLenum pname, GLdouble * params){
	LOG("Called unimplemted stub GetDoublev!\n");
}
*/

//261
extern "C" GLenum glGetError(){
	pushOp(261);

	GLenum ret;
	pushBuf(&ret, sizeof(GLenum), true);
	waitForReturn();

	return ret;
}

//262
//extern "C" void glGetFloatv(GLenum pname, GLfloat * params){
//	LOG("Called unimplemted stub GetFloatv!\n");
//}

//263
//HACK
//extern "C" void glGetIntegerv(GLenum pname, GLint * params){
//	LOG("Called unimplemted stub GetIntegerv!\n");
//}

//264
extern "C" void glGetLightfv(GLenum light, GLenum pname, GLfloat * params){
	LOG("Called untested stub GetLightfv!\n");
	pushOp(264);
	pushParam(light);
	pushParam(pname);
	pushBuf(params, sizeof(GLfloat) * getLightParamSize(pname));

}

//265
extern "C" void glGetLightiv(GLenum light, GLenum pname, GLint * params){
	LOG("Called untested stub GetLightiv!\n");
	pushOp(265);
	pushParam(light);
	pushParam(pname);
	pushBuf(params, sizeof(GLfloat) * getLightParamSize(pname));
}

//266
extern "C" void glGetMapdv(GLenum target, GLenum query, GLdouble * v){
	pushOp(266);
	pushParam(target);
	pushParam(query);
	pushBuf(v, sizeof(GLdouble) * 4, true);
}

//267
extern "C" void glGetMapfv(GLenum target, GLenum query, GLfloat * v){
	pushOp(267);
	pushParam(target);
	pushParam(query);
	pushBuf(v, sizeof(GLfloat) * 4, true);
}

//268
extern "C" void glGetMapiv(GLenum target, GLenum query, GLint * v){
	pushOp(268);
	pushParam(target);
	pushParam(query);
	pushBuf(v, sizeof(GLint) * 4, true);
}

//269
extern "C" void glGetMaterialfv(GLenum face, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetMaterialfv!\n");
}

//270
extern "C" void glGetMaterialiv(GLenum face, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetMaterialiv!\n");
}

//271
extern "C" void glGetPixelMapfv(GLenum map, GLfloat * values){
	LOG("Called unimplemted stub GetPixelMapfv!\n");
	//To determine the required size of map,call glGet()
}

//272
extern "C" void glGetPixelMapuiv(GLenum map, GLuint * values){
	LOG("Called unimplemted stub GetPixelMapuiv!\n");
}

//273
extern "C" void glGetPixelMapusv(GLenum map, GLushort * values){
	LOG("Called unimplemted stub GetPixelMapusv!\n");
}

//274
extern "C" void glGetPolygonStipple(GLubyte * mask){
	LOG("Called untested stub GetPolygonStipple!\n");
	pushBuf(mask, sizeof(GLubyte) * 32 * 32, true); //32 x 32 stipple
}

//275
//Hack! We let the native implementation handle this
/*
extern "C" const GLubyte * glGetString(GLenum name){
	pushOp(275);
	pushParam(name);

	const GLubyte * ret;
	pushBuf(&ret, sizeof(const GLubyte *), true);
	waitForReturn();

	return ret;
}

//276
extern "C" void glGetTexEnvfv(GLenum target, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetTexEnvfv!\n");
}

//277
extern "C" void glGetTexEnviv(GLenum target, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetTexEnviv!\n");
}

//278
extern "C" void glGetTexGendv(GLenum coord, GLenum pname, GLdouble * params){
	LOG("Called unimplemted stub GetTexGendv!\n");
}

//279
extern "C" void glGetTexGenfv(GLenum coord, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetTexGenfv!\n");
}

//280
extern "C" void glGetTexGeniv(GLenum coord, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetTexGeniv!\n");
}

//281
extern "C" void glGetTexImage(GLenum target, GLint level, GLenum format, GLenum type, GLvoid * pixels){
	LOG("Called unimplemted stub GetTexImage!\n");
}

//282
extern "C" void glGetTexParameterfv(GLenum target, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetTexParameterfv!\n");
}

//283
extern "C" void glGetTexParameteriv(GLenum target, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetTexParameteriv!\n");
}

//284
extern "C" void glGetTexLevelParameterfv(GLenum target, GLint level, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetTexLevelParameterfv!\n");
}

//285
extern "C" void glGetTexLevelParameteriv(GLenum target, GLint level, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetTexLevelParameteriv!\n");
}*/

//286
extern "C" GLboolean glIsEnabled(GLenum cap){
	pushOp(286);
	pushParam(cap);

	GLboolean ret;
	pushBuf(&ret, sizeof(GLboolean), true);
	waitForReturn();

	return ret;
}

//287
extern "C" GLboolean glIsList(GLuint list){
	pushOp(287);
	pushParam(list);

	GLboolean ret;
	pushBuf(&ret, sizeof(GLboolean), true);
	waitForReturn();

	return ret;
}

//288
extern "C" void glDepthRange(GLclampd zNear, GLclampd zFar){
	pushOp(288);
	pushParam(zNear);
	pushParam(zFar);
}

//289
extern "C" void glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar){
	pushOp(289);
	pushParam(left);
	pushParam(right);
	pushParam(bottom);
	pushParam(top);
	pushParam(zNear);
	pushParam(zFar);
}

//290
extern "C" void glLoadIdentity(){
	pushOp(290);
}

//291
extern "C" void glLoadMatrixf(const GLfloat * m){
	pushOp(291);
	pushBuf(m, sizeof(const GLfloat) * 16);
}

//292
extern "C" void glLoadMatrixd(const GLdouble * m){
	pushOp(292);
	pushBuf(m, sizeof(const GLdouble) * 16);
}

//293
extern "C" void glMatrixMode(GLenum mode){
	pushOp(293);
	pushParam(mode);
}

//294
extern "C" void glMultMatrixf(const GLfloat * m){
	pushOp(294);
	pushBuf(m, sizeof(const GLfloat) * 16);
}

//295
extern "C" void glMultMatrixd(const GLdouble * m){
	pushOp(295);
	pushBuf(m, sizeof(const GLdouble) * 16); 
}

//296
extern "C" void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble zNear, GLdouble zFar){
	pushOp(296);
	pushParam(left);
	pushParam(right);
	pushParam(bottom);
	pushParam(top);
	pushParam(zNear);
	pushParam(zFar);
}

//297
extern "C" void glPopMatrix(){
	pushOp(297);
}

//298
extern "C" void glPushMatrix(){
	pushOp(298);
}

//299
extern "C" void glRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z){
	pushOp(299);
	pushParam(angle);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//300
extern "C" void glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z){
	pushOp(300);
	pushParam(angle);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//301
extern "C" void glScaled(GLdouble x, GLdouble y, GLdouble z){
	pushOp(301);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//302
extern "C" void glScalef(GLfloat x, GLfloat y, GLfloat z){
	pushOp(302);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//303
extern "C" void glTranslated(GLdouble x, GLdouble y, GLdouble z){
	pushOp(303);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//304
extern "C" void glTranslatef(GLfloat x, GLfloat y, GLfloat z){
	pushOp(304);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//305
extern "C" void glViewport(GLint x, GLint y, GLsizei width, GLsizei height){
	pushOp(305);
	pushParam(x);
	pushParam(y);
	pushParam(width);
	pushParam(height);
}

//306
extern "C" void glArrayElement(GLint i){
	pushOp(306);
	pushParam(i);
}

//308
extern "C" void glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer){
	LOG("Called untested stub ColorPointer!\n");
	pushOp(308);
	pushParam(size);
	pushParam(type);
	pushParam(stride);
	pushBuf(pointer, getTypeSize(type) * size);
}

//309
extern "C" void glDisableClientState(GLenum array){
	pushOp(309);
	pushParam(array);
}

//310
extern "C" void glDrawArrays(GLenum mode, GLint first, GLsizei count){
	pushOp(310);
	pushParam(mode);
	pushParam(first);
	pushParam(count);
}

//311
extern "C" void glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid * indices){
	LOG("Called untested stub DrawElements!\n");
	pushOp(311);
	pushParam(mode);
	pushParam(count);
	pushParam(type);
	//most likely null and will crash
	//pushParam(*indices);	
}

//312
extern "C" void glEdgeFlagPointer(GLsizei stride, const GLvoid * pointer){
	LOG("Called unimplemted stub EdgeFlagPointer!\n");
}

//313
extern "C" void glEnableClientState(GLenum array){
	pushOp(313);
	pushParam(array);
}

//329
extern "C" void glGetPointerv(GLenum pname, GLvoid ** params){
	LOG("Called unimplemted stub GetPointerv!\n");
}

//314
extern "C" void glIndexPointer(GLenum type, GLsizei stride, const GLvoid * pointer){
	LOG("Called unimplemted stub IndexPointer!\n");
	//pushOp(314);
	//pushParam(type);
	//pushParam(stride);
	//pushBuf(pointer, getTypeSize(type) * (/*size of array*/ + stride));
}

//317
extern "C" void glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid * pointer){
	LOG("Called unimplemted stub InterleavedArrays!\n");
}

//318
extern "C" void glNormalPointer(GLenum type, GLsizei stride, const GLvoid * pointer){
	LOG("Called unimplemted stub NormalPointer!\n");
}

//320
extern "C" void glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer){
	pushOp(320);
	pushParam(size);
	pushParam(type);
	pushParam(stride);
	pushBuf(pointer, getTypeSize(type)  * (stride + size)); 
}

//321
extern "C" void glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer){
	pushOp(321);
	pushParam(size);
	pushParam(type);
	pushParam(stride);
	int mySize = (getTypeSize(type)  * (stride + size)) * 24;
	pushBuf(pointer, mySize);
}

//319
extern "C" void glPolygonOffset(GLfloat factor, GLfloat units){
	pushOp(319);
	pushParam(factor);
	pushParam(units);
}

//323
extern "C" void glCopyTexImage1D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border){
	pushOp(323);
	pushParam(target);
	pushParam(level);
	pushParam(internalformat);
	pushParam(x);
	pushParam(y);
	pushParam(width);
	pushParam(border);
}

//324
extern "C" void glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border){
	pushOp(324);
	pushParam(target);
	pushParam(level);
	pushParam(internalformat);
	pushParam(x);
	pushParam(y);
	pushParam(width);
	pushParam(height);
	pushParam(border);
}

//325
extern "C" void glCopyTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width){
	pushOp(325);
	pushParam(target);
	pushParam(level);
	pushParam(xoffset);
	pushParam(x);
	pushParam(y);
	pushParam(width);
}

//326
extern "C" void glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height){
	pushOp(326);
	pushParam(target);
	pushParam(level);
	pushParam(xoffset);
	pushParam(yoffset);
	pushParam(x);
	pushParam(y);
	pushParam(width);
	pushParam(height);
}

//332
extern "C" void glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, const GLvoid * pixels){
	LOG("Called untested stub TexSubImage1D!\n");
	pushOp(332);
	pushParam(target);
	pushParam(level);
	pushParam(xoffset);
	pushParam(width);
	pushParam(format);
	pushParam(type);
	pushBuf(pixels, (width + xoffset) * getTypeSize(type));
}

//333
extern "C" void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * pixels){
	pushOp(333);
	pushParam(target);
	pushParam(level);
	pushParam(xoffset);
	pushParam(yoffset);
	pushParam(width);
	pushParam(height);
	pushParam(format);
	pushParam(type);
	pushBuf(pixels, width * height * getTypeSize(type));
}

//322
extern "C" GLboolean glAreTexturesResident(GLsizei n, const GLuint * textures, GLboolean * residences){
	pushOp(322);
	pushParam(n);
	pushBuf(textures, sizeof(const GLuint) * n);
	pushBuf(residences, sizeof(GLboolean) * n);

	GLboolean ret;
	pushBuf(&ret, sizeof(GLboolean), true);
	waitForReturn();

	return ret;
}

//307
extern "C" void glBindTexture(GLenum target, GLuint texture){
	pushOp(307);
	pushParam(target);
	pushParam(texture);
}

//327
extern "C" void glDeleteTextures(GLsizei n, const GLuint * textures){
	pushOp(327);
	pushParam(n);
	pushBuf(textures, sizeof(const GLuint) * n);
}

//328
extern "C" void glGenTextures(GLsizei n, GLuint * textures){
	pushOp(328);
	pushParam(n);
	pushBuf(textures, sizeof(GLuint) * n, true);
		
	waitForReturn();
}

//330
extern "C" GLboolean glIsTexture(GLuint texture){
	pushOp(330);
	pushParam(texture);

	GLboolean ret;
	pushBuf(&ret, sizeof(GLboolean), true);
	waitForReturn();

	return ret;
}

//331
extern "C" void glPrioritizeTextures(GLsizei n, const GLuint * textures, const GLclampf * priorities){
	pushOp(331);
	pushParam(n);
	pushBuf(textures, sizeof(const GLuint) * n);
	pushBuf(priorities, sizeof(const GLclampf) * n);
}

//315
extern "C" void glIndexub(GLubyte c){
	pushOp(315);
	pushParam(c);
}

//316
extern "C" void glIndexubv(const GLubyte * c){
	pushOp(316);
	pushBuf(c, sizeof(const GLubyte) * 1);
}

//334
extern "C" void glPopClientAttrib(){
	pushOp(334);
}

//335
extern "C" void glPushClientAttrib(GLbitfield mask){
	pushOp(335);
	pushParam(mask);
}

//336
extern "C" void glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha){
	pushOp(336);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
	pushParam(alpha);
}

//337
extern "C" void glBlendEquation(GLenum mode){
	pushOp(337);
	pushParam(mode);
}

//338
extern "C" void glDrawRangeElements(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices){
	LOG("Called untested stub DrawRangeElements!\n");
	pushOp(338);
	pushParam(mode);
	pushParam(start);
	pushParam(end);
	pushParam(count);
	pushParam(type);
	pushBuf(indices, getTypeSize(type) * end);
}

//339
extern "C" void glColorTable(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid * table){
	LOG("Called untested stub ColorTable!\n");
	pushOp(339);
	pushParam(target);
	pushParam(internalformat);
	pushParam(width);
	pushParam(format);
	pushParam(type);
	pushBuf(table, getTypeSize(type) * width);
}

//340
extern "C" void glColorTableParameterfv(GLenum target, GLenum pname, const GLfloat * params){
	LOG("Called untested stub ColorTableParameterfv!\n");
	pushOp(340);
	pushParam(target);
	pushParam(pname);
	pushBuf(params, sizeof(GLfloat) * 4);
}

//341
extern "C" void glColorTableParameteriv(GLenum target, GLenum pname, const GLint * params){
	LOG("Called untested stub ColorTableParameteriv!\n");
	pushOp(341);
	pushParam(target);
	pushParam(pname);
	pushBuf(params, sizeof(GLint) * 4);
}

//342
extern "C" void glCopyColorTable(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width){
	pushOp(342);
	pushParam(target);
	pushParam(internalformat);
	pushParam(x);
	pushParam(y);
	pushParam(width);
}

//343
extern "C" void glGetColorTable(GLenum target, GLenum format, GLenum type, GLvoid * table){
	LOG("Called unimplemted stub GetColorTable!\n");
}

//344
extern "C" void glGetColorTableParameterfv(GLenum target, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetColorTableParameterfv!\n");
}

//345
extern "C" void glGetColorTableParameteriv(GLenum target, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetColorTableParameteriv!\n");
}

//346
extern "C" void glColorSubTable(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid * data){
	LOG("Called untested stub ColorSubTable!\n");
	pushOp(346);
	pushParam(target);
	pushParam(start);
	pushParam(count);
	pushParam(format);
	pushParam(type);
	pushBuf(data, getTypeSize(type) * (start + count));	
}

//347
extern "C" void glCopyColorSubTable(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width){
	pushOp(347);
	pushParam(target);
	pushParam(start);
	pushParam(x);
	pushParam(y);
	pushParam(width);
}

//348
extern "C" void glConvolutionFilter1D(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid * image){
	LOG("Called untested stub ConvolutionFilter1D!\n");
	pushOp(348);
	pushParam(target);
	pushParam(internalformat);
	pushParam(width);
	pushParam(format);
	pushParam(type);
	pushBuf(image, getTypeSize(type) * width);
}

//349
extern "C" void glConvolutionFilter2D(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * image){
	LOG("Called untested stub ConvolutionFilter2D!\n");
	pushOp(349);
	pushParam(target);
	pushParam(internalformat);
	pushParam(width);
	pushParam(height);
	pushParam(format);
	pushParam(type);
	pushBuf(image, getTypeSize(type) * width * height);
}

//350
extern "C" void glConvolutionParameterf(GLenum target, GLenum pname, GLfloat params){
	pushOp(350);
	pushParam(target);
	pushParam(pname);
	pushParam(params);
}

//351
extern "C" void glConvolutionParameterfv(GLenum target, GLenum pname, const GLfloat * params){
	LOG("Called untested stub ConvolutionParameterfv!\n");
	pushOp(351);
	pushParam(target);
	pushParam(pname);
	int size = sizeof(GLfloat);
	if(pname != GL_CONVOLUTION_BORDER_MODE)
		size *= 4;
	pushBuf(params, size);

}

//352
extern "C" void glConvolutionParameteri(GLenum target, GLenum pname, GLint params){
	pushOp(352);
	pushParam(target);
	pushParam(pname);
	pushParam(params);
}

//353
extern "C" void glConvolutionParameteriv(GLenum target, GLenum pname, const GLint * params){
	LOG("Called untested stub ConvolutionParameteriv!\n");
	pushOp(353);
	pushParam(target);
	pushParam(pname);
	int size = sizeof(GLint);
	if(pname != GL_CONVOLUTION_BORDER_MODE)
		size *= 4;
	pushBuf(params, size);
	
}

//354
extern "C" void glCopyConvolutionFilter1D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width){
	pushOp(354);
	pushParam(target);
	pushParam(internalformat);
	pushParam(x);
	pushParam(y);
	pushParam(width);
}

//355
extern "C" void glCopyConvolutionFilter2D(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height){
	pushOp(355);
	pushParam(target);
	pushParam(internalformat);
	pushParam(x);
	pushParam(y);
	pushParam(width);
	pushParam(height);
}

//356
extern "C" void glGetConvolutionFilter(GLenum target, GLenum format, GLenum type, GLvoid * image){
	LOG("Called unimplemted stub GetConvolutionFilter!\n");
}

//357
extern "C" void glGetConvolutionParameterfv(GLenum target, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetConvolutionParameterfv!\n");
}

//358
extern "C" void glGetConvolutionParameteriv(GLenum target, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetConvolutionParameteriv!\n");
}

//359
extern "C" void glGetSeparableFilter(GLenum target, GLenum format, GLenum type, GLvoid * row, GLvoid * column, GLvoid * span){
	LOG("Called unimplemted stub GetSeparableFilter!\n");
}

//360
extern "C" void glSeparableFilter2D(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * row, const GLvoid * column){
	LOG("Called untested stub SeparableFilter2D!\n");
	pushOp(360);
	pushParam(target);
	pushParam(internalformat);
	pushParam(width);
	pushParam(height);
	pushParam(format);
	pushParam(type);
	pushBuf(row, getTypeSize(type) * width);
	pushBuf(column, getTypeSize(type) * height);
}

//361
extern "C" void glGetHistogram(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid * values){
	LOG("Called unimplemted stub GetHistogram!\n");
}

//362
extern "C" void glGetHistogramParameterfv(GLenum target, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetHistogramParameterfv!\n");
}

//363
extern "C" void glGetHistogramParameteriv(GLenum target, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetHistogramParameteriv!\n");
}

//364
extern "C" void glGetMinmax(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid * values){
	LOG("Called untested stub GetMinmax!\n");
	pushOp(364);
	pushParam(target);
	pushParam(reset);
	pushParam(format);
	pushParam(type);
	pushBuf(values, getTypeSize(type) * 2, true); //returns 2 values (min, max)
}

//365
extern "C" void glGetMinmaxParameterfv(GLenum target, GLenum pname, GLfloat * params){
	LOG("Called untested stub GetMinmaxParameterfv!\n");
	pushOp(365);
	pushParam(target);
	pushParam(pname);
	pushBuf(params, sizeof(GLfloat) * 2, true); //returns 2 values (min, max)
}

//366
extern "C" void glGetMinmaxParameteriv(GLenum target, GLenum pname, GLint * params){
	LOG("Called untested stub GetMinmaxParameteriv!\n");
	pushOp(366);
	pushParam(target);
	pushParam(pname);
	pushBuf(params, sizeof(GLint) * 2, true); //returns 2 values (min, max)
}

//367
extern "C" void glHistogram(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink){
	pushOp(367);
	pushParam(target);
	pushParam(width);
	pushParam(internalformat);
	pushParam(sink);
}

//368
extern "C" void glMinmax(GLenum target, GLenum internalformat, GLboolean sink){
	pushOp(368);
	pushParam(target);
	pushParam(internalformat);
	pushParam(sink);
}

//369
extern "C" void glResetHistogram(GLenum target){
	pushOp(369);
	pushParam(target);
}

//370
extern "C" void glResetMinmax(GLenum target){
	pushOp(370);
	pushParam(target);
}

//371
extern "C" void glTexImage3D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid * pixels){
	LOG("Called untested stub TexImage3D!\n");
	pushOp(371);
	pushParam(target);
	pushParam(level);
	pushParam(internalformat);
	pushParam(width);
	pushParam(height);
	pushParam(depth);
	pushParam(border);
	pushParam(format);
	pushParam(type);
	pushBuf(pixels, getTypeSize(type) * width * height * depth);
}

//372
extern "C" void glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid * pixels){
	LOG("Called untested stub TexSubImage3D!\n");
	pushOp(372);
	pushParam(target);
	pushParam(level);
	pushParam(xoffset);
	pushParam(yoffset);
	pushParam(zoffset);
	pushParam(width);
	pushParam(height);
	pushParam(depth);
	pushParam(format);
	pushParam(type);
	pushBuf(pixels, getTypeSize(type) * width * height * depth);
}

//373
extern "C" void glCopyTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height){
	pushOp(373);
	pushParam(target);
	pushParam(level);
	pushParam(xoffset);
	pushParam(yoffset);
	pushParam(zoffset);
	pushParam(x);
	pushParam(y);
	pushParam(width);
	pushParam(height);
}

//374
extern "C" void glActiveTexture(GLenum texture){
	pushOp(374);
	pushParam(texture);
}

//375
extern "C" void glClientActiveTexture(GLenum texture){
	pushOp(375);
	pushParam(texture);
}

//376
extern "C" void glMultiTexCoord1d(GLenum target, GLdouble s){
	pushOp(376);
	pushParam(target);
	pushParam(s);
}

//377
extern "C" void glMultiTexCoord1dv(GLenum target, const GLdouble * v){
	pushOp(377);
	pushParam(target);
	pushBuf(v, sizeof(const GLdouble) * 1);
}

//378
extern "C" void glMultiTexCoord1f(GLenum target, GLfloat s){
	pushOp(378);
	pushParam(target);
	pushParam(s);
}

//379
extern "C" void glMultiTexCoord1fv(GLenum target, const GLfloat * v){
	pushOp(379);
	pushParam(target);
	pushBuf(v, sizeof(const GLfloat) * 1);
}

//380
extern "C" void glMultiTexCoord1i(GLenum target, GLint s){
	pushOp(380);
	pushParam(target);
	pushParam(s);
}

//381
extern "C" void glMultiTexCoord1iv(GLenum target, const GLint * v){
	pushOp(381);
	pushParam(target);
	pushBuf(v, sizeof(const GLint) * 1);
}

//382
extern "C" void glMultiTexCoord1s(GLenum target, GLshort s){
	pushOp(382);
	pushParam(target);
	pushParam(s);
}

//383
extern "C" void glMultiTexCoord1sv(GLenum target, const GLshort * v){
	pushOp(383);
	pushParam(target);
	pushBuf(v, sizeof(const GLshort) * 1);
}

//384
extern "C" void glMultiTexCoord2d(GLenum target, GLdouble s, GLdouble t){
	pushOp(384);
	pushParam(target);
	pushParam(s);
	pushParam(t);
}

//385
extern "C" void glMultiTexCoord2dv(GLenum target, const GLdouble * v){
	pushOp(385);
	pushParam(target);
	pushBuf(v, sizeof(const GLdouble) * 2);
}

//386
extern "C" void glMultiTexCoord2f(GLenum target, GLfloat s, GLfloat t){
	pushOp(386);
	pushParam(target);
	pushParam(s);
	pushParam(t);
}

//387
extern "C" void glMultiTexCoord2fv(GLenum target, const GLfloat * v){
	pushOp(387);
	pushParam(target);
	pushBuf(v, sizeof(const GLfloat) * 2);
}

//388
extern "C" void glMultiTexCoord2i(GLenum target, GLint s, GLint t){
	pushOp(388);
	pushParam(target);
	pushParam(s);
	pushParam(t);
}

//389
extern "C" void glMultiTexCoord2iv(GLenum target, const GLint * v){
	pushOp(389);
	pushParam(target);
	pushBuf(v, sizeof(const GLint) * 2);
}

//390
extern "C" void glMultiTexCoord2s(GLenum target, GLshort s, GLshort t){
	pushOp(390);
	pushParam(target);
	pushParam(s);
	pushParam(t);
}

//391
extern "C" void glMultiTexCoord2sv(GLenum target, const GLshort * v){
	pushOp(391);
	pushParam(target);
	pushBuf(v, sizeof(const GLshort) * 2);
}

//392
extern "C" void glMultiTexCoord3d(GLenum target, GLdouble s, GLdouble t, GLdouble r){
	pushOp(392);
	pushParam(target);
	pushParam(s);
	pushParam(t);
	pushParam(r);
}

//393
extern "C" void glMultiTexCoord3dv(GLenum target, const GLdouble * v){
	pushOp(393);
	pushParam(target);
	pushBuf(v, sizeof(const GLdouble) * 3);
}

//394
extern "C" void glMultiTexCoord3f(GLenum target, GLfloat s, GLfloat t, GLfloat r){
	pushOp(394);
	pushParam(target);
	pushParam(s);
	pushParam(t);
	pushParam(r);
}

//395
extern "C" void glMultiTexCoord3fv(GLenum target, const GLfloat * v){
	pushOp(395);
	pushParam(target);
	pushBuf(v, sizeof(const GLfloat) * 3);
}

//396
extern "C" void glMultiTexCoord3i(GLenum target, GLint s, GLint t, GLint r){
	pushOp(396);
	pushParam(target);
	pushParam(s);
	pushParam(t);
	pushParam(r);
}

//397
extern "C" void glMultiTexCoord3iv(GLenum target, const GLint * v){
	pushOp(397);
	pushParam(target);
	pushBuf(v, sizeof(const GLint) * 3);
}

//398
extern "C" void glMultiTexCoord3s(GLenum target, GLshort s, GLshort t, GLshort r){
	pushOp(398);
	pushParam(target);
	pushParam(s);
	pushParam(t);
	pushParam(r);
}

//399
extern "C" void glMultiTexCoord3sv(GLenum target, const GLshort * v){
	pushOp(399);
	pushParam(target);
	pushBuf(v, sizeof(const GLshort) * 3);
}

//400
extern "C" void glMultiTexCoord4d(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q){
	pushOp(400);
	pushParam(target);
	pushParam(s);
	pushParam(t);
	pushParam(r);
	pushParam(q);
}

//401
extern "C" void glMultiTexCoord4dv(GLenum target, const GLdouble * v){
	pushOp(401);
	pushParam(target);
	pushBuf(v, sizeof(const GLdouble) * 4);
}

//402
extern "C" void glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q){
	pushOp(402);
	pushParam(target);
	pushParam(s);
	pushParam(t);
	pushParam(r);
	pushParam(q);
}

//403
extern "C" void glMultiTexCoord4fv(GLenum target, const GLfloat * v){
	pushOp(403);
	pushParam(target);
	pushBuf(v, sizeof(const GLfloat) * 4);
}

//404
extern "C" void glMultiTexCoord4i(GLenum target, GLint s, GLint t, GLint r, GLint q){
	pushOp(404);
	pushParam(target);
	pushParam(s);
	pushParam(t);
	pushParam(r);
	pushParam(q);
}

//405
extern "C" void glMultiTexCoord4iv(GLenum target, const GLint * v){
	pushOp(405);
	pushParam(target);
	pushBuf(v, sizeof(const GLint) * 4);
}

//406
extern "C" void glMultiTexCoord4s(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q){
	pushOp(406);
	pushParam(target);
	pushParam(s);
	pushParam(t);
	pushParam(r);
	pushParam(q);
}

//407
extern "C" void glMultiTexCoord4sv(GLenum target, const GLshort * v){
	pushOp(407);
	pushParam(target);
	pushBuf(v, sizeof(const GLshort) * 4);
}

//408
extern "C" void glLoadTransposeMatrixf(const GLfloat * m){
	LOG("Called untested stub LoadTransposeMatrixf!\n");
	pushOp(408);
	pushBuf(m, sizeof(const GLfloat) * 16, true);		//4x4 matrix
	waitForReturn();
}

//409
extern "C" void glLoadTransposeMatrixd(const GLdouble * m){
	LOG("Called untested stub LoadTransposeMatrixd!\n");
	pushOp(409);
	pushBuf(m, sizeof(const GLdouble) * 16, true);		//4x4 matrix
	waitForReturn();
}

//410
extern "C" void glMultTransposeMatrixf(const GLfloat * m){
	LOG("Called untested stub MultTransposeMatrixf!\n");
	pushOp(410);
	pushBuf(m, sizeof(const GLfloat) * 16, true);		//4x4 matrix
}

//411
extern "C" void glMultTransposeMatrixd(const GLdouble * m){
	LOG("Called untested stub MultTransposeMatrixd!\n");
	pushOp(411);
	pushBuf(m, sizeof(const GLdouble) * 16, true);		//4x4 matrix
}

//412
extern "C" void glSampleCoverage(GLclampf value, GLboolean invert){
	pushOp(412);
	pushParam(value);
	pushParam(invert);
}

//413
extern "C" void glCompressedTexImage3D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid * data){
	LOG("Called untested stub CompressedTexImage3D!\n");
	pushOp(413);
	pushParam(target);
	pushParam(level);
	pushParam(internalformat);
	pushParam(width);
	pushParam(height);
	pushParam(depth);
	pushParam(border);
	pushParam(imageSize);
	pushBuf(data, sizeof(const GLubyte) * imageSize);
}

//414
extern "C" void glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid * data){
	LOG("Called untested stub CompressedTexImage2D!\n");
	pushOp(414);
	pushParam(target);
	pushParam(level);
	pushParam(internalformat);
	pushParam(width);
	pushParam(height);
	pushParam(border);
	pushParam(imageSize);
	pushBuf(data, sizeof(const GLubyte) * imageSize);
}

//415
extern "C" void glCompressedTexImage1D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid * data){
	LOG("Called untested stub CompressedTexImage1D!\n");
	pushOp(415);
	pushParam(target);
	pushParam(level);
	pushParam(internalformat);
	pushParam(width);
	pushParam(border);
	pushParam(imageSize);
	pushBuf(data, sizeof(const GLubyte) * imageSize);
}

//416
extern "C" void glCompressedTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid * data){
	LOG("Called untested stub CompressedTexSubImage3D!\n");
	pushOp(416);
	pushParam(target);
	pushParam(level);
	pushParam(xoffset);
	pushParam(yoffset);
	pushParam(zoffset);
	pushParam(width);
	pushParam(height);
	pushParam(depth);
	pushParam(format);
	pushParam(imageSize);
	pushBuf(data, sizeof(const GLubyte) * imageSize);
}

//417
extern "C" void glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid * data){
	LOG("Called untested stub CompressedTexSubImage2D!\n");
	pushOp(417);
	pushParam(target);
	pushParam(level);
	pushParam(xoffset);
	pushParam(yoffset);
	pushParam(width);
	pushParam(height);
	pushParam(format);
	pushParam(imageSize);
	pushBuf(data, sizeof(const GLubyte) * imageSize);
}

//418
extern "C" void glCompressedTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid * data){
	LOG("Called untested stub CompressedTexSubImage1D!\n");
	pushOp(418);
	pushParam(target);
	pushParam(level);
	pushParam(xoffset);
	pushParam(width);
	pushParam(format);
	pushParam(imageSize);
	pushBuf(data, sizeof(const GLubyte) * imageSize);
}

//419
extern "C" void glGetCompressedTexImage(GLenum target, GLint level, GLvoid * img){
	LOG("Called unimplemted stub GetCompressedTexImage!\n");
	//pushOp(419);
	//pushParam(target);
	//pushParam(level);
	//pushBuf(img, GL_TEXTURE_COMPRESSED_IMAGE_SIZE, true);
}

//420
extern "C" void glBlendFuncSeparate(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha){
	pushOp(420);
	pushParam(sfactorRGB);
	pushParam(dfactorRGB);
	pushParam(sfactorAlpha);
	pushParam(dfactorAlpha);
}

//421
extern "C" void glFogCoordf(GLfloat coord){
	pushOp(421);
	pushParam(coord);
}

//422
extern "C" void glFogCoordfv(const GLfloat * coord){
	LOG("Called untested stub FogCoordfv!\n");
	pushOp(421);
	pushBuf(coord, sizeof(GLfloat));
}

//423
extern "C" void glFogCoordd(GLdouble coord){
	pushOp(423);
	pushParam(coord);
}

//424
extern "C" void glFogCoorddv(const GLdouble * coord){
	LOG("Called untested stub FogCoorddv!\n");
	pushOp(424);
	pushBuf(coord, sizeof(GLdouble));
}

//425
extern "C" void glFogCoordPointer(GLenum type, GLsizei stride, const GLvoid * pointer){
	LOG("Called unimplemted stub FogCoordPointer!\n");
}

//426
extern "C" void glMultiDrawArrays(GLenum mode, GLint * first, GLsizei * count, GLsizei primcount){
	LOG("Called untested stub MultiDrawArrays!\n");
	pushOp(426);
	pushParam(mode);
	pushBuf(first, sizeof(GLint) * primcount);
	pushBuf(count, sizeof(GLsizei) * primcount);
	pushParam(primcount);
}

//427
extern "C" void glMultiDrawElements(GLenum mode, const GLsizei * count, GLenum type, const GLvoid ** indices, GLsizei primcount){
	LOG("Called unimplemted stub MultiDrawElements!\n");
	//pushOp(427);
	//pushParam(mode);
	//pushBuf(count, sizeof(GLint) * primcount);
	//pushParam(type);
	//pushBuf(indices, /*each value of count * type size*/); 
	//pushParam(primcount);
}

//428
extern "C" void glPointParameterf(GLenum pname, GLfloat param){
	pushOp(428);
	pushParam(pname);
	pushParam(param);
}

//429
extern "C" void glPointParameterfv(GLenum pname, const GLfloat * params){
	LOG("Called unimplemted stub PointParameterfv!\n");
}

//430
extern "C" void glPointParameteri(GLenum pname, GLint param){
	pushOp(430);
	pushParam(pname);
	pushParam(param);
}

//431
extern "C" void glPointParameteriv(GLenum pname, const GLint * params){
	LOG("Called unimplemted stub PointParameteriv!\n");
}

//432
extern "C" void glSecondaryColor3b(GLbyte red, GLbyte green, GLbyte blue){
	pushOp(432);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//433
extern "C" void glSecondaryColor3bv(const GLbyte * v){
	pushOp(433);
	pushBuf(v, sizeof(const GLbyte) * 3);
}

//434
extern "C" void glSecondaryColor3d(GLdouble red, GLdouble green, GLdouble blue){
	pushOp(434);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//435
extern "C" void glSecondaryColor3dv(const GLdouble * v){
	pushOp(435);
	pushBuf(v, sizeof(const GLdouble) * 3);
}

//436
extern "C" void glSecondaryColor3f(GLfloat red, GLfloat green, GLfloat blue){
	pushOp(436);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//437
extern "C" void glSecondaryColor3fv(const GLfloat * v){
	pushOp(437);
	pushBuf(v, sizeof(const GLfloat) * 3);
}

//438
extern "C" void glSecondaryColor3i(GLint red, GLint green, GLint blue){
	pushOp(438);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//439
extern "C" void glSecondaryColor3iv(const GLint * v){
	pushOp(439);
	pushBuf(v, sizeof(const GLint) * 3);
}

//440
extern "C" void glSecondaryColor3s(GLshort red, GLshort green, GLshort blue){
	pushOp(440);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//441
extern "C" void glSecondaryColor3sv(const GLshort * v){
	pushOp(441);
	pushBuf(v, sizeof(const GLshort) * 3);
}

//442
extern "C" void glSecondaryColor3ub(GLubyte red, GLubyte green, GLubyte blue){
	pushOp(442);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//443
extern "C" void glSecondaryColor3ubv(const GLubyte * v){
	pushOp(443);
	pushBuf(v, sizeof(const GLubyte) * 3);
}

//444
extern "C" void glSecondaryColor3ui(GLuint red, GLuint green, GLuint blue){
	pushOp(444);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//445
extern "C" void glSecondaryColor3uiv(const GLuint * v){
	pushOp(445);
	pushBuf(v, sizeof(const GLuint) * 3);
}

//446
extern "C" void glSecondaryColor3us(GLushort red, GLushort green, GLushort blue){
	pushOp(446);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//447
extern "C" void glSecondaryColor3usv(const GLushort * v){
	pushOp(447);
	pushBuf(v, sizeof(const GLushort) * 3);
}

//448
extern "C" void glSecondaryColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer){
	LOG("Called untested stub SecondaryColorPointer!\n");
	pushOp(448);
	pushParam(size);
	pushParam(type);
	pushParam(stride);
	pushBuf(pointer, getTypeSize(type) * (size + stride));
}

//449
extern "C" void glWindowPos2d(GLdouble x, GLdouble y){
	pushOp(449);
	pushParam(x);
	pushParam(y);
}

//450
extern "C" void glWindowPos2dv(const GLdouble * v){
	pushOp(450);
	pushBuf(v, sizeof(const GLdouble) * 2);
}

//451
extern "C" void glWindowPos2f(GLfloat x, GLfloat y){
	pushOp(451);
	pushParam(x);
	pushParam(y);
}

//452
extern "C" void glWindowPos2fv(const GLfloat * v){
	pushOp(452);
	pushBuf(v, sizeof(const GLfloat) * 2);
}

//453
extern "C" void glWindowPos2i(GLint x, GLint y){
	pushOp(453);
	pushParam(x);
	pushParam(y);
}

//454
extern "C" void glWindowPos2iv(const GLint * v){
	pushOp(454);
	pushBuf(v, sizeof(const GLint) * 2);
}

//455
extern "C" void glWindowPos2s(GLshort x, GLshort y){
	pushOp(455);
	pushParam(x);
	pushParam(y);
}

//456
extern "C" void glWindowPos2sv(const GLshort * v){
	pushOp(456);
	pushBuf(v, sizeof(const GLshort) * 2);
}

//457
extern "C" void glWindowPos3d(GLdouble x, GLdouble y, GLdouble z){
	pushOp(457);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//458
extern "C" void glWindowPos3dv(const GLdouble * v){
	pushOp(458);
	pushBuf(v, sizeof(const GLdouble) * 3);
}

//459
extern "C" void glWindowPos3f(GLfloat x, GLfloat y, GLfloat z){
	pushOp(459);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//460
extern "C" void glWindowPos3fv(const GLfloat * v){
	pushOp(460);
	pushBuf(v, sizeof(const GLfloat) * 3);
}

//461
extern "C" void glWindowPos3i(GLint x, GLint y, GLint z){
	pushOp(461);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//462
extern "C" void glWindowPos3iv(const GLint * v){
	pushOp(462);
	pushBuf(v, sizeof(const GLint) * 3);
}

//463
extern "C" void glWindowPos3s(GLshort x, GLshort y, GLshort z){
	pushOp(463);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//464
extern "C" void glWindowPos3sv(const GLshort * v){
	pushOp(464);
	pushBuf(v, sizeof(const GLshort) * 3);
}

//465
extern "C" void glBindBuffer(GLenum target, GLuint buffer){
	pushOp(465);
	pushParam(target);
	pushParam(buffer);
}

//466 BRADEN
extern "C" void glBufferData(GLenum target, GLsizeiptr size, const GLvoid * data, GLenum usage){
	LOG("Called untested stub BufferData!\n");
	pushOp(466);
	pushParam(target);
	pushParam((GLsizei) size);
	pushBuf(data, size);
	pushParam(usage);
}

//467
extern "C" void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid * data){
	LOG("Called untested stub BufferSubData!\n");
	pushOp(466);
	pushParam(target);
	pushParam((GLint) offset);
	pushParam((GLsizei) size);
	pushBuf(data, size);
}

//468
extern "C" void glDeleteBuffers(GLsizei n, const GLuint * buffer){
	LOG("Called untested stub DeleteBuffers!\n");
	pushOp(468);
	pushParam(n);
	pushBuf(buffer, sizeof(const GLuint) * n);
}

//469
extern "C" void glGenBuffers(GLsizei n, GLuint * buffer){
	pushOp(469);
	pushParam(n);
	pushBuf(buffer, sizeof(GLuint) * n);
}

//470
extern "C" void glGetBufferParameteriv(GLenum target, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetBufferParameteriv!\n");
}

//471
extern "C" void glGetBufferPointerv(GLenum target, GLenum pname, GLvoid ** params){
	LOG("Called unimplemted stub GetBufferPointerv!\n");
}

//472
extern "C" void glGetBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, GLvoid * data){
	LOG("Called untested stub GetBufferSubData!\n");
	pushOp(472);
	pushParam(target);
	pushParam((GLint) offset);		//dont think thats right
	pushParam((GLint) size);
	pushBuf(data, sizeof(GLbyte) * size);
}

//473
extern "C" GLboolean glIsBuffer(GLuint buffer){
	pushOp(473);
	pushParam(buffer);

	GLboolean ret;
	pushBuf(&ret, sizeof(GLboolean), true);
	waitForReturn();

	return ret;
}

//474
extern "C" GLvoid * glMapBuffer(GLenum target, GLenum access){
	pushOp(474);
	pushParam(target);
	pushParam(access);

	GLvoid * ret;
	pushBuf(&ret, sizeof(GLvoid *), true);
	waitForReturn();

	return ret;
}

//475
extern "C" GLboolean glUnmapBuffer(GLenum target){
	pushOp(475);
	pushParam(target);

	GLboolean ret;
	pushBuf(&ret, sizeof(GLboolean), true);
	waitForReturn();

	return ret;
}

//476
extern "C" void glGenQueries(GLsizei n, GLuint * ids){
	pushOp(476);
	pushParam(n);
	pushBuf(ids, sizeof(GLuint) * n);
}

//477
extern "C" void glDeleteQueries(GLsizei n, const GLuint * ids){
	LOG("Called untested stub DeleteQueries!\n");
	pushOp(477);
	pushParam(n);
	pushBuf(ids, sizeof(GLuint) * n);
}

//478
extern "C" GLboolean glIsQuery(GLuint id){
	pushOp(478);
	pushParam(id);

	GLboolean ret;
	pushBuf(&ret, sizeof(GLboolean), true);
	waitForReturn();

	return ret;
}

//479
extern "C" void glBeginQuery(GLenum target, GLuint id){
	pushOp(479);
	pushParam(target);
	pushParam(id);
}

//480
extern "C" void glEndQuery(GLenum target){
	pushOp(480);
	pushParam(target);
}

//481
extern "C" void glGetQueryiv(GLenum target, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetQueryiv!\n");
}

//482
extern "C" void glGetQueryObjectiv(GLuint id, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetQueryObjectiv!\n");
}

//483
extern "C" void glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint * params){
	LOG("Called unimplemted stub GetQueryObjectuiv!\n");
}

//484
extern "C" void glBlendEquationSeparate(GLenum modeRGB, GLenum modeA){
	pushOp(484);
	pushParam(modeRGB);
	pushParam(modeA);
}

//485
extern "C" void glDrawBuffers(GLsizei n, const GLenum * bufs){
	pushOp(485);
	pushParam(n);
	pushBuf(bufs, sizeof(const GLenum) * n);
}

//486
extern "C" void glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask){
	pushOp(486);
	pushParam(face);
	pushParam(func);
	pushParam(ref);
	pushParam(mask);
}

//487
extern "C" void glStencilOpSeparate(GLenum face, GLenum sfail, GLenum zfail, GLenum zpass){
	pushOp(487);
	pushParam(face);
	pushParam(sfail);
	pushParam(zfail);
	pushParam(zpass);
}

//488
extern "C" void glStencilMaskSeparate(GLenum face, GLuint mask){
	pushOp(488);
	pushParam(face);
	pushParam(mask);
}

//489
extern "C" void glAttachShader(GLuint program, GLuint shader){
	pushOp(489);
	pushParam(program);
	pushParam(shader);
}

//490
extern "C" void glBindAttribLocation(GLuint program, GLuint index, const GLchar * name){
	LOG("Called untested stub BindAttribLocation!\n");
	pushOp(490);
	pushParam(program);
	pushParam(index);
	pushBuf(name, sizeof(GLchar) * strlen(name));
}

//491
extern "C" void glCompileShader(GLuint shader){
	pushOp(491);
	pushParam(shader);
}

//492
extern "C" GLuint glCreateProgram(){
	pushOp(492);
	GLuint ret;
	pushBuf(&ret, sizeof(GLuint), true);
	waitForReturn();

	return ret;
}

//493
extern "C" GLuint glCreateShader(GLenum type){
	pushOp(493);
	pushParam(type);

	GLuint ret;
	pushBuf(&ret, sizeof(GLuint), true);
	waitForReturn();

	return ret;

}

//494
extern "C" void glDeleteProgram(GLuint program){
	pushOp(494);
	pushParam(program);
}

//495
extern "C" void glDeleteShader(GLuint program){
	pushOp(495);
	pushParam(program);
}

//496
extern "C" void glDetachShader(GLuint program, GLuint shader){
	pushOp(496);
	pushParam(program);
	pushParam(shader);
}

//497
extern "C" void glDisableVertexAttribArray(GLuint index){
	pushOp(497);
	pushParam(index);
}

//498
extern "C" void glEnableVertexAttribArray(GLuint index){
	pushOp(498);
	pushParam(index);
}

//499
extern "C" void glGetActiveAttrib(GLuint program, GLuint index, GLsizei  bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name){
	LOG("Called unimplemted stub GetActiveAttrib!\n");
}

//500
extern "C" void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLchar * name){
	LOG("Called unimplemted stub GetActiveUniform!\n");
}

//501
extern "C" void glGetAttachedShaders(GLuint program, GLsizei maxCount, GLsizei * count, GLuint * obj){
	LOG("Called unimplemted stub GetAttachedShaders!\n");
}

//502
extern "C" GLint glGetAttribLocation(GLuint program, const GLchar * name){
	LOG("Called unimplemted stub GetAttribLocation!\n");
}

//503
extern "C" void glGetProgramiv(GLuint program, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetProgramiv!\n");
}

//504
extern "C" void glGetProgramInfoLog(GLuint program, GLsizei bufSize, GLsizei * length, GLchar * infoLog){
	LOG("Called unimplemted stub GetProgramInfoLog!\n");
}

//505
extern "C" void glGetShaderiv(GLuint shader, GLenum pname, GLint * params){
	pushOp(505);
	pushParam(shader);
	pushParam(pname);
	pushBuf(params, sizeof(GLuint), true);
	waitForReturn();
}

//506
extern "C" void glGetShaderInfoLog(GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * infoLog){
	pushOp(506);
	pushParam(shader);
	pushParam(bufSize);
	if(!length)
	{
	//push a -1 length, rather than a NULL object that crashes
	pushParam((GLint) -1);
	}
	else
	{
	pushParam(*length);
	}
	pushBuf(infoLog, sizeof(GLchar) * bufSize, true);
	waitForReturn();
}

//507
extern "C" void glGetShaderSource(GLuint shader, GLsizei bufSize, GLsizei * length, GLchar * source){
	LOG("Called untested stub GetShaderSource!\n");
	pushOp(507);
	pushParam(shader);
	pushParam(bufSize);
	if(!length)
	{
	//push a -1 length, rather than a NULL object that crashes
	pushParam((GLint) -1);
	}
	else
	{
	pushParam(*length);
	}
	pushBuf(source, sizeof(GLchar) * bufSize, true);
	waitForReturn();
}

//508
extern "C" GLint glGetUniformLocation(GLuint program, const GLchar * name){
	LOG("Called unimplemted stub GetUniformLocation!\n");
}

//509
extern "C" void glGetUniformfv(GLuint program, GLint location, GLfloat * params){
	LOG("Called unimplemted stub GetUniformfv!\n");
}

//510
extern "C" void glGetUniformiv(GLuint program, GLint location, GLint * params){
	LOG("Called unimplemted stub GetUniformiv!\n");
}

//511
extern "C" void glGetVertexAttribdv(GLuint index, GLenum pname, GLdouble * params){
	LOG("Called unimplemted stub GetVertexAttribdv!\n");
}

//512
extern "C" void glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetVertexAttribfv!\n");
}

//513
extern "C" void glGetVertexAttribiv(GLuint index, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetVertexAttribiv!\n");
}

//514
extern "C" void glGetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid ** pointer){
	LOG("Called unimplemted stub GetVertexAttribPointerv!\n");
}

//515
extern "C" GLboolean glIsProgram(GLuint program){
	pushOp(515);
	pushParam(program);

	GLboolean ret;
	pushBuf(&ret, sizeof(GLboolean), true);
	waitForReturn();

	return ret;
}

//516
extern "C" GLboolean glIsShader(GLuint shader){
	pushOp(516);
	pushParam(shader);

	GLboolean ret;
	pushBuf(&ret, sizeof(GLboolean), true);
	waitForReturn();

	return ret;
}

//517
extern "C" void glLinkProgram(GLuint program){
	pushOp(517);
	pushParam(program);
}

//518
extern "C" void glShaderSource(GLuint shader, GLsizei count, const GLchar ** string, const GLint * length){
	pushOp(518);
	pushParam(shader);
	pushParam(count);
	int size = 0;
	for(int i =0; i < count; i++)
	{
	size += strlen((const GLchar *)string[i]);
	}
	GLchar * stringBuf = (GLchar *) malloc(sizeof(const GLchar *) * size);
	stringBuf = strcpy(stringBuf, string[0]);
	stringBuf = strcat(stringBuf, "\n");
	for(int i =1; i < count; i++)
	{
	stringBuf = strcat(stringBuf, string[i]);
	stringBuf = strcat(stringBuf, "\n");
	}
	pushBuf(stringBuf, sizeof(const GLchar *) * size);
	if(!length)
	{
	//push a -1 length, rather than a NULL object that crashes
	pushParam((GLint) -1);
	}
	else
	{
	pushParam(*length);
	}
}

//519
extern "C" void glUseProgram(GLuint program){
	pushOp(519);
	pushParam(program);
}

//520
extern "C" void glUniform1f(GLint location, GLfloat v0){
	pushOp(520);
	pushParam(location);
	pushParam(v0);
}

//521
extern "C" void glUniform2f(GLint location, GLfloat v0, GLfloat v1){
	pushOp(521);
	pushParam(location);
	pushParam(v0);
	pushParam(v1);
}

//522
extern "C" void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2){
	pushOp(522);
	pushParam(location);
	pushParam(v0);
	pushParam(v1);
	pushParam(v2);
}

//523
extern "C" void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3){
	pushOp(523);
	pushParam(location);
	pushParam(v0);
	pushParam(v1);
	pushParam(v2);
	pushParam(v3);
}

//524
extern "C" void glUniform1i(GLint location, GLint v0){
	pushOp(524);
	pushParam(location);
	pushParam(v0);
}

//525
extern "C" void glUniform2i(GLint location, GLint v0, GLint v1){
	pushOp(525);
	pushParam(location);
	pushParam(v0);
	pushParam(v1);
}

//526
extern "C" void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2){
	pushOp(526);
	pushParam(location);
	pushParam(v0);
	pushParam(v1);
	pushParam(v2);
}

//527
extern "C" void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3){
	pushOp(527);
	pushParam(location);
	pushParam(v0);
	pushParam(v1);
	pushParam(v2);
	pushParam(v3);
}

//528
extern "C" void glUniform1fv(GLint location, GLsizei count, const GLfloat * value){
	LOG("Called untested stub Uniform1fv!\n");
	pushOp(528);
	pushParam(location);
	pushParam(count);
	pushBuf(value, sizeof(GLfloat) * count);
}

//529
extern "C" void glUniform2fv(GLint location, GLsizei count, const GLfloat * value){
	LOG("Called untested stub Uniform2fv!\n");
	pushOp(529);
	pushParam(location);
	pushParam(count);
	pushBuf(value, sizeof(GLfloat) * count * 2);
}

//530
extern "C" void glUniform3fv(GLint location, GLsizei count, const GLfloat * value){
	LOG("Called untested stub Uniform3fv!\n");
	pushOp(530);
	pushParam(location);
	pushParam(count);
	pushBuf(value, sizeof(GLfloat) * count * 3);
}

//531
extern "C" void glUniform4fv(GLint location, GLsizei count, const GLfloat * value){
	LOG("Called untested stub Uniform4fv!\n");
	pushOp(531);
	pushParam(location);
	pushParam(count);
	pushBuf(value, sizeof(GLfloat) * count * 4);
}

//532
extern "C" void glUniform1iv(GLint location, GLsizei count, const GLint * value){
	LOG("Called untested stub Uniform1iv!\n");
	pushOp(532);
	pushParam(location);
	pushParam(count);
	pushBuf(value, sizeof(GLint) * count);
}

//533
extern "C" void glUniform2iv(GLint location, GLsizei count, const GLint * value){
	LOG("Called untested stub Uniform2iv!\n");
	pushOp(533);
	pushParam(location);
	pushParam(count);
	pushBuf(value, sizeof(GLint) * count * 2);
}

//534
extern "C" void glUniform3iv(GLint location, GLsizei count, const GLint * value){
	LOG("Called untested stub Uniform3iv!\n");
	pushOp(534);
	pushParam(location);
	pushParam(count);
	pushBuf(value, sizeof(GLint) * count * 3);
}

//535
extern "C" void glUniform4iv(GLint location, GLsizei count, const GLint * value){
	LOG("Called untested stub Uniform4iv!\n");
	pushOp(532);
	pushParam(location);
	pushParam(count);
	pushBuf(value, sizeof(GLint) * count * 4);
}

//536
extern "C" void glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value){
	LOG("Called untested stub UniformMatrix2fv!\n");
	pushOp(536);
	pushParam(location);
	pushParam(count);
	pushParam(transpose);
	pushBuf(value, sizeof(GLint) * count * 2);
}

//537
extern "C" void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value){
	LOG("Called untested stub UniformMatrix3fv!\n");
	pushOp(537);
	pushParam(location);
	pushParam(count);
	pushParam(transpose);
	pushBuf(value, sizeof(GLint) * count * 3);
}

//538
extern "C" void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value){
	LOG("Called untested stub UniformMatrix4fv!\n");
	pushOp(538);
	pushParam(location);
	pushParam(count);
	pushParam(transpose);
	pushBuf(value, sizeof(GLint) * count * 4);
}

//539
extern "C" void glValidateProgram(GLuint program){
	pushOp(539);
	pushParam(program);
}

//540
extern "C" void glVertexAttrib1d(GLuint index, GLdouble x){
	pushOp(540);
	pushParam(index);
	pushParam(x);
}

//541
extern "C" void glVertexAttrib1dv(GLuint index, const GLdouble * v){
	pushOp(541);
	pushParam(index);
	pushBuf(v, sizeof(const GLdouble) * 1);
}

//542
extern "C" void glVertexAttrib1f(GLuint index, GLfloat x){
	pushOp(542);
	pushParam(index);
	pushParam(x);
}

//543
extern "C" void glVertexAttrib1fv(GLuint index, const GLfloat * v){
	pushOp(543);
	pushParam(index);
	pushBuf(v, sizeof(const GLfloat) * 1);
}

//544
extern "C" void glVertexAttrib1s(GLuint index, GLshort x){
	pushOp(544);
	pushParam(index);
	pushParam(x);
}

//545
extern "C" void glVertexAttrib1sv(GLuint index, const GLshort * v){
	pushOp(545);
	pushParam(index);
	pushBuf(v, sizeof(const GLshort) * 1);
}

//546
extern "C" void glVertexAttrib2d(GLuint index, GLdouble x, GLdouble y){
	pushOp(546);
	pushParam(index);
	pushParam(x);
	pushParam(y);
}

//547
extern "C" void glVertexAttrib2dv(GLuint index, const GLdouble * v){
	pushOp(547);
	pushParam(index);
	pushBuf(v, sizeof(const GLdouble) * 2);
}

//548
extern "C" void glVertexAttrib2f(GLuint index, GLfloat x, GLfloat y){
	pushOp(548);
	pushParam(index);
	pushParam(x);
	pushParam(y);
}

//549
extern "C" void glVertexAttrib2fv(GLuint index, const GLfloat * v){
	pushOp(549);
	pushParam(index);
	pushBuf(v, sizeof(const GLfloat) * 2);
}

//550
extern "C" void glVertexAttrib2s(GLuint index, GLshort x, GLshort y){
	pushOp(550);
	pushParam(index);
	pushParam(x);
	pushParam(y);
}

//551
extern "C" void glVertexAttrib2sv(GLuint index, const GLshort * v){
	pushOp(551);
	pushParam(index);
	pushBuf(v, sizeof(const GLshort) * 2);
}

//552
extern "C" void glVertexAttrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z){
	pushOp(552);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//553
extern "C" void glVertexAttrib3dv(GLuint index, const GLdouble * v){
	pushOp(553);
	pushParam(index);
	pushBuf(v, sizeof(const GLdouble) * 3);
}

//554
extern "C" void glVertexAttrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z){
	pushOp(554);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//555
extern "C" void glVertexAttrib3fv(GLuint index, const GLfloat * v){
	pushOp(555);
	pushParam(index);
	pushBuf(v, sizeof(const GLfloat) * 3);
}

//556
extern "C" void glVertexAttrib3s(GLuint index, GLshort x, GLshort y, GLshort z){
	pushOp(556);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//557
extern "C" void glVertexAttrib3sv(GLuint index, const GLshort * v){
	pushOp(557);
	pushParam(index);
	pushBuf(v, sizeof(const GLshort) * 3);
}

//558
extern "C" void glVertexAttrib4Nbv(GLuint index, const GLbyte * v){
	pushOp(558);
	pushParam(index);
	pushBuf(v, sizeof(const GLbyte) * 4);
}

//559
extern "C" void glVertexAttrib4Niv(GLuint index, const GLint * v){
	pushOp(559);
	pushParam(index);
	pushBuf(v, sizeof(const GLint) * 4);
}

//560
extern "C" void glVertexAttrib4Nsv(GLuint index, const GLshort * v){
	pushOp(560);
	pushParam(index);
	pushBuf(v, sizeof(const GLshort) * 4);
}

//561
extern "C" void glVertexAttrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w){
	pushOp(561);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//562
extern "C" void glVertexAttrib4Nubv(GLuint index, const GLubyte * v){
	pushOp(562);
	pushParam(index);
	pushBuf(v, sizeof(const GLubyte) * 4);
}

//563
extern "C" void glVertexAttrib4Nuiv(GLuint index, const GLuint * v){
	pushOp(563);
	pushParam(index);
	pushBuf(v, sizeof(const GLuint) * 4);
}

//564
extern "C" void glVertexAttrib4Nusv(GLuint index, const GLushort * v){
	pushOp(564);
	pushParam(index);
	pushBuf(v, sizeof(const GLushort) * 4);
}

//565
extern "C" void glVertexAttrib4bv(GLuint index, const GLbyte * v){
	pushOp(565);
	pushParam(index);
	pushBuf(v, sizeof(const GLbyte) * 4);
}

//566
extern "C" void glVertexAttrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w){
	pushOp(566);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//567
extern "C" void glVertexAttrib4dv(GLuint index, const GLdouble * v){
	pushOp(567);
	pushParam(index);
	pushBuf(v, sizeof(const GLdouble) * 4);
}

//568
extern "C" void glVertexAttrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w){
	pushOp(568);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//569
extern "C" void glVertexAttrib4fv(GLuint index, const GLfloat * v){
	pushOp(569);
	pushParam(index);
	pushBuf(v, sizeof(const GLfloat) * 4);
}

//570
extern "C" void glVertexAttrib4iv(GLuint index, const GLint * v){
	pushOp(570);
	pushParam(index);
	pushBuf(v, sizeof(const GLint) * 4);
}

//571
extern "C" void glVertexAttrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w){
	pushOp(571);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//572
extern "C" void glVertexAttrib4sv(GLuint index, const GLshort * v){
	pushOp(572);
	pushParam(index);
	pushBuf(v, sizeof(const GLshort) * 4);
}

//573
extern "C" void glVertexAttrib4ubv(GLuint index, const GLubyte * v){
	pushOp(573);
	pushParam(index);
	pushBuf(v, sizeof(const GLubyte) * 4);
}

//574
extern "C" void glVertexAttrib4uiv(GLuint index, const GLuint * v){
	pushOp(574);
	pushParam(index);
	pushBuf(v, sizeof(const GLuint) * 4);
}

//575
extern "C" void glVertexAttrib4usv(GLuint index, const GLushort * v){
	pushOp(575);
	pushParam(index);
	pushBuf(v, sizeof(const GLushort) * 4);
}

//576
extern "C" void glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer){
	LOG("Called untested stub VertexAttribPointer!\n");
	pushOp(575);
	pushParam(index);
	pushParam(size);
	pushParam(type);
	pushParam(normalized);
	pushParam(stride);
	pushBuf(pointer, getTypeSize(type) * size * (index + stride));
}

//577
extern "C" void glUniformMatrix2x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value){
	LOG("Called untested stub UniformMatrix2x3fv!\n");
	pushOp(577);
	pushParam(location);
	pushParam(count);
	pushParam(transpose);
	pushBuf(value, sizeof(GLint) * count * 6); //2x3
}

//578
extern "C" void glUniformMatrix3x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value){
	LOG("Called untested stub UniformMatrix3x2fv!\n");
	pushOp(578);
	pushParam(location);
	pushParam(count);
	pushParam(transpose);
	pushBuf(value, sizeof(GLint) * count * 6); //3x2
}

//579
extern "C" void glUniformMatrix2x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value){
	LOG("Called untested stub UniformMatrix2x4fv!\n");
	pushOp(579);
	pushParam(location);
	pushParam(count);
	pushParam(transpose);
	pushBuf(value, sizeof(GLint) * count * 8); //2x4
}

//580
extern "C" void glUniformMatrix4x2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value){
	LOG("Called untested stub UniformMatrix4x2fv!\n");
	pushOp(580);
	pushParam(location);
	pushParam(count);
	pushParam(transpose);
	pushBuf(value, sizeof(GLint) * count * 8); //4x2
}

//581
extern "C" void glUniformMatrix3x4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value){
	LOG("Called untested stub UniformMatrix3x4fv!\n");
	pushOp(581);
	pushParam(location);
	pushParam(count);
	pushParam(transpose);
	pushBuf(value, sizeof(GLint) * count * 12); //3x4
}

//582
extern "C" void glUniformMatrix4x3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value){
	LOG("Called untested stub UniformMatrix4x3fv!\n");
	pushOp(582);
	pushParam(location);
	pushParam(count);
	pushParam(transpose);
	pushBuf(value, sizeof(GLint) * count * 12); //4x3
}

//374
extern "C" void glActiveTextureARB(GLenum texture){
	pushOp(374);
	pushParam(texture);
}

//375
extern "C" void glClientActiveTextureARB(GLenum texture){
	pushOp(375);
	pushParam(texture);
}

//376
extern "C" void glMultiTexCoord1dARB(GLenum target, GLdouble s){
	pushOp(376);
	pushParam(target);
	pushParam(s);
}

//377
extern "C" void glMultiTexCoord1dvARB(GLenum target, const GLdouble * v){
	pushOp(377);
	pushParam(target);
	pushBuf(v, sizeof(const GLdouble) * 1);
}

//378
extern "C" void glMultiTexCoord1fARB(GLenum target, GLfloat s){
	pushOp(378);
	pushParam(target);
	pushParam(s);
}

//379
extern "C" void glMultiTexCoord1fvARB(GLenum target, const GLfloat * v){
	pushOp(379);
	pushParam(target);
	pushBuf(v, sizeof(const GLfloat) * 1);
}

//380
extern "C" void glMultiTexCoord1iARB(GLenum target, GLint s){
	pushOp(380);
	pushParam(target);
	pushParam(s);
}

//381
extern "C" void glMultiTexCoord1ivARB(GLenum target, const GLint * v){
	pushOp(381);
	pushParam(target);
	pushBuf(v, sizeof(const GLint) * 1);
}

//382
extern "C" void glMultiTexCoord1sARB(GLenum target, GLshort s){
	pushOp(382);
	pushParam(target);
	pushParam(s);
}

//383
extern "C" void glMultiTexCoord1svARB(GLenum target, const GLshort * v){
	pushOp(383);
	pushParam(target);
	pushBuf(v, sizeof(const GLshort) * 1);
}

//384
extern "C" void glMultiTexCoord2dARB(GLenum target, GLdouble s, GLdouble t){
	pushOp(384);
	pushParam(target);
	pushParam(s);
	pushParam(t);
}

//385
extern "C" void glMultiTexCoord2dvARB(GLenum target, const GLdouble * v){
	pushOp(385);
	pushParam(target);
	pushBuf(v, sizeof(const GLdouble) * 2);
}

//386
extern "C" void glMultiTexCoord2fARB(GLenum target, GLfloat s, GLfloat t){
	pushOp(386);
	pushParam(target);
	pushParam(s);
	pushParam(t);
}

//387
extern "C" void glMultiTexCoord2fvARB(GLenum target, const GLfloat * v){
	pushOp(387);
	pushParam(target);
	pushBuf(v, sizeof(const GLfloat) * 2);
}

//388
extern "C" void glMultiTexCoord2iARB(GLenum target, GLint s, GLint t){
	pushOp(388);
	pushParam(target);
	pushParam(s);
	pushParam(t);
}

//389
extern "C" void glMultiTexCoord2ivARB(GLenum target, const GLint * v){
	pushOp(389);
	pushParam(target);
	pushBuf(v, sizeof(const GLint) * 2);
}

//390
extern "C" void glMultiTexCoord2sARB(GLenum target, GLshort s, GLshort t){
	pushOp(390);
	pushParam(target);
	pushParam(s);
	pushParam(t);
}

//391
extern "C" void glMultiTexCoord2svARB(GLenum target, const GLshort * v){
	pushOp(391);
	pushParam(target);
	pushBuf(v, sizeof(const GLshort) * 2);
}

//392
extern "C" void glMultiTexCoord3dARB(GLenum target, GLdouble s, GLdouble t, GLdouble r){
	pushOp(392);
	pushParam(target);
	pushParam(s);
	pushParam(t);
	pushParam(r);
}

//393
extern "C" void glMultiTexCoord3dvARB(GLenum target, const GLdouble * v){
	pushOp(393);
	pushParam(target);
	pushBuf(v, sizeof(const GLdouble) * 3);
}

//394
extern "C" void glMultiTexCoord3fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r){
	pushOp(394);
	pushParam(target);
	pushParam(s);
	pushParam(t);
	pushParam(r);
}

//395
extern "C" void glMultiTexCoord3fvARB(GLenum target, const GLfloat * v){
	pushOp(395);
	pushParam(target);
	pushBuf(v, sizeof(const GLfloat) * 3);
}

//396
extern "C" void glMultiTexCoord3iARB(GLenum target, GLint s, GLint t, GLint r){
	pushOp(396);
	pushParam(target);
	pushParam(s);
	pushParam(t);
	pushParam(r);
}

//397
extern "C" void glMultiTexCoord3ivARB(GLenum target, const GLint * v){
	pushOp(397);
	pushParam(target);
	pushBuf(v, sizeof(const GLint) * 3);
}

//398
extern "C" void glMultiTexCoord3sARB(GLenum target, GLshort s, GLshort t, GLshort r){
	pushOp(398);
	pushParam(target);
	pushParam(s);
	pushParam(t);
	pushParam(r);
}

//399
extern "C" void glMultiTexCoord3svARB(GLenum target, const GLshort * v){
	pushOp(399);
	pushParam(target);
	pushBuf(v, sizeof(const GLshort) * 3);
}

//400
extern "C" void glMultiTexCoord4dARB(GLenum target, GLdouble s, GLdouble t, GLdouble r, GLdouble q){
	pushOp(400);
	pushParam(target);
	pushParam(s);
	pushParam(t);
	pushParam(r);
	pushParam(q);
}

//401
extern "C" void glMultiTexCoord4dvARB(GLenum target, const GLdouble * v){
	pushOp(401);
	pushParam(target);
	pushBuf(v, sizeof(const GLdouble) * 4);
}

//402
extern "C" void glMultiTexCoord4fARB(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q){
	pushOp(402);
	pushParam(target);
	pushParam(s);
	pushParam(t);
	pushParam(r);
	pushParam(q);
}

//403
extern "C" void glMultiTexCoord4fvARB(GLenum target, const GLfloat * v){
	pushOp(403);
	pushParam(target);
	pushBuf(v, sizeof(const GLfloat) * 4);
}

//404
extern "C" void glMultiTexCoord4iARB(GLenum target, GLint s, GLint t, GLint r, GLint q){
	pushOp(404);
	pushParam(target);
	pushParam(s);
	pushParam(t);
	pushParam(r);
	pushParam(q);
}

//405
extern "C" void glMultiTexCoord4ivARB(GLenum target, const GLint * v){
	pushOp(405);
	pushParam(target);
	pushBuf(v, sizeof(const GLint) * 4);
}

//406
extern "C" void glMultiTexCoord4sARB(GLenum target, GLshort s, GLshort t, GLshort r, GLshort q){
	pushOp(406);
	pushParam(target);
	pushParam(s);
	pushParam(t);
	pushParam(r);
	pushParam(q);
}

//407
extern "C" void glMultiTexCoord4svARB(GLenum target, const GLshort * v){
	pushOp(407);
	pushParam(target);
	pushBuf(v, sizeof(const GLshort) * 4);
}

//617
extern "C" void glLoadTransposeMatrixfARB(const GLfloat * m){
	LOG("Called untested stub LoadTransposeMatrixfARB!\n");
	pushOp(617);
	pushBuf(m, sizeof(const GLfloat) * 16, true);		//4x4 matrix
	waitForReturn();
}

//618
extern "C" void glLoadTransposeMatrixdARB(const GLdouble * m){
	LOG("Called untested stub LoadTransposeMatrixdARB!\n");
	pushOp(618);
	pushBuf(m, sizeof(const GLdouble) * 16, true);		//4x4 matrix
	waitForReturn();
}

//619
extern "C" void glMultTransposeMatrixfARB(const GLfloat * m){
	LOG("Called untested stub MultTransposeMatrixfARB!\n");
	pushOp(619);
	pushBuf(m, sizeof(const GLfloat) * 16, true);		//4x4 matrix
	waitForReturn();
}

//620
extern "C" void glMultTransposeMatrixdARB(const GLdouble * m){
	LOG("Called untested stub MultTransposeMatrixdARB!\n");
	pushOp(620);
	pushBuf(m, sizeof(const GLdouble) * 16, true);		//4x4 matrix
	waitForReturn();
}

//621
extern "C" void glSampleCoverageARB(GLclampf value, GLboolean invert){
	pushOp(621);
	pushParam(value);
	pushParam(invert);
}

//622
extern "C" void glCompressedTexImage3DARB(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLsizei imageSize, const GLvoid * data){
	pushOp(622);
	pushParam(target);
	pushParam(level);
	pushParam(internalformat);
	pushParam(width);
	pushParam(height);
	pushParam(depth);
	pushParam(border);
	pushParam(imageSize);
	pushBuf(data, imageSize);
}

//623
extern "C" void glCompressedTexImage2DARB(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid * data){
	pushOp(623);
	pushParam(target);
	pushParam(level);
	pushParam(internalformat);
	pushParam(width);
	pushParam(height);
	pushParam(border);
	pushParam(imageSize);
	pushBuf(data, imageSize);
}

//624
extern "C" void glCompressedTexImage1DARB(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLint border, GLsizei imageSize, const GLvoid * data){
	pushOp(624);
	pushParam(target);
	pushParam(level);
	pushParam(internalformat);
	pushParam(width);
	pushParam(border);
	pushParam(imageSize);
	pushBuf(data, imageSize);
}

//625
extern "C" void glCompressedTexSubImage3DARB(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLsizei imageSize, const GLvoid * data){
	pushOp(625);
	pushParam(target);
	pushParam(level);
	pushParam(xoffset);
	pushParam(yoffset);
	pushParam(zoffset);
	pushParam(width);
	pushParam(height);
	pushParam(depth);
	pushParam(format);
	pushParam(imageSize);
	pushBuf(data, imageSize);
}

//626
extern "C" void glCompressedTexSubImage2DARB(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid * data){
	pushOp(626);
	pushParam(target);
	pushParam(level);
	pushParam(xoffset);
	pushParam(yoffset);
	pushParam(width);
	pushParam(height);
	pushParam(format);
	pushParam(imageSize);
	pushBuf(data, imageSize);
}

//627
extern "C" void glCompressedTexSubImage1DARB(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLsizei imageSize, const GLvoid * data){
	pushOp(627);
	pushParam(target);
	pushParam(level);
	pushParam(xoffset);
	pushParam(width);
	pushParam(format);
	pushParam(imageSize);
	pushBuf(data, imageSize);
}

//628
extern "C" void glGetCompressedTexImageARB(GLenum target, GLint level, GLvoid * img){
	LOG("Called unimplemted stub GetCompressedTexImageARB!\n");
}

//629
extern "C" void glPointParameterfARB(GLenum pname, GLfloat param){
	pushOp(629);
	pushParam(pname);
	pushParam(param);
}

//630
extern "C" void glPointParameterfvARB(GLenum pname, const GLfloat * params){
	LOG("Called unimplemted stub PointParameterfvARB!\n");
}

//631
extern "C" void glWeightbvARB(GLint size, const GLbyte * weights){
	LOG("Called untested stub WeightbvARB!\n");
	pushOp(631);
	pushParam(size);
	pushBuf(weights, sizeof(GLbyte) * size);
}

//632
extern "C" void glWeightsvARB(GLint size, const GLshort * weights){
	LOG("Called untested stub WeightsvARB!\n");
	pushOp(632);
	pushParam(size);
	pushBuf(weights, sizeof(GLshort) * size);
}

//633
extern "C" void glWeightivARB(GLint size, const GLint * weights){
	LOG("Called untested stub WeightivARB!\n");
	pushOp(633);
	pushParam(size);
	pushBuf(weights, sizeof(GLint) * size);
}

//634
extern "C" void glWeightfvARB(GLint size, const GLfloat * weights){
	LOG("Called untested stub WeightfvARB!\n");
	pushOp(634);
	pushParam(size);
	pushBuf(weights, sizeof(GLfloat) * size);
}

//635
extern "C" void glWeightdvARB(GLint size, const GLdouble * weights){
	LOG("Called untested stub WeightdvARB!\n");
	pushOp(635);
	pushParam(size);
	pushBuf(weights, sizeof(GLdouble) * size);
}

//636
extern "C" void glWeightubvARB(GLint size, const GLubyte * weights){
	LOG("Called untested stub WeightubvARB!\n");
	pushOp(636);
	pushParam(size);
	pushBuf(weights, sizeof(GLubyte) * size);
}

//637
extern "C" void glWeightusvARB(GLint size, const GLushort * weights){
	LOG("Called untested stub WeightusvARB!\n");
	pushOp(637);
	pushParam(size);
	pushBuf(weights, sizeof(GLushort) * size);
}

//638
extern "C" void glWeightuivARB(GLint size, const GLuint * weights){
	LOG("Called untested stub WeightuivARB!\n");
	pushOp(638);
	pushParam(size);
	pushBuf(weights, sizeof(GLuint) * size);
}

//639
extern "C" void glWeightPointerARB(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer){
	LOG("Called untested stub WeightPointerARB!\n");
	pushOp(639);
	pushParam(size);
	pushParam(type);
	pushParam(stride);
	pushBuf(pointer, getTypeSize(type) * (size + stride));
}

//640
extern "C" void glVertexBlendARB(GLint count){
	pushOp(640);
	pushParam(count);
}

//641
extern "C" void glCurrentPaletteMatrixARB(GLint index){
	pushOp(641);
	pushParam(index);
}

//642
extern "C" void glMatrixIndexubvARB(GLint size, const GLubyte * indices){
	pushOp(642);
	pushParam(size);
	pushBuf(indices, sizeof(const GLubyte) * size);
}

//643
extern "C" void glMatrixIndexusvARB(GLint size, const GLushort * indices){
	pushOp(643);
	pushParam(size);
	pushBuf(indices, sizeof(const GLushort) * size);
}

//644
extern "C" void glMatrixIndexuivARB(GLint size, const GLuint * indices){
	pushOp(644);
	pushParam(size);
	pushBuf(indices, sizeof(const GLuint) * size);
}

//645
extern "C" void glMatrixIndexPointerARB(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer){
	LOG("Called untested stub MatrixIndexPointerARB!\n");
	pushOp(645);
	pushParam(size);
	pushParam(type);
	pushParam(stride);
	pushBuf(pointer, getTypeSize(type) * (size + stride));
}

//646
extern "C" void glWindowPos2dARB(GLdouble x, GLdouble y){
	pushOp(646);
	pushParam(x);
	pushParam(y);
}

//647
extern "C" void glWindowPos2fARB(GLfloat x, GLfloat y){
	pushOp(647);
	pushParam(x);
	pushParam(y);
}

//648
extern "C" void glWindowPos2iARB(GLint x, GLint y){
	pushOp(648);
	pushParam(x);
	pushParam(y);
}

//649
extern "C" void glWindowPos2sARB(GLshort x, GLshort y){
	pushOp(649);
	pushParam(x);
	pushParam(y);
}

//650
extern "C" void glWindowPos2dvARB(const GLdouble * v){
	pushOp(650);
	pushBuf(v, sizeof(const GLdouble) * 2);
}

//651
extern "C" void glWindowPos2fvARB(const GLfloat * v){
	pushOp(651);
	pushBuf(v, sizeof(const GLfloat) * 2);
}

//652
extern "C" void glWindowPos2ivARB(const GLint * v){
	pushOp(652);
	pushBuf(v, sizeof(const GLint) * 2);
}

//653
extern "C" void glWindowPos2svARB(const GLshort * v){
	pushOp(653);
	pushBuf(v, sizeof(const GLshort) * 2);
}

//654
extern "C" void glWindowPos3dARB(GLdouble x, GLdouble y, GLdouble z){
	pushOp(654);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//655
extern "C" void glWindowPos3fARB(GLfloat x, GLfloat y, GLfloat z){
	pushOp(655);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//656
extern "C" void glWindowPos3iARB(GLint x, GLint y, GLint z){
	pushOp(656);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//657
extern "C" void glWindowPos3sARB(GLshort x, GLshort y, GLshort z){
	pushOp(657);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//658
extern "C" void glWindowPos3dvARB(const GLdouble * v){
	pushOp(658);
	pushBuf(v, sizeof(const GLdouble) * 3);
}

//659
extern "C" void glWindowPos3fvARB(const GLfloat * v){
	pushOp(659);
	pushBuf(v, sizeof(const GLfloat) * 3);
}

//660
extern "C" void glWindowPos3ivARB(const GLint * v){
	pushOp(660);
	pushBuf(v, sizeof(const GLint) * 3);
}

//661
extern "C" void glWindowPos3svARB(const GLshort * v){
	pushOp(661);
	pushBuf(v, sizeof(const GLshort) * 3);
}

//662
extern "C" void glGetVertexAttribdvARB(GLuint index, GLenum pname, GLdouble * params){
	LOG("Called unimplemted stub GetVertexAttribdvARB!\n");
}

//663
extern "C" void glGetVertexAttribfvARB(GLuint index, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetVertexAttribfvARB!\n");
}

//664
extern "C" void glGetVertexAttribivARB(GLuint index, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetVertexAttribivARB!\n");
}

//665
extern "C" void glVertexAttrib1dARB(GLuint index, GLdouble x){
	pushOp(665);
	pushParam(index);
	pushParam(x);
}

//666
extern "C" void glVertexAttrib1dvARB(GLuint index, const GLdouble * v){
	pushOp(666);
	pushParam(index);
	pushBuf(v, sizeof(const GLdouble) * 1);
}

//667
extern "C" void glVertexAttrib1fARB(GLuint index, GLfloat x){
	pushOp(667);
	pushParam(index);
	pushParam(x);
}

//668
extern "C" void glVertexAttrib1fvARB(GLuint index, const GLfloat * v){
	pushOp(668);
	pushParam(index);
	pushBuf(v, sizeof(const GLfloat) * 1);
}

//669
extern "C" void glVertexAttrib1sARB(GLuint index, GLshort x){
	pushOp(669);
	pushParam(index);
	pushParam(x);
}

//670
extern "C" void glVertexAttrib1svARB(GLuint index, const GLshort * v){
	pushOp(670);
	pushParam(index);
	pushBuf(v, sizeof(const GLshort) * 1);
}

//671
extern "C" void glVertexAttrib2dARB(GLuint index, GLdouble x, GLdouble y){
	pushOp(671);
	pushParam(index);
	pushParam(x);
	pushParam(y);
}

//672
extern "C" void glVertexAttrib2dvARB(GLuint index, const GLdouble * v){
	pushOp(672);
	pushParam(index);
	pushBuf(v, sizeof(const GLdouble) * 2);
}

//673
extern "C" void glVertexAttrib2fARB(GLuint index, GLfloat x, GLfloat y){
	pushOp(673);
	pushParam(index);
	pushParam(x);
	pushParam(y);
}

//674
extern "C" void glVertexAttrib2fvARB(GLuint index, const GLfloat * v){
	pushOp(674);
	pushParam(index);
	pushBuf(v, sizeof(const GLfloat) * 2);
}

//675
extern "C" void glVertexAttrib2sARB(GLuint index, GLshort x, GLshort y){
	pushOp(675);
	pushParam(index);
	pushParam(x);
	pushParam(y);
}

//676
extern "C" void glVertexAttrib2svARB(GLuint index, const GLshort * v){
	pushOp(676);
	pushParam(index);
	pushBuf(v, sizeof(const GLshort) * 2);
}

//677
extern "C" void glVertexAttrib3dARB(GLuint index, GLdouble x, GLdouble y, GLdouble z){
	pushOp(677);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//678
extern "C" void glVertexAttrib3dvARB(GLuint index, const GLdouble * v){
	pushOp(678);
	pushParam(index);
	pushBuf(v, sizeof(const GLdouble) * 3);
}

//679
extern "C" void glVertexAttrib3fARB(GLuint index, GLfloat x, GLfloat y, GLfloat z){
	pushOp(679);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//680
extern "C" void glVertexAttrib3fvARB(GLuint index, const GLfloat * v){
	pushOp(680);
	pushParam(index);
	pushBuf(v, sizeof(const GLfloat) * 3);
}

//681
extern "C" void glVertexAttrib3sARB(GLuint index, GLshort x, GLshort y, GLshort z){
	pushOp(681);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//682
extern "C" void glVertexAttrib3svARB(GLuint index, const GLshort * v){
	pushOp(682);
	pushParam(index);
	pushBuf(v, sizeof(const GLshort) * 3);
}

//683
extern "C" void glVertexAttrib4dARB(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w){
	pushOp(683);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//684
extern "C" void glVertexAttrib4dvARB(GLuint index, const GLdouble * v){
	pushOp(684);
	pushParam(index);
	pushBuf(v, sizeof(const GLdouble) * 4);
}

//685
extern "C" void glVertexAttrib4fARB(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w){
	pushOp(685);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//686
extern "C" void glVertexAttrib4fvARB(GLuint index, const GLfloat * v){
	pushOp(686);
	pushParam(index);
	pushBuf(v, sizeof(const GLfloat) * 4);
}

//687
extern "C" void glVertexAttrib4sARB(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w){
	pushOp(687);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//688
extern "C" void glVertexAttrib4svARB(GLuint index, const GLshort * v){
	pushOp(688);
	pushParam(index);
	pushBuf(v, sizeof(const GLshort) * 4);
}

//689
extern "C" void glVertexAttrib4NubARB(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w){
	pushOp(689);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//690
extern "C" void glVertexAttrib4NubvARB(GLuint index, const GLubyte * v){
	pushOp(690);
	pushParam(index);
	pushBuf(v, sizeof(const GLubyte) * 4);
}

//691
extern "C" void glVertexAttrib4bvARB(GLuint index, const GLbyte * v){
	pushOp(691);
	pushParam(index);
	pushBuf(v, sizeof(const GLbyte) * 4);
}

//692
extern "C" void glVertexAttrib4ivARB(GLuint index, const GLint * v){
	pushOp(692);
	pushParam(index);
	pushBuf(v, sizeof(const GLint) * 4);
}

//693
extern "C" void glVertexAttrib4ubvARB(GLuint index, const GLubyte * v){
	pushOp(693);
	pushParam(index);
	pushBuf(v, sizeof(const GLubyte) * 4);
}

//694
extern "C" void glVertexAttrib4usvARB(GLuint index, const GLushort * v){
	pushOp(694);
	pushParam(index);
	pushBuf(v, sizeof(const GLushort) * 4);
}

//695
extern "C" void glVertexAttrib4uivARB(GLuint index, const GLuint * v){
	pushOp(695);
	pushParam(index);
	pushBuf(v, sizeof(const GLuint) * 4);
}

//696
extern "C" void glVertexAttrib4NbvARB(GLuint index, const GLbyte * v){
	pushOp(696);
	pushParam(index);
	pushBuf(v, sizeof(const GLbyte) * 4);
}

//697
extern "C" void glVertexAttrib4NsvARB(GLuint index, const GLshort * v){
	pushOp(697);
	pushParam(index);
	pushBuf(v, sizeof(const GLshort) * 4);
}

//698
extern "C" void glVertexAttrib4NivARB(GLuint index, const GLint * v){
	pushOp(698);
	pushParam(index);
	pushBuf(v, sizeof(const GLint) * 4);
}

//699
extern "C" void glVertexAttrib4NusvARB(GLuint index, const GLushort * v){
	pushOp(699);
	pushParam(index);
	pushBuf(v, sizeof(const GLushort) * 4);
}

//700
extern "C" void glVertexAttrib4NuivARB(GLuint index, const GLuint * v){
	pushOp(700);
	pushParam(index);
	pushBuf(v, sizeof(const GLuint) * 4);
}

//701
extern "C" void glVertexAttribPointerARB(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid * pointer){
	LOG("Called untested stub VertexAttribPointerARB!\n");
	pushOp(701);
	pushParam(index);
	pushParam(size);
	pushParam(type);
	pushParam(normalized);
	pushParam(stride);
	pushBuf(pointer, getTypeSize(type) * (size + stride));

}

//702
extern "C" void glEnableVertexAttribArrayARB(GLuint index){
	pushOp(702);
	pushParam(index);
}

//703
extern "C" void glDisableVertexAttribArrayARB(GLuint index){
	pushOp(703);
	pushParam(index);
}

//704
extern "C" void glProgramStringARB(GLenum target, GLenum format, GLsizei len, const GLvoid * string){
	pushOp(704);
	pushParam(target);
	pushParam(format);
	pushParam(len);
	pushBuf(string, strlen((char *)string));
}

//705
extern "C" void glBindProgramARB(GLenum target, GLuint program){
	pushOp(705);
	pushParam(target);
	pushParam(program);
}

//706
extern "C" void glDeleteProgramsARB(GLsizei n, const GLuint * programs){
	LOG("Called untested stub DeleteProgramsARB!\n");
	pushOp(706);
	pushParam(n);
	pushBuf(programs, sizeof(const GLuint) * n);
}

//707
extern "C" void glGenProgramsARB(GLsizei n, GLuint * programs){
	pushOp(707);
	pushParam(n);
	pushBuf(programs, sizeof(GLuint) * n);
}

//708
extern "C" GLboolean glIsProgramARB(GLuint program){
	pushOp(708);
	pushParam(program);

	GLboolean ret;
	pushBuf(&ret, sizeof(GLboolean), true);
	waitForReturn();

	return ret;
}

//709
extern "C" void glProgramEnvParameter4dARB(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w){
	pushOp(709);
	pushParam(target);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//710
extern "C" void glProgramEnvParameter4dvARB(GLenum target, GLuint index, const GLdouble * params){
	pushOp(710);
	pushParam(target);
	pushParam(index);
	pushBuf(params, sizeof(const GLdouble) * 4);
}

//711
extern "C" void glProgramEnvParameter4fARB(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w){
	pushOp(711);
	pushParam(target);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//712
extern "C" void glProgramEnvParameter4fvARB(GLenum target, GLuint index, const GLfloat * params){
	pushOp(712);
	pushParam(target);
	pushParam(index);
	pushBuf(params, sizeof(const GLfloat) * 4);
}

//713
extern "C" void glProgramLocalParameter4dARB(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w){
	pushOp(713);
	pushParam(target);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//714
extern "C" void glProgramLocalParameter4dvARB(GLenum target, GLuint index, const GLdouble * params){
	pushOp(714);
	pushParam(target);
	pushParam(index);
	pushBuf(params, sizeof(const GLdouble) * 4);
}

//715
extern "C" void glProgramLocalParameter4fARB(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w){
	pushOp(715);
	pushParam(target);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//716
extern "C" void glProgramLocalParameter4fvARB(GLenum target, GLuint index, const GLfloat * params){
	pushOp(716);
	pushParam(target);
	pushParam(index);
	pushBuf(params, sizeof(const GLfloat) * 4);
}

//717
extern "C" void glGetProgramEnvParameterdvARB(GLenum target, GLuint index, GLdouble * params){
	pushOp(717);
	pushParam(target);
	pushParam(index);
	pushBuf(params, sizeof(GLdouble) * 4);
}

//718
extern "C" void glGetProgramEnvParameterfvARB(GLenum target, GLuint index, GLfloat * params){
	pushOp(718);
	pushParam(target);
	pushParam(index);
	pushBuf(params, sizeof(GLfloat) * 4);
}

//719
extern "C" void glGetProgramLocalParameterdvARB(GLenum target, GLuint index, GLdouble * params){
	pushOp(719);
	pushParam(target);
	pushParam(index);
	pushBuf(params, sizeof(GLdouble) * 4);
}

//720
extern "C" void glGetProgramLocalParameterfvARB(GLenum target, GLuint index, GLfloat * params){
	pushOp(720);
	pushParam(target);
	pushParam(index);
	pushBuf(params, sizeof(GLfloat) * 4);
}

//721
extern "C" void glGetProgramivARB(GLenum target, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetProgramivARB!\n");
}

//722
extern "C" void glGetProgramStringARB(GLenum target, GLenum pname, GLvoid * string){
	LOG("Called unimplemted stub GetProgramStringARB!\n");
}

//723
extern "C" void glGetVertexAttribPointervARB(GLuint index, GLenum pname, GLvoid ** pointer){
	LOG("Called unimplemted stub GetVertexAttribPointervARB!\n");
}

//724
extern "C" void glBindBufferARB(GLenum target, GLuint buffer){
	pushOp(724);
	pushParam(target);
	pushParam(buffer);
}

//725
extern "C" void glBufferDataARB(GLenum target, GLsizeiptrARB size, const GLvoid * data, GLenum usage){
	pushOp(725);
	pushParam(target);
#ifdef SYMPHONY
	pushParam((GLuint)size);
#else
	pushParam((GLuint)size);		/*compiler error */
#endif
	pushBuf(data, size);
	pushParam(usage);
}

//726
extern "C" void glBufferSubDataARB(GLenum target, GLintptrARB offset, GLsizeiptrARB size, const GLvoid * data){
	pushOp(726);
	pushParam(target);
#ifdef SYMPHONY
	pushParam((GLuint)offset);
	pushParam((GLuint)size);
#else
	pushParam((GLuint)offset);		/*compiler error */
	pushParam((GLuint)size);		/*compiler error */
#endif
	pushBuf(data, size);
}

//727
extern "C" void glDeleteBuffersARB(GLsizei n, const GLuint * buffer){
	pushOp(727);
	pushParam(n);
	pushBuf(buffer, sizeof(const GLuint) * n);
}

//728
extern "C" void glGenBuffersARB(GLsizei n, GLuint * buffer){
	pushOp(728);
	pushParam(n);
	pushBuf(buffer, sizeof(GLuint) * n);
}

//729
extern "C" void glGetBufferParameterivARB(GLenum target, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetBufferParameterivARB!\n");
}

//730
extern "C" void glGetBufferPointervARB(GLenum target, GLenum pname, GLvoid ** params){
	LOG("Called unimplemted stub GetBufferPointervARB!\n");
}

//731
extern "C" void glGetBufferSubDataARB(GLenum target, GLintptrARB offset, GLsizeiptrARB size, GLvoid * data){
	pushOp(731);
	pushParam(target);
#ifdef SYMPHONY
	pushParam((GLuint)offset);
	pushParam((GLuint)size);
#else
	pushParam((GLuint)offset);		/*compiler error */
	pushParam((GLuint)size);		/*compiler error */
#endif
	pushBuf(data, size);
}

//732
extern "C" GLboolean glIsBufferARB(GLuint buffer){
	pushOp(732);
	pushParam(buffer);

	GLboolean ret;
	pushBuf(&ret, sizeof(GLboolean), true);
	waitForReturn();

	return ret;
}

//733
extern "C" GLvoid * glMapBufferARB(GLenum target, GLenum access){
	pushOp(733);
	pushParam(target);
	pushParam(access);

	GLvoid * ret;
	pushBuf(&ret, sizeof(GLvoid *), true);
	waitForReturn();

	return ret;
}

//734
extern "C" GLboolean glUnmapBufferARB(GLenum target){
	pushOp(734);
	pushParam(target);

	GLboolean ret;
	pushBuf(&ret, sizeof(GLboolean), true);
	waitForReturn();

	return ret;
}

//735
extern "C" void glGenQueriesARB(GLsizei n, GLuint * ids){
	pushOp(735);
	pushParam(n);
	pushBuf(ids, sizeof(GLuint) * n);
}

//736
extern "C" void glDeleteQueriesARB(GLsizei n, const GLuint * ids){
	pushOp(736);
	pushParam(n);
	pushBuf(ids, sizeof(const GLuint) * n);
}

//737
extern "C" GLboolean glIsQueryARB(GLuint id){
	pushOp(737);
	pushParam(id);

	GLboolean ret;
	pushBuf(&ret, sizeof(GLboolean), true);
	waitForReturn();

	return ret;
}

//738
extern "C" void glBeginQueryARB(GLenum target, GLuint id){
	pushOp(738);
	pushParam(target);
	pushParam(id);
}

//739
extern "C" void glEndQueryARB(GLenum target){
	pushOp(739);
	pushParam(target);
}

//740
extern "C" void glGetQueryivARB(GLenum target, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetQueryivARB!\n");
}

//741
extern "C" void glGetQueryObjectivARB(GLuint id, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetQueryObjectivARB!\n");
}

//742
extern "C" void glGetQueryObjectuivARB(GLuint id, GLenum pname, GLuint * params){
	LOG("Called unimplemted stub GetQueryObjectuivARB!\n");
}

//743
extern "C" void glDeleteObjectARB(GLhandleARB obj){
	pushOp(743);
	pushParam(obj);
}

//744
extern "C" GLhandleARB glGetHandleARB(GLenum pname){
	pushOp(744);
	pushParam(pname);

	GLhandleARB ret;
	pushBuf(&ret, sizeof(GLhandleARB), true);
	waitForReturn();

	return ret;
}

//745
extern "C" void glDetachObjectARB(GLhandleARB containerObj, GLhandleARB attachedObj){
	pushOp(745);
	pushParam(containerObj);
	pushParam(attachedObj);
}

//746
extern "C" GLhandleARB glCreateShaderObjectARB(GLenum shaderType){
	pushOp(746);
	pushParam(shaderType);

	GLhandleARB ret;
	pushBuf(&ret, sizeof(GLhandleARB), true);
	waitForReturn();

	return ret;
}

//747
extern "C" void glShaderSourceARB(GLhandleARB shader, GLsizei count, const GLcharARB ** string, const GLint * length){
	LOG("Called untested stub ShaderSourceARB!\n");
	pushOp(747);
	pushParam(shader);
	pushParam(count);
	int size = 0;
	for(int i =0; i < count; i++)
	{
	size += strlen((const GLchar *)string[i]);
	}
	GLchar * stringBuf = (GLchar *) malloc(sizeof(const GLchar *) * size);
	stringBuf = strcpy(stringBuf, string[0]);
	stringBuf = strcat(stringBuf, "\n");
	for(int i =1; i < count; i++)
	{
	stringBuf = strcat(stringBuf, string[i]);
	stringBuf = strcat(stringBuf, "\n");
	}
	pushBuf(stringBuf, sizeof(const GLchar *) * size);
	if(!length)
	{
	//push a -1 length, rather than a NULL object that crashes
	pushParam((GLint) -1);
	}
	else
	{
	pushParam(*length);
	}
}

//748
extern "C" void glCompileShaderARB(GLhandleARB shader){
	pushOp(748);
	pushParam(shader);
}

//749
extern "C" GLhandleARB glCreateProgramObjectARB(){
	pushOp(749);

	GLhandleARB ret;
	pushBuf(&ret, sizeof(GLhandleARB), true);
	waitForReturn();

	return ret;
}

//750
extern "C" void glAttachObjectARB(GLhandleARB containerObj, GLhandleARB obj){
	pushOp(750);
	pushParam(containerObj);
	pushParam(obj);
}

//751
extern "C" void glLinkProgramARB(GLhandleARB program){
	pushOp(751);
	pushParam(program);
}

//752
extern "C" void glUseProgramObjectARB(GLhandleARB program){
	pushOp(752);
	pushParam(program);
}

//753
extern "C" void glValidateProgramARB(GLhandleARB program){
	pushOp(753);
	pushParam(program);
}

//754
extern "C" void glUniform1fARB(GLint location, GLfloat v0){
	pushOp(754);
	pushParam(location);
	pushParam(v0);
}

//755
extern "C" void glUniform2fARB(GLint location, GLfloat v0, GLfloat v1){
	pushOp(755);
	pushParam(location);
	pushParam(v0);
	pushParam(v1);
}

//756
extern "C" void glUniform3fARB(GLint location, GLfloat v0, GLfloat v1, GLfloat v2){
	pushOp(756);
	pushParam(location);
	pushParam(v0);
	pushParam(v1);
	pushParam(v2);
}

//757
extern "C" void glUniform4fARB(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3){
	pushOp(757);
	pushParam(location);
	pushParam(v0);
	pushParam(v1);
	pushParam(v2);
	pushParam(v3);
}

//758
extern "C" void glUniform1iARB(GLint location, GLint v0){
	pushOp(758);
	pushParam(location);
	pushParam(v0);
}

//759
extern "C" void glUniform2iARB(GLint location, GLint v0, GLint v1){
	pushOp(759);
	pushParam(location);
	pushParam(v0);
	pushParam(v1);
}

//760
extern "C" void glUniform3iARB(GLint location, GLint v0, GLint v1, GLint v2){
	pushOp(760);
	pushParam(location);
	pushParam(v0);
	pushParam(v1);
	pushParam(v2);
}

//761
extern "C" void glUniform4iARB(GLint location, GLint v0, GLint v1, GLint v2, GLint v3){
	pushOp(761);
	pushParam(location);
	pushParam(v0);
	pushParam(v1);
	pushParam(v2);
	pushParam(v3);
}

//762
extern "C" void glUniform1fvARB(GLint location, GLsizei count, const GLfloat * value){
	LOG("Called untested stub Uniform1fvARB!\n");
	pushOp(762);
	pushParam(location);
	pushParam(count);
	pushBuf(value, sizeof(GLfloat) * count);
}

//763
extern "C" void glUniform2fvARB(GLint location, GLsizei count, const GLfloat * value){
	LOG("Called untested stub Uniform2fvARB!\n");
	pushOp(763);
	pushParam(location);
	pushParam(count);
	pushBuf(value, sizeof(GLfloat) * count * 2);
}

//764
extern "C" void glUniform3fvARB(GLint location, GLsizei count, const GLfloat * value){
	LOG("Called untested stub Uniform3fvARB!\n");
	pushOp(764);
	pushParam(location);
	pushParam(count);
	pushBuf(value, sizeof(GLfloat) * count * 3);
}

//765
extern "C" void glUniform4fvARB(GLint location, GLsizei count, const GLfloat * value){
	LOG("Called untested stub Uniform4fvARB!\n");
	pushOp(765);
	pushParam(location);
	pushParam(count);
	pushBuf(value, sizeof(GLfloat) * count * 4);
}

//766
extern "C" void glUniform1ivARB(GLint location, GLsizei count, const GLint * value){
	LOG("Called untested stub Uniform1ivARB!\n");
	pushOp(766);
	pushParam(location);
	pushParam(count);
	pushBuf(value, sizeof(GLint) * count);
}

//767
extern "C" void glUniform2ivARB(GLint location, GLsizei count, const GLint * value){
	LOG("Called untested stub Uniform2ivARB!\n");
	pushOp(767);
	pushParam(location);
	pushParam(count);
	pushBuf(value, sizeof(GLint) * count * 2);
}

//768
extern "C" void glUniform3ivARB(GLint location, GLsizei count, const GLint * value){
	LOG("Called untested stub Uniform3ivARB!\n");
	pushOp(768);
	pushParam(location);
	pushParam(count);
	pushBuf(value, sizeof(GLint) * count * 3);
}

//769
extern "C" void glUniform4ivARB(GLint location, GLsizei count, const GLint * value){
	LOG("Called untested stub Uniform4ivARB!\n");
	pushOp(769);
	pushParam(location);
	pushParam(count);
	pushBuf(value, sizeof(GLint) * count * 4);
}

//770
extern "C" void glUniformMatrix2fvARB(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value){
	LOG("Called untested stub UniformMatrix2fvARB!\n");
	pushOp(770);
	pushParam(location);
	pushParam(count);
	pushParam(transpose);
	pushBuf(value, sizeof(GLint) * count * 2);
}

//771
extern "C" void glUniformMatrix3fvARB(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value){
	LOG("Called untested stub UniformMatrix3fvARB!\n");
	pushOp(771);
	pushParam(location);
	pushParam(count);
	pushParam(transpose);
	pushBuf(value, sizeof(GLint) * count * 3);
}

//772
extern "C" void glUniformMatrix4fvARB(GLint location, GLsizei count, GLboolean transpose, const GLfloat * value){
	LOG("Called untested stub UniformMatrix4fvARB!\n");
	pushOp(772);
	pushParam(location);
	pushParam(count);
	pushParam(transpose);
	pushBuf(value, sizeof(GLint) * count * 4);
}

//773
extern "C" void glGetObjectParameterfvARB(GLhandleARB obj, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetObjectParameterfvARB!\n");
}

//774
extern "C" void glGetObjectParameterivARB(GLhandleARB obj, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetObjectParameterivARB!\n");
}

//775
extern "C" void glGetInfoLogARB(GLhandleARB obj, GLsizei maxLength, GLsizei * length, GLcharARB * infoLog){
	LOG("Called unimplemted stub GetInfoLogARB!\n");
}

//776
extern "C" void glGetAttachedObjectsARB(GLhandleARB containerObj, GLsizei maxLength, GLsizei * length, GLhandleARB * infoLog){
	LOG("Called unimplemted stub GetAttachedObjectsARB!\n");
}

//777
extern "C" GLint glGetUniformLocationARB(GLhandleARB program, const GLcharARB * name){
	LOG("Called unimplemted stub GetUniformLocationARB!\n");
}

//778
extern "C" void glGetActiveUniformARB(GLhandleARB program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLcharARB * name){
	LOG("Called unimplemted stub GetActiveUniformARB!\n");
}

//779
extern "C" void glGetUniformfvARB(GLhandleARB program, GLint location, GLfloat * params){
	LOG("Called unimplemted stub GetUniformfvARB!\n");
}

//780
extern "C" void glGetUniformivARB(GLhandleARB program, GLint location, GLint * params){
	LOG("Called unimplemted stub GetUniformivARB!\n");
}

//781
extern "C" void glGetShaderSourceARB(GLhandleARB shader, GLsizei bufSize, GLsizei * length, GLcharARB * source){
	LOG("Called untested stub GetShaderSourceARB!\n");
	pushOp(781);
	pushParam(shader);
	pushParam(bufSize);
	if(!length)
	{
	//push a -1 length, rather than a NULL object that crashes
	pushParam((GLint) -1);
	}
	else
	{
	pushParam(*length);
	}
	pushBuf(source, sizeof(GLchar) * bufSize, true);
	waitForReturn();
}

//782
extern "C" void glBindAttribLocationARB(GLhandleARB program, GLuint index, const GLcharARB * name){
	LOG("Called untested stub BindAttribLocationARB!\n");
	pushOp(782);
	pushParam(program);
	pushParam(index);
	pushBuf(name, strlen(name));
}

//783
extern "C" void glGetActiveAttribARB(GLhandleARB program, GLuint index, GLsizei bufSize, GLsizei * length, GLint * size, GLenum * type, GLcharARB * name){
	LOG("Called unimplemted stub GetActiveAttribARB!\n");
}

//784
extern "C" GLint glGetAttribLocationARB(GLhandleARB program, const GLcharARB * name){
	LOG("Called unimplemted stub GetAttribLocationARB!\n");
}

//785
extern "C" void glDrawBuffersARB(GLsizei n, const GLenum * bufs){
	pushOp(785);
	pushParam(n);
	pushBuf(bufs, sizeof(const GLenum) * n);
}

//786
extern "C" void glBlendColorEXT(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha){
	pushOp(786);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
	pushParam(alpha);
}

//787
extern "C" void glPolygonOffsetEXT(GLfloat factor, GLfloat bias){
	pushOp(787);
	pushParam(factor);
	pushParam(bias);
}

//788
extern "C" void glTexImage3DEXT(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid * pixels){
	LOG("Called untested stub TexImage3DEXT!\n");
	pushOp(788);
	pushParam(target);
	pushParam(level);
	pushParam(internalformat);
	pushParam(width);
	pushParam(height);
	pushParam(depth);
	pushParam(border);
	pushParam(format);
	pushParam(type);
	pushBuf(pixels, getTypeSize(type) * width * height * depth);
}

//789
extern "C" void glTexSubImage3DEXT(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, GLuint UNUSED, const GLvoid * pixels){
	LOG("Called untested stub TexSubImage3DEXT!\n");
	pushOp(789);
	pushParam(target);
	pushParam(level);
	pushParam(xoffset);
	pushParam(yoffset);
	pushParam(zoffset);
	pushParam(width);
	pushParam(height);
	pushParam(depth);
	pushParam(format);
	pushParam(type);
	pushParam(UNUSED);
	pushBuf(pixels, getTypeSize(type) * width * height * depth);
}

//790
extern "C" void glGetTexFilterFuncSGIS(GLenum target, GLenum filter, GLfloat * weights){
	LOG("Called unimplemted stub GetTexFilterFuncSGIS!\n");
}

//791
extern "C" void glTexFilterFuncSGIS(GLenum target, GLenum filter, GLsizei n, const GLfloat * weights){
	pushOp(791);
	pushParam(target);
	pushParam(filter);
	pushParam(n);
	pushBuf(weights, sizeof(const GLfloat) * n);
}

//792
extern "C" void glTexSubImage1DEXT(GLenum target, GLint level, GLint xoffset, GLsizei width, GLenum format, GLenum type, GLuint UNUSED, const GLvoid * pixels){
	LOG("Called untested stub TexSubImage1DEXT!\n");
	pushOp(792);
	pushParam(target);
	pushParam(level);
	pushParam(xoffset);
	pushParam(width);
	pushParam(format);
	pushParam(type);
	pushParam(UNUSED);
	pushBuf(pixels, getTypeSize(type) * width);
}

//793
extern "C" void glTexSubImage2DEXT(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, GLuint UNUSED, const GLvoid * pixels){
	LOG("Called untested stub TexSubImage2DEXT!\n");
	pushOp(793);
	pushParam(target);
	pushParam(level);
	pushParam(xoffset);
	pushParam(yoffset);
	pushParam(width);
	pushParam(height);
	pushParam(format);
	pushParam(type);
	pushParam(UNUSED);
	pushBuf(pixels, getTypeSize(type) * width * height);
}

//794
extern "C" void glCopyTexImage1DEXT(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLint border){
	pushOp(794);
	pushParam(target);
	pushParam(level);
	pushParam(internalformat);
	pushParam(x);
	pushParam(y);
	pushParam(width);
	pushParam(border);
}

//795
extern "C" void glCopyTexImage2DEXT(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border){
	pushOp(795);
	pushParam(target);
	pushParam(level);
	pushParam(internalformat);
	pushParam(x);
	pushParam(y);
	pushParam(width);
	pushParam(height);
	pushParam(border);
}

//796
extern "C" void glCopyTexSubImage1DEXT(GLenum target, GLint level, GLint xoffset, GLint x, GLint y, GLsizei width){
	pushOp(796);
	pushParam(target);
	pushParam(level);
	pushParam(xoffset);
	pushParam(x);
	pushParam(y);
	pushParam(width);
}

//797
extern "C" void glCopyTexSubImage2DEXT(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height){
	pushOp(797);
	pushParam(target);
	pushParam(level);
	pushParam(xoffset);
	pushParam(yoffset);
	pushParam(x);
	pushParam(y);
	pushParam(width);
	pushParam(height);
}

//798
extern "C" void glCopyTexSubImage3DEXT(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint x, GLint y, GLsizei width, GLsizei height){
	pushOp(798);
	pushParam(target);
	pushParam(level);
	pushParam(xoffset);
	pushParam(yoffset);
	pushParam(zoffset);
	pushParam(x);
	pushParam(y);
	pushParam(width);
	pushParam(height);
}

//799
extern "C" void glGetHistogramEXT(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid * values){
	LOG("Called unimplemted stub GetHistogramEXT!\n");
}

//800
extern "C" void glGetHistogramParameterfvEXT(GLenum target, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetHistogramParameterfvEXT!\n");
}

//801
extern "C" void glGetHistogramParameterivEXT(GLenum target, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetHistogramParameterivEXT!\n");
}

//802
extern "C" void glGetMinmaxEXT(GLenum target, GLboolean reset, GLenum format, GLenum type, GLvoid * values){
	LOG("Called unimplemted stub GetMinmaxEXT!\n");
}

//803
extern "C" void glGetMinmaxParameterfvEXT(GLenum target, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetMinmaxParameterfvEXT!\n");
}

//804
extern "C" void glGetMinmaxParameterivEXT(GLenum target, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetMinmaxParameterivEXT!\n");
}

//805
extern "C" void glHistogramEXT(GLenum target, GLsizei width, GLenum internalformat, GLboolean sink){
	pushOp(805);
	pushParam(target);
	pushParam(width);
	pushParam(internalformat);
	pushParam(sink);
}

//806
extern "C" void glMinmaxEXT(GLenum target, GLenum internalformat, GLboolean sink){
	pushOp(806);
	pushParam(target);
	pushParam(internalformat);
	pushParam(sink);
}

//807
extern "C" void glResetHistogramEXT(GLenum target){
	pushOp(807);
	pushParam(target);
}

//808
extern "C" void glResetMinmaxEXT(GLenum target){
	pushOp(808);
	pushParam(target);
}

//809
extern "C" void glConvolutionFilter1DEXT(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid * image){
	LOG("Called untested stub ConvolutionFilter1DEXT!\n");
	pushOp(809);
	pushParam(target);
	pushParam(internalformat);
	pushParam(width);
	pushParam(format);
	pushParam(type);
	pushBuf(image, getTypeSize(type) * width);
}

//810
extern "C" void glConvolutionFilter2DEXT(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * image){
	LOG("Called untested stub ConvolutionFilter2DEXT!\n");
	pushOp(810);
	pushParam(target);
	pushParam(internalformat);
	pushParam(width);
	pushParam(height);
	pushParam(format);
	pushParam(type);
	pushBuf(image, getTypeSize(type) * width * height);
}

//811
extern "C" void glConvolutionParameterfEXT(GLenum target, GLenum pname, GLfloat params){
	pushOp(811);
	pushParam(target);
	pushParam(pname);
	pushParam(params);
}

//812
extern "C" void glConvolutionParameterfvEXT(GLenum target, GLenum pname, const GLfloat * params){
	LOG("Called untested stub ConvolutionParameterfvEXT!\n");
	pushOp(812);
	pushParam(target);
	pushParam(pname);
	int size = sizeof(GLfloat);
	if(pname != GL_CONVOLUTION_BORDER_MODE)
		size *= 4;
	pushBuf(params, size);

}

//813
extern "C" void glConvolutionParameteriEXT(GLenum target, GLenum pname, GLint params){
	pushOp(813);
	pushParam(target);
	pushParam(pname);
	pushParam(params);
}

//814
extern "C" void glConvolutionParameterivEXT(GLenum target, GLenum pname, const GLint * params){
	LOG("Called untested stub ConvolutionParameterivEXT!\n");
	pushOp(814);
	pushParam(target);
	pushParam(pname);
	int size = sizeof(GLint);
	if(pname != GL_CONVOLUTION_BORDER_MODE)
		size *= 4;
	pushBuf(params, size);
}

//815
extern "C" void glCopyConvolutionFilter1DEXT(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width){
	pushOp(815);
	pushParam(target);
	pushParam(internalformat);
	pushParam(x);
	pushParam(y);
	pushParam(width);
}

//816
extern "C" void glCopyConvolutionFilter2DEXT(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height){
	pushOp(816);
	pushParam(target);
	pushParam(internalformat);
	pushParam(x);
	pushParam(y);
	pushParam(width);
	pushParam(height);
}

//817
extern "C" void glGetConvolutionFilterEXT(GLenum target, GLenum format, GLenum type, GLvoid * image){
	LOG("Called unimplemted stub GetConvolutionFilterEXT!\n");
}

//818
extern "C" void glGetConvolutionParameterfvEXT(GLenum target, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetConvolutionParameterfvEXT!\n");
}

//819
extern "C" void glGetConvolutionParameterivEXT(GLenum target, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetConvolutionParameterivEXT!\n");
}

//820
extern "C" void glGetSeparableFilterEXT(GLenum target, GLenum format, GLenum type, GLvoid * row, GLvoid * column, GLvoid * span){
	LOG("Called unimplemted stub GetSeparableFilterEXT!\n");
}

//821
extern "C" void glSeparableFilter2DEXT(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid * row, const GLvoid * column){
	LOG("Called untested stub SeparableFilter2DEXT!\n");
	pushOp(821);
	pushParam(target);
	pushParam(internalformat);
	pushParam(width);
	pushParam(height);
	pushParam(format);
	pushParam(type);
	pushBuf(row, getTypeSize(type) * width);
	pushBuf(column, getTypeSize(type) * height);
}

//822
extern "C" void glColorTableSGI(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid * table){
	LOG("Called untested stub ColorTableSGI!\n");
	pushOp(822);
	pushParam(target);
	pushParam(internalformat);
	pushParam(width);
	pushParam(format);
	pushParam(type);
	pushBuf(table, getTypeSize(type) * width);
}

//823
extern "C" void glColorTableParameterfvSGI(GLenum target, GLenum pname, const GLfloat * params){
	LOG("Called unimplemted stub ColorTableParameterfvSGI!\n");
}

//824
extern "C" void glColorTableParameterivSGI(GLenum target, GLenum pname, const GLint * params){
	LOG("Called unimplemted stub ColorTableParameterivSGI!\n");
}

//825
extern "C" void glCopyColorTableSGI(GLenum target, GLenum internalformat, GLint x, GLint y, GLsizei width){
	pushOp(825);
	pushParam(target);
	pushParam(internalformat);
	pushParam(x);
	pushParam(y);
	pushParam(width);
}

//826
extern "C" void glGetColorTableSGI(GLenum target, GLenum format, GLenum type, GLvoid * table){
	LOG("Called unimplemted stub GetColorTableSGI!\n");
}

//827
extern "C" void glGetColorTableParameterfvSGI(GLenum target, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetColorTableParameterfvSGI!\n");
}

//828
extern "C" void glGetColorTableParameterivSGI(GLenum target, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetColorTableParameterivSGI!\n");
}

//829
extern "C" void glPixelTexGenParameteriSGIS(GLenum pname, GLint param){
	pushOp(829);
	pushParam(pname);
	pushParam(param);
}

//830
extern "C" void glPixelTexGenParameterivSGIS(GLenum pname, const GLint * params){
	LOG("Called unimplemted stub PixelTexGenParameterivSGIS!\n");
}

//831
extern "C" void glPixelTexGenParameterfSGIS(GLenum pname, GLfloat param){
	pushOp(831);
	pushParam(pname);
	pushParam(param);
}

//832
extern "C" void glPixelTexGenParameterfvSGIS(GLenum pname, const GLfloat * params){
	LOG("Called unimplemted stub PixelTexGenParameterfvSGIS!\n");
}

//833
extern "C" void glGetPixelTexGenParameterivSGIS(GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetPixelTexGenParameterivSGIS!\n");
}

//834
extern "C" void glGetPixelTexGenParameterfvSGIS(GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetPixelTexGenParameterfvSGIS!\n");
}

//835
extern "C" void glTexImage4DSGIS(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLsizei size4d, GLint border, GLenum format, GLenum type, const GLvoid * pixels){
	LOG("Called untested stub TexImage4DSGIS!\n");
	pushOp(837);
	pushParam(target);
	pushParam(level);
	pushParam(internalformat);
	pushParam(width);
	pushParam(height);
	pushParam(depth);
	pushParam(size4d);
	pushParam(border);
	pushParam(format);
	pushParam(type);
	pushBuf(pixels, getTypeSize(type) * width * height * depth * size4d);
}

//836
extern "C" void glTexSubImage4DSGIS(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset, GLint woffset, GLsizei width, GLsizei height, GLsizei depth, GLsizei size4d, GLenum format, GLenum type, GLuint UNUSED, const GLvoid * pixels){
	LOG("Called untested stub TexSubImage4DSGIS!\n");
	pushOp(837);
	pushParam(target);
	pushParam(level);
	pushParam(xoffset);
	pushParam(yoffset);
	pushParam(zoffset);
	pushParam(woffset);
	pushParam(width);
	pushParam(height);
	pushParam(depth);
	pushParam(size4d);
	pushParam(format);
	pushParam(type);
	pushParam(UNUSED);
	pushBuf(pixels, getTypeSize(type) * (width + xoffset) * (height + yoffset) * (depth + zoffset) * (size4d + woffset));
}

//837
extern "C" GLboolean glAreTexturesResidentEXT(GLsizei n, const GLuint * textures, GLboolean * residences){
	pushOp(837);
	pushParam(n);
	pushBuf(textures, sizeof(const GLuint) * n);
	pushBuf(residences, sizeof(GLboolean) * n);

	GLboolean ret;
	pushBuf(&ret, sizeof(GLboolean), true);
	waitForReturn();

	return ret;
}

//838
extern "C" void glBindTextureEXT(GLenum target, GLuint texture){
	pushOp(838);
	pushParam(target);
	pushParam(texture);
}

//839
extern "C" void glDeleteTexturesEXT(GLsizei n, const GLuint * textures){
	pushOp(839);
	pushParam(n);
	pushBuf(textures, sizeof(const GLuint) * n);
}

//840
extern "C" void glGenTexturesEXT(GLsizei n, GLuint * textures){
	pushOp(840);
	pushParam(n);
	pushBuf(textures, sizeof(GLuint) * n);
}

//841
extern "C" GLboolean glIsTextureEXT(GLuint texture){
	pushOp(841);
	pushParam(texture);

	GLboolean ret;
	pushBuf(&ret, sizeof(GLboolean), true);
	waitForReturn();

	return ret;
}

//842
extern "C" void glPrioritizeTexturesEXT(GLsizei n, const GLuint * textures, const GLclampf * priorities){
	LOG("Called untested stub PrioritizeTexturesEXT!\n");
	pushOp(842);
	pushParam(n);
	pushBuf(textures, sizeof(GLuint) * n);
	pushBuf(priorities, sizeof(GLclampf) * n);
}

//843
extern "C" void glDetailTexFuncSGIS(GLenum target, GLsizei n, const GLfloat * points){
	pushOp(843);
	pushParam(target);
	pushParam(n);
	pushBuf(points, sizeof(const GLfloat) * n);
}

//844
extern "C" void glGetDetailTexFuncSGIS(GLenum target, GLfloat * points){
	LOG("Called unimplemted stub GetDetailTexFuncSGIS!\n");
}

//845
extern "C" void glSharpenTexFuncSGIS(GLenum target, GLsizei n, const GLfloat * points){
	pushOp(845);
	pushParam(target);
	pushParam(n);
	pushBuf(points, sizeof(const GLfloat) * n);
}

//846
extern "C" void glGetSharpenTexFuncSGIS(GLenum target, GLfloat * points){
	LOG("Called unimplemted stub GetSharpenTexFuncSGIS!\n");
}

//847
extern "C" void glSampleMaskSGIS(GLclampf value, GLboolean invert){
	pushOp(847);
	pushParam(value);
	pushParam(invert);
}

//848
extern "C" void glSamplePatternSGIS(GLenum pattern){
	pushOp(848);
	pushParam(pattern);
}

//849
extern "C" void glArrayElementEXT(GLint i){
	pushOp(849);
	pushParam(i);
}

//850
extern "C" void glColorPointerEXT(GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid * pointer){
	LOG("Called untested stub ColorPointerEXT!\n");
	pushOp(850);
	pushParam(size);
	pushParam(type);
	pushParam(stride);
	pushParam(count);
	pushBuf(pointer, getTypeSize(type) * (stride + size));
}

//851
extern "C" void glDrawArraysEXT(GLenum mode, GLint first, GLsizei count){
	pushOp(851);
	pushParam(mode);
	pushParam(first);
	pushParam(count);
}

//852
extern "C" void glEdgeFlagPointerEXT(GLsizei stride, GLsizei count, const GLboolean * pointer){
	LOG("Called untested stub EdgeFlagPointerEXT!\n");
	pushOp(852);
	pushParam(stride);
	pushParam(count);
	pushBuf(pointer, sizeof(GLboolean) * (count + stride));
}

//853
extern "C" void glGetPointervEXT(GLenum pname, GLvoid ** params){
	LOG("Called unimplemted stub GetPointervEXT!\n");
}

//854
extern "C" void glIndexPointerEXT(GLenum type, GLsizei stride, GLsizei count, const GLvoid * pointer){
	LOG("Called untested stub IndexPointerEXT!\n");
	pushOp(854);
	pushParam(type);
	pushParam(stride);
	pushParam(count);
	pushBuf(pointer, getTypeSize(type) * (count + stride));
}

//855
extern "C" void glNormalPointerEXT(GLenum type, GLsizei stride, GLsizei count, const GLvoid * pointer){
	LOG("Called untested stub NormalPointerEXT!\n");
	pushOp(855);
	pushParam(type);
	pushParam(stride);
	pushParam(count);
	pushBuf(pointer, getTypeSize(type) * (count + stride));
}

//856
extern "C" void glTexCoordPointerEXT(GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid * pointer){
	LOG("Called untested stub TexCoordPointerEXT!\n");
	pushOp(856);
	pushParam(size);
	pushParam(type);
	pushParam(stride);
	pushParam(count);
	pushBuf(pointer, size * getTypeSize(type) * (count + stride));

}

//857
extern "C" void glVertexPointerEXT(GLint size, GLenum type, GLsizei stride, GLsizei count, const GLvoid * pointer){
	LOG("Called untested stub VertexPointerEXT!\n");
	pushOp(857);
	pushParam(size);
	pushParam(type);
	pushParam(stride);
	pushParam(count);
	pushBuf(pointer, size * getTypeSize(type) * (count + stride));
}

//858
extern "C" void glBlendEquationEXT(GLenum mode){
	pushOp(858);
	pushParam(mode);
}

//859
extern "C" void glSpriteParameterfSGIX(GLenum pname, GLfloat param){
	pushOp(859);
	pushParam(pname);
	pushParam(param);
}

//860
extern "C" void glSpriteParameterfvSGIX(GLenum pname, const GLfloat * params){
	LOG("Called unimplemted stub SpriteParameterfvSGIX!\n");
}

//861
extern "C" void glSpriteParameteriSGIX(GLenum pname, GLint param){
	pushOp(861);
	pushParam(pname);
	pushParam(param);
}

//862
extern "C" void glSpriteParameterivSGIX(GLenum pname, const GLint * params){
	LOG("Called unimplemted stub SpriteParameterivSGIX!\n");
}

//863
extern "C" void glPointParameterfEXT(GLenum pname, GLfloat param){
	pushOp(863);
	pushParam(pname);
	pushParam(param);
}

//864
extern "C" void glPointParameterfvEXT(GLenum pname, const GLfloat * params){
	LOG("Called unimplemted stub PointParameterfvEXT!\n");
}

//865
extern "C" GLint glGetInstrumentsSGIX(){
	pushOp(865);

	GLint ret;
	pushBuf(&ret, sizeof(GLint), true);
	waitForReturn();

	return ret;
}

//866
extern "C" void glInstrumentsBufferSGIX(GLsizei size, GLint * buffer){
	LOG("Called untested stub InstrumentsBufferSGIX!\n");
	pushOp(866);
	pushParam(size);
	pushBuf(buffer, size);
}

//867
extern "C" GLint glPollInstrumentsSGIX(GLint * marker_p){
	LOG("Called unimplemted stub PollInstrumentsSGIX!\n");
}

//868
extern "C" void glReadInstrumentsSGIX(GLint marker){
	pushOp(868);
	pushParam(marker);
}

//869
extern "C" void glStartInstrumentsSGIX(){
	pushOp(869);
}

//870
extern "C" void glStopInstrumentsSGIX(GLint marker){
	pushOp(870);
	pushParam(marker);
}

//871
extern "C" void glFrameZoomSGIX(GLint factor){
	pushOp(871);
	pushParam(factor);
}

//872
extern "C" void glTagSampleBufferSGIX(){
	pushOp(872);
}

//873
extern "C" void glReferencePlaneSGIX(const GLdouble * equation){
	pushOp(873);
	pushBuf(equation, sizeof(const GLdouble) * 4);
}

//874
extern "C" void glFlushRasterSGIX(){
	pushOp(874);
}

//875
extern "C" void glFogFuncSGIS(GLsizei n, const GLfloat * points){
	pushOp(875);
	pushParam(n);
	pushBuf(points, sizeof(const GLfloat) * n);
}

//876
extern "C" void glGetFogFuncSGIS(GLfloat * points){
	LOG("Called unimplemted stub GetFogFuncSGIS!\n");
}

//877
extern "C" void glImageTransformParameteriHP(GLenum target, GLenum pname, GLint param){
	pushOp(877);
	pushParam(target);
	pushParam(pname);
	pushParam(param);
}

//878
extern "C" void glImageTransformParameterfHP(GLenum target, GLenum pname, GLfloat param){
	pushOp(878);
	pushParam(target);
	pushParam(pname);
	pushParam(param);
}

//879
extern "C" void glImageTransformParameterivHP(GLenum target, GLenum pname, const GLint * params){
	LOG("Called unimplemted stub ImageTransformParameterivHP!\n");
}

//880
extern "C" void glImageTransformParameterfvHP(GLenum target, GLenum pname, const GLfloat * params){
	LOG("Called unimplemted stub ImageTransformParameterfvHP!\n");
}

//881
extern "C" void glGetImageTransformParameterivHP(GLenum target, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetImageTransformParameterivHP!\n");
}

//882
extern "C" void glGetImageTransformParameterfvHP(GLenum target, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetImageTransformParameterfvHP!\n");
}

//883
extern "C" void glColorSubTableEXT(GLenum target, GLsizei start, GLsizei count, GLenum format, GLenum type, const GLvoid * data){
	LOG("Called untested stub ColorSubTableEXT!\n");
	pushOp(883);
	pushParam(target);
	pushParam(start);
	pushParam(count);
	pushParam(format);
	pushParam(type);
	pushBuf(data, getTypeSize(type) * (start + count));
}

//884
extern "C" void glCopyColorSubTableEXT(GLenum target, GLsizei start, GLint x, GLint y, GLsizei width){
	pushOp(884);
	pushParam(target);
	pushParam(start);
	pushParam(x);
	pushParam(y);
	pushParam(width);
}

//885
extern "C" void glHintPGI(GLenum target, GLint mode){
	pushOp(885);
	pushParam(target);
	pushParam(mode);
}

//886
extern "C" void glColorTableEXT(GLenum target, GLenum internalformat, GLsizei width, GLenum format, GLenum type, const GLvoid * table){
	LOG("Called untested stub ColorTableEXT!\n");
	pushOp(886);
	pushParam(target);
	pushParam(internalformat);
	pushParam(width);
	pushParam(format);
	pushParam(type);
	pushBuf(table, getTypeSize(type) * width);
}

//887
extern "C" void glGetColorTableEXT(GLenum target, GLenum format, GLenum type, GLvoid * table){
	LOG("Called unimplemted stub GetColorTableEXT!\n");
}

//888
extern "C" void glGetColorTableParameterivEXT(GLenum target, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetColorTableParameterivEXT!\n");
}

//889
extern "C" void glGetColorTableParameterfvEXT(GLenum target, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetColorTableParameterfvEXT!\n");
}

//890
extern "C" void glGetListParameterfvSGIX(GLuint list, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetListParameterfvSGIX!\n");
}

//891
extern "C" void glGetListParameterivSGIX(GLuint list, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetListParameterivSGIX!\n");
}

//892
extern "C" void glListParameterfSGIX(GLuint list, GLenum pname, GLfloat param){
	pushOp(892);
	pushParam(list);
	pushParam(pname);
	pushParam(param);
}

//893
extern "C" void glListParameterfvSGIX(GLuint list, GLenum pname, const GLfloat * params){
	LOG("Called unimplemted stub ListParameterfvSGIX!\n");
}

//894
extern "C" void glListParameteriSGIX(GLuint list, GLenum pname, GLint param){
	pushOp(894);
	pushParam(list);
	pushParam(pname);
	pushParam(param);
}

//895
extern "C" void glListParameterivSGIX(GLuint list, GLenum pname, const GLint * params){
	LOG("Called unimplemted stub ListParameterivSGIX!\n");
}

//896
extern "C" void glIndexMaterialEXT(GLenum face, GLenum mode){
	pushOp(896);
	pushParam(face);
	pushParam(mode);
}

//897
extern "C" void glIndexFuncEXT(GLenum func, GLclampf ref){
	pushOp(897);
	pushParam(func);
	pushParam(ref);
}

//898
extern "C" void glLockArraysEXT(GLint first, GLsizei count){
	pushOp(898);
	pushParam(first);
	pushParam(count);
}

//899
extern "C" void glUnlockArraysEXT(){
	pushOp(899);
}

//900
extern "C" void glCullParameterdvEXT(GLenum pname, GLdouble * params){
	LOG("Called unimplemted stub CullParameterdvEXT!\n");
}

//901
extern "C" void glCullParameterfvEXT(GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub CullParameterfvEXT!\n");
}

//902
extern "C" void glFragmentColorMaterialSGIX(GLenum face, GLenum mode){
	pushOp(902);
	pushParam(face);
	pushParam(mode);
}

//903
extern "C" void glFragmentLightfSGIX(GLenum light, GLenum pname, GLfloat param){
	pushOp(903);
	pushParam(light);
	pushParam(pname);
	pushParam(param);
}

//904
extern "C" void glFragmentLightfvSGIX(GLenum light, GLenum pname, const GLfloat * params){
	LOG("Called unimplemted stub FragmentLightfvSGIX!\n");
}

//905
extern "C" void glFragmentLightiSGIX(GLenum light, GLenum pname, GLint param){
	pushOp(905);
	pushParam(light);
	pushParam(pname);
	pushParam(param);
}

//906
extern "C" void glFragmentLightivSGIX(GLenum light, GLenum pname, const GLint * params){
	LOG("Called unimplemted stub FragmentLightivSGIX!\n");
}

//907
extern "C" void glFragmentLightModelfSGIX(GLenum pname, GLfloat param){
	pushOp(907);
	pushParam(pname);
	pushParam(param);
}

//908
extern "C" void glFragmentLightModelfvSGIX(GLenum pname, const GLfloat * params){
	LOG("Called unimplemted stub FragmentLightModelfvSGIX!\n");
}

//909
extern "C" void glFragmentLightModeliSGIX(GLenum pname, GLint param){
	pushOp(909);
	pushParam(pname);
	pushParam(param);
}

//910
extern "C" void glFragmentLightModelivSGIX(GLenum pname, const GLint * params){
	LOG("Called unimplemted stub FragmentLightModelivSGIX!\n");
}

//911
extern "C" void glFragmentMaterialfSGIX(GLenum face, GLenum pname, GLfloat param){
	pushOp(911);
	pushParam(face);
	pushParam(pname);
	pushParam(param);
}

//912
extern "C" void glFragmentMaterialfvSGIX(GLenum face, GLenum pname, const GLfloat * params){
	LOG("Called unimplemted stub FragmentMaterialfvSGIX!\n");
}

//913
extern "C" void glFragmentMaterialiSGIX(GLenum face, GLenum pname, GLint param){
	pushOp(913);
	pushParam(face);
	pushParam(pname);
	pushParam(param);
}

//914
extern "C" void glFragmentMaterialivSGIX(GLenum face, GLenum pname, const GLint * params){
	LOG("Called unimplemted stub FragmentMaterialivSGIX!\n");
}

//915
extern "C" void glGetFragmentLightfvSGIX(GLenum light, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetFragmentLightfvSGIX!\n");
}

//916
extern "C" void glGetFragmentLightivSGIX(GLenum light, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetFragmentLightivSGIX!\n");
}

//917
extern "C" void glGetFragmentMaterialfvSGIX(GLenum face, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetFragmentMaterialfvSGIX!\n");
}

//918
extern "C" void glGetFragmentMaterialivSGIX(GLenum face, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetFragmentMaterialivSGIX!\n");
}

//919
extern "C" void glLightEnviSGIX(GLenum pname, GLint param){
	pushOp(919);
	pushParam(pname);
	pushParam(param);
}

//920
extern "C" void glDrawRangeElementsEXT(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const GLvoid * indices){
	LOG("Called untested stub DrawRangeElementsEXT!\n");
	pushOp(920);
	pushParam(mode);
	pushParam(start);
	pushParam(end);
	pushParam(count);
	pushParam(type);
	pushBuf(indices, getTypeSize(type) * end);
}

//921
extern "C" void glApplyTextureEXT(GLenum mode){
	pushOp(921);
	pushParam(mode);
}

//922
extern "C" void glTextureLightEXT(GLenum pname){
	pushOp(922);
	pushParam(pname);
}

//923
extern "C" void glTextureMaterialEXT(GLenum face, GLenum mode){
	pushOp(923);
	pushParam(face);
	pushParam(mode);
}

//924
extern "C" void glAsyncMarkerSGIX(GLuint marker){
	pushOp(924);
	pushParam(marker);
}

//925
extern "C" GLint glFinishAsyncSGIX(GLuint * markerp){
	LOG("Called unimplemted stub FinishAsyncSGIX!\n");
}

//926
extern "C" GLint glPollAsyncSGIX(GLuint * markerp){
	LOG("Called unimplemted stub PollAsyncSGIX!\n");
}

//927
extern "C" GLuint glGenAsyncMarkersSGIX(GLsizei range){
	pushOp(927);
	pushParam(range);

	GLuint ret;
	pushBuf(&ret, sizeof(GLuint), true);
	waitForReturn();

	return ret;
}

//928
extern "C" void glDeleteAsyncMarkersSGIX(GLuint marker, GLsizei range){
	pushOp(928);
	pushParam(marker);
	pushParam(range);
}

//929
extern "C" GLboolean glIsAsyncMarkerSGIX(GLuint marker){
	pushOp(929);
	pushParam(marker);

	GLboolean ret;
	pushBuf(&ret, sizeof(GLboolean), true);
	waitForReturn();

	return ret;
}

//930
extern "C" void glVertexPointervINTEL(GLint size, GLenum type, const GLvoid ** pointer){
	LOG("Called unimplemted stub VertexPointervINTEL!\n");
	//pushOp(930);
	//pushParam(size);
	//pushParam(type);
	//pushBuf(pointer, size * getTypeSize(type));

}

//931
extern "C" void glNormalPointervINTEL(GLenum type, const GLvoid ** pointer){
	LOG("Called unimplemted stub NormalPointervINTEL!\n");
}

//932
extern "C" void glColorPointervINTEL(GLint size, GLenum type, const GLvoid ** pointer){
	LOG("Called unimplemted stub ColorPointervINTEL!\n");
}

//933
extern "C" void glTexCoordPointervINTEL(GLint size, GLenum type, const GLvoid ** pointer){
	LOG("Called unimplemted stub TexCoordPointervINTEL!\n");
}

//934
extern "C" void glPixelTransformParameteriEXT(GLenum target, GLenum pname, GLint param){
	pushOp(934);
	pushParam(target);
	pushParam(pname);
	pushParam(param);
}

//935
extern "C" void glPixelTransformParameterfEXT(GLenum target, GLenum pname, GLfloat param){
	pushOp(935);
	pushParam(target);
	pushParam(pname);
	pushParam(param);
}

//936
extern "C" void glPixelTransformParameterivEXT(GLenum target, GLenum pname, const GLint * params){
	LOG("Called unimplemted stub PixelTransformParameterivEXT!\n");
}

//937
extern "C" void glPixelTransformParameterfvEXT(GLenum target, GLenum pname, const GLfloat * params){
	LOG("Called unimplemted stub PixelTransformParameterfvEXT!\n");
}

//938
extern "C" void glSecondaryColor3bEXT(GLbyte red, GLbyte green, GLbyte blue){
	pushOp(938);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//939
extern "C" void glSecondaryColor3bvEXT(const GLbyte * v){
	pushOp(939);
	pushBuf(v, sizeof(const GLbyte) * 3);
}

//940
extern "C" void glSecondaryColor3dEXT(GLdouble red, GLdouble green, GLdouble blue){
	pushOp(940);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//941
extern "C" void glSecondaryColor3dvEXT(const GLdouble * v){
	pushOp(941);
	pushBuf(v, sizeof(const GLdouble) * 3);
}

//942
extern "C" void glSecondaryColor3fEXT(GLfloat red, GLfloat green, GLfloat blue){
	pushOp(942);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//943
extern "C" void glSecondaryColor3fvEXT(const GLfloat * v){
	pushOp(943);
	pushBuf(v, sizeof(const GLfloat) * 3);
}

//944
extern "C" void glSecondaryColor3iEXT(GLint red, GLint green, GLint blue){
	pushOp(944);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//945
extern "C" void glSecondaryColor3ivEXT(const GLint * v){
	pushOp(945);
	pushBuf(v, sizeof(const GLint) * 3);
}

//946
extern "C" void glSecondaryColor3sEXT(GLshort red, GLshort green, GLshort blue){
	pushOp(946);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//947
extern "C" void glSecondaryColor3svEXT(const GLshort * v){
	pushOp(947);
	pushBuf(v, sizeof(const GLshort) * 3);
}

//948
extern "C" void glSecondaryColor3ubEXT(GLubyte red, GLubyte green, GLubyte blue){
	pushOp(948);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//949
extern "C" void glSecondaryColor3ubvEXT(const GLubyte * v){
	pushOp(949);
	pushBuf(v, sizeof(const GLubyte) * 3);
}

//950
extern "C" void glSecondaryColor3uiEXT(GLuint red, GLuint green, GLuint blue){
	pushOp(950);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//951
extern "C" void glSecondaryColor3uivEXT(const GLuint * v){
	pushOp(951);
	pushBuf(v, sizeof(const GLuint) * 3);
}

//952
extern "C" void glSecondaryColor3usEXT(GLushort red, GLushort green, GLushort blue){
	pushOp(952);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
}

//953
extern "C" void glSecondaryColor3usvEXT(const GLushort * v){
	pushOp(953);
	pushBuf(v, sizeof(const GLushort) * 3);
}

//954
extern "C" void glSecondaryColorPointerEXT(GLint size, GLenum type, GLsizei stride, const GLvoid * pointer){
	LOG("Called untested stub SecondaryColorPointerEXT!\n");
	pushOp(954);
	pushParam(size);
	pushParam(type);
	pushParam(stride);
	pushBuf(pointer, getTypeSize(type) * (size + stride));
}

//955
extern "C" void glTextureNormalEXT(GLenum mode){
	pushOp(955);
	pushParam(mode);
}

//956
extern "C" void glMultiDrawArraysEXT(GLenum mode, GLint * first, GLsizei * count, GLsizei primcount){
	LOG("Called untested stub MultiDrawArraysEXT!\n");
	pushOp(956);
	pushParam(mode);
	pushBuf(first, primcount);	
	pushBuf(count, primcount) ;	
	pushParam(primcount);
	
}

//957
extern "C" void glMultiDrawElementsEXT(GLenum mode, const GLsizei * count, GLenum type, const GLvoid ** indices, GLsizei primcount){
	LOG("Called unimplemted stub MultiDrawElementsEXT!\n");
}

//958
extern "C" void glFogCoordfEXT(GLfloat coord){
	pushOp(958);
	pushParam(coord);
}

//959
extern "C" void glFogCoordfvEXT(const GLfloat * coord){
	pushOp(959);
	pushBuf(coord, sizeof(const GLfloat) * 1);
}

//960
extern "C" void glFogCoorddEXT(GLdouble coord){
	pushOp(960);
	pushParam(coord);
}

//961
extern "C" void glFogCoorddvEXT(const GLdouble * coord){
	pushOp(961);
	pushBuf(coord, sizeof(const GLdouble) * 1);
}

//962
extern "C" void glFogCoordPointerEXT(GLenum type, GLsizei stride, const GLvoid * pointer){
	LOG("Called unimplemted stub FogCoordPointerEXT!\n");
}

//963
extern "C" void glTangent3bEXT(GLbyte tx, GLbyte ty, GLbyte tz){
	pushOp(963);
	pushParam(tx);
	pushParam(ty);
	pushParam(tz);
}

//964
extern "C" void glTangent3bvEXT(const GLbyte * v){
	pushOp(964);
	pushBuf(v, sizeof(const GLbyte) * 3);
}

//965
extern "C" void glTangent3dEXT(GLdouble tx, GLdouble ty, GLdouble tz){
	pushOp(965);
	pushParam(tx);
	pushParam(ty);
	pushParam(tz);
}

//966
extern "C" void glTangent3dvEXT(const GLdouble * v){
	pushOp(966);
	pushBuf(v, sizeof(const GLdouble) * 3);
}

//967
extern "C" void glTangent3fEXT(GLfloat tx, GLfloat ty, GLfloat tz){
	pushOp(967);
	pushParam(tx);
	pushParam(ty);
	pushParam(tz);
}

//968
extern "C" void glTangent3fvEXT(const GLfloat * v){
	pushOp(968);
	pushBuf(v, sizeof(const GLfloat) * 3);
}

//969
extern "C" void glTangent3iEXT(GLint tx, GLint ty, GLint tz){
	pushOp(969);
	pushParam(tx);
	pushParam(ty);
	pushParam(tz);
}

//970
extern "C" void glTangent3ivEXT(const GLint * v){
	pushOp(970);
	pushBuf(v, sizeof(const GLint) * 3);
}

//971
extern "C" void glTangent3sEXT(GLshort tx, GLshort ty, GLshort tz){
	pushOp(971);
	pushParam(tx);
	pushParam(ty);
	pushParam(tz);
}

//972
extern "C" void glTangent3svEXT(const GLshort * v){
	pushOp(972);
	pushBuf(v, sizeof(const GLshort) * 3);
}

//973
extern "C" void glBinormal3bEXT(GLbyte bx, GLbyte by, GLbyte bz){
	pushOp(973);
	pushParam(bx);
	pushParam(by);
	pushParam(bz);
}

//974
extern "C" void glBinormal3bvEXT(const GLbyte * v){
	pushOp(974);
	pushBuf(v, sizeof(const GLbyte) * 3);
}

//975
extern "C" void glBinormal3dEXT(GLdouble bx, GLdouble by, GLdouble bz){
	pushOp(975);
	pushParam(bx);
	pushParam(by);
	pushParam(bz);
}

//976
extern "C" void glBinormal3dvEXT(const GLdouble * v){
	pushOp(976);
	pushBuf(v, sizeof(const GLdouble) * 3);
}

//977
extern "C" void glBinormal3fEXT(GLfloat bx, GLfloat by, GLfloat bz){
	pushOp(977);
	pushParam(bx);
	pushParam(by);
	pushParam(bz);
}

//978
extern "C" void glBinormal3fvEXT(const GLfloat * v){
	pushOp(978);
	pushBuf(v, sizeof(const GLfloat) * 3);
}

//979
extern "C" void glBinormal3iEXT(GLint bx, GLint by, GLint bz){
	pushOp(979);
	pushParam(bx);
	pushParam(by);
	pushParam(bz);
}

//980
extern "C" void glBinormal3ivEXT(const GLint * v){
	pushOp(980);
	pushBuf(v, sizeof(const GLint) * 3);
}

//981
extern "C" void glBinormal3sEXT(GLshort bx, GLshort by, GLshort bz){
	pushOp(981);
	pushParam(bx);
	pushParam(by);
	pushParam(bz);
}

//982
extern "C" void glBinormal3svEXT(const GLshort * v){
	pushOp(982);
	pushBuf(v, sizeof(const GLshort) * 3);
}

//983
extern "C" void glTangentPointerEXT(GLenum type, GLsizei stride, const GLvoid * pointer){
	LOG("Called unimplemted stub TangentPointerEXT!\n");
}

//984
extern "C" void glBinormalPointerEXT(GLenum type, GLsizei stride, const GLvoid * pointer){
	LOG("Called unimplemted stub BinormalPointerEXT!\n");
}

//985
extern "C" void glPixelTexGenSGIX(GLenum mode){
	pushOp(985);
	pushParam(mode);
}

//986
extern "C" void glFinishTextureSUNX(){
	pushOp(986);
}

//987
extern "C" void glGlobalAlphaFactorbSUN(GLbyte factor){
	pushOp(987);
	pushParam(factor);
}

//988
extern "C" void glGlobalAlphaFactorsSUN(GLshort factor){
	pushOp(988);
	pushParam(factor);
}

//989
extern "C" void glGlobalAlphaFactoriSUN(GLint factor){
	pushOp(989);
	pushParam(factor);
}

//990
extern "C" void glGlobalAlphaFactorfSUN(GLfloat factor){
	pushOp(990);
	pushParam(factor);
}

//991
extern "C" void glGlobalAlphaFactordSUN(GLdouble factor){
	pushOp(991);
	pushParam(factor);
}

//992
extern "C" void glGlobalAlphaFactorubSUN(GLubyte factor){
	pushOp(992);
	pushParam(factor);
}

//993
extern "C" void glGlobalAlphaFactorusSUN(GLushort factor){
	pushOp(993);
	pushParam(factor);
}

//994
extern "C" void glGlobalAlphaFactoruiSUN(GLuint factor){
	pushOp(994);
	pushParam(factor);
}

//995
extern "C" void glReplacementCodeuiSUN(GLuint code){
	pushOp(995);
	pushParam(code);
}

//996
extern "C" void glReplacementCodeusSUN(GLushort code){
	pushOp(996);
	pushParam(code);
}

//997
extern "C" void glReplacementCodeubSUN(GLubyte code){
	pushOp(997);
	pushParam(code);
}

//998
extern "C" void glReplacementCodeuivSUN(const GLuint * code){
	LOG("Called unimplemted stub ReplacementCodeuivSUN!\n");
}

//999
extern "C" void glReplacementCodeusvSUN(const GLushort * code){
	LOG("Called unimplemted stub ReplacementCodeusvSUN!\n");
}

//1000
extern "C" void glReplacementCodeubvSUN(const GLubyte * code){
	LOG("Called unimplemted stub ReplacementCodeubvSUN!\n");
}

//1001
extern "C" void glReplacementCodePointerSUN(GLenum type, GLsizei stride, const GLvoid * pointer){
	LOG("Called unimplemted stub ReplacementCodePointerSUN!\n");
}

//1002
extern "C" void glColor4ubVertex2fSUN(GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y){
	pushOp(1002);
	pushParam(r);
	pushParam(g);
	pushParam(b);
	pushParam(a);
	pushParam(x);
	pushParam(y);
}

//1003
extern "C" void glColor4ubVertex2fvSUN(const GLubyte * c, const GLfloat * v){
	LOG("Called unimplemted stub Color4ubVertex2fvSUN!\n");
}

//1004
extern "C" void glColor4ubVertex3fSUN(GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z){
	pushOp(1004);
	pushParam(r);
	pushParam(g);
	pushParam(b);
	pushParam(a);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1005
extern "C" void glColor4ubVertex3fvSUN(const GLubyte * c, const GLfloat * v){
	LOG("Called unimplemted stub Color4ubVertex3fvSUN!\n");
}

//1006
extern "C" void glColor3fVertex3fSUN(GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z){
	pushOp(1006);
	pushParam(r);
	pushParam(g);
	pushParam(b);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1007
extern "C" void glColor3fVertex3fvSUN(const GLfloat * c, const GLfloat * v){
	LOG("Called unimplemted stub Color3fVertex3fvSUN!\n");
}

//1008
extern "C" void glNormal3fVertex3fSUN(GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z){
	pushOp(1008);
	pushParam(nx);
	pushParam(ny);
	pushParam(nz);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1009
extern "C" void glNormal3fVertex3fvSUN(const GLfloat * n, const GLfloat * v){
	LOG("Called unimplemted stub Normal3fVertex3fvSUN!\n");
}

//1010
extern "C" void glColor4fNormal3fVertex3fSUN(GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z){
	pushOp(1010);
	pushParam(r);
	pushParam(g);
	pushParam(b);
	pushParam(a);
	pushParam(nx);
	pushParam(ny);
	pushParam(nz);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1011
extern "C" void glColor4fNormal3fVertex3fvSUN(const GLfloat * c, const GLfloat * n, const GLfloat * v){
	LOG("Called unimplemted stub Color4fNormal3fVertex3fvSUN!\n");
}

//1012
extern "C" void glTexCoord2fVertex3fSUN(GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z){
	pushOp(1012);
	pushParam(s);
	pushParam(t);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1013
extern "C" void glTexCoord2fVertex3fvSUN(const GLfloat * tc, const GLfloat * v){
	LOG("Called unimplemted stub TexCoord2fVertex3fvSUN!\n");
}

//1014
extern "C" void glTexCoord4fVertex4fSUN(GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat x, GLfloat y, GLfloat z, GLfloat w){
	pushOp(1014);
	pushParam(s);
	pushParam(t);
	pushParam(p);
	pushParam(q);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//1015
extern "C" void glTexCoord4fVertex4fvSUN(const GLfloat * tc, const GLfloat * v){
	LOG("Called unimplemted stub TexCoord4fVertex4fvSUN!\n");
}

//1016
extern "C" void glTexCoord2fColor4ubVertex3fSUN(GLfloat s, GLfloat t, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z){
	pushOp(1016);
	pushParam(s);
	pushParam(t);
	pushParam(r);
	pushParam(g);
	pushParam(b);
	pushParam(a);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1017
extern "C" void glTexCoord2fColor4ubVertex3fvSUN(const GLfloat * tc, const GLubyte * c, const GLfloat * v){
	LOG("Called unimplemted stub TexCoord2fColor4ubVertex3fvSUN!\n");
}

//1018
extern "C" void glTexCoord2fColor3fVertex3fSUN(GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z){
	pushOp(1018);
	pushParam(s);
	pushParam(t);
	pushParam(r);
	pushParam(g);
	pushParam(b);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1019
extern "C" void glTexCoord2fColor3fVertex3fvSUN(const GLfloat * tc, const GLfloat * c, const GLfloat * v){
	LOG("Called unimplemted stub TexCoord2fColor3fVertex3fvSUN!\n");
}

//1020
extern "C" void glTexCoord2fNormal3fVertex3fSUN(GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z){
	pushOp(1020);
	pushParam(s);
	pushParam(t);
	pushParam(nx);
	pushParam(ny);
	pushParam(nz);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1021
extern "C" void glTexCoord2fNormal3fVertex3fvSUN(const GLfloat * tc, const GLfloat * n, const GLfloat * v){
	LOG("Called unimplemted stub TexCoord2fNormal3fVertex3fvSUN!\n");
}

//1022
extern "C" void glTexCoord2fColor4fNormal3fVertex3fSUN(GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z){
	pushOp(1022);
	pushParam(s);
	pushParam(t);
	pushParam(r);
	pushParam(g);
	pushParam(b);
	pushParam(a);
	pushParam(nx);
	pushParam(ny);
	pushParam(nz);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1023
extern "C" void glTexCoord2fColor4fNormal3fVertex3fvSUN(const GLfloat * tc, const GLfloat * c, const GLfloat * n, const GLfloat * v){
	LOG("Called unimplemted stub TexCoord2fColor4fNormal3fVertex3fvSUN!\n");
}

//1024
extern "C" void glTexCoord4fColor4fNormal3fVertex4fSUN(GLfloat s, GLfloat t, GLfloat p, GLfloat q, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z, GLfloat w){
	pushOp(1024);
	pushParam(s);
	pushParam(t);
	pushParam(p);
	pushParam(q);
	pushParam(r);
	pushParam(g);
	pushParam(b);
	pushParam(a);
	pushParam(nx);
	pushParam(ny);
	pushParam(nz);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//1025
extern "C" void glTexCoord4fColor4fNormal3fVertex4fvSUN(const GLfloat * tc, const GLfloat * c, const GLfloat * n, const GLfloat * v){
	LOG("Called unimplemted stub TexCoord4fColor4fNormal3fVertex4fvSUN!\n");
}

//1026
extern "C" void glReplacementCodeuiVertex3fSUN(GLuint rc, GLfloat x, GLfloat y, GLfloat z){
	pushOp(1026);
	pushParam(rc);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1027
extern "C" void glReplacementCodeuiVertex3fvSUN(const GLuint * rc, const GLfloat * v){
	LOG("Called unimplemted stub ReplacementCodeuiVertex3fvSUN!\n");
}

//1028
extern "C" void glReplacementCodeuiColor4ubVertex3fSUN(GLuint rc, GLubyte r, GLubyte g, GLubyte b, GLubyte a, GLfloat x, GLfloat y, GLfloat z){
	pushOp(1028);
	pushParam(rc);
	pushParam(r);
	pushParam(g);
	pushParam(b);
	pushParam(a);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1029
extern "C" void glReplacementCodeuiColor4ubVertex3fvSUN(const GLuint * rc, const GLubyte * c, const GLfloat * v){
	LOG("Called unimplemted stub ReplacementCodeuiColor4ubVertex3fvSUN!\n");
}

//1030
extern "C" void glReplacementCodeuiColor3fVertex3fSUN(GLuint rc, GLfloat r, GLfloat g, GLfloat b, GLfloat x, GLfloat y, GLfloat z){
	pushOp(1030);
	pushParam(rc);
	pushParam(r);
	pushParam(g);
	pushParam(b);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1031
extern "C" void glReplacementCodeuiColor3fVertex3fvSUN(const GLuint * rc, const GLfloat * c, const GLfloat * v){
	LOG("Called unimplemted stub ReplacementCodeuiColor3fVertex3fvSUN!\n");
}

//1032
extern "C" void glReplacementCodeuiNormal3fVertex3fSUN(GLuint rc, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z){
	pushOp(1032);
	pushParam(rc);
	pushParam(nx);
	pushParam(ny);
	pushParam(nz);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1033
extern "C" void glReplacementCodeuiNormal3fVertex3fvSUN(const GLuint * rc, const GLfloat * n, const GLfloat * v){
	LOG("Called unimplemted stub ReplacementCodeuiNormal3fVertex3fvSUN!\n");
}

//1034
extern "C" void glReplacementCodeuiColor4fNormal3fVertex3fSUN(GLuint rc, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z){
	pushOp(1034);
	pushParam(rc);
	pushParam(r);
	pushParam(g);
	pushParam(b);
	pushParam(a);
	pushParam(nx);
	pushParam(ny);
	pushParam(nz);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1035
extern "C" void glReplacementCodeuiColor4fNormal3fVertex3fvSUN(const GLuint * rc, const GLfloat * c, const GLfloat * n, const GLfloat * v){
	LOG("Called unimplemted stub ReplacementCodeuiColor4fNormal3fVertex3fvSUN!\n");
}

//1036
extern "C" void glReplacementCodeuiTexCoord2fVertex3fSUN(GLuint rc, GLfloat s, GLfloat t, GLfloat x, GLfloat y, GLfloat z){
	pushOp(1036);
	pushParam(rc);
	pushParam(s);
	pushParam(t);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1037
extern "C" void glReplacementCodeuiTexCoord2fVertex3fvSUN(const GLuint * rc, const GLfloat * tc, const GLfloat * v){
	LOG("Called unimplemted stub ReplacementCodeuiTexCoord2fVertex3fvSUN!\n");
}

//1038
extern "C" void glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN(GLuint rc, GLfloat s, GLfloat t, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z){
	pushOp(1038);
	pushParam(rc);
	pushParam(s);
	pushParam(t);
	pushParam(nx);
	pushParam(ny);
	pushParam(nz);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1039
extern "C" void glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN(const GLuint * rc, const GLfloat * tc, const GLfloat * n, const GLfloat * v){
	LOG("Called unimplemted stub ReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN!\n");
}

//1040
extern "C" void glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN(GLuint rc, GLfloat s, GLfloat t, GLfloat r, GLfloat g, GLfloat b, GLfloat a, GLfloat nx, GLfloat ny, GLfloat nz, GLfloat x, GLfloat y, GLfloat z){
	pushOp(1040);
	pushParam(rc);
	pushParam(s);
	pushParam(t);
	pushParam(r);
	pushParam(g);
	pushParam(b);
	pushParam(a);
	pushParam(nx);
	pushParam(ny);
	pushParam(nz);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1041
extern "C" void glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN(const GLuint * rc, const GLfloat * tc, const GLfloat * c, const GLfloat * n, const GLfloat * v){
	LOG("Called unimplemted stub ReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN!\n");
}

//1042
extern "C" void glBlendFuncSeparateEXT(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha){
	pushOp(1042);
	pushParam(sfactorRGB);
	pushParam(dfactorRGB);
	pushParam(sfactorAlpha);
	pushParam(dfactorAlpha);
}

//1043
extern "C" void glVertexWeightfEXT(GLfloat weight){
	pushOp(1043);
	pushParam(weight);
}

//1044
extern "C" void glVertexWeightfvEXT(const GLfloat * weight){
	pushOp(1044);
	pushBuf(weight, sizeof(const GLfloat) * 1);
}

//1045
extern "C" void glVertexWeightPointerEXT(GLsizei size, GLenum type, GLsizei stride, const GLvoid * pointer){
	LOG("Called untested stub VertexWeightPointerEXT!\n");
	pushOp(1045);
	pushParam(size);
	pushParam(type);
	pushParam(stride);
	pushBuf(pointer, getTypeSize(type) * (size + stride));
}

//1046
extern "C" void glFlushVertexArrayRangeNV(){
	pushOp(1046);
}

//1047
extern "C" void glVertexArrayRangeNV(GLsizei length, const GLvoid * pointer){
	LOG("Called untested stub VertexArrayRangeNV!\n");
	pushOp(1047);
	pushParam(length);
	pushBuf(pointer, length);
}

//1048
extern "C" void glCombinerParameterfvNV(GLenum pname, const GLfloat * params){
	LOG("Called unimplemted stub CombinerParameterfvNV!\n");
}

//1049
extern "C" void glCombinerParameterfNV(GLenum pname, GLfloat param){
	pushOp(1049);
	pushParam(pname);
	pushParam(param);
}

//1050
extern "C" void glCombinerParameterivNV(GLenum pname, const GLint * params){
	LOG("Called unimplemted stub CombinerParameterivNV!\n");
}

//1051
extern "C" void glCombinerParameteriNV(GLenum pname, GLint param){
	pushOp(1051);
	pushParam(pname);
	pushParam(param);
}

//1052
extern "C" void glCombinerInputNV(GLenum stage, GLenum portion, GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage){
	pushOp(1052);
	pushParam(stage);
	pushParam(portion);
	pushParam(variable);
	pushParam(input);
	pushParam(mapping);
	pushParam(componentUsage);
}

//1053
extern "C" void glCombinerOutputNV(GLenum stage, GLenum portion, GLenum abOutput, GLenum cdOutput, GLenum sumOutput, GLenum scale, GLenum bias, GLboolean abDotProduct, GLboolean cdDotProduct, GLboolean muxSum){
	pushOp(1053);
	pushParam(stage);
	pushParam(portion);
	pushParam(abOutput);
	pushParam(cdOutput);
	pushParam(sumOutput);
	pushParam(scale);
	pushParam(bias);
	pushParam(abDotProduct);
	pushParam(cdDotProduct);
	pushParam(muxSum);
}

//1054
extern "C" void glFinalCombinerInputNV(GLenum variable, GLenum input, GLenum mapping, GLenum componentUsage){
	pushOp(1054);
	pushParam(variable);
	pushParam(input);
	pushParam(mapping);
	pushParam(componentUsage);
}

//1055
extern "C" void glGetCombinerInputParameterfvNV(GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetCombinerInputParameterfvNV!\n");
}

//1056
extern "C" void glGetCombinerInputParameterivNV(GLenum stage, GLenum portion, GLenum variable, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetCombinerInputParameterivNV!\n");
}

//1057
extern "C" void glGetCombinerOutputParameterfvNV(GLenum stage, GLenum portion, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetCombinerOutputParameterfvNV!\n");
}

//1058
extern "C" void glGetCombinerOutputParameterivNV(GLenum stage, GLenum portion, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetCombinerOutputParameterivNV!\n");
}

//1059
extern "C" void glGetFinalCombinerInputParameterfvNV(GLenum variable, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetFinalCombinerInputParameterfvNV!\n");
}

//1060
extern "C" void glGetFinalCombinerInputParameterivNV(GLenum variable, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetFinalCombinerInputParameterivNV!\n");
}

//1061
extern "C" void glResizeBuffersMESA(){
	pushOp(1061);
}

//1062
extern "C" void glWindowPos2dMESA(GLdouble x, GLdouble y){
	pushOp(1062);
	pushParam(x);
	pushParam(y);
}

//1063
extern "C" void glWindowPos2dvMESA(const GLdouble * v){
	pushOp(1063);
	pushBuf(v, sizeof(const GLdouble) * 2);
}

//1064
extern "C" void glWindowPos2fMESA(GLfloat x, GLfloat y){
	pushOp(1064);
	pushParam(x);
	pushParam(y);
}

//1065
extern "C" void glWindowPos2fvMESA(const GLfloat * v){
	pushOp(1065);
	pushBuf(v, sizeof(const GLfloat) * 2);
}

//1066
extern "C" void glWindowPos2iMESA(GLint x, GLint y){
	pushOp(1066);
	pushParam(x);
	pushParam(y);
}

//1067
extern "C" void glWindowPos2ivMESA(const GLint * v){
	pushOp(1067);
	pushBuf(v, sizeof(const GLint) * 2);
}

//1068
extern "C" void glWindowPos2sMESA(GLshort x, GLshort y){
	pushOp(1068);
	pushParam(x);
	pushParam(y);
}

//1069
extern "C" void glWindowPos2svMESA(const GLshort * v){
	pushOp(1069);
	pushBuf(v, sizeof(const GLshort) * 2);
}

//1070
extern "C" void glWindowPos3dMESA(GLdouble x, GLdouble y, GLdouble z){
	pushOp(1070);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1071
extern "C" void glWindowPos3dvMESA(const GLdouble * v){
	pushOp(1071);
	pushBuf(v, sizeof(const GLdouble) * 3);
}

//1072
extern "C" void glWindowPos3fMESA(GLfloat x, GLfloat y, GLfloat z){
	pushOp(1072);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1073
extern "C" void glWindowPos3fvMESA(const GLfloat * v){
	pushOp(1073);
	pushBuf(v, sizeof(const GLfloat) * 3);
}

//1074
extern "C" void glWindowPos3iMESA(GLint x, GLint y, GLint z){
	pushOp(1074);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1075
extern "C" void glWindowPos3ivMESA(const GLint * v){
	pushOp(1075);
	pushBuf(v, sizeof(const GLint) * 3);
}

//1076
extern "C" void glWindowPos3sMESA(GLshort x, GLshort y, GLshort z){
	pushOp(1076);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1077
extern "C" void glWindowPos3svMESA(const GLshort * v){
	pushOp(1077);
	pushBuf(v, sizeof(const GLshort) * 3);
}

//1078
extern "C" void glWindowPos4dMESA(GLdouble x, GLdouble y, GLdouble z, GLdouble w){
	pushOp(1078);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//1079
extern "C" void glWindowPos4dvMESA(const GLdouble * v){
	pushOp(1079);
	pushBuf(v, sizeof(const GLdouble) * 4);
}

//1080
extern "C" void glWindowPos4fMESA(GLfloat x, GLfloat y, GLfloat z, GLfloat w){
	pushOp(1080);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//1081
extern "C" void glWindowPos4fvMESA(const GLfloat * v){
	pushOp(1081);
	pushBuf(v, sizeof(const GLfloat) * 4);
}

//1082
extern "C" void glWindowPos4iMESA(GLint x, GLint y, GLint z, GLint w){
	pushOp(1082);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//1083
extern "C" void glWindowPos4ivMESA(const GLint * v){
	pushOp(1083);
	pushBuf(v, sizeof(const GLint) * 4);
}

//1084
extern "C" void glWindowPos4sMESA(GLshort x, GLshort y, GLshort z, GLshort w){
	pushOp(1084);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//1085
extern "C" void glWindowPos4svMESA(const GLshort * v){
	pushOp(1085);
	pushBuf(v, sizeof(const GLshort) * 4);
}

//1086
extern "C" void glMultiModeDrawArraysIBM(const GLenum * mode, const GLint * first, const GLsizei * count, GLsizei primcount, GLint modestride){
	LOG("Called untested stub MultiModeDrawArraysIBM!\n");
	pushOp(1086);
	pushBuf(mode, sizeof(GLenum) * (primcount + modestride));
	pushBuf(first, sizeof(GLenum) * primcount);
	pushBuf(count, sizeof(GLenum) * primcount);
	pushParam(primcount);
	pushParam(modestride);
}

//1087
extern "C" void glMultiModeDrawElementsIBM(const GLenum * mode, const GLsizei * count, GLenum type, const GLvoid * const * indices, GLsizei primcount, GLint modestride){
	LOG("Called unimplemted stub MultiModeDrawElementsIBM!\n");
}

//1088
extern "C" void glColorPointerListIBM(GLint size, GLenum type, GLint stride, const GLvoid ** pointer, GLint ptrstride){
	LOG("Called unimplemted stub ColorPointerListIBM!\n");
}

//1089
extern "C" void glSecondaryColorPointerListIBM(GLint size, GLenum type, GLint stride, const GLvoid ** pointer, GLint ptrstride){
	LOG("Called unimplemted stub SecondaryColorPointerListIBM!\n");
}

//1090
extern "C" void glEdgeFlagPointerListIBM(GLint stride, const GLboolean ** pointer, GLint ptrstride){
	LOG("Called unimplemted stub EdgeFlagPointerListIBM!\n");
}

//1091
extern "C" void glFogCoordPointerListIBM(GLenum type, GLint stride, const GLvoid ** pointer, GLint ptrstride){
	LOG("Called unimplemted stub FogCoordPointerListIBM!\n");
}

//1092
extern "C" void glIndexPointerListIBM(GLenum type, GLint stride, const GLvoid ** pointer, GLint ptrstride){
	LOG("Called unimplemted stub IndexPointerListIBM!\n");
}

//1093
extern "C" void glNormalPointerListIBM(GLenum type, GLint stride, const GLvoid ** pointer, GLint ptrstride){
	LOG("Called unimplemted stub NormalPointerListIBM!\n");
}

//1094
extern "C" void glTexCoordPointerListIBM(GLint size, GLenum type, GLint stride, const GLvoid ** pointer, GLint ptrstride){
	LOG("Called unimplemted stub TexCoordPointerListIBM!\n");
}

//1095
extern "C" void glVertexPointerListIBM(GLint size, GLenum type, GLint stride, const GLvoid ** pointer, GLint ptrstride){
	LOG("Called unimplemted stub VertexPointerListIBM!\n");
}

//1096
extern "C" void glTbufferMask3DFX(GLuint mask){
	pushOp(1096);
	pushParam(mask);
}

//1097
extern "C" void glSampleMaskEXT(GLclampf value, GLboolean invert){
	pushOp(1097);
	pushParam(value);
	pushParam(invert);
}

//1098
extern "C" void glSamplePatternEXT(GLenum pattern){
	pushOp(1098);
	pushParam(pattern);
}

//1099
extern "C" void glTextureColorMaskSGIS(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha){
	pushOp(1099);
	pushParam(red);
	pushParam(green);
	pushParam(blue);
	pushParam(alpha);
}

//1100
extern "C" void glDeleteFencesNV(GLsizei n, const GLuint * fences){
	LOG("Called untested stub DeleteFencesNV!\n");
	pushOp(1100);
	pushParam(n);
	pushBuf(fences, sizeof(GLuint) * n);
}

//1101
extern "C" void glGenFencesNV(GLsizei n, GLuint * fences){
	pushOp(1101);
	pushParam(n);
	pushBuf(fences, sizeof(GLuint) * n);
}

//1102
extern "C" GLboolean glIsFenceNV(GLuint fence){
	pushOp(1102);
	pushParam(fence);

	GLboolean ret;
	pushBuf(&ret, sizeof(GLboolean), true);
	waitForReturn();

	return ret;
}

//1103
extern "C" GLboolean glTestFenceNV(GLuint fence){
	pushOp(1103);
	pushParam(fence);

	GLboolean ret;
	pushBuf(&ret, sizeof(GLboolean), true);
	waitForReturn();

	return ret;
}

//1104
extern "C" void glGetFenceivNV(GLuint fence, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetFenceivNV!\n");
}

//1105
extern "C" void glFinishFenceNV(GLuint fence){
	pushOp(1105);
	pushParam(fence);
}

//1106
extern "C" void glSetFenceNV(GLuint fence, GLenum condition){
	pushOp(1106);
	pushParam(fence);
	pushParam(condition);
}

//1107
extern "C" void glMapControlPointsNV(GLenum target, GLuint index, GLenum type, GLsizei ustride, GLsizei vstride, GLint uorder, GLint vorder, GLboolean packed, const GLvoid * points){
	LOG("Called untested stub MapControlPointsNV!\n");
	pushOp(1107);
	pushParam(target);
	pushParam(index);
	pushParam(type);
	pushParam(ustride);
	pushParam(vstride);
	pushParam(uorder);
	pushParam(vorder);
	pushParam(packed);
	pushBuf(points, getTypeSize(type) * (uorder + ustride) * (vorder + vstride));

}

//1108
extern "C" void glMapParameterivNV(GLenum target, GLenum pname, const GLint * params){
	LOG("Called unimplemted stub MapParameterivNV!\n");
}

//1109
extern "C" void glMapParameterfvNV(GLenum target, GLenum pname, const GLfloat * params){
	LOG("Called unimplemted stub MapParameterfvNV!\n");
}

//1110
extern "C" void glGetMapControlPointsNV(GLenum target, GLuint index, GLenum type, GLsizei ustride, GLsizei vstride, GLboolean packed, GLvoid * points){
	LOG("Called unimplemted stub GetMapControlPointsNV!\n");
	//pushOp(1107);
	//pushParam(target);
	//pushParam(index);
	//pushParam(type);
	//pushParam(ustride);
	//pushParam(vstride);
	//pushParam(packed);
	//pushBuf(points, getTypeSize(type) * (/*size*/ + ustride) * ((/*size*/ + vstride), true);
}

//1111
extern "C" void glGetMapParameterivNV(GLenum target, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetMapParameterivNV!\n");
}

//1112
extern "C" void glGetMapParameterfvNV(GLenum target, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetMapParameterfvNV!\n");
}

//1113
extern "C" void glGetMapAttribParameterivNV(GLenum target, GLuint index, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetMapAttribParameterivNV!\n");
}

//1114
extern "C" void glGetMapAttribParameterfvNV(GLenum target, GLuint index, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetMapAttribParameterfvNV!\n");
}

//1115
extern "C" void glEvalMapsNV(GLenum target, GLenum mode){
	pushOp(1115);
	pushParam(target);
	pushParam(mode);
}

//1116
extern "C" void glCombinerStageParameterfvNV(GLenum stage, GLenum pname, const GLfloat * params){
	LOG("Called unimplemted stub CombinerStageParameterfvNV!\n");
}

//1117
extern "C" void glGetCombinerStageParameterfvNV(GLenum stage, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetCombinerStageParameterfvNV!\n");
}

//1118
extern "C" GLboolean glAreProgramsResidentNV(GLsizei n, const GLuint * ids, GLboolean * residences){
	pushOp(1118);
	pushParam(n);
	pushBuf(ids, sizeof(const GLuint) * n);
	pushBuf(residences, sizeof(GLboolean) * n);

	GLboolean ret;
	pushBuf(&ret, sizeof(GLboolean), true);
	waitForReturn();

	return ret;
}

//1119
extern "C" void glBindProgramNV(GLenum target, GLuint program){
	pushOp(1119);
	pushParam(target);
	pushParam(program);
}

//1120
extern "C" void glDeleteProgramsNV(GLsizei n, const GLuint * programs){
	pushOp(1120);
	pushParam(n);
	pushBuf(programs, sizeof(const GLuint) * n);
}

//1121
extern "C" void glExecuteProgramNV(GLenum target, GLuint id, const GLfloat * params){
	pushOp(1121);
	pushParam(target);
	pushParam(id);
	pushBuf(params, sizeof(const GLfloat) * 4);
}

//1122
extern "C" void glGenProgramsNV(GLsizei n, GLuint * programs){
	pushOp(1122);
	pushParam(n);
	pushBuf(programs, sizeof(GLuint) * n);
}

//1123
extern "C" void glGetProgramParameterdvNV(GLenum target, GLuint index, GLenum pname, GLdouble * params){
	pushOp(1123);
	pushParam(target);
	pushParam(index);
	pushParam(pname);
	pushBuf(params, sizeof(GLdouble) * 4);
}

//1124
extern "C" void glGetProgramParameterfvNV(GLenum target, GLuint index, GLenum pname, GLfloat * params){
	pushOp(1124);
	pushParam(target);
	pushParam(index);
	pushParam(pname);
	pushBuf(params, sizeof(GLfloat) * 4);
}

//1125
extern "C" void glGetProgramivNV(GLuint id, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetProgramivNV!\n");
}

//1126
extern "C" void glGetProgramStringNV(GLuint id, GLenum pname, GLubyte * program){
	LOG("Called unimplemted stub GetProgramStringNV!\n");
}

//1127
extern "C" void glGetTrackMatrixivNV(GLenum target, GLuint address, GLenum pname, GLint * params){
	pushOp(1127);
	pushParam(target);
	pushParam(address);
	pushParam(pname);
	pushBuf(params, sizeof(GLint) * 1);
}

//1128
extern "C" void glGetVertexAttribdvNV(GLuint index, GLenum pname, GLdouble * params){
	LOG("Called unimplemted stub GetVertexAttribdvNV!\n");
}

//1129
extern "C" void glGetVertexAttribfvNV(GLuint index, GLenum pname, GLfloat * params){
	LOG("Called unimplemted stub GetVertexAttribfvNV!\n");
}

//1130
extern "C" void glGetVertexAttribivNV(GLuint index, GLenum pname, GLint * params){
	LOG("Called unimplemted stub GetVertexAttribivNV!\n");
}

//1131
extern "C" void glGetVertexAttribPointervNV(GLuint index, GLenum pname, GLvoid ** pointer){
	LOG("Called unimplemted stub GetVertexAttribPointervNV!\n");
}

//1132
extern "C" GLboolean glIsProgramNV(GLuint program){
	pushOp(1132);
	pushParam(program);

	GLboolean ret;
	pushBuf(&ret, sizeof(GLboolean), true);
	waitForReturn();

	return ret;
}

//1133
extern "C" void glLoadProgramNV(GLenum target, GLuint id, GLsizei len, const GLubyte * program){
	pushOp(1133);
	pushParam(target);
	pushParam(id);
	pushParam(len);
	pushBuf(program, sizeof(const GLubyte) * len);
}

//1134
extern "C" void glProgramParameter4dNV(GLenum target, GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w){
	pushOp(1134);
	pushParam(target);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//1135
extern "C" void glProgramParameter4dvNV(GLenum target, GLuint index, const GLdouble * params){
	LOG("Called unimplemted stub ProgramParameter4dvNV!\n");
}

//1136
extern "C" void glProgramParameter4fNV(GLenum target, GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w){
	pushOp(1136);
	pushParam(target);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//1137
extern "C" void glProgramParameter4fvNV(GLenum target, GLuint index, const GLfloat * params){
	LOG("Called unimplemted stub ProgramParameter4fvNV!\n");
}

//1138
extern "C" void glProgramParameters4dvNV(GLenum target, GLuint index, GLuint num, const GLdouble * params){
	pushOp(1138);
	pushParam(target);
	pushParam(index);
	pushParam(num);
	pushBuf(params, sizeof(const GLdouble) * num);
}

//1139
extern "C" void glProgramParameters4fvNV(GLenum target, GLuint index, GLuint num, const GLfloat * params){
	pushOp(1139);
	pushParam(target);
	pushParam(index);
	pushParam(num);
	pushBuf(params, sizeof(const GLfloat) * num);
}

//1140
extern "C" void glRequestResidentProgramsNV(GLsizei n, const GLuint * ids){
	pushOp(1140);
	pushParam(n);
	pushBuf(ids, sizeof(const GLuint) * n);
}

//1141
extern "C" void glTrackMatrixNV(GLenum target, GLuint address, GLenum matrix, GLenum transform){
	pushOp(1141);
	pushParam(target);
	pushParam(address);
	pushParam(matrix);
	pushParam(transform);
}

//1142
extern "C" void glVertexAttribPointerNV(GLuint index, GLint size, GLenum type, GLsizei stride, const GLvoid * pointer){
	LOG("Called unimplemted stub VertexAttribPointerNV!\n");
}

//1143
extern "C" void glVertexAttrib1sNV(GLuint index, GLshort x){
	pushOp(1143);
	pushParam(index);
	pushParam(x);
}

//1144
extern "C" void glVertexAttrib1svNV(GLuint index, const GLshort * v){
	pushOp(1144);
	pushParam(index);
	pushBuf(v, sizeof(const GLshort) * 1);
}

//1145
extern "C" void glVertexAttrib2sNV(GLuint index, GLshort x, GLshort y){
	pushOp(1145);
	pushParam(index);
	pushParam(x);
	pushParam(y);
}

//1146
extern "C" void glVertexAttrib2svNV(GLuint index, const GLshort * v){
	pushOp(1146);
	pushParam(index);
	pushBuf(v, sizeof(const GLshort) * 2);
}

//1147
extern "C" void glVertexAttrib3sNV(GLuint index, GLshort x, GLshort y, GLshort z){
	pushOp(1147);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1148
extern "C" void glVertexAttrib3svNV(GLuint index, const GLshort * v){
	pushOp(1148);
	pushParam(index);
	pushBuf(v, sizeof(const GLshort) * 3);
}

//1149
extern "C" void glVertexAttrib4sNV(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w){
	pushOp(1149);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//1150
extern "C" void glVertexAttrib4svNV(GLuint index, const GLshort * v){
	pushOp(1150);
	pushParam(index);
	pushBuf(v, sizeof(const GLshort) * 4);
}

//1151
extern "C" void glVertexAttrib1fNV(GLuint index, GLfloat x){
	pushOp(1151);
	pushParam(index);
	pushParam(x);
}

//1152
extern "C" void glVertexAttrib1fvNV(GLuint index, const GLfloat * v){
	pushOp(1152);
	pushParam(index);
	pushBuf(v, sizeof(const GLfloat) * 1);
}

//1153
extern "C" void glVertexAttrib2fNV(GLuint index, GLfloat x, GLfloat y){
	pushOp(1153);
	pushParam(index);
	pushParam(x);
	pushParam(y);
}

//1154
extern "C" void glVertexAttrib2fvNV(GLuint index, const GLfloat * v){
	pushOp(1154);
	pushParam(index);
	pushBuf(v, sizeof(const GLfloat) * 2);
}

//1155
extern "C" void glVertexAttrib3fNV(GLuint index, GLfloat x, GLfloat y, GLfloat z){
	pushOp(1155);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1156
extern "C" void glVertexAttrib3fvNV(GLuint index, const GLfloat * v){
	pushOp(1156);
	pushParam(index);
	pushBuf(v, sizeof(const GLfloat) * 3);
}

//1157
extern "C" void glVertexAttrib4fNV(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w){
	pushOp(1157);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//1158
extern "C" void glVertexAttrib4fvNV(GLuint index, const GLfloat * v){
	pushOp(1158);
	pushParam(index);
	pushBuf(v, sizeof(const GLfloat) * 4);
}

//1159
extern "C" void glVertexAttrib1dNV(GLuint index, GLdouble x){
	pushOp(1159);
	pushParam(index);
	pushParam(x);
}

//1160
extern "C" void glVertexAttrib1dvNV(GLuint index, const GLdouble * v){
	pushOp(1160);
	pushParam(index);
	pushBuf(v, sizeof(const GLdouble) * 1);
}

//1161
extern "C" void glVertexAttrib2dNV(GLuint index, GLdouble x, GLdouble y){
	pushOp(1161);
	pushParam(index);
	pushParam(x);
	pushParam(y);
}

//1162
extern "C" void glVertexAttrib2dvNV(GLuint index, const GLdouble * v){
	pushOp(1162);
	pushParam(index);
	pushBuf(v, sizeof(const GLdouble) * 2);
}

//1163
extern "C" void glVertexAttrib3dNV(GLuint index, GLdouble x, GLdouble y, GLdouble z){
	pushOp(1163);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
}

//1164
extern "C" void glVertexAttrib3dvNV(GLuint index, const GLdouble * v){
	pushOp(1164);
	pushParam(index);
	pushBuf(v, sizeof(const GLdouble) * 3);
}

//1165
extern "C" void glVertexAttrib4dNV(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w){
	pushOp(1165);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//1166
extern "C" void glVertexAttrib4dvNV(GLuint index, const GLdouble * v){
	pushOp(1166);
	pushParam(index);
	pushBuf(v, sizeof(const GLdouble) * 4);
}

//1167
extern "C" void glVertexAttrib4ubNV(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w){
	pushOp(1167);
	pushParam(index);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//1168
extern "C" void glVertexAttrib4ubvNV(GLuint index, const GLubyte * v){
	pushOp(1168);
	pushParam(index);
	pushBuf(v, sizeof(const GLubyte) * 4);
}

//1169
extern "C" void glVertexAttribs1svNV(GLuint index, GLsizei n, const GLshort * v){
	pushOp(1169);
	pushParam(index);
	pushParam(n);
	pushBuf(v, sizeof(const GLshort) * n);
}

//1170
extern "C" void glVertexAttribs2svNV(GLuint index, GLsizei n, const GLshort * v){
	pushOp(1170);
	pushParam(index);
	pushParam(n);
	pushBuf(v, sizeof(const GLshort) * n);
}

//1171
extern "C" void glVertexAttribs3svNV(GLuint index, GLsizei n, const GLshort * v){
	pushOp(1171);
	pushParam(index);
	pushParam(n);
	pushBuf(v, sizeof(const GLshort) * n);
}

//1172
extern "C" void glVertexAttribs4svNV(GLuint index, GLsizei n, const GLshort * v){
	pushOp(1172);
	pushParam(index);
	pushParam(n);
	pushBuf(v, sizeof(const GLshort) * n);
}

//1173
extern "C" void glVertexAttribs1fvNV(GLuint index, GLsizei n, const GLfloat * v){
	pushOp(1173);
	pushParam(index);
	pushParam(n);
	pushBuf(v, sizeof(const GLfloat) * n);
}

//1174
extern "C" void glVertexAttribs2fvNV(GLuint index, GLsizei n, const GLfloat * v){
	pushOp(1174);
	pushParam(index);
	pushParam(n);
	pushBuf(v, sizeof(const GLfloat) * n);
}

//1175
extern "C" void glVertexAttribs3fvNV(GLuint index, GLsizei n, const GLfloat * v){
	pushOp(1175);
	pushParam(index);
	pushParam(n);
	pushBuf(v, sizeof(const GLfloat) * n);
}

//1176
extern "C" void glVertexAttribs4fvNV(GLuint index, GLsizei n, const GLfloat * v){
	pushOp(1176);
	pushParam(index);
	pushParam(n);
	pushBuf(v, sizeof(const GLfloat) * n);
}

//1177
extern "C" void glVertexAttribs1dvNV(GLuint index, GLsizei n, const GLdouble * v){
	pushOp(1177);
	pushParam(index);
	pushParam(n);
	pushBuf(v, sizeof(const GLdouble) * n);
}

//1178
extern "C" void glVertexAttribs2dvNV(GLuint index, GLsizei n, const GLdouble * v){
	pushOp(1178);
	pushParam(index);
	pushParam(n);
	pushBuf(v, sizeof(const GLdouble) * n);
}

//1179
extern "C" void glVertexAttribs3dvNV(GLuint index, GLsizei n, const GLdouble * v){
	pushOp(1179);
	pushParam(index);
	pushParam(n);
	pushBuf(v, sizeof(const GLdouble) * n);
}

//1180
extern "C" void glVertexAttribs4dvNV(GLuint index, GLsizei n, const GLdouble * v){
	pushOp(1180);
	pushParam(index);
	pushParam(n);
	pushBuf(v, sizeof(const GLdouble) * n);
}

//1181
extern "C" void glVertexAttribs4ubvNV(GLuint index, GLsizei n, const GLubyte * v){
	pushOp(1181);
	pushParam(index);
	pushParam(n);
	pushBuf(v, sizeof(const GLubyte) * n);
}

//1182
extern "C" GLuint glGenFragmentShadersATI(GLuint range){
	pushOp(1182);
	pushParam(range);

	GLuint ret;
	pushBuf(&ret, sizeof(GLuint), true);
	waitForReturn();

	return ret;
}

//1183
extern "C" void glBindFragmentShaderATI(GLuint id){
	pushOp(1183);
	pushParam(id);
}

//1184
extern "C" void glDeleteFragmentShaderATI(GLuint id){
	pushOp(1184);
	pushParam(id);
}

//1185
extern "C" void glBeginFragmentShaderATI(){
	pushOp(1185);
}

//1186
extern "C" void glEndFragmentShaderATI(){
	pushOp(1186);
}

//1187
extern "C" void glPassTexCoordATI(GLuint dst, GLuint coord, GLenum swizzle){
	pushOp(1187);
	pushParam(dst);
	pushParam(coord);
	pushParam(swizzle);
}

//1188
extern "C" void glSampleMapATI(GLuint dst, GLuint interp, GLenum swizzle){
	pushOp(1188);
	pushParam(dst);
	pushParam(interp);
	pushParam(swizzle);
}

//1189
extern "C" void glColorFragmentOp1ATI(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod){
	pushOp(1189);
	pushParam(op);
	pushParam(dst);
	pushParam(dstMask);
	pushParam(dstMod);
	pushParam(arg1);
	pushParam(arg1Rep);
	pushParam(arg1Mod);
}

//1190
extern "C" void glColorFragmentOp2ATI(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod){
	pushOp(1190);
	pushParam(op);
	pushParam(dst);
	pushParam(dstMask);
	pushParam(dstMod);
	pushParam(arg1);
	pushParam(arg1Rep);
	pushParam(arg1Mod);
	pushParam(arg2);
	pushParam(arg2Rep);
	pushParam(arg2Mod);
}

//1191
extern "C" void glColorFragmentOp3ATI(GLenum op, GLuint dst, GLuint dstMask, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod){
	pushOp(1191);
	pushParam(op);
	pushParam(dst);
	pushParam(dstMask);
	pushParam(dstMod);
	pushParam(arg1);
	pushParam(arg1Rep);
	pushParam(arg1Mod);
	pushParam(arg2);
	pushParam(arg2Rep);
	pushParam(arg2Mod);
	pushParam(arg3);
	pushParam(arg3Rep);
	pushParam(arg3Mod);
}

//1192
extern "C" void glAlphaFragmentOp1ATI(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod){
	pushOp(1192);
	pushParam(op);
	pushParam(dst);
	pushParam(dstMod);
	pushParam(arg1);
	pushParam(arg1Rep);
	pushParam(arg1Mod);
}

//1193
extern "C" void glAlphaFragmentOp2ATI(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod){
	pushOp(1193);
	pushParam(op);
	pushParam(dst);
	pushParam(dstMod);
	pushParam(arg1);
	pushParam(arg1Rep);
	pushParam(arg1Mod);
	pushParam(arg2);
	pushParam(arg2Rep);
	pushParam(arg2Mod);
}

//1194
extern "C" void glAlphaFragmentOp3ATI(GLenum op, GLuint dst, GLuint dstMod, GLuint arg1, GLuint arg1Rep, GLuint arg1Mod, GLuint arg2, GLuint arg2Rep, GLuint arg2Mod, GLuint arg3, GLuint arg3Rep, GLuint arg3Mod){
	pushOp(1194);
	pushParam(op);
	pushParam(dst);
	pushParam(dstMod);
	pushParam(arg1);
	pushParam(arg1Rep);
	pushParam(arg1Mod);
	pushParam(arg2);
	pushParam(arg2Rep);
	pushParam(arg2Mod);
	pushParam(arg3);
	pushParam(arg3Rep);
	pushParam(arg3Mod);
}

//1195
extern "C" void glSetFragmentShaderConstantATI(GLuint dst, const GLfloat * value){
	LOG("Called unimplemted stub SetFragmentShaderConstantATI!\n");
}

//1196
extern "C" void glDrawMeshArraysSUN(GLenum mode, GLint first, GLsizei count, GLsizei width){
	pushOp(1196);
	pushParam(mode);
	pushParam(first);
	pushParam(count);
	pushParam(width);
}

//1197
extern "C" void glPointParameteriNV(GLenum pname, GLint param){
	pushOp(1197);
	pushParam(pname);
	pushParam(param);
}

//1198
extern "C" void glPointParameterivNV(GLenum pname, const GLint * params){
	LOG("Called unimplemted stub PointParameterivNV!\n");
}

//1199
extern "C" void glActiveStencilFaceEXT(GLenum face){
	pushOp(1199);
	pushParam(face);
}

//1200
extern "C" void glDrawBuffersATI(GLsizei n, const GLenum * bufs){
	pushOp(1200);
	pushParam(n);
	pushBuf(bufs, sizeof(const GLenum) * n);
}

//1201
extern "C" void glProgramNamedParameter4fNV(GLuint id, GLsizei len, const GLubyte * name, GLfloat x, GLfloat y, GLfloat z, GLfloat w){
	pushOp(1201);
	pushParam(id);
	pushParam(len);
	pushBuf(name, sizeof(const GLubyte) * len);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//1202
extern "C" void glProgramNamedParameter4dNV(GLuint id, GLsizei len, const GLubyte * name, GLdouble x, GLdouble y, GLdouble z, GLdouble w){
	pushOp(1202);
	pushParam(id);
	pushParam(len);
	pushBuf(name, sizeof(const GLubyte) * len);
	pushParam(x);
	pushParam(y);
	pushParam(z);
	pushParam(w);
}

//1203
extern "C" void glProgramNamedParameter4fvNV(GLuint id, GLsizei len, const GLubyte * name, const GLfloat * v){
	pushOp(1203);
	pushParam(id);
	pushParam(len);
	pushBuf(name, sizeof(const GLubyte) * len);
	pushBuf(v, sizeof(const GLfloat) * 4);
}

//1204
extern "C" void glProgramNamedParameter4dvNV(GLuint id, GLsizei len, const GLubyte * name, const GLdouble * v){
	pushOp(1204);
	pushParam(id);
	pushParam(len);
	pushBuf(name, sizeof(const GLubyte) * len);
	pushBuf(v, sizeof(const GLdouble) * 4);
}

//1205
extern "C" void glGetProgramNamedParameterfvNV(GLuint id, GLsizei len, const GLubyte * name, GLfloat * params){
	pushOp(1205);
	pushParam(id);
	pushParam(len);
	pushBuf(name, sizeof(const GLubyte) * len);
	pushBuf(params, sizeof(GLfloat) * 4);
}

//1206
extern "C" void glGetProgramNamedParameterdvNV(GLuint id, GLsizei len, const GLubyte * name, GLdouble * params){
	pushOp(1206);
	pushParam(id);
	pushParam(len);
	pushBuf(name, sizeof(const GLubyte) * len);
	pushBuf(params, sizeof(GLdouble) * 4);
}

//1207
extern "C" void glDepthBoundsEXT(GLclampd zmin, GLclampd zmax){
	pushOp(1207);
	pushParam(zmin);
	pushParam(zmax);
}

//1208
extern "C" void glBlendEquationSeparateEXT(GLenum modeRGB, GLenum modeA){
	pushOp(1208);
	pushParam(modeRGB);
	pushParam(modeA);
}

//1209
extern "C" void glBlitFramebufferEXT(GLint srcX0, GLint srcY0, GLint srcX1, GLint srcY1, GLint dstX0, GLint dstY0, GLint dstX1, GLint dstY1, GLbitfield mask, GLenum filter){
	pushOp(1209);
	pushParam(srcX0);
	pushParam(srcY0);
	pushParam(srcX1);
	pushParam(srcY1);
	pushParam(dstX0);
	pushParam(dstY0);
	pushParam(dstX1);
	pushParam(dstY1);
	pushParam(mask);
	pushParam(filter);
}

//1210
extern "C" void glBlendEquationSeparateATI(GLenum modeRGB, GLenum modeA){
	pushOp(1210);
	pushParam(modeRGB);
	pushParam(modeA);
}

//1211
extern "C" void glStencilOpSeparateATI(GLenum face, GLenum sfail, GLenum zfail, GLenum zpass){
	pushOp(1211);
	pushParam(face);
	pushParam(sfail);
	pushParam(zfail);
	pushParam(zpass);
}

//1212
extern "C" void glStencilFuncSeparateATI(GLenum frontfunc, GLenum backfunc, GLint ref, GLuint mask){
	pushOp(1212);
	pushParam(frontfunc);
	pushParam(backfunc);
	pushParam(ref);
	pushParam(mask);
}

//1213
extern "C" void glProgramEnvParameters4fvEXT(GLenum target, GLuint index, GLsizei count, const GLfloat * params){
	LOG("Called unimplemted stub ProgramEnvParameters4fvEXT!\n");
}

//1214
extern "C" void glProgramLocalParameters4fvEXT(GLenum target, GLuint index, GLsizei count, const GLfloat * params){
	LOG("Called unimplemted stub ProgramLocalParameters4fvEXT!\n");
}

//1215
extern "C" void glGetQueryObjecti64vEXT(GLuint id, GLenum pname, GLint64EXT * params){
	LOG("Called unimplemted stub GetQueryObjecti64vEXT!\n");
}

//1216
extern "C" void glGetQueryObjectui64vEXT(GLuint id, GLenum pname, GLuint64EXT * params){
	LOG("Called unimplemted stub GetQueryObjectui64vEXT!\n");
}

//1217
extern "C" void glBlendFuncSeparateINGR(GLenum sfactorRGB, GLenum dfactorRGB, GLenum sfactorAlpha, GLenum dfactorAlpha){
	pushOp(1217);
	pushParam(sfactorRGB);
	pushParam(dfactorRGB);
	pushParam(sfactorAlpha);
	pushParam(dfactorAlpha);
}

//1218
extern "C" GLhandleARB glCreateDebugObjectMESA(){
	pushOp(1218);

	GLhandleARB ret;
	pushBuf(&ret, sizeof(GLhandleARB), true);
	waitForReturn();

	return ret;
}

//1219
extern "C" void glClearDebugLogMESA(GLhandleARB obj, GLenum logType, GLenum shaderType){
	pushOp(1219);
	pushParam(obj);
	pushParam(logType);
	pushParam(shaderType);
}

//1220
extern "C" void glGetDebugLogMESA(GLhandleARB obj, GLenum logType, GLenum shaderType, GLsizei maxLength, GLsizei * length, GLcharARB * debugLog){
	LOG("Called unimplemted stub GetDebugLogMESA!\n");
}

//1221
extern "C" GLsizei glGetDebugLogLengthMESA(GLhandleARB obj, GLenum logType, GLenum shaderType){
	pushOp(1221);
	pushParam(obj);
	pushParam(logType);
	pushParam(shaderType);

	GLsizei ret;
	pushBuf(&ret, sizeof(GLsizei), true);
	waitForReturn();

	return ret;
}

//1222
extern "C" void glPointParameterfSGIS(GLenum pname, GLfloat param){
	pushOp(1222);
	pushParam(pname);
	pushParam(param);
}

//1223
extern "C" void glPointParameterfvSGIS(GLenum pname, const GLfloat * params){
	LOG("Called unimplemted stub PointParameterfvSGIS!\n");
}

//1224
extern "C" void glIglooInterfaceSGIX(GLenum pname, const GLvoid * params){
	LOG("Called unimplemted stub IglooInterfaceSGIX!\n");
}

//1225
extern "C" void glDeformationMap3dSGIX(GLenum target, GLdouble u1, GLdouble u2, GLint ustride, GLint uorder, GLdouble v1, GLdouble v2, GLint vstride, GLint vorder, GLdouble w1, GLdouble w2, GLint wstride, GLint worder, const GLdouble * points){
	LOG("Called untested stub DeformationMap3dSGIX!\n");
	pushOp(1225);
	pushParam(target);
	pushParam(u1);
	pushParam(u2);
	pushParam(ustride);
	pushParam(uorder);
	pushParam(v1);
	pushParam(v2);
	pushParam(vstride);
	pushParam(vorder);
	pushParam(w1);
	pushParam(w2);
	pushParam(wstride);
	pushParam(worder);
	pushBuf(points, sizeof(GLdouble) * (uorder + ustride) * (vorder + vstride) * (worder + wstride));
}

//1226
extern "C" void glDeformationMap3fSGIX(GLenum target, GLfloat u1, GLfloat u2, GLint ustride, GLint uorder, GLfloat v1, GLfloat v2, GLint vstride, GLint vorder, GLfloat w1, GLfloat w2, GLint wstride, GLint worder, const GLfloat * points){
	LOG("Called untested stub DeformationMap3fSGIX!\n");
	pushOp(1226);
	pushParam(target);
	pushParam(u1);
	pushParam(u2);
	pushParam(ustride);
	pushParam(uorder);
	pushParam(v1);
	pushParam(v2);
	pushParam(vstride);
	pushParam(vorder);
	pushParam(w1);
	pushParam(w2);
	pushParam(wstride);
	pushParam(worder);
	pushBuf(points, sizeof(GLfloat) * (uorder + ustride) * (vorder + vstride) * (worder + wstride));
}

//1227
extern "C" void glDeformSGIX(GLenum mask){
	pushOp(1227);
	pushParam(mask);
}

//1228
extern "C" void glLoadIdentityDeformationMapSGIX(GLenum mask){
	pushOp(1228);
	pushParam(mask);
}

#include "main.h"

typedef void (*TextFunc)(byte *buf);

/*********************************************************
	Text Module Globals
*********************************************************/

static bool hasSetup = false;
static void setupPointers();

static TextFunc mFunctions[1700];

void LOG_INSTRUCTION(Instruction *iter){

	if(!iter){
		LOG("(null instruction)\n");
		return;
	}
	
	if(!hasSetup){
		setupPointers();
	}
	
	if(iter->id < 1500){
		mFunctions[iter->id](iter->args);
	}else{
		LOG("(Invalid instruction %d)\n", iter->id);
	}
}

TextModule::TextModule()
{
	init();
}

/*********************************************************
	Text Module Process Instructions
*********************************************************/

bool TextModule::process(vector<Instruction *> *list)
{

	LOG("*** Start of frame: %d instructions\n", (int)list->size());
	int counter = 0;
	for(int i=0;i<(int)list->size();i++){
		Instruction *iter = (*list)[i];
		counter++;
		if(iter->id < 1500)
			mFunctions[iter->id](iter->args);
	}

	LOG("*** End of frame\n\n");

	return true;
}


bool TextModule::sync()
{
	return true;
}


/*********************************************************
	Text Module CGL Methods
*********************************************************/

static void EXEC_CGLSwapBuffers(byte *commandbuf)
{
	LOG("CGL_SwapBuffers()\n");
}

static void EXEC_CGLRepeat(byte *commandbuf)
{
	uint32_t *i = (uint32_t*)commandbuf;  commandbuf += sizeof(GLuint);
	LOG("CGL_Repeat(%d)\n", *i);
}


/*********************************************************
	Text Module GL Methods
*********************************************************/
//0
static void EXEC_glNewList(byte *commandbuf)
{
	GLuint *list = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glNewList(list=%0.1f, mode=%0.1f)\n", (float)*list, (float)*mode);
}


//1
static void EXEC_glEndList(byte *commandbuf)
{

	LOG("glEndList()\n");
}


//2
static void EXEC_glCallList(byte *commandbuf)
{
	GLuint *list = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	LOG("glCallList(list=%0.1f)\n", (float)*list);
}


//3
static void EXEC_glCallLists(byte *commandbuf)
{

	LOG("glCallLists()\n");

}


//4
static void EXEC_glDeleteLists(byte *commandbuf)
{
	GLuint *list = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLsizei *range = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	LOG("glDeleteLists(list=%0.1f, range=%0.1f)\n", (float)*list, (float)*range);
}


//5
static void EXEC_glGenLists(byte *commandbuf)
{
	GLsizei *range = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	LOG("glGenLists(range=%0.1f)\n", (float)*range);
}


//6
static void EXEC_glListBase(byte *commandbuf)
{
	GLuint *base = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	LOG("glListBase(base=%0.1f)\n", (float)*base);
}


//7
static void EXEC_glBegin(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glBegin(mode=%0.1f)\n", (float)*mode);
}


//8
static void EXEC_glBitmap(byte *commandbuf)
{

	LOG("glBitmap()\n");

}


//9
static void EXEC_glColor3b(byte *commandbuf)
{
	GLbyte *red = (GLbyte*)commandbuf;   commandbuf += sizeof(GLbyte);
	GLbyte *green = (GLbyte*)commandbuf;     commandbuf += sizeof(GLbyte);
	GLbyte *blue = (GLbyte*)commandbuf;  commandbuf += sizeof(GLbyte);

	LOG("glColor3b(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//10
static void EXEC_glColor3bv(byte *commandbuf)
{

	LOG("glColor3bv()\n");

}


//11
static void EXEC_glColor3d(byte *commandbuf)
{
	GLdouble *red = (GLdouble*)commandbuf;   commandbuf += sizeof(GLdouble);
	GLdouble *green = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *blue = (GLdouble*)commandbuf;  commandbuf += sizeof(GLdouble);

	LOG("glColor3d(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//12
static void EXEC_glColor3dv(byte *commandbuf)
{

	LOG("glColor3dv()\n");

}


//13
static void EXEC_glColor3f(byte *commandbuf)
{
	GLfloat *red = (GLfloat*)commandbuf;     commandbuf += sizeof(GLfloat);
	GLfloat *green = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *blue = (GLfloat*)commandbuf;    commandbuf += sizeof(GLfloat);

	LOG("glColor3f(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//14
static void EXEC_glColor3fv(byte *commandbuf)
{

	LOG("glColor3fv()\n");

}


//15
static void EXEC_glColor3i(byte *commandbuf)
{
	GLint *red = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *green = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *blue = (GLint*)commandbuf;    commandbuf += sizeof(GLint);

	LOG("glColor3i(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//16
static void EXEC_glColor3iv(byte *commandbuf)
{

	LOG("glColor3iv()\n");

}


//17
static void EXEC_glColor3s(byte *commandbuf)
{
	GLshort *red = (GLshort*)commandbuf;     commandbuf += sizeof(GLshort);
	GLshort *green = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *blue = (GLshort*)commandbuf;    commandbuf += sizeof(GLshort);

	LOG("glColor3s(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//18
static void EXEC_glColor3sv(byte *commandbuf)
{

	LOG("glColor3sv()\n");

}


//19
static void EXEC_glColor3ub(byte *commandbuf)
{
	GLubyte *red = (GLubyte*)commandbuf;     commandbuf += sizeof(GLubyte);
	GLubyte *green = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *blue = (GLubyte*)commandbuf;    commandbuf += sizeof(GLubyte);

	LOG("glColor3ub(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//20
static void EXEC_glColor3ubv(byte *commandbuf)
{

	LOG("glColor3ubv()\n");

}


//21
static void EXEC_glColor3ui(byte *commandbuf)
{
	GLuint *red = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *green = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLuint *blue = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	LOG("glColor3ui(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//22
static void EXEC_glColor3uiv(byte *commandbuf)
{

	LOG("glColor3uiv()\n");

}


//23
static void EXEC_glColor3us(byte *commandbuf)
{
	GLushort *red = (GLushort*)commandbuf;   commandbuf += sizeof(GLushort);
	GLushort *green = (GLushort*)commandbuf;     commandbuf += sizeof(GLushort);
	GLushort *blue = (GLushort*)commandbuf;  commandbuf += sizeof(GLushort);

	LOG("glColor3us(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//24
static void EXEC_glColor3usv(byte *commandbuf)
{

	LOG("glColor3usv()\n");

}


//25
static void EXEC_glColor4b(byte *commandbuf)
{
	GLbyte *red = (GLbyte*)commandbuf;   commandbuf += sizeof(GLbyte);
	GLbyte *green = (GLbyte*)commandbuf;     commandbuf += sizeof(GLbyte);
	GLbyte *blue = (GLbyte*)commandbuf;  commandbuf += sizeof(GLbyte);
	GLbyte *alpha = (GLbyte*)commandbuf;     commandbuf += sizeof(GLbyte);

	LOG("glColor4b(red=%0.1f, green=%0.1f, blue=%0.1f, alpha=%0.1f)\n", (float)*red, (float)*green, (float)*blue, (float)*alpha);
}


//26
static void EXEC_glColor4bv(byte *commandbuf)
{

	LOG("glColor4bv()\n");

}


//27
static void EXEC_glColor4d(byte *commandbuf)
{
	GLdouble *red = (GLdouble*)commandbuf;   commandbuf += sizeof(GLdouble);
	GLdouble *green = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *blue = (GLdouble*)commandbuf;  commandbuf += sizeof(GLdouble);
	GLdouble *alpha = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glColor4d(red=%0.1f, green=%0.1f, blue=%0.1f, alpha=%0.1f)\n", (float)*red, (float)*green, (float)*blue, (float)*alpha);
}


//28
static void EXEC_glColor4dv(byte *commandbuf)
{

	LOG("glColor4dv()\n");

}


//29
static void EXEC_glColor4f(byte *commandbuf)
{
	GLfloat *red = (GLfloat*)commandbuf;     commandbuf += sizeof(GLfloat);
	GLfloat *green = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *blue = (GLfloat*)commandbuf;    commandbuf += sizeof(GLfloat);
	GLfloat *alpha = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glColor4f(red=%0.1f, green=%0.1f, blue=%0.1f, alpha=%0.1f)\n", (float)*red, (float)*green, (float)*blue, (float)*alpha);
}


//30
static void EXEC_glColor4fv(byte *commandbuf)
{

	LOG("glColor4fv()\n");

}


//31
static void EXEC_glColor4i(byte *commandbuf)
{
	GLint *red = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *green = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *blue = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLint *alpha = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glColor4i(red=%0.1f, green=%0.1f, blue=%0.1f, alpha=%0.1f)\n", (float)*red, (float)*green, (float)*blue, (float)*alpha);
}


//32
static void EXEC_glColor4iv(byte *commandbuf)
{

	LOG("glColor4iv()\n");

}


//33
static void EXEC_glColor4s(byte *commandbuf)
{
	GLshort *red = (GLshort*)commandbuf;     commandbuf += sizeof(GLshort);
	GLshort *green = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *blue = (GLshort*)commandbuf;    commandbuf += sizeof(GLshort);
	GLshort *alpha = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glColor4s(red=%0.1f, green=%0.1f, blue=%0.1f, alpha=%0.1f)\n", (float)*red, (float)*green, (float)*blue, (float)*alpha);
}


//34
static void EXEC_glColor4sv(byte *commandbuf)
{

	LOG("glColor4sv()\n");

}


//35
static void EXEC_glColor4ub(byte *commandbuf)
{
	GLubyte *red = (GLubyte*)commandbuf;     commandbuf += sizeof(GLubyte);
	GLubyte *green = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *blue = (GLubyte*)commandbuf;    commandbuf += sizeof(GLubyte);
	GLubyte *alpha = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);

	LOG("glColor4ub(red=%0.1f, green=%0.1f, blue=%0.1f, alpha=%0.1f)\n", (float)*red, (float)*green, (float)*blue, (float)*alpha);
}


//36
static void EXEC_glColor4ubv(byte *commandbuf)
{

	LOG("glColor4ubv()\n");

}


//37
static void EXEC_glColor4ui(byte *commandbuf)
{
	GLuint *red = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *green = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLuint *blue = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLuint *alpha = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	LOG("glColor4ui(red=%0.1f, green=%0.1f, blue=%0.1f, alpha=%0.1f)\n", (float)*red, (float)*green, (float)*blue, (float)*alpha);
}


//38
static void EXEC_glColor4uiv(byte *commandbuf)
{

	LOG("glColor4uiv()\n");

}


//39
static void EXEC_glColor4us(byte *commandbuf)
{
	GLushort *red = (GLushort*)commandbuf;   commandbuf += sizeof(GLushort);
	GLushort *green = (GLushort*)commandbuf;     commandbuf += sizeof(GLushort);
	GLushort *blue = (GLushort*)commandbuf;  commandbuf += sizeof(GLushort);
	GLushort *alpha = (GLushort*)commandbuf;     commandbuf += sizeof(GLushort);

	LOG("glColor4us(red=%0.1f, green=%0.1f, blue=%0.1f, alpha=%0.1f)\n", (float)*red, (float)*green, (float)*blue, (float)*alpha);
}


//40
static void EXEC_glColor4usv(byte *commandbuf)
{

	LOG("glColor4usv()\n");

}


//41
static void EXEC_glEdgeFlag(byte *commandbuf)
{
	GLboolean *flag = (GLboolean*)commandbuf;    commandbuf += sizeof(GLboolean);

	LOG("glEdgeFlag(flag=%0.1f)\n", (float)*flag);
}


//42
static void EXEC_glEdgeFlagv(byte *commandbuf)
{

	LOG("glEdgeFlagv()\n");

}


//43
static void EXEC_glEnd(byte *commandbuf)
{

	LOG("glEnd()\n");
}


//44
static void EXEC_glIndexd(byte *commandbuf)
{
	GLdouble *c = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glIndexd(c=%0.1f)\n", (float)*c);
}


//45
static void EXEC_glIndexdv(byte *commandbuf)
{

	LOG("glIndexdv()\n");

}


//46
static void EXEC_glIndexf(byte *commandbuf)
{
	GLfloat *c = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glIndexf(c=%0.1f)\n", (float)*c);
}


//47
static void EXEC_glIndexfv(byte *commandbuf)
{

	LOG("glIndexfv()\n");

}


//48
static void EXEC_glIndexi(byte *commandbuf)
{
	GLint *c = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glIndexi(c=%0.1f)\n", (float)*c);
}


//49
static void EXEC_glIndexiv(byte *commandbuf)
{

	LOG("glIndexiv()\n");

}


//50
static void EXEC_glIndexs(byte *commandbuf)
{
	GLshort *c = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glIndexs(c=%0.1f)\n", (float)*c);
}


//51
static void EXEC_glIndexsv(byte *commandbuf)
{

	LOG("glIndexsv()\n");

}


//52
static void EXEC_glNormal3b(byte *commandbuf)
{
	GLbyte *nx = (GLbyte*)commandbuf;    commandbuf += sizeof(GLbyte);
	GLbyte *ny = (GLbyte*)commandbuf;    commandbuf += sizeof(GLbyte);
	GLbyte *nz = (GLbyte*)commandbuf;    commandbuf += sizeof(GLbyte);

	LOG("glNormal3b(nx=%0.1f, ny=%0.1f, nz=%0.1f)\n", (float)*nx, (float)*ny, (float)*nz);
}


//53
static void EXEC_glNormal3bv(byte *commandbuf)
{

	LOG("glNormal3bv()\n");

}


//54
static void EXEC_glNormal3d(byte *commandbuf)
{
	GLdouble *nx = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *ny = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *nz = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);

	LOG("glNormal3d(nx=%0.1f, ny=%0.1f, nz=%0.1f)\n", (float)*nx, (float)*ny, (float)*nz);
}


//55
static void EXEC_glNormal3dv(byte *commandbuf)
{

	LOG("glNormal3dv()\n");

}


//56
static void EXEC_glNormal3f(byte *commandbuf)
{
	GLfloat *nx = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *ny = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *nz = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	LOG("glNormal3f(nx=%0.1f, ny=%0.1f, nz=%0.1f)\n", (float)*nx, (float)*ny, (float)*nz);
}


//57
static void EXEC_glNormal3fv(byte *commandbuf)
{

	LOG("glNormal3fv()\n");

}


//58
static void EXEC_glNormal3i(byte *commandbuf)
{
	GLint *nx = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *ny = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *nz = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	LOG("glNormal3i(nx=%0.1f, ny=%0.1f, nz=%0.1f)\n", (float)*nx, (float)*ny, (float)*nz);
}


//59
static void EXEC_glNormal3iv(byte *commandbuf)
{

	LOG("glNormal3iv()\n");

}


//60
static void EXEC_glNormal3s(byte *commandbuf)
{
	GLshort *nx = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);
	GLshort *ny = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);
	GLshort *nz = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);

	LOG("glNormal3s(nx=%0.1f, ny=%0.1f, nz=%0.1f)\n", (float)*nx, (float)*ny, (float)*nz);
}


//61
static void EXEC_glNormal3sv(byte *commandbuf)
{

	LOG("glNormal3sv()\n");

}


//62
static void EXEC_glRasterPos2d(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glRasterPos2d(x=%0.1f, y=%0.1f)\n", (float)*x, (float)*y);
}


//63
static void EXEC_glRasterPos2dv(byte *commandbuf)
{

	LOG("glRasterPos2dv()\n");

}


//64
static void EXEC_glRasterPos2f(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glRasterPos2f(x=%0.1f, y=%0.1f)\n", (float)*x, (float)*y);
}


//65
static void EXEC_glRasterPos2fv(byte *commandbuf)
{

	LOG("glRasterPos2fv()\n");

}


//66
static void EXEC_glRasterPos2i(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glRasterPos2i(x=%0.1f, y=%0.1f)\n", (float)*x, (float)*y);
}


//67
static void EXEC_glRasterPos2iv(byte *commandbuf)
{

	LOG("glRasterPos2iv()\n");

}


//68
static void EXEC_glRasterPos2s(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glRasterPos2s(x=%0.1f, y=%0.1f)\n", (float)*x, (float)*y);
}


//69
static void EXEC_glRasterPos2sv(byte *commandbuf)
{

	LOG("glRasterPos2sv()\n");

}


//70
static void EXEC_glRasterPos3d(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glRasterPos3d(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//71
static void EXEC_glRasterPos3dv(byte *commandbuf)
{

	LOG("glRasterPos3dv()\n");

}


//72
static void EXEC_glRasterPos3f(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glRasterPos3f(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//73
static void EXEC_glRasterPos3fv(byte *commandbuf)
{

	LOG("glRasterPos3fv()\n");

}


//74
static void EXEC_glRasterPos3i(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *z = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glRasterPos3i(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//75
static void EXEC_glRasterPos3iv(byte *commandbuf)
{

	LOG("glRasterPos3iv()\n");

}


//76
static void EXEC_glRasterPos3s(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glRasterPos3s(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//77
static void EXEC_glRasterPos3sv(byte *commandbuf)
{

	LOG("glRasterPos3sv()\n");

}


//78
static void EXEC_glRasterPos4d(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *w = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glRasterPos4d(x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*x, (float)*y, (float)*z, (float)*w);
}


//79
static void EXEC_glRasterPos4dv(byte *commandbuf)
{

	LOG("glRasterPos4dv()\n");

}


//80
static void EXEC_glRasterPos4f(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *w = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glRasterPos4f(x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*x, (float)*y, (float)*z, (float)*w);
}


//81
static void EXEC_glRasterPos4fv(byte *commandbuf)
{

	LOG("glRasterPos4fv()\n");

}


//82
static void EXEC_glRasterPos4i(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *z = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *w = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glRasterPos4i(x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*x, (float)*y, (float)*z, (float)*w);
}


//83
static void EXEC_glRasterPos4iv(byte *commandbuf)
{

	LOG("glRasterPos4iv()\n");

}


//84
static void EXEC_glRasterPos4s(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *w = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glRasterPos4s(x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*x, (float)*y, (float)*z, (float)*w);
}


//85
static void EXEC_glRasterPos4sv(byte *commandbuf)
{

	LOG("glRasterPos4sv()\n");

}


//86
static void EXEC_glRectd(byte *commandbuf)
{
	GLdouble *x1 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *y1 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *x2 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *y2 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);

	LOG("glRectd(x1=%0.1f, y1=%0.1f, x2=%0.1f, y2=%0.1f)\n", (float)*x1, (float)*y1, (float)*x2, (float)*y2);
}


//87
static void EXEC_glRectdv(byte *commandbuf)
{

	LOG("glRectdv()\n");

}


//88
static void EXEC_glRectf(byte *commandbuf)
{
	GLfloat *x1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *y1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *x2 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *y2 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	LOG("glRectf(x1=%0.1f, y1=%0.1f, x2=%0.1f, y2=%0.1f)\n", (float)*x1, (float)*y1, (float)*x2, (float)*y2);
}


//89
static void EXEC_glRectfv(byte *commandbuf)
{

	LOG("glRectfv()\n");

}


//90
static void EXEC_glRecti(byte *commandbuf)
{
	GLint *x1 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *y1 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *x2 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *y2 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	LOG("glRecti(x1=%0.1f, y1=%0.1f, x2=%0.1f, y2=%0.1f)\n", (float)*x1, (float)*y1, (float)*x2, (float)*y2);
}


//91
static void EXEC_glRectiv(byte *commandbuf)
{

	LOG("glRectiv()\n");

}


//92
static void EXEC_glRects(byte *commandbuf)
{
	GLshort *x1 = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);
	GLshort *y1 = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);
	GLshort *x2 = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);
	GLshort *y2 = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);

	LOG("glRects(x1=%0.1f, y1=%0.1f, x2=%0.1f, y2=%0.1f)\n", (float)*x1, (float)*y1, (float)*x2, (float)*y2);
}


//93
static void EXEC_glRectsv(byte *commandbuf)
{

	LOG("glRectsv()\n");

}


//94
static void EXEC_glTexCoord1d(byte *commandbuf)
{
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glTexCoord1d(s=%0.1f)\n", (float)*s);
}


//95
static void EXEC_glTexCoord1dv(byte *commandbuf)
{

	LOG("glTexCoord1dv()\n");

}


//96
static void EXEC_glTexCoord1f(byte *commandbuf)
{
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glTexCoord1f(s=%0.1f)\n", (float)*s);
}


//97
static void EXEC_glTexCoord1fv(byte *commandbuf)
{

	LOG("glTexCoord1fv()\n");

}


//98
static void EXEC_glTexCoord1i(byte *commandbuf)
{
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glTexCoord1i(s=%0.1f)\n", (float)*s);
}


//99
static void EXEC_glTexCoord1iv(byte *commandbuf)
{

	LOG("glTexCoord1iv()\n");

}


//100
static void EXEC_glTexCoord1s(byte *commandbuf)
{
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glTexCoord1s(s=%0.1f)\n", (float)*s);
}


//101
static void EXEC_glTexCoord1sv(byte *commandbuf)
{

	LOG("glTexCoord1sv()\n");

}


//102
static void EXEC_glTexCoord2d(byte *commandbuf)
{
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *t = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glTexCoord2d(s=%0.1f, t=%0.1f)\n", (float)*s, (float)*t);
}


//103
static void EXEC_glTexCoord2dv(byte *commandbuf)
{

	LOG("glTexCoord2dv()\n");

}


//104
static void EXEC_glTexCoord2f(byte *commandbuf)
{
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glTexCoord2f(s=%0.1f, t=%0.1f)\n", (float)*s, (float)*t);
}


//105
static void EXEC_glTexCoord2fv(byte *commandbuf)
{

	LOG("glTexCoord2fv()\n");

}


//106
static void EXEC_glTexCoord2i(byte *commandbuf)
{
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *t = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glTexCoord2i(s=%0.1f, t=%0.1f)\n", (float)*s, (float)*t);
}


//107
static void EXEC_glTexCoord2iv(byte *commandbuf)
{

	LOG("glTexCoord2iv()\n");

}


//108
static void EXEC_glTexCoord2s(byte *commandbuf)
{
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *t = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glTexCoord2s(s=%0.1f, t=%0.1f)\n", (float)*s, (float)*t);
}


//109
static void EXEC_glTexCoord2sv(byte *commandbuf)
{

	LOG("glTexCoord2sv()\n");

}


//110
static void EXEC_glTexCoord3d(byte *commandbuf)
{
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *t = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *r = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glTexCoord3d(s=%0.1f, t=%0.1f, r=%0.1f)\n", (float)*s, (float)*t, (float)*r);
}


//111
static void EXEC_glTexCoord3dv(byte *commandbuf)
{

	LOG("glTexCoord3dv()\n");

}


//112
static void EXEC_glTexCoord3f(byte *commandbuf)
{
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *r = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glTexCoord3f(s=%0.1f, t=%0.1f, r=%0.1f)\n", (float)*s, (float)*t, (float)*r);
}


//113
static void EXEC_glTexCoord3fv(byte *commandbuf)
{

	LOG("glTexCoord3fv()\n");

}


//114
static void EXEC_glTexCoord3i(byte *commandbuf)
{
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *t = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *r = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glTexCoord3i(s=%0.1f, t=%0.1f, r=%0.1f)\n", (float)*s, (float)*t, (float)*r);
}


//115
static void EXEC_glTexCoord3iv(byte *commandbuf)
{

	LOG("glTexCoord3iv()\n");

}


//116
static void EXEC_glTexCoord3s(byte *commandbuf)
{
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *t = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *r = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glTexCoord3s(s=%0.1f, t=%0.1f, r=%0.1f)\n", (float)*s, (float)*t, (float)*r);
}


//117
static void EXEC_glTexCoord3sv(byte *commandbuf)
{

	LOG("glTexCoord3sv()\n");

}


//118
static void EXEC_glTexCoord4d(byte *commandbuf)
{
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *t = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *r = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *q = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glTexCoord4d(s=%0.1f, t=%0.1f, r=%0.1f, q=%0.1f)\n", (float)*s, (float)*t, (float)*r, (float)*q);
}


//119
static void EXEC_glTexCoord4dv(byte *commandbuf)
{

	LOG("glTexCoord4dv()\n");

}


//120
static void EXEC_glTexCoord4f(byte *commandbuf)
{
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *r = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *q = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glTexCoord4f(s=%0.1f, t=%0.1f, r=%0.1f, q=%0.1f)\n", (float)*s, (float)*t, (float)*r, (float)*q);
}


//121
static void EXEC_glTexCoord4fv(byte *commandbuf)
{

	LOG("glTexCoord4fv()\n");

}


//122
static void EXEC_glTexCoord4i(byte *commandbuf)
{
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *t = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *r = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *q = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glTexCoord4i(s=%0.1f, t=%0.1f, r=%0.1f, q=%0.1f)\n", (float)*s, (float)*t, (float)*r, (float)*q);
}


//123
static void EXEC_glTexCoord4iv(byte *commandbuf)
{

	LOG("glTexCoord4iv()\n");

}


//124
static void EXEC_glTexCoord4s(byte *commandbuf)
{
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *t = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *r = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *q = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glTexCoord4s(s=%0.1f, t=%0.1f, r=%0.1f, q=%0.1f)\n", (float)*s, (float)*t, (float)*r, (float)*q);
}


//125
static void EXEC_glTexCoord4sv(byte *commandbuf)
{

	LOG("glTexCoord4sv()\n");

}


//126
static void EXEC_glVertex2d(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glVertex2d(x=%0.1f, y=%0.1f)\n", (float)*x, (float)*y);
}


//127
static void EXEC_glVertex2dv(byte *commandbuf)
{

	LOG("glVertex2dv()\n");

}


//128
static void EXEC_glVertex2f(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glVertex2f(x=%0.1f, y=%0.1f)\n", (float)*x, (float)*y);
}


//129
static void EXEC_glVertex2fv(byte *commandbuf)
{

	LOG("glVertex2fv()\n");

}


//130
static void EXEC_glVertex2i(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glVertex2i(x=%0.1f, y=%0.1f)\n", (float)*x, (float)*y);
}


//131
static void EXEC_glVertex2iv(byte *commandbuf)
{

	LOG("glVertex2iv()\n");

}


//132
static void EXEC_glVertex2s(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glVertex2s(x=%0.1f, y=%0.1f)\n", (float)*x, (float)*y);
}


//133
static void EXEC_glVertex2sv(byte *commandbuf)
{

	LOG("glVertex2sv()\n");

}


//134
static void EXEC_glVertex3d(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glVertex3d(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//135
static void EXEC_glVertex3dv(byte *commandbuf)
{

	LOG("glVertex3dv()\n");

}


//136
static void EXEC_glVertex3f(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glVertex3f(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//137
static void EXEC_glVertex3fv(byte *commandbuf)
{

	LOG("glVertex3fv()\n");

}


//138
static void EXEC_glVertex3i(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *z = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glVertex3i(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//139
static void EXEC_glVertex3iv(byte *commandbuf)
{

	LOG("glVertex3iv()\n");

}


//140
static void EXEC_glVertex3s(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glVertex3s(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//141
static void EXEC_glVertex3sv(byte *commandbuf)
{

	LOG("glVertex3sv()\n");

}


//142
static void EXEC_glVertex4d(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *w = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glVertex4d(x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*x, (float)*y, (float)*z, (float)*w);
}


//143
static void EXEC_glVertex4dv(byte *commandbuf)
{

	LOG("glVertex4dv()\n");

}


//144
static void EXEC_glVertex4f(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *w = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glVertex4f(x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*x, (float)*y, (float)*z, (float)*w);
}


//145
static void EXEC_glVertex4fv(byte *commandbuf)
{

	LOG("glVertex4fv()\n");

}


//146
static void EXEC_glVertex4i(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *z = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *w = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glVertex4i(x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*x, (float)*y, (float)*z, (float)*w);
}


//147
static void EXEC_glVertex4iv(byte *commandbuf)
{

	LOG("glVertex4iv()\n");

}


//148
static void EXEC_glVertex4s(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *w = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glVertex4s(x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*x, (float)*y, (float)*z, (float)*w);
}


//149
static void EXEC_glVertex4sv(byte *commandbuf)
{

	LOG("glVertex4sv()\n");

}


//150
static void EXEC_glClipPlane(byte *commandbuf)
{

	LOG("glClipPlane()\n");

}


//151
static void EXEC_glColorMaterial(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glColorMaterial(face=%0.1f, mode=%0.1f)\n", (float)*face, (float)*mode);
}


//152
static void EXEC_glCullFace(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glCullFace(mode=%0.1f)\n", (float)*mode);
}


//153
static void EXEC_glFogf(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glFogf(pname=%0.1f, param=%0.1f)\n", (float)*pname, (float)*param);
}


//154
static void EXEC_glFogfv(byte *commandbuf)
{

	LOG("glFogfv()\n");

}


//155
static void EXEC_glFogi(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glFogi(pname=%0.1f, param=%0.1f)\n", (float)*pname, (float)*param);
}


//156
static void EXEC_glFogiv(byte *commandbuf)
{

	LOG("glFogiv()\n");

}


//157
static void EXEC_glFrontFace(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glFrontFace(mode=%0.1f)\n", (float)*mode);
}


//158
static void EXEC_glHint(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glHint(target=%0.1f, mode=%0.1f)\n", (float)*target, (float)*mode);
}


//159
static void EXEC_glLightf(byte *commandbuf)
{
	GLenum *light = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glLightf(light=%0.1f, pname=%0.1f, param=%0.1f)\n", (float)*light, (float)*pname, (float)*param);
}


//160
static void EXEC_glLightfv(byte *commandbuf)
{

	LOG("glLightfv()\n");

}


//161
static void EXEC_glLighti(byte *commandbuf)
{
	GLenum *light = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glLighti(light=%0.1f, pname=%0.1f, param=%0.1f)\n", (float)*light, (float)*pname, (float)*param);
}


//162
static void EXEC_glLightiv(byte *commandbuf)
{

	LOG("glLightiv()\n");

}


//163
static void EXEC_glLightModelf(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glLightModelf(pname=%0.1f, param=%0.1f)\n", (float)*pname, (float)*param);
}


//164
static void EXEC_glLightModelfv(byte *commandbuf)
{

	LOG("glLightModelfv()\n");

}


//165
static void EXEC_glLightModeli(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glLightModeli(pname=%0.1f, param=%0.1f)\n", (float)*pname, (float)*param);
}


//166
static void EXEC_glLightModeliv(byte *commandbuf)
{

	LOG("glLightModeliv()\n");

}


//167
static void EXEC_glLineStipple(byte *commandbuf)
{
	GLint *factor = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLushort *pattern = (GLushort*)commandbuf;   commandbuf += sizeof(GLushort);

	LOG("glLineStipple(factor=%0.1f, pattern=%0.1f)\n", (float)*factor, (float)*pattern);
}


//168
static void EXEC_glLineWidth(byte *commandbuf)
{
	GLfloat *width = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glLineWidth(width=%0.1f)\n", (float)*width);
}


//169
static void EXEC_glMaterialf(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glMaterialf(face=%0.1f, pname=%0.1f, param=%0.1f)\n", (float)*face, (float)*pname, (float)*param);
}


//170
static void EXEC_glMaterialfv(byte *commandbuf)
{

	LOG("glMaterialfv()\n");

}


//171
static void EXEC_glMateriali(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glMateriali(face=%0.1f, pname=%0.1f, param=%0.1f)\n", (float)*face, (float)*pname, (float)*param);
}


//172
static void EXEC_glMaterialiv(byte *commandbuf)
{

	LOG("glMaterialiv()\n");

}


//173
static void EXEC_glPointSize(byte *commandbuf)
{
	GLfloat *size = (GLfloat*)commandbuf;    commandbuf += sizeof(GLfloat);

	LOG("glPointSize(size=%0.1f)\n", (float)*size);
}


//174
static void EXEC_glPolygonMode(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glPolygonMode(face=%0.1f, mode=%0.1f)\n", (float)*face, (float)*mode);
}


//175
static void EXEC_glPolygonStipple(byte *commandbuf)
{

	LOG("glPolygonStipple()\n");

}


//176
static void EXEC_glScissor(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	LOG("glScissor(x=%0.1f, y=%0.1f, width=%0.1f, height=%0.1f)\n", (float)*x, (float)*y, (float)*width, (float)*height);
}


//177
static void EXEC_glShadeModel(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glShadeModel(mode=%0.1f)\n", (float)*mode);
}


//178
static void EXEC_glTexParameterf(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glTexParameterf(target=%0.1f, pname=%0.1f, param=%0.1f)\n", (float)*target, (float)*pname, (float)*param);
}


//179
static void EXEC_glTexParameterfv(byte *commandbuf)
{

	LOG("glTexParameterfv()\n");

}


//180
static void EXEC_glTexParameteri(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glTexParameteri(target=%0.1f, pname=%0.1f, param=%0.1f)\n", (float)*target, (float)*pname, (float)*param);
}


//181
static void EXEC_glTexParameteriv(byte *commandbuf)
{

	LOG("glTexParameteriv()\n");

}


//182
static void EXEC_glTexImage1D(byte *commandbuf)
{

	LOG("glTexImage1D()\n");

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

	LOG("glTexImage2D(target =%0.1f, level=%d, internalformat=%d, width=%0.1f, height=%0.1f, border=%d, format=%0.1f, type=%0.1f)\n", (float)*target, *level, *internalformat, (float)*width, (float)*height, *border, (float)*format, (float)*type);

}


//184
static void EXEC_glTexEnvf(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glTexEnvf(target=%0.1f, pname=%0.1f, param=%0.1f)\n", (float)*target, (float)*pname, (float)*param);
}


//185
static void EXEC_glTexEnvfv(byte *commandbuf)
{

	LOG("glTexEnvfv()\n");

}


//186
static void EXEC_glTexEnvi(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glTexEnvi(target=%0.1f, pname=%0.1f, param=%0.1f)\n", (float)*target, (float)*pname, (float)*param);
}


//187
static void EXEC_glTexEnviv(byte *commandbuf)
{

	LOG("glTexEnviv()\n");

}


//188
static void EXEC_glTexGend(byte *commandbuf)
{
	GLenum *coord = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLdouble *param = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glTexGend(coord=%0.1f, pname=%0.1f, param=%0.1f)\n", (float)*coord, (float)*pname, (float)*param);
}


//189
static void EXEC_glTexGendv(byte *commandbuf)
{

	LOG("glTexGendv()\n");

}


//190
static void EXEC_glTexGenf(byte *commandbuf)
{
	GLenum *coord = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glTexGenf(coord=%0.1f, pname=%0.1f, param=%0.1f)\n", (float)*coord, (float)*pname, (float)*param);
}


//191
static void EXEC_glTexGenfv(byte *commandbuf)
{

	LOG("glTexGenfv()\n");

}


//192
static void EXEC_glTexGeni(byte *commandbuf)
{
	GLenum *coord = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glTexGeni(coord=%0.1f, pname=%0.1f, param=%0.1f)\n", (float)*coord, (float)*pname, (float)*param);
}


//193
static void EXEC_glTexGeniv(byte *commandbuf)
{

	LOG("glTexGeniv()\n");

}


//194
static void EXEC_glFeedbackBuffer(byte *commandbuf)
{

	LOG("glFeedbackBuffer()\n");

}


//195
static void EXEC_glSelectBuffer(byte *commandbuf)
{

	LOG("glSelectBuffer()\n");

}


//196
static void EXEC_glRenderMode(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glRenderMode(mode=%0.1f)\n", (float)*mode);
}


//197
static void EXEC_glInitNames(byte *commandbuf)
{

	LOG("glInitNames()\n");
}


//198
static void EXEC_glLoadName(byte *commandbuf)
{
	GLuint *name = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	LOG("glLoadName(name=%0.1f)\n", (float)*name);
}


//199
static void EXEC_glPassThrough(byte *commandbuf)
{
	GLfloat *token = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glPassThrough(token=%0.1f)\n", (float)*token);
}


//200
static void EXEC_glPopName(byte *commandbuf)
{

	LOG("glPopName()\n");
}


//201
static void EXEC_glPushName(byte *commandbuf)
{
	GLuint *name = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	LOG("glPushName(name=%0.1f)\n", (float)*name);
}


//202
static void EXEC_glDrawBuffer(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glDrawBuffer(mode=%0.1f)\n", (float)*mode);
}


//203
static void EXEC_glClear(byte *commandbuf)
{
	GLbitfield *mask = (GLbitfield*)commandbuf;  commandbuf += sizeof(GLbitfield);

	LOG("glClear(mask=%0.1f)\n", (float)*mask);
}


//204
static void EXEC_glClearAccum(byte *commandbuf)
{
	GLfloat *red = (GLfloat*)commandbuf;     commandbuf += sizeof(GLfloat);
	GLfloat *green = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *blue = (GLfloat*)commandbuf;    commandbuf += sizeof(GLfloat);
	GLfloat *alpha = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glClearAccum(red=%0.1f, green=%0.1f, blue=%0.1f, alpha=%0.1f)\n", (float)*red, (float)*green, (float)*blue, (float)*alpha);
}


//205
static void EXEC_glClearIndex(byte *commandbuf)
{
	GLfloat *c = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glClearIndex(c=%0.1f)\n", (float)*c);
}


//206
static void EXEC_glClearColor(byte *commandbuf)
{
	GLclampf *red = (GLclampf*)commandbuf;   commandbuf += sizeof(GLclampf);
	GLclampf *green = (GLclampf*)commandbuf;     commandbuf += sizeof(GLclampf);
	GLclampf *blue = (GLclampf*)commandbuf;  commandbuf += sizeof(GLclampf);
	GLclampf *alpha = (GLclampf*)commandbuf;     commandbuf += sizeof(GLclampf);

	LOG("glClearColor(red=%0.1f, green=%0.1f, blue=%0.1f, alpha=%0.1f)\n", (float)*red, (float)*green, (float)*blue, (float)*alpha);
}


//207
static void EXEC_glClearStencil(byte *commandbuf)
{
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glClearStencil(s=%0.1f)\n", (float)*s);
}


//208
static void EXEC_glClearDepth(byte *commandbuf)
{
	GLclampd *depth = (GLclampd*)commandbuf;     commandbuf += sizeof(GLclampd);

	LOG("glClearDepth(depth=%0.1f)\n", (float)*depth);
}


//209
static void EXEC_glStencilMask(byte *commandbuf)
{
	GLuint *mask = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	LOG("glStencilMask(mask=%0.1f)\n", (float)*mask);
}


//210
static void EXEC_glColorMask(byte *commandbuf)
{
	GLboolean *red = (GLboolean*)commandbuf;     commandbuf += sizeof(GLboolean);
	GLboolean *green = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);
	GLboolean *blue = (GLboolean*)commandbuf;    commandbuf += sizeof(GLboolean);
	GLboolean *alpha = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);

	LOG("glColorMask(red=%0.1f, green=%0.1f, blue=%0.1f, alpha=%0.1f)\n", (float)*red, (float)*green, (float)*blue, (float)*alpha);
}


//211
static void EXEC_glDepthMask(byte *commandbuf)
{
	GLboolean *flag = (GLboolean*)commandbuf;    commandbuf += sizeof(GLboolean);

	LOG("glDepthMask(flag=%0.1f)\n", (float)*flag);
}


//212
static void EXEC_glIndexMask(byte *commandbuf)
{
	GLuint *mask = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	LOG("glIndexMask(mask=%0.1f)\n", (float)*mask);
}


//213
static void EXEC_glAccum(byte *commandbuf)
{
	GLenum *op = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLfloat *value = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glAccum(op=%0.1f, value=%0.1f)\n", (float)*op, (float)*value);
}


//214
static void EXEC_glDisable(byte *commandbuf)
{
	GLenum *cap = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	LOG("glDisable(cap=%0.1f)\n", (float)*cap);
}


//215
static void EXEC_glEnable(byte *commandbuf)
{
	GLenum *cap = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	LOG("glEnable(cap=%0.1f)\n", (float)*cap);
}


//216
static void EXEC_glFinish(byte *commandbuf)
{

	LOG("glFinish()\n");
}


//217
static void EXEC_glFlush(byte *commandbuf)
{

	LOG("glFlush()\n");
}


//218
static void EXEC_glPopAttrib(byte *commandbuf)
{

	LOG("glPopAttrib()\n");
}


//219
static void EXEC_glPushAttrib(byte *commandbuf)
{
	GLbitfield *mask = (GLbitfield*)commandbuf;  commandbuf += sizeof(GLbitfield);

	LOG("glPushAttrib(mask=%0.1f)\n", (float)*mask);
}


//220
static void EXEC_glMap1d(byte *commandbuf)
{

	LOG("glMap1d()\n");

}


//221
static void EXEC_glMap1f(byte *commandbuf)
{

	LOG("glMap1f()\n");

}


//222
static void EXEC_glMap2d(byte *commandbuf)
{

	LOG("glMap2d()\n");

}


//223
static void EXEC_glMap2f(byte *commandbuf)
{

	LOG("glMap2f()\n");

}


//224
static void EXEC_glMapGrid1d(byte *commandbuf)
{
	GLint *un = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLdouble *u1 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *u2 = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);

	LOG("glMapGrid1d(un=%0.1f, u1=%0.1f, u2=%0.1f)\n", (float)*un, (float)*u1, (float)*u2);
}


//225
static void EXEC_glMapGrid1f(byte *commandbuf)
{
	GLint *un = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLfloat *u1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *u2 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	LOG("glMapGrid1f(un=%0.1f, u1=%0.1f, u2=%0.1f)\n", (float)*un, (float)*u1, (float)*u2);
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

	LOG("glMapGrid2d(un=%0.1f, u1=%0.1f, u2=%0.1f, vn=%0.1f, v1=%0.1f, v2=%0.1f)\n", (float)*un, (float)*u1, (float)*u2, (float)*vn, (float)*v1, (float)*v2);
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

	LOG("glMapGrid2f(un=%0.1f, u1=%0.1f, u2=%0.1f, vn=%0.1f, v1=%0.1f, v2=%0.1f)\n", (float)*un, (float)*u1, (float)*u2, (float)*vn, (float)*v1, (float)*v2);
}


//228
static void EXEC_glEvalCoord1d(byte *commandbuf)
{
	GLdouble *u = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glEvalCoord1d(u=%0.1f)\n", (float)*u);
}


//229
static void EXEC_glEvalCoord1dv(byte *commandbuf)
{

	LOG("glEvalCoord1dv()\n");

}


//230
static void EXEC_glEvalCoord1f(byte *commandbuf)
{
	GLfloat *u = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glEvalCoord1f(u=%0.1f)\n", (float)*u);
}


//231
static void EXEC_glEvalCoord1fv(byte *commandbuf)
{

	LOG("glEvalCoord1fv()\n");

}


//232
static void EXEC_glEvalCoord2d(byte *commandbuf)
{
	GLdouble *u = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *v = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glEvalCoord2d(u=%0.1f, v=%0.1f)\n", (float)*u, (float)*v);
}


//233
static void EXEC_glEvalCoord2dv(byte *commandbuf)
{

	LOG("glEvalCoord2dv()\n");

}


//234
static void EXEC_glEvalCoord2f(byte *commandbuf)
{
	GLfloat *u = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *v = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glEvalCoord2f(u=%0.1f, v=%0.1f)\n", (float)*u, (float)*v);
}


//235
static void EXEC_glEvalCoord2fv(byte *commandbuf)
{

	LOG("glEvalCoord2fv()\n");

}


//236
static void EXEC_glEvalMesh1(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLint *i1 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *i2 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	LOG("glEvalMesh1(mode=%0.1f, i1=%0.1f, i2=%0.1f)\n", (float)*mode, (float)*i1, (float)*i2);
}


//237
static void EXEC_glEvalPoint1(byte *commandbuf)
{
	GLint *i = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glEvalPoint1(i=%0.1f)\n", (float)*i);
}


//238
static void EXEC_glEvalMesh2(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLint *i1 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *i2 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *j1 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *j2 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	LOG("glEvalMesh2(mode=%0.1f, i1=%0.1f, i2=%0.1f, j1=%0.1f, j2=%0.1f)\n", (float)*mode, (float)*i1, (float)*i2, (float)*j1, (float)*j2);
}


//239
static void EXEC_glEvalPoint2(byte *commandbuf)
{
	GLint *i = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *j = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glEvalPoint2(i=%0.1f, j=%0.1f)\n", (float)*i, (float)*j);
}


//240
static void EXEC_glAlphaFunc(byte *commandbuf)
{
	GLenum *func = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLclampf *ref = (GLclampf*)commandbuf;   commandbuf += sizeof(GLclampf);

	LOG("glAlphaFunc(func=%0.1f, ref=%0.1f)\n", (float)*func, (float)*ref);
}


//241
static void EXEC_glBlendFunc(byte *commandbuf)
{
	GLenum *sfactor = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLenum *dfactor = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	LOG("glBlendFunc(sfactor=%0.1f, dfactor=%0.1f)\n", (float)*sfactor, (float)*dfactor);
}


//242
static void EXEC_glLogicOp(byte *commandbuf)
{
	GLenum *opcode = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	LOG("glLogicOp(opcode=%0.1f)\n", (float)*opcode);
}


//243
static void EXEC_glStencilFunc(byte *commandbuf)
{
	GLenum *func = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLint *ref = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLuint *mask = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	LOG("glStencilFunc(func=%0.1f, ref=%0.1f, mask=%0.1f)\n", (float)*func, (float)*ref, (float)*mask);
}


//244
static void EXEC_glStencilOp(byte *commandbuf)
{
	GLenum *fail = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *zfail = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *zpass = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	LOG("glStencilOp(fail=%0.1f, zfail=%0.1f, zpass=%0.1f)\n", (float)*fail, (float)*zfail, (float)*zpass);
}


//245
static void EXEC_glDepthFunc(byte *commandbuf)
{
	GLenum *func = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glDepthFunc(func=%0.1f)\n", (float)*func);
}


//246
static void EXEC_glPixelZoom(byte *commandbuf)
{
	GLfloat *xfactor = (GLfloat*)commandbuf;     commandbuf += sizeof(GLfloat);
	GLfloat *yfactor = (GLfloat*)commandbuf;     commandbuf += sizeof(GLfloat);

	LOG("glPixelZoom(xfactor=%0.1f, yfactor=%0.1f)\n", (float)*xfactor, (float)*yfactor);
}


//247
static void EXEC_glPixelTransferf(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glPixelTransferf(pname=%0.1f, param=%0.1f)\n", (float)*pname, (float)*param);
}


//248
static void EXEC_glPixelTransferi(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glPixelTransferi(pname=%0.1f, param=%0.1f)\n", (float)*pname, (float)*param);
}


//249
static void EXEC_glPixelStoref(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glPixelStoref(pname=%0.1f, param=%0.1f)\n", (float)*pname, (float)*param);
}


//250
static void EXEC_glPixelStorei(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glPixelStorei(pname=%0.1f, param=%0.1f)\n", (float)*pname, (float)*param);
}


//251
static void EXEC_glPixelMapfv(byte *commandbuf)
{

	LOG("glPixelMapfv()\n");

}


//252
static void EXEC_glPixelMapuiv(byte *commandbuf)
{

	LOG("glPixelMapuiv()\n");

}


//253
static void EXEC_glPixelMapusv(byte *commandbuf)
{

	LOG("glPixelMapusv()\n");

}


//254
static void EXEC_glReadBuffer(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glReadBuffer(mode=%0.1f)\n", (float)*mode);
}


//255
static void EXEC_glCopyPixels(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glCopyPixels(x=%0.1f, y=%0.1f, width=%0.1f, height=%0.1f, type=%0.1f)\n", (float)*x, (float)*y, (float)*width, (float)*height, (float)*type);
}


//256
static void EXEC_glReadPixels(byte *commandbuf)
{

	LOG("glReadPixels()\n");

}


//257
static void EXEC_glDrawPixels(byte *commandbuf)
{

	LOG("glDrawPixels()\n");

}


//258
static void EXEC_glGetBooleanv(byte *commandbuf)
{

	LOG("glGetBooleanv()\n");

}


//259
static void EXEC_glGetClipPlane(byte *commandbuf)
{

	LOG("glGetClipPlane()\n");

}


//260
static void EXEC_glGetDoublev(byte *commandbuf)
{

	LOG("glGetDoublev()\n");

}


//261
static void EXEC_glGetError(byte *commandbuf)
{

	LOG("glGetError()\n");
}


//262
static void EXEC_glGetFloatv(byte *commandbuf)
{

	LOG("glGetFloatv()\n");

}


//263
static void EXEC_glGetIntegerv(byte *commandbuf)
{

	LOG("glGetIntegerv()\n");

}


//264
static void EXEC_glGetLightfv(byte *commandbuf)
{

	LOG("glGetLightfv()\n");

}


//265
static void EXEC_glGetLightiv(byte *commandbuf)
{

	LOG("glGetLightiv()\n");

}


//266
static void EXEC_glGetMapdv(byte *commandbuf)
{

	LOG("glGetMapdv()\n");

}


//267
static void EXEC_glGetMapfv(byte *commandbuf)
{

	LOG("glGetMapfv()\n");

}


//268
static void EXEC_glGetMapiv(byte *commandbuf)
{

	LOG("glGetMapiv()\n");

}


//269
static void EXEC_glGetMaterialfv(byte *commandbuf)
{

	LOG("glGetMaterialfv()\n");

}


//270
static void EXEC_glGetMaterialiv(byte *commandbuf)
{

	LOG("glGetMaterialiv()\n");

}


//271
static void EXEC_glGetPixelMapfv(byte *commandbuf)
{

	LOG("glGetPixelMapfv()\n");

}


//272
static void EXEC_glGetPixelMapuiv(byte *commandbuf)
{

	LOG("glGetPixelMapuiv()\n");

}


//273
static void EXEC_glGetPixelMapusv(byte *commandbuf)
{

	LOG("glGetPixelMapusv()\n");

}


//274
static void EXEC_glGetPolygonStipple(byte *commandbuf)
{

	LOG("glGetPolygonStipple()\n");

}


//275
static void EXEC_glGetString(byte *commandbuf)
{
	GLenum *name = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glGetString(name=%0.1f)\n", (float)*name);
}


//276
static void EXEC_glGetTexEnvfv(byte *commandbuf)
{

	LOG("glGetTexEnvfv()\n");

}


//277
static void EXEC_glGetTexEnviv(byte *commandbuf)
{

	LOG("glGetTexEnviv()\n");

}


//278
static void EXEC_glGetTexGendv(byte *commandbuf)
{

	LOG("glGetTexGendv()\n");

}


//279
static void EXEC_glGetTexGenfv(byte *commandbuf)
{

	LOG("glGetTexGenfv()\n");

}


//280
static void EXEC_glGetTexGeniv(byte *commandbuf)
{

	LOG("glGetTexGeniv()\n");

}


//281
static void EXEC_glGetTexImage(byte *commandbuf)
{

	LOG("glGetTexImage()\n");

}


//282
static void EXEC_glGetTexParameterfv(byte *commandbuf)
{

	LOG("glGetTexParameterfv()\n");

}


//283
static void EXEC_glGetTexParameteriv(byte *commandbuf)
{

	LOG("glGetTexParameteriv()\n");

}


//284
static void EXEC_glGetTexLevelParameterfv(byte *commandbuf)
{

	LOG("glGetTexLevelParameterfv()\n");

}


//285
static void EXEC_glGetTexLevelParameteriv(byte *commandbuf)
{

	LOG("glGetTexLevelParameteriv()\n");

}


//286
static void EXEC_glIsEnabled(byte *commandbuf)
{
	GLenum *cap = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	LOG("glIsEnabled(cap=%0.1f)\n", (float)*cap);
}


//287
static void EXEC_glIsList(byte *commandbuf)
{
	GLuint *list = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	LOG("glIsList(list=%0.1f)\n", (float)*list);
}


//288
static void EXEC_glDepthRange(byte *commandbuf)
{
	GLclampd *zNear = (GLclampd*)commandbuf;     commandbuf += sizeof(GLclampd);
	GLclampd *zFar = (GLclampd*)commandbuf;  commandbuf += sizeof(GLclampd);

	LOG("glDepthRange(zNear=%0.1f, zFar=%0.1f)\n", (float)*zNear, (float)*zFar);
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

	LOG("glFrustum(left=%0.1f, right=%0.1f, bottom=%0.1f, top=%0.1f, zNear=%0.1f, zFar=%0.1f)\n", (float)*left, (float)*right, (float)*bottom, (float)*top, (float)*zNear, (float)*zFar);
}


//290
static void EXEC_glLoadIdentity(byte *commandbuf)
{

	LOG("glLoadIdentity()\n");
}


//291
static void EXEC_glLoadMatrixf(byte *commandbuf)
{

	LOG("glLoadMatrixf()\n");

}


//292
static void EXEC_glLoadMatrixd(byte *commandbuf)
{

	LOG("glLoadMatrixd()\n");

}


//293
static void EXEC_glMatrixMode(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glMatrixMode(mode=%0.1f)\n", (float)*mode);
}


//294
static void EXEC_glMultMatrixf(byte *commandbuf)
{

	LOG("glMultMatrixf()\n");

}


//295
static void EXEC_glMultMatrixd(byte *commandbuf)
{

	LOG("glMultMatrixd()\n");

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

	LOG("glOrtho(left=%0.1f, right=%0.1f, bottom=%0.1f, top=%0.1f, zNear=%0.1f, zFar=%0.1f)\n", (float)*left, (float)*right, (float)*bottom, (float)*top, (float)*zNear, (float)*zFar);
}


//297
static void EXEC_glPopMatrix(byte *commandbuf)
{

	LOG("glPopMatrix()\n");
}


//298
static void EXEC_glPushMatrix(byte *commandbuf)
{

	LOG("glPushMatrix()\n");
}


//299
static void EXEC_glRotated(byte *commandbuf)
{
	GLdouble *angle = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glRotated(angle=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*angle, (float)*x, (float)*y, (float)*z);
}


//300
static void EXEC_glRotatef(byte *commandbuf)
{
	GLfloat *angle = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glRotatef(angle=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*angle, (float)*x, (float)*y, (float)*z);
}


//301
static void EXEC_glScaled(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glScaled(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//302
static void EXEC_glScalef(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glScalef(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//303
static void EXEC_glTranslated(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glTranslated(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//304
static void EXEC_glTranslatef(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glTranslatef(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//305
static void EXEC_glViewport(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *height = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);

	LOG("glViewport(x=%0.1f, y=%0.1f, width=%0.1f, height=%0.1f)\n", (float)*x, (float)*y, (float)*width, (float)*height);
}


//306
static void EXEC_glArrayElement(byte *commandbuf)
{
	GLint *i = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glArrayElement(i=%0.1f)\n", (float)*i);
}


//308
static void EXEC_glColorPointer(byte *commandbuf)
{
	GLint *size = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLint);
	GLsizei *stride = (GLsizei*)commandbuf;  commandbuf += sizeof(GLsizei);
	LOG("glColorPointer(size=%d, type=%0.1f, stride=%0.1f)\n", *size, (float)*type, (float)*stride);
}


//309
static void EXEC_glDisableClientState(byte *commandbuf)
{
	GLenum *array = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	LOG("glDisableClientState(array=%0.1f)\n", (float)*array);
}


//310
static void EXEC_glDrawArrays(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLint *first = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	LOG("glDrawArrays(mode=%0.1f, first=%0.1f, count=%0.1f)\n", (float)*mode, (float)*first, (float)*count);
}


//311
static void EXEC_glDrawElements(byte *commandbuf)
{

	LOG("glDrawElements()\n");
}


//312
static void EXEC_glEdgeFlagPointer(byte *commandbuf)
{

	LOG("glEdgeFlagPointer()\n");

}


//313
static void EXEC_glEnableClientState(byte *commandbuf)
{
	GLenum *array = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	LOG("glEnableClientState(array=%0.1f)\n", (float)*array);
}


//329
static void EXEC_glGetPointerv(byte *commandbuf)
{

	LOG("glGetPointerv()\n");

}


//314
static void EXEC_glIndexPointer(byte *commandbuf)
{

	LOG("glIndexPointer()\n");

}


//317
static void EXEC_glInterleavedArrays(byte *commandbuf)
{

	LOG("glInterleavedArrays()\n");

}


//318
static void EXEC_glNormalPointer(byte *commandbuf)
{

	LOG("glNormalPointer()\n");

}


//320
static void EXEC_glTexCoordPointer(byte *commandbuf)
{

	LOG("glTexCoordPointer()\n");

}


//321
static void EXEC_glVertexPointer(byte *commandbuf)
{

	LOG("glVertexPointer()\n");

}


//319
static void EXEC_glPolygonOffset(byte *commandbuf)
{
	GLfloat *factor = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *units = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glPolygonOffset(factor=%0.1f, units=%0.1f)\n", (float)*factor, (float)*units);
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

	LOG("glCopyTexImage1D(target=%0.1f, level=%0.1f, internalformat=%0.1f, x=%0.1f, y=%0.1f, width=%0.1f, border=%0.1f)\n", (float)*target, (float)*level, (float)*internalformat, (float)*x, (float)*y, (float)*width, (float)*border);
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

	LOG("glCopyTexImage2D(target=%0.1f, level=%0.1f, internalformat=%0.1f, x=%0.1f, y=%0.1f, width=%0.1f, height=%0.1f, border=%0.1f)\n", (float)*target, (float)*level, (float)*internalformat, (float)*x, (float)*y, (float)*width, (float)*height, (float)*border);
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

	LOG("glCopyTexSubImage1D(target=%0.1f, level=%0.1f, xoffset=%0.1f, x=%0.1f, y=%0.1f, width=%0.1f)\n", (float)*target, (float)*level, (float)*xoffset, (float)*x, (float)*y, (float)*width);
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

	LOG("glCopyTexSubImage2D(target=%0.1f, level=%0.1f, xoffset=%0.1f, yoffset=%0.1f, x=%0.1f, y=%0.1f, width=%0.1f, height=%0.1f)\n", (float)*target, (float)*level, (float)*xoffset, (float)*yoffset, (float)*x, (float)*y, (float)*width, (float)*height);
}


//332
static void EXEC_glTexSubImage1D(byte *commandbuf)
{

	LOG("glTexSubImage1D()\n");

}


//333
static void EXEC_glTexSubImage2D(byte *commandbuf)
{

	LOG("glTexSubImage2D()\n");

}


//322
static void EXEC_glAreTexturesResident(byte *commandbuf)
{

	LOG("glAreTexturesResident()\n");

}


//307
static void EXEC_glBindTexture(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *texture = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	LOG("glBindTexture(target=%0.1f, texture=%0.1f)\n", (float)*target, (float)*texture);
}


//327
static void EXEC_glDeleteTextures(byte *commandbuf)
{

	LOG("glDeleteTextures()\n");

}


//328
static void EXEC_glGenTextures(byte *commandbuf)
{

	LOG("glGenTextures()\n");

}


//330
static void EXEC_glIsTexture(byte *commandbuf)
{
	GLuint *texture = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	LOG("glIsTexture(texture=%0.1f)\n", (float)*texture);
}


//331
static void EXEC_glPrioritizeTextures(byte *commandbuf)
{

	LOG("glPrioritizeTextures()\n");

}


//315
static void EXEC_glIndexub(byte *commandbuf)
{
	GLubyte *c = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);

	LOG("glIndexub(c=%0.1f)\n", (float)*c);
}


//316
static void EXEC_glIndexubv(byte *commandbuf)
{

	LOG("glIndexubv()\n");

}


//334
static void EXEC_glPopClientAttrib(byte *commandbuf)
{

	LOG("glPopClientAttrib()\n");
}


//335
static void EXEC_glPushClientAttrib(byte *commandbuf)
{
	GLbitfield *mask = (GLbitfield*)commandbuf;  commandbuf += sizeof(GLbitfield);

	LOG("glPushClientAttrib(mask=%0.1f)\n", (float)*mask);
}


//336
static void EXEC_glBlendColor(byte *commandbuf)
{
	GLclampf *red = (GLclampf*)commandbuf;   commandbuf += sizeof(GLclampf);
	GLclampf *green = (GLclampf*)commandbuf;     commandbuf += sizeof(GLclampf);
	GLclampf *blue = (GLclampf*)commandbuf;  commandbuf += sizeof(GLclampf);
	GLclampf *alpha = (GLclampf*)commandbuf;     commandbuf += sizeof(GLclampf);

	LOG("glBlendColor(red=%0.1f, green=%0.1f, blue=%0.1f, alpha=%0.1f)\n", (float)*red, (float)*green, (float)*blue, (float)*alpha);
}


//337
static void EXEC_glBlendEquation(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glBlendEquation(mode=%0.1f)\n", (float)*mode);
}


//338
static void EXEC_glDrawRangeElements(byte *commandbuf)
{

	LOG("glDrawRangeElements()\n");

}


//339
static void EXEC_glColorTable(byte *commandbuf)
{

	LOG("glColorTable()\n");

}


//340
static void EXEC_glColorTableParameterfv(byte *commandbuf)
{

	LOG("glColorTableParameterfv()\n");

}


//341
static void EXEC_glColorTableParameteriv(byte *commandbuf)
{

	LOG("glColorTableParameteriv()\n");

}


//342
static void EXEC_glCopyColorTable(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	LOG("glCopyColorTable(target=%0.1f, internalformat=%0.1f, x=%0.1f, y=%0.1f, width=%0.1f)\n", (float)*target, (float)*internalformat, (float)*x, (float)*y, (float)*width);
}


//343
static void EXEC_glGetColorTable(byte *commandbuf)
{

	LOG("glGetColorTable()\n");

}


//344
static void EXEC_glGetColorTableParameterfv(byte *commandbuf)
{

	LOG("glGetColorTableParameterfv()\n");

}


//345
static void EXEC_glGetColorTableParameteriv(byte *commandbuf)
{

	LOG("glGetColorTableParameteriv()\n");

}


//346
static void EXEC_glColorSubTable(byte *commandbuf)
{

	LOG("glColorSubTable()\n");

}


//347
static void EXEC_glCopyColorSubTable(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *start = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	LOG("glCopyColorSubTable(target=%0.1f, start=%0.1f, x=%0.1f, y=%0.1f, width=%0.1f)\n", (float)*target, (float)*start, (float)*x, (float)*y, (float)*width);
}


//348
static void EXEC_glConvolutionFilter1D(byte *commandbuf)
{

	LOG("glConvolutionFilter1D()\n");

}


//349
static void EXEC_glConvolutionFilter2D(byte *commandbuf)
{

	LOG("glConvolutionFilter2D()\n");

}


//350
static void EXEC_glConvolutionParameterf(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *params = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	LOG("glConvolutionParameterf(target=%0.1f, pname=%0.1f, params=%0.1f)\n", (float)*target, (float)*pname, (float)*params);
}


//351
static void EXEC_glConvolutionParameterfv(byte *commandbuf)
{

	LOG("glConvolutionParameterfv()\n");

}


//352
static void EXEC_glConvolutionParameteri(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *params = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	LOG("glConvolutionParameteri(target=%0.1f, pname=%0.1f, params=%0.1f)\n", (float)*target, (float)*pname, (float)*params);
}


//353
static void EXEC_glConvolutionParameteriv(byte *commandbuf)
{

	LOG("glConvolutionParameteriv()\n");

}


//354
static void EXEC_glCopyConvolutionFilter1D(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	LOG("glCopyConvolutionFilter1D(target=%0.1f, internalformat=%0.1f, x=%0.1f, y=%0.1f, width=%0.1f)\n", (float)*target, (float)*internalformat, (float)*x, (float)*y, (float)*width);
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

	LOG("glCopyConvolutionFilter2D(target=%0.1f, internalformat=%0.1f, x=%0.1f, y=%0.1f, width=%0.1f, height=%0.1f)\n", (float)*target, (float)*internalformat, (float)*x, (float)*y, (float)*width, (float)*height);
}


//356
static void EXEC_glGetConvolutionFilter(byte *commandbuf)
{

	LOG("glGetConvolutionFilter()\n");

}


//357
static void EXEC_glGetConvolutionParameterfv(byte *commandbuf)
{

	LOG("glGetConvolutionParameterfv()\n");

}


//358
static void EXEC_glGetConvolutionParameteriv(byte *commandbuf)
{

	LOG("glGetConvolutionParameteriv()\n");

}


//359
static void EXEC_glGetSeparableFilter(byte *commandbuf)
{

	LOG("glGetSeparableFilter()\n");

}


//360
static void EXEC_glSeparableFilter2D(byte *commandbuf)
{

	LOG("glSeparableFilter2D()\n");

}


//361
static void EXEC_glGetHistogram(byte *commandbuf)
{

	LOG("glGetHistogram()\n");

}


//362
static void EXEC_glGetHistogramParameterfv(byte *commandbuf)
{

	LOG("glGetHistogramParameterfv()\n");

}


//363
static void EXEC_glGetHistogramParameteriv(byte *commandbuf)
{

	LOG("glGetHistogramParameteriv()\n");

}


//364
static void EXEC_glGetMinmax(byte *commandbuf)
{

	LOG("glGetMinmax()\n");

}


//365
static void EXEC_glGetMinmaxParameterfv(byte *commandbuf)
{

	LOG("glGetMinmaxParameterfv()\n");

}


//366
static void EXEC_glGetMinmaxParameteriv(byte *commandbuf)
{

	LOG("glGetMinmaxParameteriv()\n");

}


//367
static void EXEC_glHistogram(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLboolean *sink = (GLboolean*)commandbuf;    commandbuf += sizeof(GLboolean);

	LOG("glHistogram(target=%0.1f, width=%0.1f, internalformat=%0.1f, sink=%0.1f)\n", (float)*target, (float)*width, (float)*internalformat, (float)*sink);
}


//368
static void EXEC_glMinmax(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLboolean *sink = (GLboolean*)commandbuf;    commandbuf += sizeof(GLboolean);

	LOG("glMinmax(target=%0.1f, internalformat=%0.1f, sink=%0.1f)\n", (float)*target, (float)*internalformat, (float)*sink);
}


//369
static void EXEC_glResetHistogram(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	LOG("glResetHistogram(target=%0.1f)\n", (float)*target);
}


//370
static void EXEC_glResetMinmax(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	LOG("glResetMinmax(target=%0.1f)\n", (float)*target);
}


//371
static void EXEC_glTexImage3D(byte *commandbuf)
{

	LOG("glTexImage3D()\n");

}


//372
static void EXEC_glTexSubImage3D(byte *commandbuf)
{

	LOG("glTexSubImage3D()\n");

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

	LOG("glCopyTexSubImage3D(target=%0.1f, level=%0.1f, xoffset=%0.1f, yoffset=%0.1f, zoffset=%0.1f, x=%0.1f, y=%0.1f, width=%0.1f, height=%0.1f)\n", (float)*target, (float)*level, (float)*xoffset, (float)*yoffset, (float)*zoffset, (float)*x, (float)*y, (float)*width, (float)*height);
}


//374
static void EXEC_glActiveTexture(byte *commandbuf)
{
	GLenum *texture = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	LOG("glActiveTexture(texture=%0.1f)\n", (float)*texture);
}


//375
static void EXEC_glClientActiveTexture(byte *commandbuf)
{
	GLenum *texture = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	LOG("glClientActiveTexture(texture=%0.1f)\n", (float)*texture);
}


//376
static void EXEC_glMultiTexCoord1d(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glMultiTexCoord1d(target=%0.1f, s=%0.1f)\n", (float)*target, (float)*s);
}


//377
static void EXEC_glMultiTexCoord1dv(byte *commandbuf)
{

	LOG("glMultiTexCoord1dv()\n");

}


//378
static void EXEC_glMultiTexCoord1f(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glMultiTexCoord1f(target=%0.1f, s=%0.1f)\n", (float)*target, (float)*s);
}


//379
static void EXEC_glMultiTexCoord1fv(byte *commandbuf)
{

	LOG("glMultiTexCoord1fv()\n");

}


//380
static void EXEC_glMultiTexCoord1i(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glMultiTexCoord1i(target=%0.1f, s=%0.1f)\n", (float)*target, (float)*s);
}


//381
static void EXEC_glMultiTexCoord1iv(byte *commandbuf)
{

	LOG("glMultiTexCoord1iv()\n");

}


//382
static void EXEC_glMultiTexCoord1s(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glMultiTexCoord1s(target=%0.1f, s=%0.1f)\n", (float)*target, (float)*s);
}


//383
static void EXEC_glMultiTexCoord1sv(byte *commandbuf)
{

	LOG("glMultiTexCoord1sv()\n");

}


//384
static void EXEC_glMultiTexCoord2d(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *t = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glMultiTexCoord2d(target=%0.1f, s=%0.1f, t=%0.1f)\n", (float)*target, (float)*s, (float)*t);
}


//385
static void EXEC_glMultiTexCoord2dv(byte *commandbuf)
{

	LOG("glMultiTexCoord2dv()\n");

}


//386
static void EXEC_glMultiTexCoord2f(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glMultiTexCoord2f(target=%0.1f, s=%0.1f, t=%0.1f)\n", (float)*target, (float)*s, (float)*t);
}


//387
static void EXEC_glMultiTexCoord2fv(byte *commandbuf)
{

	LOG("glMultiTexCoord2fv()\n");

}


//388
static void EXEC_glMultiTexCoord2i(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *t = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glMultiTexCoord2i(target=%0.1f, s=%0.1f, t=%0.1f)\n", (float)*target, (float)*s, (float)*t);
}


//389
static void EXEC_glMultiTexCoord2iv(byte *commandbuf)
{

	LOG("glMultiTexCoord2iv()\n");

}


//390
static void EXEC_glMultiTexCoord2s(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *t = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glMultiTexCoord2s(target=%0.1f, s=%0.1f, t=%0.1f)\n", (float)*target, (float)*s, (float)*t);
}


//391
static void EXEC_glMultiTexCoord2sv(byte *commandbuf)
{

	LOG("glMultiTexCoord2sv()\n");

}


//392
static void EXEC_glMultiTexCoord3d(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *t = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *r = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glMultiTexCoord3d(target=%0.1f, s=%0.1f, t=%0.1f, r=%0.1f)\n", (float)*target, (float)*s, (float)*t, (float)*r);
}


//393
static void EXEC_glMultiTexCoord3dv(byte *commandbuf)
{

	LOG("glMultiTexCoord3dv()\n");

}


//394
static void EXEC_glMultiTexCoord3f(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *r = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glMultiTexCoord3f(target=%0.1f, s=%0.1f, t=%0.1f, r=%0.1f)\n", (float)*target, (float)*s, (float)*t, (float)*r);
}


//395
static void EXEC_glMultiTexCoord3fv(byte *commandbuf)
{

	LOG("glMultiTexCoord3fv()\n");

}


//396
static void EXEC_glMultiTexCoord3i(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *t = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *r = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glMultiTexCoord3i(target=%0.1f, s=%0.1f, t=%0.1f, r=%0.1f)\n", (float)*target, (float)*s, (float)*t, (float)*r);
}


//397
static void EXEC_glMultiTexCoord3iv(byte *commandbuf)
{

	LOG("glMultiTexCoord3iv()\n");

}


//398
static void EXEC_glMultiTexCoord3s(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *t = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *r = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glMultiTexCoord3s(target=%0.1f, s=%0.1f, t=%0.1f, r=%0.1f)\n", (float)*target, (float)*s, (float)*t, (float)*r);
}


//399
static void EXEC_glMultiTexCoord3sv(byte *commandbuf)
{

	LOG("glMultiTexCoord3sv()\n");

}


//400
static void EXEC_glMultiTexCoord4d(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *t = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *r = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *q = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glMultiTexCoord4d(target=%0.1f, s=%0.1f, t=%0.1f, r=%0.1f, q=%0.1f)\n", (float)*target, (float)*s, (float)*t, (float)*r, (float)*q);
}


//401
static void EXEC_glMultiTexCoord4dv(byte *commandbuf)
{

	LOG("glMultiTexCoord4dv()\n");

}


//402
static void EXEC_glMultiTexCoord4f(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *r = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *q = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glMultiTexCoord4f(target=%0.1f, s=%0.1f, t=%0.1f, r=%0.1f, q=%0.1f)\n", (float)*target, (float)*s, (float)*t, (float)*r, (float)*q);
}


//403
static void EXEC_glMultiTexCoord4fv(byte *commandbuf)
{

	LOG("glMultiTexCoord4fv()\n");

}


//404
static void EXEC_glMultiTexCoord4i(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *t = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *r = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *q = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glMultiTexCoord4i(target=%0.1f, s=%0.1f, t=%0.1f, r=%0.1f, q=%0.1f)\n", (float)*target, (float)*s, (float)*t, (float)*r, (float)*q);
}


//405
static void EXEC_glMultiTexCoord4iv(byte *commandbuf)
{

	LOG("glMultiTexCoord4iv()\n");

}


//406
static void EXEC_glMultiTexCoord4s(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *t = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *r = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *q = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glMultiTexCoord4s(target=%0.1f, s=%0.1f, t=%0.1f, r=%0.1f, q=%0.1f)\n", (float)*target, (float)*s, (float)*t, (float)*r, (float)*q);
}


//407
static void EXEC_glMultiTexCoord4sv(byte *commandbuf)
{

	LOG("glMultiTexCoord4sv()\n");

}


//408
static void EXEC_glLoadTransposeMatrixf(byte *commandbuf)
{

	LOG("glLoadTransposeMatrixf()\n");

}


//409
static void EXEC_glLoadTransposeMatrixd(byte *commandbuf)
{

	LOG("glLoadTransposeMatrixd()\n");

}


//410
static void EXEC_glMultTransposeMatrixf(byte *commandbuf)
{

	LOG("glMultTransposeMatrixf()\n");

}


//411
static void EXEC_glMultTransposeMatrixd(byte *commandbuf)
{

	LOG("glMultTransposeMatrixd()\n");

}


//412
static void EXEC_glSampleCoverage(byte *commandbuf)
{
	GLclampf *value = (GLclampf*)commandbuf;     commandbuf += sizeof(GLclampf);
	GLboolean *invert = (GLboolean*)commandbuf;  commandbuf += sizeof(GLboolean);

	LOG("glSampleCoverage(value=%0.1f, invert=%0.1f)\n", (float)*value, (float)*invert);
}


//413
static void EXEC_glCompressedTexImage3D(byte *commandbuf)
{

	LOG("glCompressedTexImage3D()\n");

}


//414
static void EXEC_glCompressedTexImage2D(byte *commandbuf)
{

	LOG("glCompressedTexImage2D()\n");

}


//415
static void EXEC_glCompressedTexImage1D(byte *commandbuf)
{

	LOG("glCompressedTexImage1D()\n");

}


//416
static void EXEC_glCompressedTexSubImage3D(byte *commandbuf)
{

	LOG("glCompressedTexSubImage3D()\n");

}


//417
static void EXEC_glCompressedTexSubImage2D(byte *commandbuf)
{

	LOG("glCompressedTexSubImage2D()\n");

}


//418
static void EXEC_glCompressedTexSubImage1D(byte *commandbuf)
{

	LOG("glCompressedTexSubImage1D()\n");

}


//419
static void EXEC_glGetCompressedTexImage(byte *commandbuf)
{

	LOG("glGetCompressedTexImage()\n");

}


//420
static void EXEC_glBlendFuncSeparate(byte *commandbuf)
{
	GLenum *sfactorRGB = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *dfactorRGB = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *sfactorAlpha = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *dfactorAlpha = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glBlendFuncSeparate(sfactorRGB=%0.1f, dfactorRGB=%0.1f, sfactorAlpha=%0.1f, dfactorAlpha=%0.1f)\n", (float)*sfactorRGB, (float)*dfactorRGB, (float)*sfactorAlpha, (float)*dfactorAlpha);
}


//421
static void EXEC_glFogCoordf(byte *commandbuf)
{
	GLfloat *coord = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glFogCoordf(coord=%0.1f)\n", (float)*coord);
}


//422
static void EXEC_glFogCoordfv(byte *commandbuf)
{

	LOG("glFogCoordfv()\n");

}


//423
static void EXEC_glFogCoordd(byte *commandbuf)
{
	GLdouble *coord = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glFogCoordd(coord=%0.1f)\n", (float)*coord);
}


//424
static void EXEC_glFogCoorddv(byte *commandbuf)
{

	LOG("glFogCoorddv()\n");

}


//425
static void EXEC_glFogCoordPointer(byte *commandbuf)
{

	LOG("glFogCoordPointer()\n");

}


//426
static void EXEC_glMultiDrawArrays(byte *commandbuf)
{

	LOG("glMultiDrawArrays()\n");

}


//427
static void EXEC_glMultiDrawElements(byte *commandbuf)
{

	LOG("glMultiDrawElements()\n");

}


//428
static void EXEC_glPointParameterf(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glPointParameterf(pname=%0.1f, param=%0.1f)\n", (float)*pname, (float)*param);
}


//429
static void EXEC_glPointParameterfv(byte *commandbuf)
{

	LOG("glPointParameterfv()\n");

}


//430
static void EXEC_glPointParameteri(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glPointParameteri(pname=%0.1f, param=%0.1f)\n", (float)*pname, (float)*param);
}


//431
static void EXEC_glPointParameteriv(byte *commandbuf)
{

	LOG("glPointParameteriv()\n");

}


//432
static void EXEC_glSecondaryColor3b(byte *commandbuf)
{
	GLbyte *red = (GLbyte*)commandbuf;   commandbuf += sizeof(GLbyte);
	GLbyte *green = (GLbyte*)commandbuf;     commandbuf += sizeof(GLbyte);
	GLbyte *blue = (GLbyte*)commandbuf;  commandbuf += sizeof(GLbyte);

	LOG("glSecondaryColor3b(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//433
static void EXEC_glSecondaryColor3bv(byte *commandbuf)
{

	LOG("glSecondaryColor3bv()\n");

}


//434
static void EXEC_glSecondaryColor3d(byte *commandbuf)
{
	GLdouble *red = (GLdouble*)commandbuf;   commandbuf += sizeof(GLdouble);
	GLdouble *green = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *blue = (GLdouble*)commandbuf;  commandbuf += sizeof(GLdouble);

	LOG("glSecondaryColor3d(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//435
static void EXEC_glSecondaryColor3dv(byte *commandbuf)
{

	LOG("glSecondaryColor3dv()\n");

}


//436
static void EXEC_glSecondaryColor3f(byte *commandbuf)
{
	GLfloat *red = (GLfloat*)commandbuf;     commandbuf += sizeof(GLfloat);
	GLfloat *green = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *blue = (GLfloat*)commandbuf;    commandbuf += sizeof(GLfloat);

	LOG("glSecondaryColor3f(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//437
static void EXEC_glSecondaryColor3fv(byte *commandbuf)
{

	LOG("glSecondaryColor3fv()\n");

}


//438
static void EXEC_glSecondaryColor3i(byte *commandbuf)
{
	GLint *red = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *green = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *blue = (GLint*)commandbuf;    commandbuf += sizeof(GLint);

	LOG("glSecondaryColor3i(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//439
static void EXEC_glSecondaryColor3iv(byte *commandbuf)
{

	LOG("glSecondaryColor3iv()\n");

}


//440
static void EXEC_glSecondaryColor3s(byte *commandbuf)
{
	GLshort *red = (GLshort*)commandbuf;     commandbuf += sizeof(GLshort);
	GLshort *green = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *blue = (GLshort*)commandbuf;    commandbuf += sizeof(GLshort);

	LOG("glSecondaryColor3s(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//441
static void EXEC_glSecondaryColor3sv(byte *commandbuf)
{

	LOG("glSecondaryColor3sv()\n");

}


//442
static void EXEC_glSecondaryColor3ub(byte *commandbuf)
{
	GLubyte *red = (GLubyte*)commandbuf;     commandbuf += sizeof(GLubyte);
	GLubyte *green = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *blue = (GLubyte*)commandbuf;    commandbuf += sizeof(GLubyte);

	LOG("glSecondaryColor3ub(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//443
static void EXEC_glSecondaryColor3ubv(byte *commandbuf)
{

	LOG("glSecondaryColor3ubv()\n");

}


//444
static void EXEC_glSecondaryColor3ui(byte *commandbuf)
{
	GLuint *red = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *green = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLuint *blue = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	LOG("glSecondaryColor3ui(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//445
static void EXEC_glSecondaryColor3uiv(byte *commandbuf)
{

	LOG("glSecondaryColor3uiv()\n");

}


//446
static void EXEC_glSecondaryColor3us(byte *commandbuf)
{
	GLushort *red = (GLushort*)commandbuf;   commandbuf += sizeof(GLushort);
	GLushort *green = (GLushort*)commandbuf;     commandbuf += sizeof(GLushort);
	GLushort *blue = (GLushort*)commandbuf;  commandbuf += sizeof(GLushort);

	LOG("glSecondaryColor3us(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//447
static void EXEC_glSecondaryColor3usv(byte *commandbuf)
{

	LOG("glSecondaryColor3usv()\n");

}


//448
static void EXEC_glSecondaryColorPointer(byte *commandbuf)
{

	LOG("glSecondaryColorPointer()\n");

}


//449
static void EXEC_glWindowPos2d(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glWindowPos2d(x=%0.1f, y=%0.1f)\n", (float)*x, (float)*y);
}


//450
static void EXEC_glWindowPos2dv(byte *commandbuf)
{

	LOG("glWindowPos2dv()\n");

}


//451
static void EXEC_glWindowPos2f(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glWindowPos2f(x=%0.1f, y=%0.1f)\n", (float)*x, (float)*y);
}


//452
static void EXEC_glWindowPos2fv(byte *commandbuf)
{

	LOG("glWindowPos2fv()\n");

}


//453
static void EXEC_glWindowPos2i(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glWindowPos2i(x=%0.1f, y=%0.1f)\n", (float)*x, (float)*y);
}


//454
static void EXEC_glWindowPos2iv(byte *commandbuf)
{

	LOG("glWindowPos2iv()\n");

}


//455
static void EXEC_glWindowPos2s(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glWindowPos2s(x=%0.1f, y=%0.1f)\n", (float)*x, (float)*y);
}


//456
static void EXEC_glWindowPos2sv(byte *commandbuf)
{

	LOG("glWindowPos2sv()\n");

}


//457
static void EXEC_glWindowPos3d(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glWindowPos3d(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//458
static void EXEC_glWindowPos3dv(byte *commandbuf)
{

	LOG("glWindowPos3dv()\n");

}


//459
static void EXEC_glWindowPos3f(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glWindowPos3f(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//460
static void EXEC_glWindowPos3fv(byte *commandbuf)
{

	LOG("glWindowPos3fv()\n");

}


//461
static void EXEC_glWindowPos3i(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *z = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glWindowPos3i(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//462
static void EXEC_glWindowPos3iv(byte *commandbuf)
{

	LOG("glWindowPos3iv()\n");

}


//463
static void EXEC_glWindowPos3s(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glWindowPos3s(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//464
static void EXEC_glWindowPos3sv(byte *commandbuf)
{

	LOG("glWindowPos3sv()\n");

}


//465
static void EXEC_glBindBuffer(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *buffer = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	LOG("glBindBuffer(target=%0.1f, buffer=%0.1f)\n", (float)*target, (float)*buffer);
}


//466
static void EXEC_glBufferData(byte *commandbuf)
{

	LOG("glBufferData()\n");

}


//467
static void EXEC_glBufferSubData(byte *commandbuf)
{

	LOG("glBufferSubData()\n");

}


//468
static void EXEC_glDeleteBuffers(byte *commandbuf)
{

	LOG("glDeleteBuffers()\n");

}


//469
static void EXEC_glGenBuffers(byte *commandbuf)
{

	LOG("glGenBuffers()\n");

}


//470
static void EXEC_glGetBufferParameteriv(byte *commandbuf)
{

	LOG("glGetBufferParameteriv()\n");

}


//471
static void EXEC_glGetBufferPointerv(byte *commandbuf)
{

	LOG("glGetBufferPointerv()\n");

}


//472
static void EXEC_glGetBufferSubData(byte *commandbuf)
{

	LOG("glGetBufferSubData()\n");

}


//473
static void EXEC_glIsBuffer(byte *commandbuf)
{
	GLuint *buffer = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	LOG("glIsBuffer(buffer=%0.1f)\n", (float)*buffer);
}


//474
static void EXEC_glMapBuffer(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *access = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	LOG("glMapBuffer(target=%0.1f, access=%0.1f)\n", (float)*target, (float)*access);
}


//475
static void EXEC_glUnmapBuffer(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	LOG("glUnmapBuffer(target=%0.1f)\n", (float)*target);
}


//476
static void EXEC_glGenQueries(byte *commandbuf)
{

	LOG("glGenQueries()\n");

}


//477
static void EXEC_glDeleteQueries(byte *commandbuf)
{

	LOG("glDeleteQueries()\n");

}


//478
static void EXEC_glIsQuery(byte *commandbuf)
{
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	LOG("glIsQuery(id=%0.1f)\n", (float)*id);
}


//479
static void EXEC_glBeginQuery(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	LOG("glBeginQuery(target=%0.1f, id=%0.1f)\n", (float)*target, (float)*id);
}


//480
static void EXEC_glEndQuery(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	LOG("glEndQuery(target=%0.1f)\n", (float)*target);
}


//481
static void EXEC_glGetQueryiv(byte *commandbuf)
{

	LOG("glGetQueryiv()\n");

}


//482
static void EXEC_glGetQueryObjectiv(byte *commandbuf)
{

	LOG("glGetQueryObjectiv()\n");

}


//483
static void EXEC_glGetQueryObjectuiv(byte *commandbuf)
{

	LOG("glGetQueryObjectuiv()\n");

}


//484
static void EXEC_glBlendEquationSeparate(byte *commandbuf)
{
	GLenum *modeRGB = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLenum *modeA = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	LOG("glBlendEquationSeparate(modeRGB=%0.1f, modeA=%0.1f)\n", (float)*modeRGB, (float)*modeA);
}


//485
static void EXEC_glDrawBuffers(byte *commandbuf)
{

	LOG("glDrawBuffers()\n");

}


//486
static void EXEC_glStencilFuncSeparate(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *func = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLint *ref = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLuint *mask = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	LOG("glStencilFuncSeparate(face=%0.1f, func=%0.1f, ref=%0.1f, mask=%0.1f)\n", (float)*face, (float)*func, (float)*ref, (float)*mask);
}


//487
static void EXEC_glStencilOpSeparate(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *sfail = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *zfail = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *zpass = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	LOG("glStencilOpSeparate(face=%0.1f, sfail=%0.1f, zfail=%0.1f, zpass=%0.1f)\n", (float)*face, (float)*sfail, (float)*zfail, (float)*zpass);
}


//488
static void EXEC_glStencilMaskSeparate(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLuint *mask = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	LOG("glStencilMaskSeparate(face=%0.1f, mask=%0.1f)\n", (float)*face, (float)*mask);
}


//489
static void EXEC_glAttachShader(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *shader = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	LOG("glAttachShader(program=%0.1f, shader=%0.1f)\n", (float)*program, (float)*shader);
}


//490
static void EXEC_glBindAttribLocation(byte *commandbuf)
{

	LOG("glBindAttribLocation()\n");

}


//491
static void EXEC_glCompileShader(byte *commandbuf)
{
	GLuint *shader = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	LOG("glCompileShader(shader=%0.1f)\n", (float)*shader);
}


//492
static void EXEC_glCreateProgram(byte *commandbuf)
{

	LOG("glCreateProgram()\n");
}


//493
static void EXEC_glCreateShader(byte *commandbuf)
{
	GLenum *type = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glCreateShader(type=%0.1f)\n", (float)*type);
}


//494
static void EXEC_glDeleteProgram(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	LOG("glDeleteProgram(program=%0.1f)\n", (float)*program);
}


//495
static void EXEC_glDeleteShader(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	LOG("glDeleteShader(program=%0.1f)\n", (float)*program);
}


//496
static void EXEC_glDetachShader(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *shader = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	LOG("glDetachShader(program=%0.1f, shader=%0.1f)\n", (float)*program, (float)*shader);
}


//497
static void EXEC_glDisableVertexAttribArray(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	LOG("glDisableVertexAttribArray(index=%0.1f)\n", (float)*index);
}


//498
static void EXEC_glEnableVertexAttribArray(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	LOG("glEnableVertexAttribArray(index=%0.1f)\n", (float)*index);
}


//499
static void EXEC_glGetActiveAttrib(byte *commandbuf)
{

	LOG("glGetActiveAttrib()\n");

}


//500
static void EXEC_glGetActiveUniform(byte *commandbuf)
{

	LOG("glGetActiveUniform()\n");

}


//501
static void EXEC_glGetAttachedShaders(byte *commandbuf)
{

	LOG("glGetAttachedShaders()\n");

}


//502
static void EXEC_glGetAttribLocation(byte *commandbuf)
{

	LOG("glGetAttribLocation()\n");

}


//503
static void EXEC_glGetProgramiv(byte *commandbuf)
{

	LOG("glGetProgramiv()\n");

}


//504
static void EXEC_glGetProgramInfoLog(byte *commandbuf)
{

	LOG("glGetProgramInfoLog()\n");

}


//505
static void EXEC_glGetShaderiv(byte *commandbuf)
{

	LOG("glGetShaderiv()\n");

}


//506
static void EXEC_glGetShaderInfoLog(byte *commandbuf)
{

	LOG("glGetShaderInfoLog()\n");

}


//507
static void EXEC_glGetShaderSource(byte *commandbuf)
{

	LOG("glGetShaderSource()\n");

}


//508
static void EXEC_glGetUniformLocation(byte *commandbuf)
{

	LOG("glGetUniformLocation()\n");

}


//509
static void EXEC_glGetUniformfv(byte *commandbuf)
{

	LOG("glGetUniformfv()\n");

}


//510
static void EXEC_glGetUniformiv(byte *commandbuf)
{

	LOG("glGetUniformiv()\n");

}


//511
static void EXEC_glGetVertexAttribdv(byte *commandbuf)
{

	LOG("glGetVertexAttribdv()\n");

}


//512
static void EXEC_glGetVertexAttribfv(byte *commandbuf)
{

	LOG("glGetVertexAttribfv()\n");

}


//513
static void EXEC_glGetVertexAttribiv(byte *commandbuf)
{

	LOG("glGetVertexAttribiv()\n");

}


//514
static void EXEC_glGetVertexAttribPointerv(byte *commandbuf)
{

	LOG("glGetVertexAttribPointerv()\n");

}


//515
static void EXEC_glIsProgram(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	LOG("glIsProgram(program=%0.1f)\n", (float)*program);
}


//516
static void EXEC_glIsShader(byte *commandbuf)
{
	GLuint *shader = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	LOG("glIsShader(shader=%0.1f)\n", (float)*shader);
}


//517
static void EXEC_glLinkProgram(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	LOG("glLinkProgram(program=%0.1f)\n", (float)*program);
}


//518
static void EXEC_glShaderSource(byte *commandbuf)
{

	LOG("glShaderSource()\n");

}


//519
static void EXEC_glUseProgram(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	LOG("glUseProgram(program=%0.1f)\n", (float)*program);
}


//520
static void EXEC_glUniform1f(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLfloat *v0 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	LOG("glUniform1f(location=%0.1f, v0=%0.1f)\n", (float)*location, (float)*v0);
}


//521
static void EXEC_glUniform2f(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLfloat *v0 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	LOG("glUniform2f(location=%0.1f, v0=%0.1f, v1=%0.1f)\n", (float)*location, (float)*v0, (float)*v1);
}


//522
static void EXEC_glUniform3f(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLfloat *v0 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v2 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	LOG("glUniform3f(location=%0.1f, v0=%0.1f, v1=%0.1f, v2=%0.1f)\n", (float)*location, (float)*v0, (float)*v1, (float)*v2);
}


//523
static void EXEC_glUniform4f(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLfloat *v0 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v2 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v3 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	LOG("glUniform4f(location=%0.1f, v0=%0.1f, v1=%0.1f, v2=%0.1f, v3=%0.1f)\n", (float)*location, (float)*v0, (float)*v1, (float)*v2, (float)*v3);
}


//524
static void EXEC_glUniform1i(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLint *v0 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	LOG("glUniform1i(location=%0.1f, v0=%0.1f)\n", (float)*location, (float)*v0);
}


//525
static void EXEC_glUniform2i(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLint *v0 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v1 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	LOG("glUniform2i(location=%0.1f, v0=%0.1f, v1=%0.1f)\n", (float)*location, (float)*v0, (float)*v1);
}


//526
static void EXEC_glUniform3i(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLint *v0 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v1 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v2 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	LOG("glUniform3i(location=%0.1f, v0=%0.1f, v1=%0.1f, v2=%0.1f)\n", (float)*location, (float)*v0, (float)*v1, (float)*v2);
}


//527
static void EXEC_glUniform4i(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLint *v0 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v1 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v2 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v3 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	LOG("glUniform4i(location=%0.1f, v0=%0.1f, v1=%0.1f, v2=%0.1f, v3=%0.1f)\n", (float)*location, (float)*v0, (float)*v1, (float)*v2, (float)*v3);
}


//528
static void EXEC_glUniform1fv(byte *commandbuf)
{

	LOG("glUniform1fv()\n");

}


//529
static void EXEC_glUniform2fv(byte *commandbuf)
{

	LOG("glUniform2fv()\n");

}


//530
static void EXEC_glUniform3fv(byte *commandbuf)
{

	LOG("glUniform3fv()\n");

}


//531
static void EXEC_glUniform4fv(byte *commandbuf)
{

	LOG("glUniform4fv()\n");

}


//532
static void EXEC_glUniform1iv(byte *commandbuf)
{

	LOG("glUniform1iv()\n");

}


//533
static void EXEC_glUniform2iv(byte *commandbuf)
{

	LOG("glUniform2iv()\n");

}


//534
static void EXEC_glUniform3iv(byte *commandbuf)
{

	LOG("glUniform3iv()\n");

}


//535
static void EXEC_glUniform4iv(byte *commandbuf)
{

	LOG("glUniform4iv()\n");

}


//536
static void EXEC_glUniformMatrix2fv(byte *commandbuf)
{

	LOG("glUniformMatrix2fv()\n");

}


//537
static void EXEC_glUniformMatrix3fv(byte *commandbuf)
{

	LOG("glUniformMatrix3fv()\n");

}


//538
static void EXEC_glUniformMatrix4fv(byte *commandbuf)
{

	LOG("glUniformMatrix4fv()\n");

}


//539
static void EXEC_glValidateProgram(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	LOG("glValidateProgram(program=%0.1f)\n", (float)*program);
}


//540
static void EXEC_glVertexAttrib1d(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glVertexAttrib1d(index=%0.1f, x=%0.1f)\n", (float)*index, (float)*x);
}


//541
static void EXEC_glVertexAttrib1dv(byte *commandbuf)
{

	LOG("glVertexAttrib1dv()\n");

}


//542
static void EXEC_glVertexAttrib1f(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glVertexAttrib1f(index=%0.1f, x=%0.1f)\n", (float)*index, (float)*x);
}


//543
static void EXEC_glVertexAttrib1fv(byte *commandbuf)
{

	LOG("glVertexAttrib1fv()\n");

}


//544
static void EXEC_glVertexAttrib1s(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glVertexAttrib1s(index=%0.1f, x=%0.1f)\n", (float)*index, (float)*x);
}


//545
static void EXEC_glVertexAttrib1sv(byte *commandbuf)
{

	LOG("glVertexAttrib1sv()\n");

}


//546
static void EXEC_glVertexAttrib2d(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glVertexAttrib2d(index=%0.1f, x=%0.1f, y=%0.1f)\n", (float)*index, (float)*x, (float)*y);
}


//547
static void EXEC_glVertexAttrib2dv(byte *commandbuf)
{

	LOG("glVertexAttrib2dv()\n");

}


//548
static void EXEC_glVertexAttrib2f(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glVertexAttrib2f(index=%0.1f, x=%0.1f, y=%0.1f)\n", (float)*index, (float)*x, (float)*y);
}


//549
static void EXEC_glVertexAttrib2fv(byte *commandbuf)
{

	LOG("glVertexAttrib2fv()\n");

}


//550
static void EXEC_glVertexAttrib2s(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glVertexAttrib2s(index=%0.1f, x=%0.1f, y=%0.1f)\n", (float)*index, (float)*x, (float)*y);
}


//551
static void EXEC_glVertexAttrib2sv(byte *commandbuf)
{

	LOG("glVertexAttrib2sv()\n");

}


//552
static void EXEC_glVertexAttrib3d(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glVertexAttrib3d(index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*index, (float)*x, (float)*y, (float)*z);
}


//553
static void EXEC_glVertexAttrib3dv(byte *commandbuf)
{

	LOG("glVertexAttrib3dv()\n");

}


//554
static void EXEC_glVertexAttrib3f(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glVertexAttrib3f(index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*index, (float)*x, (float)*y, (float)*z);
}


//555
static void EXEC_glVertexAttrib3fv(byte *commandbuf)
{

	LOG("glVertexAttrib3fv()\n");

}


//556
static void EXEC_glVertexAttrib3s(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glVertexAttrib3s(index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*index, (float)*x, (float)*y, (float)*z);
}


//557
static void EXEC_glVertexAttrib3sv(byte *commandbuf)
{

	LOG("glVertexAttrib3sv()\n");

}


//558
static void EXEC_glVertexAttrib4Nbv(byte *commandbuf)
{

	LOG("glVertexAttrib4Nbv()\n");

}


//559
static void EXEC_glVertexAttrib4Niv(byte *commandbuf)
{

	LOG("glVertexAttrib4Niv()\n");

}


//560
static void EXEC_glVertexAttrib4Nsv(byte *commandbuf)
{

	LOG("glVertexAttrib4Nsv()\n");

}


//561
static void EXEC_glVertexAttrib4Nub(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLubyte *x = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *y = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *z = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *w = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);

	LOG("glVertexAttrib4Nub(index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*index, (float)*x, (float)*y, (float)*z, (float)*w);
}


//562
static void EXEC_glVertexAttrib4Nubv(byte *commandbuf)
{

	LOG("glVertexAttrib4Nubv()\n");

}


//563
static void EXEC_glVertexAttrib4Nuiv(byte *commandbuf)
{

	LOG("glVertexAttrib4Nuiv()\n");

}


//564
static void EXEC_glVertexAttrib4Nusv(byte *commandbuf)
{

	LOG("glVertexAttrib4Nusv()\n");

}


//565
static void EXEC_glVertexAttrib4bv(byte *commandbuf)
{

	LOG("glVertexAttrib4bv()\n");

}


//566
static void EXEC_glVertexAttrib4d(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *w = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glVertexAttrib4d(index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*index, (float)*x, (float)*y, (float)*z, (float)*w);
}


//567
static void EXEC_glVertexAttrib4dv(byte *commandbuf)
{

	LOG("glVertexAttrib4dv()\n");

}


//568
static void EXEC_glVertexAttrib4f(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *w = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glVertexAttrib4f(index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*index, (float)*x, (float)*y, (float)*z, (float)*w);
}


//569
static void EXEC_glVertexAttrib4fv(byte *commandbuf)
{

	LOG("glVertexAttrib4fv()\n");

}


//570
static void EXEC_glVertexAttrib4iv(byte *commandbuf)
{

	LOG("glVertexAttrib4iv()\n");

}


//571
static void EXEC_glVertexAttrib4s(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *w = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glVertexAttrib4s(index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*index, (float)*x, (float)*y, (float)*z, (float)*w);
}


//572
static void EXEC_glVertexAttrib4sv(byte *commandbuf)
{

	LOG("glVertexAttrib4sv()\n");

}


//573
static void EXEC_glVertexAttrib4ubv(byte *commandbuf)
{

	LOG("glVertexAttrib4ubv()\n");

}


//574
static void EXEC_glVertexAttrib4uiv(byte *commandbuf)
{

	LOG("glVertexAttrib4uiv()\n");

}


//575
static void EXEC_glVertexAttrib4usv(byte *commandbuf)
{

	LOG("glVertexAttrib4usv()\n");

}


//576
static void EXEC_glVertexAttribPointer(byte *commandbuf)
{

	LOG("glVertexAttribPointer()\n");

}


//577
static void EXEC_glUniformMatrix2x3fv(byte *commandbuf)
{

	LOG("glUniformMatrix2x3fv()\n");

}


//578
static void EXEC_glUniformMatrix3x2fv(byte *commandbuf)
{

	LOG("glUniformMatrix3x2fv()\n");

}


//579
static void EXEC_glUniformMatrix2x4fv(byte *commandbuf)
{

	LOG("glUniformMatrix2x4fv()\n");

}


//580
static void EXEC_glUniformMatrix4x2fv(byte *commandbuf)
{

	LOG("glUniformMatrix4x2fv()\n");

}


//581
static void EXEC_glUniformMatrix3x4fv(byte *commandbuf)
{

	LOG("glUniformMatrix3x4fv()\n");

}


//582
static void EXEC_glUniformMatrix4x3fv(byte *commandbuf)
{

	LOG("glUniformMatrix4x3fv()\n");

}


//374
static void EXEC_glActiveTextureARB(byte *commandbuf)
{
	GLenum *texture = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	LOG("glActiveTextureARB(texture=%0.1f)\n", (float)*texture);
}


//375
static void EXEC_glClientActiveTextureARB(byte *commandbuf)
{
	GLenum *texture = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	LOG("glClientActiveTextureARB(texture=%0.1f)\n", (float)*texture);
}


//376
static void EXEC_glMultiTexCoord1dARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glMultiTexCoord1dARB(target=%0.1f, s=%0.1f)\n", (float)*target, (float)*s);
}


//377
static void EXEC_glMultiTexCoord1dvARB(byte *commandbuf)
{

	LOG("glMultiTexCoord1dvARB()\n");

}


//378
static void EXEC_glMultiTexCoord1fARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glMultiTexCoord1fARB(target=%0.1f, s=%0.1f)\n", (float)*target, (float)*s);
}


//379
static void EXEC_glMultiTexCoord1fvARB(byte *commandbuf)
{

	LOG("glMultiTexCoord1fvARB()\n");

}


//380
static void EXEC_glMultiTexCoord1iARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glMultiTexCoord1iARB(target=%0.1f, s=%0.1f)\n", (float)*target, (float)*s);
}


//381
static void EXEC_glMultiTexCoord1ivARB(byte *commandbuf)
{

	LOG("glMultiTexCoord1ivARB()\n");

}


//382
static void EXEC_glMultiTexCoord1sARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glMultiTexCoord1sARB(target=%0.1f, s=%0.1f)\n", (float)*target, (float)*s);
}


//383
static void EXEC_glMultiTexCoord1svARB(byte *commandbuf)
{

	LOG("glMultiTexCoord1svARB()\n");

}


//384
static void EXEC_glMultiTexCoord2dARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *t = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glMultiTexCoord2dARB(target=%0.1f, s=%0.1f, t=%0.1f)\n", (float)*target, (float)*s, (float)*t);
}


//385
static void EXEC_glMultiTexCoord2dvARB(byte *commandbuf)
{

	LOG("glMultiTexCoord2dvARB()\n");

}


//386
static void EXEC_glMultiTexCoord2fARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glMultiTexCoord2fARB(target=%0.1f, s=%0.1f, t=%0.1f)\n", (float)*target, (float)*s, (float)*t);
}


//387
static void EXEC_glMultiTexCoord2fvARB(byte *commandbuf)
{

	LOG("glMultiTexCoord2fvARB()\n");

}


//388
static void EXEC_glMultiTexCoord2iARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *t = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glMultiTexCoord2iARB(target=%0.1f, s=%0.1f, t=%0.1f)\n", (float)*target, (float)*s, (float)*t);
}


//389
static void EXEC_glMultiTexCoord2ivARB(byte *commandbuf)
{

	LOG("glMultiTexCoord2ivARB()\n");

}


//390
static void EXEC_glMultiTexCoord2sARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *t = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glMultiTexCoord2sARB(target=%0.1f, s=%0.1f, t=%0.1f)\n", (float)*target, (float)*s, (float)*t);
}


//391
static void EXEC_glMultiTexCoord2svARB(byte *commandbuf)
{

	LOG("glMultiTexCoord2svARB()\n");

}


//392
static void EXEC_glMultiTexCoord3dARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *t = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *r = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glMultiTexCoord3dARB(target=%0.1f, s=%0.1f, t=%0.1f, r=%0.1f)\n", (float)*target, (float)*s, (float)*t, (float)*r);
}


//393
static void EXEC_glMultiTexCoord3dvARB(byte *commandbuf)
{

	LOG("glMultiTexCoord3dvARB()\n");

}


//394
static void EXEC_glMultiTexCoord3fARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *r = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glMultiTexCoord3fARB(target=%0.1f, s=%0.1f, t=%0.1f, r=%0.1f)\n", (float)*target, (float)*s, (float)*t, (float)*r);
}


//395
static void EXEC_glMultiTexCoord3fvARB(byte *commandbuf)
{

	LOG("glMultiTexCoord3fvARB()\n");

}


//396
static void EXEC_glMultiTexCoord3iARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *t = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *r = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glMultiTexCoord3iARB(target=%0.1f, s=%0.1f, t=%0.1f, r=%0.1f)\n", (float)*target, (float)*s, (float)*t, (float)*r);
}


//397
static void EXEC_glMultiTexCoord3ivARB(byte *commandbuf)
{

	LOG("glMultiTexCoord3ivARB()\n");

}


//398
static void EXEC_glMultiTexCoord3sARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *t = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *r = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glMultiTexCoord3sARB(target=%0.1f, s=%0.1f, t=%0.1f, r=%0.1f)\n", (float)*target, (float)*s, (float)*t, (float)*r);
}


//399
static void EXEC_glMultiTexCoord3svARB(byte *commandbuf)
{

	LOG("glMultiTexCoord3svARB()\n");

}


//400
static void EXEC_glMultiTexCoord4dARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLdouble *s = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *t = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *r = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *q = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glMultiTexCoord4dARB(target=%0.1f, s=%0.1f, t=%0.1f, r=%0.1f, q=%0.1f)\n", (float)*target, (float)*s, (float)*t, (float)*r, (float)*q);
}


//401
static void EXEC_glMultiTexCoord4dvARB(byte *commandbuf)
{

	LOG("glMultiTexCoord4dvARB()\n");

}


//402
static void EXEC_glMultiTexCoord4fARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *r = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *q = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glMultiTexCoord4fARB(target=%0.1f, s=%0.1f, t=%0.1f, r=%0.1f, q=%0.1f)\n", (float)*target, (float)*s, (float)*t, (float)*r, (float)*q);
}


//403
static void EXEC_glMultiTexCoord4fvARB(byte *commandbuf)
{

	LOG("glMultiTexCoord4fvARB()\n");

}


//404
static void EXEC_glMultiTexCoord4iARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *s = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *t = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *r = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *q = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glMultiTexCoord4iARB(target=%0.1f, s=%0.1f, t=%0.1f, r=%0.1f, q=%0.1f)\n", (float)*target, (float)*s, (float)*t, (float)*r, (float)*q);
}


//405
static void EXEC_glMultiTexCoord4ivARB(byte *commandbuf)
{

	LOG("glMultiTexCoord4ivARB()\n");

}


//406
static void EXEC_glMultiTexCoord4sARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLshort *s = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *t = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *r = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *q = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glMultiTexCoord4sARB(target=%0.1f, s=%0.1f, t=%0.1f, r=%0.1f, q=%0.1f)\n", (float)*target, (float)*s, (float)*t, (float)*r, (float)*q);
}


//407
static void EXEC_glMultiTexCoord4svARB(byte *commandbuf)
{

	LOG("glMultiTexCoord4svARB()\n");

}


//617
static void EXEC_glLoadTransposeMatrixfARB(byte *commandbuf)
{

	LOG("glLoadTransposeMatrixfARB()\n");

}


//618
static void EXEC_glLoadTransposeMatrixdARB(byte *commandbuf)
{

	LOG("glLoadTransposeMatrixdARB()\n");

}


//619
static void EXEC_glMultTransposeMatrixfARB(byte *commandbuf)
{

	LOG("glMultTransposeMatrixfARB()\n");

}


//620
static void EXEC_glMultTransposeMatrixdARB(byte *commandbuf)
{

	LOG("glMultTransposeMatrixdARB()\n");

}


//621
static void EXEC_glSampleCoverageARB(byte *commandbuf)
{
	GLclampf *value = (GLclampf*)commandbuf;     commandbuf += sizeof(GLclampf);
	GLboolean *invert = (GLboolean*)commandbuf;  commandbuf += sizeof(GLboolean);

	LOG("glSampleCoverageARB(value=%0.1f, invert=%0.1f)\n", (float)*value, (float)*invert);
}


//622
static void EXEC_glCompressedTexImage3DARB(byte *commandbuf)
{

	LOG("glCompressedTexImage3DARB()\n");

}


//623
static void EXEC_glCompressedTexImage2DARB(byte *commandbuf)
{

	LOG("glCompressedTexImage2DARB()\n");

}


//624
static void EXEC_glCompressedTexImage1DARB(byte *commandbuf)
{

	LOG("glCompressedTexImage1DARB()\n");

}


//625
static void EXEC_glCompressedTexSubImage3DARB(byte *commandbuf)
{

	LOG("glCompressedTexSubImage3DARB()\n");

}


//626
static void EXEC_glCompressedTexSubImage2DARB(byte *commandbuf)
{

	LOG("glCompressedTexSubImage2DARB()\n");

}


//627
static void EXEC_glCompressedTexSubImage1DARB(byte *commandbuf)
{

	LOG("glCompressedTexSubImage1DARB()\n");

}


//628
static void EXEC_glGetCompressedTexImageARB(byte *commandbuf)
{

	LOG("glGetCompressedTexImageARB()\n");

}


//629
static void EXEC_glPointParameterfARB(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glPointParameterfARB(pname=%0.1f, param=%0.1f)\n", (float)*pname, (float)*param);
}


//630
static void EXEC_glPointParameterfvARB(byte *commandbuf)
{

	LOG("glPointParameterfvARB()\n");

}


//631
static void EXEC_glWeightbvARB(byte *commandbuf)
{

	LOG("glWeightbvARB()\n");

}


//632
static void EXEC_glWeightsvARB(byte *commandbuf)
{

	LOG("glWeightsvARB()\n");

}


//633
static void EXEC_glWeightivARB(byte *commandbuf)
{

	LOG("glWeightivARB()\n");

}


//634
static void EXEC_glWeightfvARB(byte *commandbuf)
{

	LOG("glWeightfvARB()\n");

}


//635
static void EXEC_glWeightdvARB(byte *commandbuf)
{

	LOG("glWeightdvARB()\n");

}


//636
static void EXEC_glWeightubvARB(byte *commandbuf)
{

	LOG("glWeightubvARB()\n");

}


//637
static void EXEC_glWeightusvARB(byte *commandbuf)
{

	LOG("glWeightusvARB()\n");

}


//638
static void EXEC_glWeightuivARB(byte *commandbuf)
{

	LOG("glWeightuivARB()\n");

}


//639
static void EXEC_glWeightPointerARB(byte *commandbuf)
{

	LOG("glWeightPointerARB()\n");

}


//640
static void EXEC_glVertexBlendARB(byte *commandbuf)
{
	GLint *count = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glVertexBlendARB(count=%0.1f)\n", (float)*count);
}


//641
static void EXEC_glCurrentPaletteMatrixARB(byte *commandbuf)
{
	GLint *index = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glCurrentPaletteMatrixARB(index=%0.1f)\n", (float)*index);
}


//642
static void EXEC_glMatrixIndexubvARB(byte *commandbuf)
{

	LOG("glMatrixIndexubvARB()\n");

}


//643
static void EXEC_glMatrixIndexusvARB(byte *commandbuf)
{

	LOG("glMatrixIndexusvARB()\n");

}


//644
static void EXEC_glMatrixIndexuivARB(byte *commandbuf)
{

	LOG("glMatrixIndexuivARB()\n");

}


//645
static void EXEC_glMatrixIndexPointerARB(byte *commandbuf)
{

	LOG("glMatrixIndexPointerARB()\n");

}


//646
static void EXEC_glWindowPos2dARB(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glWindowPos2dARB(x=%0.1f, y=%0.1f)\n", (float)*x, (float)*y);
}


//647
static void EXEC_glWindowPos2fARB(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glWindowPos2fARB(x=%0.1f, y=%0.1f)\n", (float)*x, (float)*y);
}


//648
static void EXEC_glWindowPos2iARB(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glWindowPos2iARB(x=%0.1f, y=%0.1f)\n", (float)*x, (float)*y);
}


//649
static void EXEC_glWindowPos2sARB(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glWindowPos2sARB(x=%0.1f, y=%0.1f)\n", (float)*x, (float)*y);
}


//650
static void EXEC_glWindowPos2dvARB(byte *commandbuf)
{

	LOG("glWindowPos2dvARB()\n");

}


//651
static void EXEC_glWindowPos2fvARB(byte *commandbuf)
{

	LOG("glWindowPos2fvARB()\n");

}


//652
static void EXEC_glWindowPos2ivARB(byte *commandbuf)
{

	LOG("glWindowPos2ivARB()\n");

}


//653
static void EXEC_glWindowPos2svARB(byte *commandbuf)
{

	LOG("glWindowPos2svARB()\n");

}


//654
static void EXEC_glWindowPos3dARB(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glWindowPos3dARB(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//655
static void EXEC_glWindowPos3fARB(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glWindowPos3fARB(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//656
static void EXEC_glWindowPos3iARB(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *z = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glWindowPos3iARB(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//657
static void EXEC_glWindowPos3sARB(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glWindowPos3sARB(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//658
static void EXEC_glWindowPos3dvARB(byte *commandbuf)
{

	LOG("glWindowPos3dvARB()\n");

}


//659
static void EXEC_glWindowPos3fvARB(byte *commandbuf)
{

	LOG("glWindowPos3fvARB()\n");

}


//660
static void EXEC_glWindowPos3ivARB(byte *commandbuf)
{

	LOG("glWindowPos3ivARB()\n");

}


//661
static void EXEC_glWindowPos3svARB(byte *commandbuf)
{

	LOG("glWindowPos3svARB()\n");

}


//662
static void EXEC_glGetVertexAttribdvARB(byte *commandbuf)
{

	LOG("glGetVertexAttribdvARB()\n");

}


//663
static void EXEC_glGetVertexAttribfvARB(byte *commandbuf)
{

	LOG("glGetVertexAttribfvARB()\n");

}


//664
static void EXEC_glGetVertexAttribivARB(byte *commandbuf)
{

	LOG("glGetVertexAttribivARB()\n");

}


//665
static void EXEC_glVertexAttrib1dARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glVertexAttrib1dARB(index=%0.1f, x=%0.1f)\n", (float)*index, (float)*x);
}


//666
static void EXEC_glVertexAttrib1dvARB(byte *commandbuf)
{

	LOG("glVertexAttrib1dvARB()\n");

}


//667
static void EXEC_glVertexAttrib1fARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glVertexAttrib1fARB(index=%0.1f, x=%0.1f)\n", (float)*index, (float)*x);
}


//668
static void EXEC_glVertexAttrib1fvARB(byte *commandbuf)
{

	LOG("glVertexAttrib1fvARB()\n");

}


//669
static void EXEC_glVertexAttrib1sARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glVertexAttrib1sARB(index=%0.1f, x=%0.1f)\n", (float)*index, (float)*x);
}


//670
static void EXEC_glVertexAttrib1svARB(byte *commandbuf)
{

	LOG("glVertexAttrib1svARB()\n");

}


//671
static void EXEC_glVertexAttrib2dARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glVertexAttrib2dARB(index=%0.1f, x=%0.1f, y=%0.1f)\n", (float)*index, (float)*x, (float)*y);
}


//672
static void EXEC_glVertexAttrib2dvARB(byte *commandbuf)
{

	LOG("glVertexAttrib2dvARB()\n");

}


//673
static void EXEC_glVertexAttrib2fARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glVertexAttrib2fARB(index=%0.1f, x=%0.1f, y=%0.1f)\n", (float)*index, (float)*x, (float)*y);
}


//674
static void EXEC_glVertexAttrib2fvARB(byte *commandbuf)
{

	LOG("glVertexAttrib2fvARB()\n");

}


//675
static void EXEC_glVertexAttrib2sARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glVertexAttrib2sARB(index=%0.1f, x=%0.1f, y=%0.1f)\n", (float)*index, (float)*x, (float)*y);
}


//676
static void EXEC_glVertexAttrib2svARB(byte *commandbuf)
{

	LOG("glVertexAttrib2svARB()\n");

}


//677
static void EXEC_glVertexAttrib3dARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glVertexAttrib3dARB(index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*index, (float)*x, (float)*y, (float)*z);
}


//678
static void EXEC_glVertexAttrib3dvARB(byte *commandbuf)
{

	LOG("glVertexAttrib3dvARB()\n");

}


//679
static void EXEC_glVertexAttrib3fARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glVertexAttrib3fARB(index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*index, (float)*x, (float)*y, (float)*z);
}


//680
static void EXEC_glVertexAttrib3fvARB(byte *commandbuf)
{

	LOG("glVertexAttrib3fvARB()\n");

}


//681
static void EXEC_glVertexAttrib3sARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glVertexAttrib3sARB(index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*index, (float)*x, (float)*y, (float)*z);
}


//682
static void EXEC_glVertexAttrib3svARB(byte *commandbuf)
{

	LOG("glVertexAttrib3svARB()\n");

}


//683
static void EXEC_glVertexAttrib4dARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *w = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glVertexAttrib4dARB(index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*index, (float)*x, (float)*y, (float)*z, (float)*w);
}


//684
static void EXEC_glVertexAttrib4dvARB(byte *commandbuf)
{

	LOG("glVertexAttrib4dvARB()\n");

}


//685
static void EXEC_glVertexAttrib4fARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *w = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glVertexAttrib4fARB(index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*index, (float)*x, (float)*y, (float)*z, (float)*w);
}


//686
static void EXEC_glVertexAttrib4fvARB(byte *commandbuf)
{

	LOG("glVertexAttrib4fvARB()\n");

}


//687
static void EXEC_glVertexAttrib4sARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *w = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glVertexAttrib4sARB(index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*index, (float)*x, (float)*y, (float)*z, (float)*w);
}


//688
static void EXEC_glVertexAttrib4svARB(byte *commandbuf)
{

	LOG("glVertexAttrib4svARB()\n");

}


//689
static void EXEC_glVertexAttrib4NubARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLubyte *x = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *y = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *z = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *w = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);

	LOG("glVertexAttrib4NubARB(index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*index, (float)*x, (float)*y, (float)*z, (float)*w);
}


//690
static void EXEC_glVertexAttrib4NubvARB(byte *commandbuf)
{

	LOG("glVertexAttrib4NubvARB()\n");

}


//691
static void EXEC_glVertexAttrib4bvARB(byte *commandbuf)
{

	LOG("glVertexAttrib4bvARB()\n");

}


//692
static void EXEC_glVertexAttrib4ivARB(byte *commandbuf)
{

	LOG("glVertexAttrib4ivARB()\n");

}


//693
static void EXEC_glVertexAttrib4ubvARB(byte *commandbuf)
{

	LOG("glVertexAttrib4ubvARB()\n");

}


//694
static void EXEC_glVertexAttrib4usvARB(byte *commandbuf)
{

	LOG("glVertexAttrib4usvARB()\n");

}


//695
static void EXEC_glVertexAttrib4uivARB(byte *commandbuf)
{

	LOG("glVertexAttrib4uivARB()\n");

}


//696
static void EXEC_glVertexAttrib4NbvARB(byte *commandbuf)
{

	LOG("glVertexAttrib4NbvARB()\n");

}


//697
static void EXEC_glVertexAttrib4NsvARB(byte *commandbuf)
{

	LOG("glVertexAttrib4NsvARB()\n");

}


//698
static void EXEC_glVertexAttrib4NivARB(byte *commandbuf)
{

	LOG("glVertexAttrib4NivARB()\n");

}


//699
static void EXEC_glVertexAttrib4NusvARB(byte *commandbuf)
{

	LOG("glVertexAttrib4NusvARB()\n");

}


//700
static void EXEC_glVertexAttrib4NuivARB(byte *commandbuf)
{

	LOG("glVertexAttrib4NuivARB()\n");

}


//701
static void EXEC_glVertexAttribPointerARB(byte *commandbuf)
{

	LOG("glVertexAttribPointerARB()\n");

}


//702
static void EXEC_glEnableVertexAttribArrayARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	LOG("glEnableVertexAttribArrayARB(index=%0.1f)\n", (float)*index);
}


//703
static void EXEC_glDisableVertexAttribArrayARB(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	LOG("glDisableVertexAttribArrayARB(index=%0.1f)\n", (float)*index);
}


//704
static void EXEC_glProgramStringARB(byte *commandbuf)
{

	LOG("glProgramStringARB()\n");

}


//705
static void EXEC_glBindProgramARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	LOG("glBindProgramARB(target=%0.1f, program=%0.1f)\n", (float)*target, (float)*program);
}


//706
static void EXEC_glDeleteProgramsARB(byte *commandbuf)
{

	LOG("glDeleteProgramsARB()\n");

}


//707
static void EXEC_glGenProgramsARB(byte *commandbuf)
{

	LOG("glGenProgramsARB()\n");

}


//708
static void EXEC_glIsProgramARB(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	LOG("glIsProgramARB(program=%0.1f)\n", (float)*program);
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

	LOG("glProgramEnvParameter4dARB(target=%0.1f, index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*target, (float)*index, (float)*x, (float)*y, (float)*z, (float)*w);
}


//710
static void EXEC_glProgramEnvParameter4dvARB(byte *commandbuf)
{

	LOG("glProgramEnvParameter4dvARB()\n");

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

	LOG("glProgramEnvParameter4fARB(target=%0.1f, index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*target, (float)*index, (float)*x, (float)*y, (float)*z, (float)*w);
}


//712
static void EXEC_glProgramEnvParameter4fvARB(byte *commandbuf)
{

	LOG("glProgramEnvParameter4fvARB()\n");

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

	LOG("glProgramLocalParameter4dARB(target=%0.1f, index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*target, (float)*index, (float)*x, (float)*y, (float)*z, (float)*w);
}


//714
static void EXEC_glProgramLocalParameter4dvARB(byte *commandbuf)
{

	LOG("glProgramLocalParameter4dvARB()\n");

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

	LOG("glProgramLocalParameter4fARB(target=%0.1f, index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*target, (float)*index, (float)*x, (float)*y, (float)*z, (float)*w);
}


//716
static void EXEC_glProgramLocalParameter4fvARB(byte *commandbuf)
{

	LOG("glProgramLocalParameter4fvARB()\n");

}


//717
static void EXEC_glGetProgramEnvParameterdvARB(byte *commandbuf)
{

	LOG("glGetProgramEnvParameterdvARB()\n");

}


//718
static void EXEC_glGetProgramEnvParameterfvARB(byte *commandbuf)
{

	LOG("glGetProgramEnvParameterfvARB()\n");

}


//719
static void EXEC_glGetProgramLocalParameterdvARB(byte *commandbuf)
{

	LOG("glGetProgramLocalParameterdvARB()\n");

}


//720
static void EXEC_glGetProgramLocalParameterfvARB(byte *commandbuf)
{

	LOG("glGetProgramLocalParameterfvARB()\n");

}


//721
static void EXEC_glGetProgramivARB(byte *commandbuf)
{

	LOG("glGetProgramivARB()\n");

}


//722
static void EXEC_glGetProgramStringARB(byte *commandbuf)
{

	LOG("glGetProgramStringARB()\n");

}


//723
static void EXEC_glGetVertexAttribPointervARB(byte *commandbuf)
{

	LOG("glGetVertexAttribPointervARB()\n");

}


//724
static void EXEC_glBindBufferARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *buffer = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	LOG("glBindBufferARB(target=%0.1f, buffer=%0.1f)\n", (float)*target, (float)*buffer);
}


//725
static void EXEC_glBufferDataARB(byte *commandbuf)
{

	LOG("glBufferDataARB()\n");

}


//726
static void EXEC_glBufferSubDataARB(byte *commandbuf)
{

	LOG("glBufferSubDataARB()\n");

}


//727
static void EXEC_glDeleteBuffersARB(byte *commandbuf)
{

	LOG("glDeleteBuffersARB()\n");

}


//728
static void EXEC_glGenBuffersARB(byte *commandbuf)
{

	LOG("glGenBuffersARB()\n");

}


//729
static void EXEC_glGetBufferParameterivARB(byte *commandbuf)
{

	LOG("glGetBufferParameterivARB()\n");

}


//730
static void EXEC_glGetBufferPointervARB(byte *commandbuf)
{

	LOG("glGetBufferPointervARB()\n");

}


//731
static void EXEC_glGetBufferSubDataARB(byte *commandbuf)
{

	LOG("glGetBufferSubDataARB()\n");

}


//732
static void EXEC_glIsBufferARB(byte *commandbuf)
{
	GLuint *buffer = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	LOG("glIsBufferARB(buffer=%0.1f)\n", (float)*buffer);
}


//733
static void EXEC_glMapBufferARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *access = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	LOG("glMapBufferARB(target=%0.1f, access=%0.1f)\n", (float)*target, (float)*access);
}


//734
static void EXEC_glUnmapBufferARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	LOG("glUnmapBufferARB(target=%0.1f)\n", (float)*target);
}


//735
static void EXEC_glGenQueriesARB(byte *commandbuf)
{

	LOG("glGenQueriesARB()\n");

}


//736
static void EXEC_glDeleteQueriesARB(byte *commandbuf)
{

	LOG("glDeleteQueriesARB()\n");

}


//737
static void EXEC_glIsQueryARB(byte *commandbuf)
{
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	LOG("glIsQueryARB(id=%0.1f)\n", (float)*id);
}


//738
static void EXEC_glBeginQueryARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	LOG("glBeginQueryARB(target=%0.1f, id=%0.1f)\n", (float)*target, (float)*id);
}


//739
static void EXEC_glEndQueryARB(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	LOG("glEndQueryARB(target=%0.1f)\n", (float)*target);
}


//740
static void EXEC_glGetQueryivARB(byte *commandbuf)
{

	LOG("glGetQueryivARB()\n");

}


//741
static void EXEC_glGetQueryObjectivARB(byte *commandbuf)
{

	LOG("glGetQueryObjectivARB()\n");

}


//742
static void EXEC_glGetQueryObjectuivARB(byte *commandbuf)
{

	LOG("glGetQueryObjectuivARB()\n");

}


//743
static void EXEC_glDeleteObjectARB(byte *commandbuf)
{
#ifndef __APPLE__
	GLhandleARB *obj = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);

	LOG("glDeleteObjectARB(obj=%0.1f)\n", (int)*obj);
#endif
}


//744
static void EXEC_glGetHandleARB(byte *commandbuf)
{
#ifndef __APPLE__
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	LOG("glGetHandleARB(pname=%0.1f)\n", (float)*pname);
#endif
}


//745
static void EXEC_glDetachObjectARB(byte *commandbuf)
{
#ifndef __APPLE__
	GLhandleARB *containerObj = (GLhandleARB*)commandbuf;    commandbuf += sizeof(GLhandleARB);
	GLhandleARB *attachedObj = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);

	LOG("glDetachObjectARB(containerObj=%0.1f, attachedObj=%0.1f)\n", (float)*containerObj, (float)*attachedObj);
#endif
}


//746
static void EXEC_glCreateShaderObjectARB(byte *commandbuf)
{
#ifndef __APPLE__
	GLenum *shaderType = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	LOG("glCreateShaderObjectARB(shaderType=%0.1f)\n", (float)*shaderType);
#endif
}


//747
static void EXEC_glShaderSourceARB(byte *commandbuf)
{

	LOG("glShaderSourceARB()\n");

}


//748
static void EXEC_glCompileShaderARB(byte *commandbuf)
{
#ifndef __APPLE__
	GLhandleARB *shader = (GLhandleARB*)commandbuf;  commandbuf += sizeof(GLhandleARB);

	LOG("glCompileShaderARB(shader=%0.1f)\n", (float)*shader);
#endif
}


//749
static void EXEC_glCreateProgramObjectARB(byte *commandbuf)
{

	LOG("glCreateProgramObjectARB()\n");
}


//750
static void EXEC_glAttachObjectARB(byte *commandbuf)
{
#ifndef __APPLE__
	GLhandleARB *containerObj = (GLhandleARB*)commandbuf;    commandbuf += sizeof(GLhandleARB);
	GLhandleARB *obj = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);

	LOG("glAttachObjectARB(containerObj=%0.1f, obj=%0.1f)\n", (float)*containerObj, (float)*obj);
#endif
}


//751
static void EXEC_glLinkProgramARB(byte *commandbuf)
{
	GLhandleARB *program = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);
#ifndef __APPLE__
	LOG("glLinkProgramARB(program=%0.1f)\n", (float)*program);
#endif
}



//752
static void EXEC_glUseProgramObjectARB(byte *commandbuf)
{
	GLhandleARB *program = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);
#ifndef __APPLE__
	LOG("glUseProgramObjectARB(program=%0.1f)\n", (float)*program);
#endif
}


//753
static void EXEC_glValidateProgramARB(byte *commandbuf)
{
	GLhandleARB *program = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);
#ifndef __APPLE__
	LOG("glValidateProgramARB(program=%0.1f)\n", (float)*program);
#endif
}


//754
static void EXEC_glUniform1fARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLfloat *v0 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	LOG("glUniform1fARB(location=%0.1f, v0=%0.1f)\n", (float)*location, (float)*v0);
}


//755
static void EXEC_glUniform2fARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLfloat *v0 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	LOG("glUniform2fARB(location=%0.1f, v0=%0.1f, v1=%0.1f)\n", (float)*location, (float)*v0, (float)*v1);
}


//756
static void EXEC_glUniform3fARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLfloat *v0 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v2 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	LOG("glUniform3fARB(location=%0.1f, v0=%0.1f, v1=%0.1f, v2=%0.1f)\n", (float)*location, (float)*v0, (float)*v1, (float)*v2);
}


//757
static void EXEC_glUniform4fARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLfloat *v0 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v1 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v2 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *v3 = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	LOG("glUniform4fARB(location=%0.1f, v0=%0.1f, v1=%0.1f, v2=%0.1f, v3=%0.1f)\n", (float)*location, (float)*v0, (float)*v1, (float)*v2, (float)*v3);
}


//758
static void EXEC_glUniform1iARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLint *v0 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	LOG("glUniform1iARB(location=%0.1f, v0=%0.1f)\n", (float)*location, (float)*v0);
}


//759
static void EXEC_glUniform2iARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLint *v0 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v1 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	LOG("glUniform2iARB(location=%0.1f, v0=%0.1f, v1=%0.1f)\n", (float)*location, (float)*v0, (float)*v1);
}


//760
static void EXEC_glUniform3iARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLint *v0 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v1 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v2 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	LOG("glUniform3iARB(location=%0.1f, v0=%0.1f, v1=%0.1f, v2=%0.1f)\n", (float)*location, (float)*v0, (float)*v1, (float)*v2);
}


//761
static void EXEC_glUniform4iARB(byte *commandbuf)
{
	GLint *location = (GLint*)commandbuf;    commandbuf += sizeof(GLint);
	GLint *v0 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v1 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v2 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *v3 = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	LOG("glUniform4iARB(location=%0.1f, v0=%0.1f, v1=%0.1f, v2=%0.1f, v3=%0.1f)\n", (float)*location, (float)*v0, (float)*v1, (float)*v2, (float)*v3);
}


//762
static void EXEC_glUniform1fvARB(byte *commandbuf)
{

	LOG("glUniform1fvARB()\n");

}


//763
static void EXEC_glUniform2fvARB(byte *commandbuf)
{

	LOG("glUniform2fvARB()\n");

}


//764
static void EXEC_glUniform3fvARB(byte *commandbuf)
{

	LOG("glUniform3fvARB()\n");

}


//765
static void EXEC_glUniform4fvARB(byte *commandbuf)
{

	LOG("glUniform4fvARB()\n");

}


//766
static void EXEC_glUniform1ivARB(byte *commandbuf)
{

	LOG("glUniform1ivARB()\n");

}


//767
static void EXEC_glUniform2ivARB(byte *commandbuf)
{

	LOG("glUniform2ivARB()\n");

}


//768
static void EXEC_glUniform3ivARB(byte *commandbuf)
{

	LOG("glUniform3ivARB()\n");

}


//769
static void EXEC_glUniform4ivARB(byte *commandbuf)
{

	LOG("glUniform4ivARB()\n");

}


//770
static void EXEC_glUniformMatrix2fvARB(byte *commandbuf)
{

	LOG("glUniformMatrix2fvARB()\n");

}


//771
static void EXEC_glUniformMatrix3fvARB(byte *commandbuf)
{

	LOG("glUniformMatrix3fvARB()\n");

}


//772
static void EXEC_glUniformMatrix4fvARB(byte *commandbuf)
{

	LOG("glUniformMatrix4fvARB()\n");

}


//773
static void EXEC_glGetObjectParameterfvARB(byte *commandbuf)
{

	LOG("glGetObjectParameterfvARB()\n");

}


//774
static void EXEC_glGetObjectParameterivARB(byte *commandbuf)
{

	LOG("glGetObjectParameterivARB()\n");

}


//775
static void EXEC_glGetInfoLogARB(byte *commandbuf)
{

	LOG("glGetInfoLogARB()\n");

}


//776
static void EXEC_glGetAttachedObjectsARB(byte *commandbuf)
{

	LOG("glGetAttachedObjectsARB()\n");

}


//777
static void EXEC_glGetUniformLocationARB(byte *commandbuf)
{

	LOG("glGetUniformLocationARB()\n");

}


//778
static void EXEC_glGetActiveUniformARB(byte *commandbuf)
{

	LOG("glGetActiveUniformARB()\n");

}


//779
static void EXEC_glGetUniformfvARB(byte *commandbuf)
{

	LOG("glGetUniformfvARB()\n");

}


//780
static void EXEC_glGetUniformivARB(byte *commandbuf)
{

	LOG("glGetUniformivARB()\n");

}


//781
static void EXEC_glGetShaderSourceARB(byte *commandbuf)
{

	LOG("glGetShaderSourceARB()\n");

}


//782
static void EXEC_glBindAttribLocationARB(byte *commandbuf)
{

	LOG("glBindAttribLocationARB()\n");

}


//783
static void EXEC_glGetActiveAttribARB(byte *commandbuf)
{

	LOG("glGetActiveAttribARB()\n");

}


//784
static void EXEC_glGetAttribLocationARB(byte *commandbuf)
{

	LOG("glGetAttribLocationARB()\n");

}


//785
static void EXEC_glDrawBuffersARB(byte *commandbuf)
{

	LOG("glDrawBuffersARB()\n");

}


//786
static void EXEC_glBlendColorEXT(byte *commandbuf)
{
	GLclampf *red = (GLclampf*)commandbuf;   commandbuf += sizeof(GLclampf);
	GLclampf *green = (GLclampf*)commandbuf;     commandbuf += sizeof(GLclampf);
	GLclampf *blue = (GLclampf*)commandbuf;  commandbuf += sizeof(GLclampf);
	GLclampf *alpha = (GLclampf*)commandbuf;     commandbuf += sizeof(GLclampf);

	LOG("glBlendColorEXT(red=%0.1f, green=%0.1f, blue=%0.1f, alpha=%0.1f)\n", (float)*red, (float)*green, (float)*blue, (float)*alpha);
}


//787
static void EXEC_glPolygonOffsetEXT(byte *commandbuf)
{
	GLfloat *factor = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *bias = (GLfloat*)commandbuf;    commandbuf += sizeof(GLfloat);

	LOG("glPolygonOffsetEXT(factor=%0.1f, bias=%0.1f)\n", (float)*factor, (float)*bias);
}


//788
static void EXEC_glTexImage3DEXT(byte *commandbuf)
{

	LOG("glTexImage3DEXT()\n");

}


//789
static void EXEC_glTexSubImage3DEXT(byte *commandbuf)
{

	LOG("glTexSubImage3DEXT()\n");

}


//790
static void EXEC_glGetTexFilterFuncSGIS(byte *commandbuf)
{

	LOG("glGetTexFilterFuncSGIS()\n");

}


//791
static void EXEC_glTexFilterFuncSGIS(byte *commandbuf)
{

	LOG("glTexFilterFuncSGIS()\n");

}


//792
static void EXEC_glTexSubImage1DEXT(byte *commandbuf)
{

	LOG("glTexSubImage1DEXT()\n");

}


//793
static void EXEC_glTexSubImage2DEXT(byte *commandbuf)
{

	LOG("glTexSubImage2DEXT()\n");

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

	LOG("glCopyTexImage1DEXT(target=%0.1f, level=%0.1f, internalformat=%0.1f, x=%0.1f, y=%0.1f, width=%0.1f, border=%0.1f)\n", (float)*target, (float)*level, (float)*internalformat, (float)*x, (float)*y, (float)*width, (float)*border);
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

	LOG("glCopyTexImage2DEXT(target=%0.1f, level=%0.1f, internalformat=%0.1f, x=%0.1f, y=%0.1f, width=%0.1f, height=%0.1f, border=%0.1f)\n", (float)*target, (float)*level, (float)*internalformat, (float)*x, (float)*y, (float)*width, (float)*height, (float)*border);
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

	LOG("glCopyTexSubImage1DEXT(target=%0.1f, level=%0.1f, xoffset=%0.1f, x=%0.1f, y=%0.1f, width=%0.1f)\n", (float)*target, (float)*level, (float)*xoffset, (float)*x, (float)*y, (float)*width);
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

	LOG("glCopyTexSubImage2DEXT(target=%0.1f, level=%0.1f, xoffset=%0.1f, yoffset=%0.1f, x=%0.1f, y=%0.1f, width=%0.1f, height=%0.1f)\n", (float)*target, (float)*level, (float)*xoffset, (float)*yoffset, (float)*x, (float)*y, (float)*width, (float)*height);
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

	LOG("glCopyTexSubImage3DEXT(target=%0.1f, level=%0.1f, xoffset=%0.1f, yoffset=%0.1f, zoffset=%0.1f, x=%0.1f, y=%0.1f, width=%0.1f, height=%0.1f)\n", (float)*target, (float)*level, (float)*xoffset, (float)*yoffset, (float)*zoffset, (float)*x, (float)*y, (float)*width, (float)*height);
}


//799
static void EXEC_glGetHistogramEXT(byte *commandbuf)
{

	LOG("glGetHistogramEXT()\n");

}


//800
static void EXEC_glGetHistogramParameterfvEXT(byte *commandbuf)
{

	LOG("glGetHistogramParameterfvEXT()\n");

}


//801
static void EXEC_glGetHistogramParameterivEXT(byte *commandbuf)
{

	LOG("glGetHistogramParameterivEXT()\n");

}


//802
static void EXEC_glGetMinmaxEXT(byte *commandbuf)
{

	LOG("glGetMinmaxEXT()\n");

}


//803
static void EXEC_glGetMinmaxParameterfvEXT(byte *commandbuf)
{

	LOG("glGetMinmaxParameterfvEXT()\n");

}


//804
static void EXEC_glGetMinmaxParameterivEXT(byte *commandbuf)
{

	LOG("glGetMinmaxParameterivEXT()\n");

}


//805
static void EXEC_glHistogramEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLboolean *sink = (GLboolean*)commandbuf;    commandbuf += sizeof(GLboolean);

	LOG("glHistogramEXT(target=%0.1f, width=%0.1f, internalformat=%0.1f, sink=%0.1f)\n", (float)*target, (float)*width, (float)*internalformat, (float)*sink);
}


//806
static void EXEC_glMinmaxEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLboolean *sink = (GLboolean*)commandbuf;    commandbuf += sizeof(GLboolean);

	LOG("glMinmaxEXT(target=%0.1f, internalformat=%0.1f, sink=%0.1f)\n", (float)*target, (float)*internalformat, (float)*sink);
}


//807
static void EXEC_glResetHistogramEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	LOG("glResetHistogramEXT(target=%0.1f)\n", (float)*target);
}


//808
static void EXEC_glResetMinmaxEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	LOG("glResetMinmaxEXT(target=%0.1f)\n", (float)*target);
}


//809
static void EXEC_glConvolutionFilter1DEXT(byte *commandbuf)
{

	LOG("glConvolutionFilter1DEXT()\n");

}


//810
static void EXEC_glConvolutionFilter2DEXT(byte *commandbuf)
{

	LOG("glConvolutionFilter2DEXT()\n");

}


//811
static void EXEC_glConvolutionParameterfEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *params = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	LOG("glConvolutionParameterfEXT(target=%0.1f, pname=%0.1f, params=%0.1f)\n", (float)*target, (float)*pname, (float)*params);
}


//812
static void EXEC_glConvolutionParameterfvEXT(byte *commandbuf)
{

	LOG("glConvolutionParameterfvEXT()\n");

}


//813
static void EXEC_glConvolutionParameteriEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *params = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	LOG("glConvolutionParameteriEXT(target=%0.1f, pname=%0.1f, params=%0.1f)\n", (float)*target, (float)*pname, (float)*params);
}


//814
static void EXEC_glConvolutionParameterivEXT(byte *commandbuf)
{

	LOG("glConvolutionParameterivEXT()\n");

}


//815
static void EXEC_glCopyConvolutionFilter1DEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	LOG("glCopyConvolutionFilter1DEXT(target=%0.1f, internalformat=%0.1f, x=%0.1f, y=%0.1f, width=%0.1f)\n", (float)*target, (float)*internalformat, (float)*x, (float)*y, (float)*width);
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

	LOG("glCopyConvolutionFilter2DEXT(target=%0.1f, internalformat=%0.1f, x=%0.1f, y=%0.1f, width=%0.1f, height=%0.1f)\n", (float)*target, (float)*internalformat, (float)*x, (float)*y, (float)*width, (float)*height);
}


//817
static void EXEC_glGetConvolutionFilterEXT(byte *commandbuf)
{

	LOG("glGetConvolutionFilterEXT()\n");

}


//818
static void EXEC_glGetConvolutionParameterfvEXT(byte *commandbuf)
{

	LOG("glGetConvolutionParameterfvEXT()\n");

}


//819
static void EXEC_glGetConvolutionParameterivEXT(byte *commandbuf)
{

	LOG("glGetConvolutionParameterivEXT()\n");

}


//820
static void EXEC_glGetSeparableFilterEXT(byte *commandbuf)
{

	LOG("glGetSeparableFilterEXT()\n");

}


//821
static void EXEC_glSeparableFilter2DEXT(byte *commandbuf)
{

	LOG("glSeparableFilter2DEXT()\n");

}


//822
static void EXEC_glColorTableSGI(byte *commandbuf)
{

	LOG("glColorTableSGI()\n");

}


//823
static void EXEC_glColorTableParameterfvSGI(byte *commandbuf)
{

	LOG("glColorTableParameterfvSGI()\n");

}


//824
static void EXEC_glColorTableParameterivSGI(byte *commandbuf)
{

	LOG("glColorTableParameterivSGI()\n");

}


//825
static void EXEC_glCopyColorTableSGI(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *internalformat = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	LOG("glCopyColorTableSGI(target=%0.1f, internalformat=%0.1f, x=%0.1f, y=%0.1f, width=%0.1f)\n", (float)*target, (float)*internalformat, (float)*x, (float)*y, (float)*width);
}


//826
static void EXEC_glGetColorTableSGI(byte *commandbuf)
{

	LOG("glGetColorTableSGI()\n");

}


//827
static void EXEC_glGetColorTableParameterfvSGI(byte *commandbuf)
{

	LOG("glGetColorTableParameterfvSGI()\n");

}


//828
static void EXEC_glGetColorTableParameterivSGI(byte *commandbuf)
{

	LOG("glGetColorTableParameterivSGI()\n");

}


//829
static void EXEC_glPixelTexGenParameteriSGIS(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glPixelTexGenParameteriSGIS(pname=%0.1f, param=%0.1f)\n", (float)*pname, (float)*param);
}


//830
static void EXEC_glPixelTexGenParameterivSGIS(byte *commandbuf)
{

	LOG("glPixelTexGenParameterivSGIS()\n");

}


//831
static void EXEC_glPixelTexGenParameterfSGIS(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glPixelTexGenParameterfSGIS(pname=%0.1f, param=%0.1f)\n", (float)*pname, (float)*param);
}


//832
static void EXEC_glPixelTexGenParameterfvSGIS(byte *commandbuf)
{

	LOG("glPixelTexGenParameterfvSGIS()\n");

}


//833
static void EXEC_glGetPixelTexGenParameterivSGIS(byte *commandbuf)
{

	LOG("glGetPixelTexGenParameterivSGIS()\n");

}


//834
static void EXEC_glGetPixelTexGenParameterfvSGIS(byte *commandbuf)
{

	LOG("glGetPixelTexGenParameterfvSGIS()\n");

}


//835
static void EXEC_glTexImage4DSGIS(byte *commandbuf)
{

	LOG("glTexImage4DSGIS()\n");

}


//836
static void EXEC_glTexSubImage4DSGIS(byte *commandbuf)
{

	LOG("glTexSubImage4DSGIS()\n");

}


//837
static void EXEC_glAreTexturesResidentEXT(byte *commandbuf)
{

	LOG("glAreTexturesResidentEXT()\n");

}


//838
static void EXEC_glBindTextureEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *texture = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	LOG("glBindTextureEXT(target=%0.1f, texture=%0.1f)\n", (float)*target, (float)*texture);
}


//839
static void EXEC_glDeleteTexturesEXT(byte *commandbuf)
{

	LOG("glDeleteTexturesEXT()\n");

}


//840
static void EXEC_glGenTexturesEXT(byte *commandbuf)
{

	LOG("glGenTexturesEXT()\n");

}


//841
static void EXEC_glIsTextureEXT(byte *commandbuf)
{
	GLuint *texture = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	LOG("glIsTextureEXT(texture=%0.1f)\n", (float)*texture);
}


//842
static void EXEC_glPrioritizeTexturesEXT(byte *commandbuf)
{

	LOG("glPrioritizeTexturesEXT()\n");

}


//843
static void EXEC_glDetailTexFuncSGIS(byte *commandbuf)
{

	LOG("glDetailTexFuncSGIS()\n");

}


//844
static void EXEC_glGetDetailTexFuncSGIS(byte *commandbuf)
{

	LOG("glGetDetailTexFuncSGIS()\n");

}


//845
static void EXEC_glSharpenTexFuncSGIS(byte *commandbuf)
{

	LOG("glSharpenTexFuncSGIS()\n");

}


//846
static void EXEC_glGetSharpenTexFuncSGIS(byte *commandbuf)
{

	LOG("glGetSharpenTexFuncSGIS()\n");

}


//847
static void EXEC_glSampleMaskSGIS(byte *commandbuf)
{
	GLclampf *value = (GLclampf*)commandbuf;     commandbuf += sizeof(GLclampf);
	GLboolean *invert = (GLboolean*)commandbuf;  commandbuf += sizeof(GLboolean);

	LOG("glSampleMaskSGIS(value=%0.1f, invert=%0.1f)\n", (float)*value, (float)*invert);
}


//848
static void EXEC_glSamplePatternSGIS(byte *commandbuf)
{
	GLenum *pattern = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	LOG("glSamplePatternSGIS(pattern=%0.1f)\n", (float)*pattern);
}


//849
static void EXEC_glArrayElementEXT(byte *commandbuf)
{
	GLint *i = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glArrayElementEXT(i=%0.1f)\n", (float)*i);
}


//850
static void EXEC_glColorPointerEXT(byte *commandbuf)
{

	LOG("glColorPointerEXT()\n");

}


//851
static void EXEC_glDrawArraysEXT(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLint *first = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	LOG("glDrawArraysEXT(mode=%0.1f, first=%0.1f, count=%0.1f)\n", (float)*mode, (float)*first, (float)*count);
}


//852
static void EXEC_glEdgeFlagPointerEXT(byte *commandbuf)
{

	LOG("glEdgeFlagPointerEXT()\n");

}


//853
static void EXEC_glGetPointervEXT(byte *commandbuf)
{

	LOG("glGetPointervEXT()\n");

}


//854
static void EXEC_glIndexPointerEXT(byte *commandbuf)
{

	LOG("glIndexPointerEXT()\n");

}


//855
static void EXEC_glNormalPointerEXT(byte *commandbuf)
{

	LOG("glNormalPointerEXT()\n");

}


//856
static void EXEC_glTexCoordPointerEXT(byte *commandbuf)
{

	LOG("glTexCoordPointerEXT()\n");

}


//857
static void EXEC_glVertexPointerEXT(byte *commandbuf)
{

	LOG("glVertexPointerEXT()\n");

}


//858
static void EXEC_glBlendEquationEXT(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glBlendEquationEXT(mode=%0.1f)\n", (float)*mode);
}


//859
static void EXEC_glSpriteParameterfSGIX(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glSpriteParameterfSGIX(pname=%0.1f, param=%0.1f)\n", (float)*pname, (float)*param);
}


//860
static void EXEC_glSpriteParameterfvSGIX(byte *commandbuf)
{

	LOG("glSpriteParameterfvSGIX()\n");

}


//861
static void EXEC_glSpriteParameteriSGIX(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glSpriteParameteriSGIX(pname=%0.1f, param=%0.1f)\n", (float)*pname, (float)*param);
}


//862
static void EXEC_glSpriteParameterivSGIX(byte *commandbuf)
{

	LOG("glSpriteParameterivSGIX()\n");

}


//863
static void EXEC_glPointParameterfEXT(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glPointParameterfEXT(pname=%0.1f, param=%0.1f)\n", (float)*pname, (float)*param);
}


//864
static void EXEC_glPointParameterfvEXT(byte *commandbuf)
{

	LOG("glPointParameterfvEXT()\n");

}


//865
static void EXEC_glGetInstrumentsSGIX(byte *commandbuf)
{

	LOG("glGetInstrumentsSGIX()\n");
}


//866
static void EXEC_glInstrumentsBufferSGIX(byte *commandbuf)
{

	LOG("glInstrumentsBufferSGIX()\n");

}


//867
static void EXEC_glPollInstrumentsSGIX(byte *commandbuf)
{

	LOG("glPollInstrumentsSGIX()\n");

}


//868
static void EXEC_glReadInstrumentsSGIX(byte *commandbuf)
{
	GLint *marker = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	LOG("glReadInstrumentsSGIX(marker=%0.1f)\n", (float)*marker);
}


//869
static void EXEC_glStartInstrumentsSGIX(byte *commandbuf)
{

	LOG("glStartInstrumentsSGIX()\n");
}


//870
static void EXEC_glStopInstrumentsSGIX(byte *commandbuf)
{
	GLint *marker = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	LOG("glStopInstrumentsSGIX(marker=%0.1f)\n", (float)*marker);
}


//871
static void EXEC_glFrameZoomSGIX(byte *commandbuf)
{
	GLint *factor = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	LOG("glFrameZoomSGIX(factor=%0.1f)\n", (float)*factor);
}


//872
static void EXEC_glTagSampleBufferSGIX(byte *commandbuf)
{

	LOG("glTagSampleBufferSGIX()\n");
}


//873
static void EXEC_glReferencePlaneSGIX(byte *commandbuf)
{

	LOG("glReferencePlaneSGIX()\n");

}


//874
static void EXEC_glFlushRasterSGIX(byte *commandbuf)
{

	LOG("glFlushRasterSGIX()\n");
}


//875
static void EXEC_glFogFuncSGIS(byte *commandbuf)
{

	LOG("glFogFuncSGIS()\n");

}


//876
static void EXEC_glGetFogFuncSGIS(byte *commandbuf)
{

	LOG("glGetFogFuncSGIS()\n");

}


//877
static void EXEC_glImageTransformParameteriHP(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glImageTransformParameteriHP(target=%0.1f, pname=%0.1f, param=%0.1f)\n", (float)*target, (float)*pname, (float)*param);
}


//878
static void EXEC_glImageTransformParameterfHP(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glImageTransformParameterfHP(target=%0.1f, pname=%0.1f, param=%0.1f)\n", (float)*target, (float)*pname, (float)*param);
}


//879
static void EXEC_glImageTransformParameterivHP(byte *commandbuf)
{

	LOG("glImageTransformParameterivHP()\n");

}


//880
static void EXEC_glImageTransformParameterfvHP(byte *commandbuf)
{

	LOG("glImageTransformParameterfvHP()\n");

}


//881
static void EXEC_glGetImageTransformParameterivHP(byte *commandbuf)
{

	LOG("glGetImageTransformParameterivHP()\n");

}


//882
static void EXEC_glGetImageTransformParameterfvHP(byte *commandbuf)
{

	LOG("glGetImageTransformParameterfvHP()\n");

}


//883
static void EXEC_glColorSubTableEXT(byte *commandbuf)
{

	LOG("glColorSubTableEXT()\n");

}


//884
static void EXEC_glCopyColorSubTableEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLsizei *start = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	LOG("glCopyColorSubTableEXT(target=%0.1f, start=%0.1f, x=%0.1f, y=%0.1f, width=%0.1f)\n", (float)*target, (float)*start, (float)*x, (float)*y, (float)*width);
}


//885
static void EXEC_glHintPGI(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLint *mode = (GLint*)commandbuf;    commandbuf += sizeof(GLint);

	LOG("glHintPGI(target=%0.1f, mode=%0.1f)\n", (float)*target, (float)*mode);
}


//886
static void EXEC_glColorTableEXT(byte *commandbuf)
{

	LOG("glColorTableEXT()\n");

}


//887
static void EXEC_glGetColorTableEXT(byte *commandbuf)
{

	LOG("glGetColorTableEXT()\n");

}


//888
static void EXEC_glGetColorTableParameterivEXT(byte *commandbuf)
{

	LOG("glGetColorTableParameterivEXT()\n");

}


//889
static void EXEC_glGetColorTableParameterfvEXT(byte *commandbuf)
{

	LOG("glGetColorTableParameterfvEXT()\n");

}


//890
static void EXEC_glGetListParameterfvSGIX(byte *commandbuf)
{

	LOG("glGetListParameterfvSGIX()\n");

}


//891
static void EXEC_glGetListParameterivSGIX(byte *commandbuf)
{

	LOG("glGetListParameterivSGIX()\n");

}


//892
static void EXEC_glListParameterfSGIX(byte *commandbuf)
{
	GLuint *list = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glListParameterfSGIX(list=%0.1f, pname=%0.1f, param=%0.1f)\n", (float)*list, (float)*pname, (float)*param);
}


//893
static void EXEC_glListParameterfvSGIX(byte *commandbuf)
{

	LOG("glListParameterfvSGIX()\n");

}


//894
static void EXEC_glListParameteriSGIX(byte *commandbuf)
{
	GLuint *list = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glListParameteriSGIX(list=%0.1f, pname=%0.1f, param=%0.1f)\n", (float)*list, (float)*pname, (float)*param);
}


//895
static void EXEC_glListParameterivSGIX(byte *commandbuf)
{

	LOG("glListParameterivSGIX()\n");

}


//896
static void EXEC_glIndexMaterialEXT(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glIndexMaterialEXT(face=%0.1f, mode=%0.1f)\n", (float)*face, (float)*mode);
}


//897
static void EXEC_glIndexFuncEXT(byte *commandbuf)
{
	GLenum *func = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLclampf *ref = (GLclampf*)commandbuf;   commandbuf += sizeof(GLclampf);

	LOG("glIndexFuncEXT(func=%0.1f, ref=%0.1f)\n", (float)*func, (float)*ref);
}


//898
static void EXEC_glLockArraysEXT(byte *commandbuf)
{
	GLint *first = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	LOG("glLockArraysEXT(first=%0.1f, count=%0.1f)\n", (float)*first, (float)*count);
}


//899
static void EXEC_glUnlockArraysEXT(byte *commandbuf)
{

	LOG("glUnlockArraysEXT()\n");
}


//900
static void EXEC_glCullParameterdvEXT(byte *commandbuf)
{

	LOG("glCullParameterdvEXT()\n");

}


//901
static void EXEC_glCullParameterfvEXT(byte *commandbuf)
{

	LOG("glCullParameterfvEXT()\n");

}


//902
static void EXEC_glFragmentColorMaterialSGIX(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glFragmentColorMaterialSGIX(face=%0.1f, mode=%0.1f)\n", (float)*face, (float)*mode);
}


//903
static void EXEC_glFragmentLightfSGIX(byte *commandbuf)
{
	GLenum *light = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glFragmentLightfSGIX(light=%0.1f, pname=%0.1f, param=%0.1f)\n", (float)*light, (float)*pname, (float)*param);
}


//904
static void EXEC_glFragmentLightfvSGIX(byte *commandbuf)
{

	LOG("glFragmentLightfvSGIX()\n");

}


//905
static void EXEC_glFragmentLightiSGIX(byte *commandbuf)
{
	GLenum *light = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glFragmentLightiSGIX(light=%0.1f, pname=%0.1f, param=%0.1f)\n", (float)*light, (float)*pname, (float)*param);
}


//906
static void EXEC_glFragmentLightivSGIX(byte *commandbuf)
{

	LOG("glFragmentLightivSGIX()\n");

}


//907
static void EXEC_glFragmentLightModelfSGIX(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glFragmentLightModelfSGIX(pname=%0.1f, param=%0.1f)\n", (float)*pname, (float)*param);
}


//908
static void EXEC_glFragmentLightModelfvSGIX(byte *commandbuf)
{

	LOG("glFragmentLightModelfvSGIX()\n");

}


//909
static void EXEC_glFragmentLightModeliSGIX(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glFragmentLightModeliSGIX(pname=%0.1f, param=%0.1f)\n", (float)*pname, (float)*param);
}


//910
static void EXEC_glFragmentLightModelivSGIX(byte *commandbuf)
{

	LOG("glFragmentLightModelivSGIX()\n");

}


//911
static void EXEC_glFragmentMaterialfSGIX(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glFragmentMaterialfSGIX(face=%0.1f, pname=%0.1f, param=%0.1f)\n", (float)*face, (float)*pname, (float)*param);
}


//912
static void EXEC_glFragmentMaterialfvSGIX(byte *commandbuf)
{

	LOG("glFragmentMaterialfvSGIX()\n");

}


//913
static void EXEC_glFragmentMaterialiSGIX(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glFragmentMaterialiSGIX(face=%0.1f, pname=%0.1f, param=%0.1f)\n", (float)*face, (float)*pname, (float)*param);
}


//914
static void EXEC_glFragmentMaterialivSGIX(byte *commandbuf)
{

	LOG("glFragmentMaterialivSGIX()\n");

}


//915
static void EXEC_glGetFragmentLightfvSGIX(byte *commandbuf)
{

	LOG("glGetFragmentLightfvSGIX()\n");

}


//916
static void EXEC_glGetFragmentLightivSGIX(byte *commandbuf)
{

	LOG("glGetFragmentLightivSGIX()\n");

}


//917
static void EXEC_glGetFragmentMaterialfvSGIX(byte *commandbuf)
{

	LOG("glGetFragmentMaterialfvSGIX()\n");

}


//918
static void EXEC_glGetFragmentMaterialivSGIX(byte *commandbuf)
{

	LOG("glGetFragmentMaterialivSGIX()\n");

}


//919
static void EXEC_glLightEnviSGIX(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glLightEnviSGIX(pname=%0.1f, param=%0.1f)\n", (float)*pname, (float)*param);
}


//920
static void EXEC_glDrawRangeElementsEXT(byte *commandbuf)
{

	LOG("glDrawRangeElementsEXT()\n");

}


//921
static void EXEC_glApplyTextureEXT(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glApplyTextureEXT(mode=%0.1f)\n", (float)*mode);
}


//922
static void EXEC_glTextureLightEXT(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	LOG("glTextureLightEXT(pname=%0.1f)\n", (float)*pname);
}


//923
static void EXEC_glTextureMaterialEXT(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glTextureMaterialEXT(face=%0.1f, mode=%0.1f)\n", (float)*face, (float)*mode);
}


//924
static void EXEC_glAsyncMarkerSGIX(byte *commandbuf)
{
	GLuint *marker = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	LOG("glAsyncMarkerSGIX(marker=%0.1f)\n", (float)*marker);
}


//925
static void EXEC_glFinishAsyncSGIX(byte *commandbuf)
{

	LOG("glFinishAsyncSGIX()\n");

}


//926
static void EXEC_glPollAsyncSGIX(byte *commandbuf)
{

	LOG("glPollAsyncSGIX()\n");

}


//927
static void EXEC_glGenAsyncMarkersSGIX(byte *commandbuf)
{
	GLsizei *range = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	LOG("glGenAsyncMarkersSGIX(range=%0.1f)\n", (float)*range);
}


//928
static void EXEC_glDeleteAsyncMarkersSGIX(byte *commandbuf)
{
	GLuint *marker = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLsizei *range = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	LOG("glDeleteAsyncMarkersSGIX(marker=%0.1f, range=%0.1f)\n", (float)*marker, (float)*range);
}


//929
static void EXEC_glIsAsyncMarkerSGIX(byte *commandbuf)
{
	GLuint *marker = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	LOG("glIsAsyncMarkerSGIX(marker=%0.1f)\n", (float)*marker);
}


//930
static void EXEC_glVertexPointervINTEL(byte *commandbuf)
{

	LOG("glVertexPointervINTEL()\n");

}


//931
static void EXEC_glNormalPointervINTEL(byte *commandbuf)
{

	LOG("glNormalPointervINTEL()\n");

}


//932
static void EXEC_glColorPointervINTEL(byte *commandbuf)
{

	LOG("glColorPointervINTEL()\n");

}


//933
static void EXEC_glTexCoordPointervINTEL(byte *commandbuf)
{

	LOG("glTexCoordPointervINTEL()\n");

}


//934
static void EXEC_glPixelTransformParameteriEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glPixelTransformParameteriEXT(target=%0.1f, pname=%0.1f, param=%0.1f)\n", (float)*target, (float)*pname, (float)*param);
}


//935
static void EXEC_glPixelTransformParameterfEXT(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glPixelTransformParameterfEXT(target=%0.1f, pname=%0.1f, param=%0.1f)\n", (float)*target, (float)*pname, (float)*param);
}


//936
static void EXEC_glPixelTransformParameterivEXT(byte *commandbuf)
{

	LOG("glPixelTransformParameterivEXT()\n");

}


//937
static void EXEC_glPixelTransformParameterfvEXT(byte *commandbuf)
{

	LOG("glPixelTransformParameterfvEXT()\n");

}


//938
static void EXEC_glSecondaryColor3bEXT(byte *commandbuf)
{
	GLbyte *red = (GLbyte*)commandbuf;   commandbuf += sizeof(GLbyte);
	GLbyte *green = (GLbyte*)commandbuf;     commandbuf += sizeof(GLbyte);
	GLbyte *blue = (GLbyte*)commandbuf;  commandbuf += sizeof(GLbyte);

	LOG("glSecondaryColor3bEXT(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//939
static void EXEC_glSecondaryColor3bvEXT(byte *commandbuf)
{

	LOG("glSecondaryColor3bvEXT()\n");

}


//940
static void EXEC_glSecondaryColor3dEXT(byte *commandbuf)
{
	GLdouble *red = (GLdouble*)commandbuf;   commandbuf += sizeof(GLdouble);
	GLdouble *green = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *blue = (GLdouble*)commandbuf;  commandbuf += sizeof(GLdouble);

	LOG("glSecondaryColor3dEXT(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//941
static void EXEC_glSecondaryColor3dvEXT(byte *commandbuf)
{

	LOG("glSecondaryColor3dvEXT()\n");

}


//942
static void EXEC_glSecondaryColor3fEXT(byte *commandbuf)
{
	GLfloat *red = (GLfloat*)commandbuf;     commandbuf += sizeof(GLfloat);
	GLfloat *green = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *blue = (GLfloat*)commandbuf;    commandbuf += sizeof(GLfloat);

	LOG("glSecondaryColor3fEXT(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//943
static void EXEC_glSecondaryColor3fvEXT(byte *commandbuf)
{

	LOG("glSecondaryColor3fvEXT()\n");

}


//944
static void EXEC_glSecondaryColor3iEXT(byte *commandbuf)
{
	GLint *red = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLint *green = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *blue = (GLint*)commandbuf;    commandbuf += sizeof(GLint);

	LOG("glSecondaryColor3iEXT(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//945
static void EXEC_glSecondaryColor3ivEXT(byte *commandbuf)
{

	LOG("glSecondaryColor3ivEXT()\n");

}


//946
static void EXEC_glSecondaryColor3sEXT(byte *commandbuf)
{
	GLshort *red = (GLshort*)commandbuf;     commandbuf += sizeof(GLshort);
	GLshort *green = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *blue = (GLshort*)commandbuf;    commandbuf += sizeof(GLshort);

	LOG("glSecondaryColor3sEXT(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//947
static void EXEC_glSecondaryColor3svEXT(byte *commandbuf)
{

	LOG("glSecondaryColor3svEXT()\n");

}


//948
static void EXEC_glSecondaryColor3ubEXT(byte *commandbuf)
{
	GLubyte *red = (GLubyte*)commandbuf;     commandbuf += sizeof(GLubyte);
	GLubyte *green = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *blue = (GLubyte*)commandbuf;    commandbuf += sizeof(GLubyte);

	LOG("glSecondaryColor3ubEXT(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//949
static void EXEC_glSecondaryColor3ubvEXT(byte *commandbuf)
{

	LOG("glSecondaryColor3ubvEXT()\n");

}


//950
static void EXEC_glSecondaryColor3uiEXT(byte *commandbuf)
{
	GLuint *red = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *green = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLuint *blue = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	LOG("glSecondaryColor3uiEXT(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//951
static void EXEC_glSecondaryColor3uivEXT(byte *commandbuf)
{

	LOG("glSecondaryColor3uivEXT()\n");

}


//952
static void EXEC_glSecondaryColor3usEXT(byte *commandbuf)
{
	GLushort *red = (GLushort*)commandbuf;   commandbuf += sizeof(GLushort);
	GLushort *green = (GLushort*)commandbuf;     commandbuf += sizeof(GLushort);
	GLushort *blue = (GLushort*)commandbuf;  commandbuf += sizeof(GLushort);

	LOG("glSecondaryColor3usEXT(red=%0.1f, green=%0.1f, blue=%0.1f)\n", (float)*red, (float)*green, (float)*blue);
}


//953
static void EXEC_glSecondaryColor3usvEXT(byte *commandbuf)
{

	LOG("glSecondaryColor3usvEXT()\n");

}


//954
static void EXEC_glSecondaryColorPointerEXT(byte *commandbuf)
{

	LOG("glSecondaryColorPointerEXT()\n");

}


//955
static void EXEC_glTextureNormalEXT(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glTextureNormalEXT(mode=%0.1f)\n", (float)*mode);
}


//956
static void EXEC_glMultiDrawArraysEXT(byte *commandbuf)
{

	LOG("glMultiDrawArraysEXT()\n");

}


//957
static void EXEC_glMultiDrawElementsEXT(byte *commandbuf)
{

	LOG("glMultiDrawElementsEXT()\n");

}


//958
static void EXEC_glFogCoordfEXT(byte *commandbuf)
{
	GLfloat *coord = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glFogCoordfEXT(coord=%0.1f)\n", (float)*coord);
}


//959
static void EXEC_glFogCoordfvEXT(byte *commandbuf)
{

	LOG("glFogCoordfvEXT()\n");

}


//960
static void EXEC_glFogCoorddEXT(byte *commandbuf)
{
	GLdouble *coord = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glFogCoorddEXT(coord=%0.1f)\n", (float)*coord);
}


//961
static void EXEC_glFogCoorddvEXT(byte *commandbuf)
{

	LOG("glFogCoorddvEXT()\n");

}


//962
static void EXEC_glFogCoordPointerEXT(byte *commandbuf)
{

	LOG("glFogCoordPointerEXT()\n");

}


//963
static void EXEC_glTangent3bEXT(byte *commandbuf)
{
	GLbyte *tx = (GLbyte*)commandbuf;    commandbuf += sizeof(GLbyte);
	GLbyte *ty = (GLbyte*)commandbuf;    commandbuf += sizeof(GLbyte);
	GLbyte *tz = (GLbyte*)commandbuf;    commandbuf += sizeof(GLbyte);

	LOG("glTangent3bEXT(tx=%0.1f, ty=%0.1f, tz=%0.1f)\n", (float)*tx, (float)*ty, (float)*tz);
}


//964
static void EXEC_glTangent3bvEXT(byte *commandbuf)
{

	LOG("glTangent3bvEXT()\n");

}


//965
static void EXEC_glTangent3dEXT(byte *commandbuf)
{
	GLdouble *tx = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *ty = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *tz = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);

	LOG("glTangent3dEXT(tx=%0.1f, ty=%0.1f, tz=%0.1f)\n", (float)*tx, (float)*ty, (float)*tz);
}


//966
static void EXEC_glTangent3dvEXT(byte *commandbuf)
{

	LOG("glTangent3dvEXT()\n");

}


//967
static void EXEC_glTangent3fEXT(byte *commandbuf)
{
	GLfloat *tx = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *ty = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *tz = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	LOG("glTangent3fEXT(tx=%0.1f, ty=%0.1f, tz=%0.1f)\n", (float)*tx, (float)*ty, (float)*tz);
}


//968
static void EXEC_glTangent3fvEXT(byte *commandbuf)
{

	LOG("glTangent3fvEXT()\n");

}


//969
static void EXEC_glTangent3iEXT(byte *commandbuf)
{
	GLint *tx = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *ty = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *tz = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	LOG("glTangent3iEXT(tx=%0.1f, ty=%0.1f, tz=%0.1f)\n", (float)*tx, (float)*ty, (float)*tz);
}


//970
static void EXEC_glTangent3ivEXT(byte *commandbuf)
{

	LOG("glTangent3ivEXT()\n");

}


//971
static void EXEC_glTangent3sEXT(byte *commandbuf)
{
	GLshort *tx = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);
	GLshort *ty = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);
	GLshort *tz = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);

	LOG("glTangent3sEXT(tx=%0.1f, ty=%0.1f, tz=%0.1f)\n", (float)*tx, (float)*ty, (float)*tz);
}


//972
static void EXEC_glTangent3svEXT(byte *commandbuf)
{

	LOG("glTangent3svEXT()\n");

}


//973
static void EXEC_glBinormal3bEXT(byte *commandbuf)
{
	GLbyte *bx = (GLbyte*)commandbuf;    commandbuf += sizeof(GLbyte);
	GLbyte *by = (GLbyte*)commandbuf;    commandbuf += sizeof(GLbyte);
	GLbyte *bz = (GLbyte*)commandbuf;    commandbuf += sizeof(GLbyte);

	LOG("glBinormal3bEXT(bx=%0.1f, by=%0.1f, bz=%0.1f)\n", (float)*bx, (float)*by, (float)*bz);
}


//974
static void EXEC_glBinormal3bvEXT(byte *commandbuf)
{

	LOG("glBinormal3bvEXT()\n");

}


//975
static void EXEC_glBinormal3dEXT(byte *commandbuf)
{
	GLdouble *bx = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *by = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);
	GLdouble *bz = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);

	LOG("glBinormal3dEXT(bx=%0.1f, by=%0.1f, bz=%0.1f)\n", (float)*bx, (float)*by, (float)*bz);
}


//976
static void EXEC_glBinormal3dvEXT(byte *commandbuf)
{

	LOG("glBinormal3dvEXT()\n");

}


//977
static void EXEC_glBinormal3fEXT(byte *commandbuf)
{
	GLfloat *bx = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *by = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);
	GLfloat *bz = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	LOG("glBinormal3fEXT(bx=%0.1f, by=%0.1f, bz=%0.1f)\n", (float)*bx, (float)*by, (float)*bz);
}


//978
static void EXEC_glBinormal3fvEXT(byte *commandbuf)
{

	LOG("glBinormal3fvEXT()\n");

}


//979
static void EXEC_glBinormal3iEXT(byte *commandbuf)
{
	GLint *bx = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *by = (GLint*)commandbuf;  commandbuf += sizeof(GLint);
	GLint *bz = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	LOG("glBinormal3iEXT(bx=%0.1f, by=%0.1f, bz=%0.1f)\n", (float)*bx, (float)*by, (float)*bz);
}


//980
static void EXEC_glBinormal3ivEXT(byte *commandbuf)
{

	LOG("glBinormal3ivEXT()\n");

}


//981
static void EXEC_glBinormal3sEXT(byte *commandbuf)
{
	GLshort *bx = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);
	GLshort *by = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);
	GLshort *bz = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);

	LOG("glBinormal3sEXT(bx=%0.1f, by=%0.1f, bz=%0.1f)\n", (float)*bx, (float)*by, (float)*bz);
}


//982
static void EXEC_glBinormal3svEXT(byte *commandbuf)
{

	LOG("glBinormal3svEXT()\n");

}


//983
static void EXEC_glTangentPointerEXT(byte *commandbuf)
{

	LOG("glTangentPointerEXT()\n");

}


//984
static void EXEC_glBinormalPointerEXT(byte *commandbuf)
{

	LOG("glBinormalPointerEXT()\n");

}


//985
static void EXEC_glPixelTexGenSGIX(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glPixelTexGenSGIX(mode=%0.1f)\n", (float)*mode);
}


//986
static void EXEC_glFinishTextureSUNX(byte *commandbuf)
{

	LOG("glFinishTextureSUNX()\n");
}


//987
static void EXEC_glGlobalAlphaFactorbSUN(byte *commandbuf)
{
	GLbyte *factor = (GLbyte*)commandbuf;    commandbuf += sizeof(GLbyte);

	LOG("glGlobalAlphaFactorbSUN(factor=%0.1f)\n", (float)*factor);
}


//988
static void EXEC_glGlobalAlphaFactorsSUN(byte *commandbuf)
{
	GLshort *factor = (GLshort*)commandbuf;  commandbuf += sizeof(GLshort);

	LOG("glGlobalAlphaFactorsSUN(factor=%0.1f)\n", (float)*factor);
}


//989
static void EXEC_glGlobalAlphaFactoriSUN(byte *commandbuf)
{
	GLint *factor = (GLint*)commandbuf;  commandbuf += sizeof(GLint);

	LOG("glGlobalAlphaFactoriSUN(factor=%0.1f)\n", (float)*factor);
}


//990
static void EXEC_glGlobalAlphaFactorfSUN(byte *commandbuf)
{
	GLfloat *factor = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	LOG("glGlobalAlphaFactorfSUN(factor=%0.1f)\n", (float)*factor);
}


//991
static void EXEC_glGlobalAlphaFactordSUN(byte *commandbuf)
{
	GLdouble *factor = (GLdouble*)commandbuf;    commandbuf += sizeof(GLdouble);

	LOG("glGlobalAlphaFactordSUN(factor=%0.1f)\n", (float)*factor);
}


//992
static void EXEC_glGlobalAlphaFactorubSUN(byte *commandbuf)
{
	GLubyte *factor = (GLubyte*)commandbuf;  commandbuf += sizeof(GLubyte);

	LOG("glGlobalAlphaFactorubSUN(factor=%0.1f)\n", (float)*factor);
}


//993
static void EXEC_glGlobalAlphaFactorusSUN(byte *commandbuf)
{
	GLushort *factor = (GLushort*)commandbuf;    commandbuf += sizeof(GLushort);

	LOG("glGlobalAlphaFactorusSUN(factor=%0.1f)\n", (float)*factor);
}


//994
static void EXEC_glGlobalAlphaFactoruiSUN(byte *commandbuf)
{
	GLuint *factor = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	LOG("glGlobalAlphaFactoruiSUN(factor=%0.1f)\n", (float)*factor);
}


//995
static void EXEC_glReplacementCodeuiSUN(byte *commandbuf)
{
	GLuint *code = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	LOG("glReplacementCodeuiSUN(code=%0.1f)\n", (float)*code);
}


//996
static void EXEC_glReplacementCodeusSUN(byte *commandbuf)
{
	GLushort *code = (GLushort*)commandbuf;  commandbuf += sizeof(GLushort);

	LOG("glReplacementCodeusSUN(code=%0.1f)\n", (float)*code);
}


//997
static void EXEC_glReplacementCodeubSUN(byte *commandbuf)
{
	GLubyte *code = (GLubyte*)commandbuf;    commandbuf += sizeof(GLubyte);

	LOG("glReplacementCodeubSUN(code=%0.1f)\n", (float)*code);
}


//998
static void EXEC_glReplacementCodeuivSUN(byte *commandbuf)
{

	LOG("glReplacementCodeuivSUN()\n");

}


//999
static void EXEC_glReplacementCodeusvSUN(byte *commandbuf)
{

	LOG("glReplacementCodeusvSUN()\n");

}


//1000
static void EXEC_glReplacementCodeubvSUN(byte *commandbuf)
{

	LOG("glReplacementCodeubvSUN()\n");

}


//1001
static void EXEC_glReplacementCodePointerSUN(byte *commandbuf)
{

	LOG("glReplacementCodePointerSUN()\n");

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

	LOG("glColor4ubVertex2fSUN(r=%0.1f, g=%0.1f, b=%0.1f, a=%0.1f, x=%0.1f, y=%0.1f)\n", (float)*r, (float)*g, (float)*b, (float)*a, (float)*x, (float)*y);
}


//1003
static void EXEC_glColor4ubVertex2fvSUN(byte *commandbuf)
{

	LOG("glColor4ubVertex2fvSUN()\n");

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

	LOG("glColor4ubVertex3fSUN(r=%0.1f, g=%0.1f, b=%0.1f, a=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*r, (float)*g, (float)*b, (float)*a, (float)*x, (float)*y, (float)*z);
}


//1005
static void EXEC_glColor4ubVertex3fvSUN(byte *commandbuf)
{

	LOG("glColor4ubVertex3fvSUN()\n");

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

	LOG("glColor3fVertex3fSUN(r=%0.1f, g=%0.1f, b=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*r, (float)*g, (float)*b, (float)*x, (float)*y, (float)*z);
}


//1007
static void EXEC_glColor3fVertex3fvSUN(byte *commandbuf)
{

	LOG("glColor3fVertex3fvSUN()\n");

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

	LOG("glNormal3fVertex3fSUN(nx=%0.1f, ny=%0.1f, nz=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*nx, (float)*ny, (float)*nz, (float)*x, (float)*y, (float)*z);
}


//1009
static void EXEC_glNormal3fVertex3fvSUN(byte *commandbuf)
{

	LOG("glNormal3fVertex3fvSUN()\n");

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

	LOG("glColor4fNormal3fVertex3fSUN(r=%0.1f, g=%0.1f, b=%0.1f, a=%0.1f, nx=%0.1f, ny=%0.1f, nz=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*r, (float)*g, (float)*b, (float)*a, (float)*nx, (float)*ny, (float)*nz, (float)*x, (float)*y, (float)*z);
}


//1011
static void EXEC_glColor4fNormal3fVertex3fvSUN(byte *commandbuf)
{

	LOG("glColor4fNormal3fVertex3fvSUN()\n");

}


//1012
static void EXEC_glTexCoord2fVertex3fSUN(byte *commandbuf)
{
	GLfloat *s = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *t = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glTexCoord2fVertex3fSUN(s=%0.1f, t=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*s, (float)*t, (float)*x, (float)*y, (float)*z);
}


//1013
static void EXEC_glTexCoord2fVertex3fvSUN(byte *commandbuf)
{

	LOG("glTexCoord2fVertex3fvSUN()\n");

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

	LOG("glTexCoord4fVertex4fSUN(s=%0.1f, t=%0.1f, p=%0.1f, q=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*s, (float)*t, (float)*p, (float)*q, (float)*x, (float)*y, (float)*z, (float)*w);
}


//1015
static void EXEC_glTexCoord4fVertex4fvSUN(byte *commandbuf)
{

	LOG("glTexCoord4fVertex4fvSUN()\n");

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

	LOG("glTexCoord2fColor4ubVertex3fSUN(s=%0.1f, t=%0.1f, r=%0.1f, g=%0.1f, b=%0.1f, a=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*s, (float)*t, (float)*r, (float)*g, (float)*b, (float)*a, (float)*x, (float)*y, (float)*z);
}


//1017
static void EXEC_glTexCoord2fColor4ubVertex3fvSUN(byte *commandbuf)
{

	LOG("glTexCoord2fColor4ubVertex3fvSUN()\n");

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

	LOG("glTexCoord2fColor3fVertex3fSUN(s=%0.1f, t=%0.1f, r=%0.1f, g=%0.1f, b=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*s, (float)*t, (float)*r, (float)*g, (float)*b, (float)*x, (float)*y, (float)*z);
}


//1019
static void EXEC_glTexCoord2fColor3fVertex3fvSUN(byte *commandbuf)
{

	LOG("glTexCoord2fColor3fVertex3fvSUN()\n");

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

	LOG("glTexCoord2fNormal3fVertex3fSUN(s=%0.1f, t=%0.1f, nx=%0.1f, ny=%0.1f, nz=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*s, (float)*t, (float)*nx, (float)*ny, (float)*nz, (float)*x, (float)*y, (float)*z);
}


//1021
static void EXEC_glTexCoord2fNormal3fVertex3fvSUN(byte *commandbuf)
{

	LOG("glTexCoord2fNormal3fVertex3fvSUN()\n");

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

	LOG("glTexCoord2fColor4fNormal3fVertex3fSUN(s=%0.1f, t=%0.1f, r=%0.1f, g=%0.1f, b=%0.1f, a=%0.1f, nx=%0.1f, ny=%0.1f, nz=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*s, (float)*t, (float)*r, (float)*g, (float)*b, (float)*a, (float)*nx, (float)*ny, (float)*nz, (float)*x, (float)*y, (float)*z);
}


//1023
static void EXEC_glTexCoord2fColor4fNormal3fVertex3fvSUN(byte *commandbuf)
{

	LOG("glTexCoord2fColor4fNormal3fVertex3fvSUN()\n");

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

	LOG("glTexCoord4fColor4fNormal3fVertex4fSUN(s=%0.1f, t=%0.1f, p=%0.1f, q=%0.1f, r=%0.1f, g=%0.1f, b=%0.1f, a=%0.1f, nx=%0.1f, ny=%0.1f, nz=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*s, (float)*t, (float)*p, (float)*q, (float)*r, (float)*g, (float)*b, (float)*a, (float)*nx, (float)*ny, (float)*nz, (float)*x, (float)*y, (float)*z, (float)*w);
}


//1025
static void EXEC_glTexCoord4fColor4fNormal3fVertex4fvSUN(byte *commandbuf)
{

	LOG("glTexCoord4fColor4fNormal3fVertex4fvSUN()\n");

}


//1026
static void EXEC_glReplacementCodeuiVertex3fSUN(byte *commandbuf)
{
	GLuint *rc = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glReplacementCodeuiVertex3fSUN(rc=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*rc, (float)*x, (float)*y, (float)*z);
}


//1027
static void EXEC_glReplacementCodeuiVertex3fvSUN(byte *commandbuf)
{

	LOG("glReplacementCodeuiVertex3fvSUN()\n");

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

	LOG("glReplacementCodeuiColor4ubVertex3fSUN(rc=%0.1f, r=%0.1f, g=%0.1f, b=%0.1f, a=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*rc, (float)*r, (float)*g, (float)*b, (float)*a, (float)*x, (float)*y, (float)*z);
}


//1029
static void EXEC_glReplacementCodeuiColor4ubVertex3fvSUN(byte *commandbuf)
{

	LOG("glReplacementCodeuiColor4ubVertex3fvSUN()\n");

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

	LOG("glReplacementCodeuiColor3fVertex3fSUN(rc=%0.1f, r=%0.1f, g=%0.1f, b=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*rc, (float)*r, (float)*g, (float)*b, (float)*x, (float)*y, (float)*z);
}


//1031
static void EXEC_glReplacementCodeuiColor3fVertex3fvSUN(byte *commandbuf)
{

	LOG("glReplacementCodeuiColor3fVertex3fvSUN()\n");

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

	LOG("glReplacementCodeuiNormal3fVertex3fSUN(rc=%0.1f, nx=%0.1f, ny=%0.1f, nz=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*rc, (float)*nx, (float)*ny, (float)*nz, (float)*x, (float)*y, (float)*z);
}


//1033
static void EXEC_glReplacementCodeuiNormal3fVertex3fvSUN(byte *commandbuf)
{

	LOG("glReplacementCodeuiNormal3fVertex3fvSUN()\n");

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

	LOG("glReplacementCodeuiColor4fNormal3fVertex3fSUN(rc=%0.1f, r=%0.1f, g=%0.1f, b=%0.1f, a=%0.1f, nx=%0.1f, ny=%0.1f, nz=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*rc, (float)*r, (float)*g, (float)*b, (float)*a, (float)*nx, (float)*ny, (float)*nz, (float)*x, (float)*y, (float)*z);
}


//1035
static void EXEC_glReplacementCodeuiColor4fNormal3fVertex3fvSUN(byte *commandbuf)
{

	LOG("glReplacementCodeuiColor4fNormal3fVertex3fvSUN()\n");

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

	LOG("glReplacementCodeuiTexCoord2fVertex3fSUN(rc=%0.1f, s=%0.1f, t=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*rc, (float)*s, (float)*t, (float)*x, (float)*y, (float)*z);
}


//1037
static void EXEC_glReplacementCodeuiTexCoord2fVertex3fvSUN(byte *commandbuf)
{

	LOG("glReplacementCodeuiTexCoord2fVertex3fvSUN()\n");

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

	LOG("glReplacementCodeuiTexCoord2fNormal3fVertex3fSUN(rc=%0.1f, s=%0.1f, t=%0.1f, nx=%0.1f, ny=%0.1f, nz=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*rc, (float)*s, (float)*t, (float)*nx, (float)*ny, (float)*nz, (float)*x, (float)*y, (float)*z);
}


//1039
static void EXEC_glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN(byte *commandbuf)
{

	LOG("glReplacementCodeuiTexCoord2fNormal3fVertex3fvSUN()\n");

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

	LOG("glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fSUN(rc=%0.1f, s=%0.1f, t=%0.1f, r=%0.1f, g=%0.1f, b=%0.1f, a=%0.1f, nx=%0.1f, ny=%0.1f, nz=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*rc, (float)*s, (float)*t, (float)*r, (float)*g, (float)*b, (float)*a, (float)*nx, (float)*ny, (float)*nz, (float)*x, (float)*y, (float)*z);
}


//1041
static void EXEC_glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN(byte *commandbuf)
{

	LOG("glReplacementCodeuiTexCoord2fColor4fNormal3fVertex3fvSUN()\n");

}


//1042
static void EXEC_glBlendFuncSeparateEXT(byte *commandbuf)
{
	GLenum *sfactorRGB = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *dfactorRGB = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *sfactorAlpha = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *dfactorAlpha = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glBlendFuncSeparateEXT(sfactorRGB=%0.1f, dfactorRGB=%0.1f, sfactorAlpha=%0.1f, dfactorAlpha=%0.1f)\n", (float)*sfactorRGB, (float)*dfactorRGB, (float)*sfactorAlpha, (float)*dfactorAlpha);
}


//1043
static void EXEC_glVertexWeightfEXT(byte *commandbuf)
{
	GLfloat *weight = (GLfloat*)commandbuf;  commandbuf += sizeof(GLfloat);

	LOG("glVertexWeightfEXT(weight=%0.1f)\n", (float)*weight);
}


//1044
static void EXEC_glVertexWeightfvEXT(byte *commandbuf)
{

	LOG("glVertexWeightfvEXT()\n");

}


//1045
static void EXEC_glVertexWeightPointerEXT(byte *commandbuf)
{

	LOG("glVertexWeightPointerEXT()\n");

}


//1046
static void EXEC_glFlushVertexArrayRangeNV(byte *commandbuf)
{

	LOG("glFlushVertexArrayRangeNV()\n");
}


//1047
static void EXEC_glVertexArrayRangeNV(byte *commandbuf)
{

	LOG("glVertexArrayRangeNV()\n");

}


//1048
static void EXEC_glCombinerParameterfvNV(byte *commandbuf)
{

	LOG("glCombinerParameterfvNV()\n");

}


//1049
static void EXEC_glCombinerParameterfNV(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glCombinerParameterfNV(pname=%0.1f, param=%0.1f)\n", (float)*pname, (float)*param);
}


//1050
static void EXEC_glCombinerParameterivNV(byte *commandbuf)
{

	LOG("glCombinerParameterivNV()\n");

}


//1051
static void EXEC_glCombinerParameteriNV(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glCombinerParameteriNV(pname=%0.1f, param=%0.1f)\n", (float)*pname, (float)*param);
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

	LOG("glCombinerInputNV(stage=%0.1f, portion=%0.1f, variable=%0.1f, input=%0.1f, mapping=%0.1f, componentUsage=%0.1f)\n", (float)*stage, (float)*portion, (float)*variable, (float)*input, (float)*mapping, (float)*componentUsage);
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

	LOG("glCombinerOutputNV(stage=%0.1f, portion=%0.1f, abOutput=%0.1f, cdOutput=%0.1f, sumOutput=%0.1f, scale=%0.1f, bias=%0.1f, abDotProduct=%0.1f, cdDotProduct=%0.1f, muxSum=%0.1f)\n", (float)*stage, (float)*portion, (float)*abOutput, (float)*cdOutput, (float)*sumOutput, (float)*scale, (float)*bias, (float)*abDotProduct, (float)*cdDotProduct, (float)*muxSum);
}


//1054
static void EXEC_glFinalCombinerInputNV(byte *commandbuf)
{
	GLenum *variable = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *input = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *mapping = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLenum *componentUsage = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);

	LOG("glFinalCombinerInputNV(variable=%0.1f, input=%0.1f, mapping=%0.1f, componentUsage=%0.1f)\n", (float)*variable, (float)*input, (float)*mapping, (float)*componentUsage);
}


//1055
static void EXEC_glGetCombinerInputParameterfvNV(byte *commandbuf)
{

	LOG("glGetCombinerInputParameterfvNV()\n");

}


//1056
static void EXEC_glGetCombinerInputParameterivNV(byte *commandbuf)
{

	LOG("glGetCombinerInputParameterivNV()\n");

}


//1057
static void EXEC_glGetCombinerOutputParameterfvNV(byte *commandbuf)
{

	LOG("glGetCombinerOutputParameterfvNV()\n");

}


//1058
static void EXEC_glGetCombinerOutputParameterivNV(byte *commandbuf)
{

	LOG("glGetCombinerOutputParameterivNV()\n");

}


//1059
static void EXEC_glGetFinalCombinerInputParameterfvNV(byte *commandbuf)
{

	LOG("glGetFinalCombinerInputParameterfvNV()\n");

}


//1060
static void EXEC_glGetFinalCombinerInputParameterivNV(byte *commandbuf)
{

	LOG("glGetFinalCombinerInputParameterivNV()\n");

}


//1061
static void EXEC_glResizeBuffersMESA(byte *commandbuf)
{

	LOG("glResizeBuffersMESA()\n");
}


//1062
static void EXEC_glWindowPos2dMESA(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glWindowPos2dMESA(x=%0.1f, y=%0.1f)\n", (float)*x, (float)*y);
}


//1063
static void EXEC_glWindowPos2dvMESA(byte *commandbuf)
{

	LOG("glWindowPos2dvMESA()\n");

}


//1064
static void EXEC_glWindowPos2fMESA(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glWindowPos2fMESA(x=%0.1f, y=%0.1f)\n", (float)*x, (float)*y);
}


//1065
static void EXEC_glWindowPos2fvMESA(byte *commandbuf)
{

	LOG("glWindowPos2fvMESA()\n");

}


//1066
static void EXEC_glWindowPos2iMESA(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glWindowPos2iMESA(x=%0.1f, y=%0.1f)\n", (float)*x, (float)*y);
}


//1067
static void EXEC_glWindowPos2ivMESA(byte *commandbuf)
{

	LOG("glWindowPos2ivMESA()\n");

}


//1068
static void EXEC_glWindowPos2sMESA(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glWindowPos2sMESA(x=%0.1f, y=%0.1f)\n", (float)*x, (float)*y);
}


//1069
static void EXEC_glWindowPos2svMESA(byte *commandbuf)
{

	LOG("glWindowPos2svMESA()\n");

}


//1070
static void EXEC_glWindowPos3dMESA(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glWindowPos3dMESA(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//1071
static void EXEC_glWindowPos3dvMESA(byte *commandbuf)
{

	LOG("glWindowPos3dvMESA()\n");

}


//1072
static void EXEC_glWindowPos3fMESA(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glWindowPos3fMESA(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//1073
static void EXEC_glWindowPos3fvMESA(byte *commandbuf)
{

	LOG("glWindowPos3fvMESA()\n");

}


//1074
static void EXEC_glWindowPos3iMESA(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *z = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glWindowPos3iMESA(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//1075
static void EXEC_glWindowPos3ivMESA(byte *commandbuf)
{

	LOG("glWindowPos3ivMESA()\n");

}


//1076
static void EXEC_glWindowPos3sMESA(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glWindowPos3sMESA(x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*x, (float)*y, (float)*z);
}


//1077
static void EXEC_glWindowPos3svMESA(byte *commandbuf)
{

	LOG("glWindowPos3svMESA()\n");

}


//1078
static void EXEC_glWindowPos4dMESA(byte *commandbuf)
{
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *w = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glWindowPos4dMESA(x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*x, (float)*y, (float)*z, (float)*w);
}


//1079
static void EXEC_glWindowPos4dvMESA(byte *commandbuf)
{

	LOG("glWindowPos4dvMESA()\n");

}


//1080
static void EXEC_glWindowPos4fMESA(byte *commandbuf)
{
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *w = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glWindowPos4fMESA(x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*x, (float)*y, (float)*z, (float)*w);
}


//1081
static void EXEC_glWindowPos4fvMESA(byte *commandbuf)
{

	LOG("glWindowPos4fvMESA()\n");

}


//1082
static void EXEC_glWindowPos4iMESA(byte *commandbuf)
{
	GLint *x = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *y = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *z = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLint *w = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glWindowPos4iMESA(x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*x, (float)*y, (float)*z, (float)*w);
}


//1083
static void EXEC_glWindowPos4ivMESA(byte *commandbuf)
{

	LOG("glWindowPos4ivMESA()\n");

}


//1084
static void EXEC_glWindowPos4sMESA(byte *commandbuf)
{
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *w = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glWindowPos4sMESA(x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*x, (float)*y, (float)*z, (float)*w);
}


//1085
static void EXEC_glWindowPos4svMESA(byte *commandbuf)
{

	LOG("glWindowPos4svMESA()\n");

}


//1086
static void EXEC_glMultiModeDrawArraysIBM(byte *commandbuf)
{

	LOG("glMultiModeDrawArraysIBM()\n");

}


//1087
static void EXEC_glMultiModeDrawElementsIBM(byte *commandbuf)
{

	LOG("glMultiModeDrawElementsIBM()\n");

}


//1088
static void EXEC_glColorPointerListIBM(byte *commandbuf)
{

	LOG("glColorPointerListIBM()\n");

}


//1089
static void EXEC_glSecondaryColorPointerListIBM(byte *commandbuf)
{

	LOG("glSecondaryColorPointerListIBM()\n");

}


//1090
static void EXEC_glEdgeFlagPointerListIBM(byte *commandbuf)
{

	LOG("glEdgeFlagPointerListIBM()\n");

}


//1091
static void EXEC_glFogCoordPointerListIBM(byte *commandbuf)
{

	LOG("glFogCoordPointerListIBM()\n");

}


//1092
static void EXEC_glIndexPointerListIBM(byte *commandbuf)
{

	LOG("glIndexPointerListIBM()\n");

}


//1093
static void EXEC_glNormalPointerListIBM(byte *commandbuf)
{

	LOG("glNormalPointerListIBM()\n");

}


//1094
static void EXEC_glTexCoordPointerListIBM(byte *commandbuf)
{

	LOG("glTexCoordPointerListIBM()\n");

}


//1095
static void EXEC_glVertexPointerListIBM(byte *commandbuf)
{

	LOG("glVertexPointerListIBM()\n");

}


//1096
static void EXEC_glTbufferMask3DFX(byte *commandbuf)
{
	GLuint *mask = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	LOG("glTbufferMask3DFX(mask=%0.1f)\n", (float)*mask);
}


//1097
static void EXEC_glSampleMaskEXT(byte *commandbuf)
{
	GLclampf *value = (GLclampf*)commandbuf;     commandbuf += sizeof(GLclampf);
	GLboolean *invert = (GLboolean*)commandbuf;  commandbuf += sizeof(GLboolean);

	LOG("glSampleMaskEXT(value=%0.1f, invert=%0.1f)\n", (float)*value, (float)*invert);
}


//1098
static void EXEC_glSamplePatternEXT(byte *commandbuf)
{
	GLenum *pattern = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	LOG("glSamplePatternEXT(pattern=%0.1f)\n", (float)*pattern);
}


//1099
static void EXEC_glTextureColorMaskSGIS(byte *commandbuf)
{
	GLboolean *red = (GLboolean*)commandbuf;     commandbuf += sizeof(GLboolean);
	GLboolean *green = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);
	GLboolean *blue = (GLboolean*)commandbuf;    commandbuf += sizeof(GLboolean);
	GLboolean *alpha = (GLboolean*)commandbuf;   commandbuf += sizeof(GLboolean);

	LOG("glTextureColorMaskSGIS(red=%0.1f, green=%0.1f, blue=%0.1f, alpha=%0.1f)\n", (float)*red, (float)*green, (float)*blue, (float)*alpha);
}


//1100
static void EXEC_glDeleteFencesNV(byte *commandbuf)
{

	LOG("glDeleteFencesNV()\n");

}


//1101
static void EXEC_glGenFencesNV(byte *commandbuf)
{

	LOG("glGenFencesNV()\n");

}


//1102
static void EXEC_glIsFenceNV(byte *commandbuf)
{
	GLuint *fence = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	LOG("glIsFenceNV(fence=%0.1f)\n", (float)*fence);
}


//1103
static void EXEC_glTestFenceNV(byte *commandbuf)
{
	GLuint *fence = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	LOG("glTestFenceNV(fence=%0.1f)\n", (float)*fence);
}


//1104
static void EXEC_glGetFenceivNV(byte *commandbuf)
{

	LOG("glGetFenceivNV()\n");

}


//1105
static void EXEC_glFinishFenceNV(byte *commandbuf)
{
	GLuint *fence = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	LOG("glFinishFenceNV(fence=%0.1f)\n", (float)*fence);
}


//1106
static void EXEC_glSetFenceNV(byte *commandbuf)
{
	GLuint *fence = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLenum *condition = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	LOG("glSetFenceNV(fence=%0.1f, condition=%0.1f)\n", (float)*fence, (float)*condition);
}


//1107
static void EXEC_glMapControlPointsNV(byte *commandbuf)
{

	LOG("glMapControlPointsNV()\n");

}


//1108
static void EXEC_glMapParameterivNV(byte *commandbuf)
{

	LOG("glMapParameterivNV()\n");

}


//1109
static void EXEC_glMapParameterfvNV(byte *commandbuf)
{

	LOG("glMapParameterfvNV()\n");

}


//1110
static void EXEC_glGetMapControlPointsNV(byte *commandbuf)
{

	LOG("glGetMapControlPointsNV()\n");

}


//1111
static void EXEC_glGetMapParameterivNV(byte *commandbuf)
{

	LOG("glGetMapParameterivNV()\n");

}


//1112
static void EXEC_glGetMapParameterfvNV(byte *commandbuf)
{

	LOG("glGetMapParameterfvNV()\n");

}


//1113
static void EXEC_glGetMapAttribParameterivNV(byte *commandbuf)
{

	LOG("glGetMapAttribParameterivNV()\n");

}


//1114
static void EXEC_glGetMapAttribParameterfvNV(byte *commandbuf)
{

	LOG("glGetMapAttribParameterfvNV()\n");

}


//1115
static void EXEC_glEvalMapsNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glEvalMapsNV(target=%0.1f, mode=%0.1f)\n", (float)*target, (float)*mode);
}


//1116
static void EXEC_glCombinerStageParameterfvNV(byte *commandbuf)
{

	LOG("glCombinerStageParameterfvNV()\n");

}


//1117
static void EXEC_glGetCombinerStageParameterfvNV(byte *commandbuf)
{

	LOG("glGetCombinerStageParameterfvNV()\n");

}


//1118
static void EXEC_glAreProgramsResidentNV(byte *commandbuf)
{

	LOG("glAreProgramsResidentNV()\n");

}


//1119
static void EXEC_glBindProgramNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	LOG("glBindProgramNV(target=%0.1f, program=%0.1f)\n", (float)*target, (float)*program);
}


//1120
static void EXEC_glDeleteProgramsNV(byte *commandbuf)
{

	LOG("glDeleteProgramsNV()\n");

}


//1121
static void EXEC_glExecuteProgramNV(byte *commandbuf)
{

	LOG("glExecuteProgramNV()\n");

}


//1122
static void EXEC_glGenProgramsNV(byte *commandbuf)
{

	LOG("glGenProgramsNV()\n");

}


//1123
static void EXEC_glGetProgramParameterdvNV(byte *commandbuf)
{

	LOG("glGetProgramParameterdvNV()\n");

}


//1124
static void EXEC_glGetProgramParameterfvNV(byte *commandbuf)
{

	LOG("glGetProgramParameterfvNV()\n");

}


//1125
static void EXEC_glGetProgramivNV(byte *commandbuf)
{

	LOG("glGetProgramivNV()\n");

}


//1126
static void EXEC_glGetProgramStringNV(byte *commandbuf)
{

	LOG("glGetProgramStringNV()\n");

}


//1127
static void EXEC_glGetTrackMatrixivNV(byte *commandbuf)
{

	LOG("glGetTrackMatrixivNV()\n");

}


//1128
static void EXEC_glGetVertexAttribdvNV(byte *commandbuf)
{

	LOG("glGetVertexAttribdvNV()\n");

}


//1129
static void EXEC_glGetVertexAttribfvNV(byte *commandbuf)
{

	LOG("glGetVertexAttribfvNV()\n");

}


//1130
static void EXEC_glGetVertexAttribivNV(byte *commandbuf)
{

	LOG("glGetVertexAttribivNV()\n");

}


//1131
static void EXEC_glGetVertexAttribPointervNV(byte *commandbuf)
{

	LOG("glGetVertexAttribPointervNV()\n");

}


//1132
static void EXEC_glIsProgramNV(byte *commandbuf)
{
	GLuint *program = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);

	LOG("glIsProgramNV(program=%0.1f)\n", (float)*program);
}


//1133
static void EXEC_glLoadProgramNV(byte *commandbuf)
{

	LOG("glLoadProgramNV()\n");

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

	LOG("glProgramParameter4dNV(target=%0.1f, index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*target, (float)*index, (float)*x, (float)*y, (float)*z, (float)*w);
}


//1135
static void EXEC_glProgramParameter4dvNV(byte *commandbuf)
{

	LOG("glProgramParameter4dvNV()\n");

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

	LOG("glProgramParameter4fNV(target=%0.1f, index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*target, (float)*index, (float)*x, (float)*y, (float)*z, (float)*w);
}


//1137
static void EXEC_glProgramParameter4fvNV(byte *commandbuf)
{

	LOG("glProgramParameter4fvNV()\n");

}


//1138
static void EXEC_glProgramParameters4dvNV(byte *commandbuf)
{

	LOG("glProgramParameters4dvNV()\n");

}


//1139
static void EXEC_glProgramParameters4fvNV(byte *commandbuf)
{

	LOG("glProgramParameters4fvNV()\n");

}


//1140
static void EXEC_glRequestResidentProgramsNV(byte *commandbuf)
{

	LOG("glRequestResidentProgramsNV()\n");

}


//1141
static void EXEC_glTrackMatrixNV(byte *commandbuf)
{
	GLenum *target = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLuint *address = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLenum *matrix = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *transform = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	LOG("glTrackMatrixNV(target=%0.1f, address=%0.1f, matrix=%0.1f, transform=%0.1f)\n", (float)*target, (float)*address, (float)*matrix, (float)*transform);
}


//1142
static void EXEC_glVertexAttribPointerNV(byte *commandbuf)
{

	LOG("glVertexAttribPointerNV()\n");

}


//1143
static void EXEC_glVertexAttrib1sNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glVertexAttrib1sNV(index=%0.1f, x=%0.1f)\n", (float)*index, (float)*x);
}


//1144
static void EXEC_glVertexAttrib1svNV(byte *commandbuf)
{

	LOG("glVertexAttrib1svNV()\n");

}


//1145
static void EXEC_glVertexAttrib2sNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glVertexAttrib2sNV(index=%0.1f, x=%0.1f, y=%0.1f)\n", (float)*index, (float)*x, (float)*y);
}


//1146
static void EXEC_glVertexAttrib2svNV(byte *commandbuf)
{

	LOG("glVertexAttrib2svNV()\n");

}


//1147
static void EXEC_glVertexAttrib3sNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glVertexAttrib3sNV(index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*index, (float)*x, (float)*y, (float)*z);
}


//1148
static void EXEC_glVertexAttrib3svNV(byte *commandbuf)
{

	LOG("glVertexAttrib3svNV()\n");

}


//1149
static void EXEC_glVertexAttrib4sNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLshort *x = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *y = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *z = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);
	GLshort *w = (GLshort*)commandbuf;   commandbuf += sizeof(GLshort);

	LOG("glVertexAttrib4sNV(index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*index, (float)*x, (float)*y, (float)*z, (float)*w);
}


//1150
static void EXEC_glVertexAttrib4svNV(byte *commandbuf)
{

	LOG("glVertexAttrib4svNV()\n");

}


//1151
static void EXEC_glVertexAttrib1fNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glVertexAttrib1fNV(index=%0.1f, x=%0.1f)\n", (float)*index, (float)*x);
}


//1152
static void EXEC_glVertexAttrib1fvNV(byte *commandbuf)
{

	LOG("glVertexAttrib1fvNV()\n");

}


//1153
static void EXEC_glVertexAttrib2fNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glVertexAttrib2fNV(index=%0.1f, x=%0.1f, y=%0.1f)\n", (float)*index, (float)*x, (float)*y);
}


//1154
static void EXEC_glVertexAttrib2fvNV(byte *commandbuf)
{

	LOG("glVertexAttrib2fvNV()\n");

}


//1155
static void EXEC_glVertexAttrib3fNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glVertexAttrib3fNV(index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*index, (float)*x, (float)*y, (float)*z);
}


//1156
static void EXEC_glVertexAttrib3fvNV(byte *commandbuf)
{

	LOG("glVertexAttrib3fvNV()\n");

}


//1157
static void EXEC_glVertexAttrib4fNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLfloat *x = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *y = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *z = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);
	GLfloat *w = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glVertexAttrib4fNV(index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*index, (float)*x, (float)*y, (float)*z, (float)*w);
}


//1158
static void EXEC_glVertexAttrib4fvNV(byte *commandbuf)
{

	LOG("glVertexAttrib4fvNV()\n");

}


//1159
static void EXEC_glVertexAttrib1dNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glVertexAttrib1dNV(index=%0.1f, x=%0.1f)\n", (float)*index, (float)*x);
}


//1160
static void EXEC_glVertexAttrib1dvNV(byte *commandbuf)
{

	LOG("glVertexAttrib1dvNV()\n");

}


//1161
static void EXEC_glVertexAttrib2dNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glVertexAttrib2dNV(index=%0.1f, x=%0.1f, y=%0.1f)\n", (float)*index, (float)*x, (float)*y);
}


//1162
static void EXEC_glVertexAttrib2dvNV(byte *commandbuf)
{

	LOG("glVertexAttrib2dvNV()\n");

}


//1163
static void EXEC_glVertexAttrib3dNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glVertexAttrib3dNV(index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f)\n", (float)*index, (float)*x, (float)*y, (float)*z);
}


//1164
static void EXEC_glVertexAttrib3dvNV(byte *commandbuf)
{

	LOG("glVertexAttrib3dvNV()\n");

}


//1165
static void EXEC_glVertexAttrib4dNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLdouble *x = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *y = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *z = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);
	GLdouble *w = (GLdouble*)commandbuf;     commandbuf += sizeof(GLdouble);

	LOG("glVertexAttrib4dNV(index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*index, (float)*x, (float)*y, (float)*z, (float)*w);
}


//1166
static void EXEC_glVertexAttrib4dvNV(byte *commandbuf)
{

	LOG("glVertexAttrib4dvNV()\n");

}


//1167
static void EXEC_glVertexAttrib4ubNV(byte *commandbuf)
{
	GLuint *index = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLubyte *x = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *y = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *z = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);
	GLubyte *w = (GLubyte*)commandbuf;   commandbuf += sizeof(GLubyte);

	LOG("glVertexAttrib4ubNV(index=%0.1f, x=%0.1f, y=%0.1f, z=%0.1f, w=%0.1f)\n", (float)*index, (float)*x, (float)*y, (float)*z, (float)*w);
}


//1168
static void EXEC_glVertexAttrib4ubvNV(byte *commandbuf)
{

	LOG("glVertexAttrib4ubvNV()\n");

}


//1169
static void EXEC_glVertexAttribs1svNV(byte *commandbuf)
{

	LOG("glVertexAttribs1svNV()\n");

}


//1170
static void EXEC_glVertexAttribs2svNV(byte *commandbuf)
{

	LOG("glVertexAttribs2svNV()\n");

}


//1171
static void EXEC_glVertexAttribs3svNV(byte *commandbuf)
{

	LOG("glVertexAttribs3svNV()\n");

}


//1172
static void EXEC_glVertexAttribs4svNV(byte *commandbuf)
{

	LOG("glVertexAttribs4svNV()\n");

}


//1173
static void EXEC_glVertexAttribs1fvNV(byte *commandbuf)
{

	LOG("glVertexAttribs1fvNV()\n");

}


//1174
static void EXEC_glVertexAttribs2fvNV(byte *commandbuf)
{

	LOG("glVertexAttribs2fvNV()\n");

}


//1175
static void EXEC_glVertexAttribs3fvNV(byte *commandbuf)
{

	LOG("glVertexAttribs3fvNV()\n");

}


//1176
static void EXEC_glVertexAttribs4fvNV(byte *commandbuf)
{

	LOG("glVertexAttribs4fvNV()\n");

}


//1177
static void EXEC_glVertexAttribs1dvNV(byte *commandbuf)
{

	LOG("glVertexAttribs1dvNV()\n");

}


//1178
static void EXEC_glVertexAttribs2dvNV(byte *commandbuf)
{

	LOG("glVertexAttribs2dvNV()\n");

}


//1179
static void EXEC_glVertexAttribs3dvNV(byte *commandbuf)
{

	LOG("glVertexAttribs3dvNV()\n");

}


//1180
static void EXEC_glVertexAttribs4dvNV(byte *commandbuf)
{

	LOG("glVertexAttribs4dvNV()\n");

}


//1181
static void EXEC_glVertexAttribs4ubvNV(byte *commandbuf)
{

	LOG("glVertexAttribs4ubvNV()\n");

}


//1182
static void EXEC_glGenFragmentShadersATI(byte *commandbuf)
{
	GLuint *range = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);

	LOG("glGenFragmentShadersATI(range=%0.1f)\n", (float)*range);
}


//1183
static void EXEC_glBindFragmentShaderATI(byte *commandbuf)
{
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	LOG("glBindFragmentShaderATI(id=%0.1f)\n", (float)*id);
}


//1184
static void EXEC_glDeleteFragmentShaderATI(byte *commandbuf)
{
	GLuint *id = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);

	LOG("glDeleteFragmentShaderATI(id=%0.1f)\n", (float)*id);
}


//1185
static void EXEC_glBeginFragmentShaderATI(byte *commandbuf)
{

	LOG("glBeginFragmentShaderATI()\n");
}


//1186
static void EXEC_glEndFragmentShaderATI(byte *commandbuf)
{

	LOG("glEndFragmentShaderATI()\n");
}


//1187
static void EXEC_glPassTexCoordATI(byte *commandbuf)
{
	GLuint *dst = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *coord = (GLuint*)commandbuf;     commandbuf += sizeof(GLuint);
	GLenum *swizzle = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	LOG("glPassTexCoordATI(dst=%0.1f, coord=%0.1f, swizzle=%0.1f)\n", (float)*dst, (float)*coord, (float)*swizzle);
}


//1188
static void EXEC_glSampleMapATI(byte *commandbuf)
{
	GLuint *dst = (GLuint*)commandbuf;   commandbuf += sizeof(GLuint);
	GLuint *interp = (GLuint*)commandbuf;    commandbuf += sizeof(GLuint);
	GLenum *swizzle = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);

	LOG("glSampleMapATI(dst=%0.1f, interp=%0.1f, swizzle=%0.1f)\n", (float)*dst, (float)*interp, (float)*swizzle);
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

	LOG("glColorFragmentOp1ATI(op=%0.1f, dst=%0.1f, dstMask=%0.1f, dstMod=%0.1f, arg1=%0.1f, arg1Rep=%0.1f, arg1Mod=%0.1f)\n", (float)*op, (float)*dst, (float)*dstMask, (float)*dstMod, (float)*arg1, (float)*arg1Rep, (float)*arg1Mod);
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

	LOG("glColorFragmentOp2ATI(op=%0.1f, dst=%0.1f, dstMask=%0.1f, dstMod=%0.1f, arg1=%0.1f, arg1Rep=%0.1f, arg1Mod=%0.1f, arg2=%0.1f, arg2Rep=%0.1f, arg2Mod=%0.1f)\n", (float)*op, (float)*dst, (float)*dstMask, (float)*dstMod, (float)*arg1, (float)*arg1Rep, (float)*arg1Mod, (float)*arg2, (float)*arg2Rep, (float)*arg2Mod);
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

	LOG("glColorFragmentOp3ATI(op=%0.1f, dst=%0.1f, dstMask=%0.1f, dstMod=%0.1f, arg1=%0.1f, arg1Rep=%0.1f, arg1Mod=%0.1f, arg2=%0.1f, arg2Rep=%0.1f, arg2Mod=%0.1f, arg3=%0.1f, arg3Rep=%0.1f, arg3Mod=%0.1f)\n", (float)*op, (float)*dst, (float)*dstMask, (float)*dstMod, (float)*arg1, (float)*arg1Rep, (float)*arg1Mod, (float)*arg2, (float)*arg2Rep, (float)*arg2Mod, (float)*arg3, (float)*arg3Rep, (float)*arg3Mod);
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

	LOG("glAlphaFragmentOp1ATI(op=%0.1f, dst=%0.1f, dstMod=%0.1f, arg1=%0.1f, arg1Rep=%0.1f, arg1Mod=%0.1f)\n", (float)*op, (float)*dst, (float)*dstMod, (float)*arg1, (float)*arg1Rep, (float)*arg1Mod);
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

	LOG("glAlphaFragmentOp2ATI(op=%0.1f, dst=%0.1f, dstMod=%0.1f, arg1=%0.1f, arg1Rep=%0.1f, arg1Mod=%0.1f, arg2=%0.1f, arg2Rep=%0.1f, arg2Mod=%0.1f)\n", (float)*op, (float)*dst, (float)*dstMod, (float)*arg1, (float)*arg1Rep, (float)*arg1Mod, (float)*arg2, (float)*arg2Rep, (float)*arg2Mod);
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

	LOG("glAlphaFragmentOp3ATI(op=%0.1f, dst=%0.1f, dstMod=%0.1f, arg1=%0.1f, arg1Rep=%0.1f, arg1Mod=%0.1f, arg2=%0.1f, arg2Rep=%0.1f, arg2Mod=%0.1f, arg3=%0.1f, arg3Rep=%0.1f, arg3Mod=%0.1f)\n", (float)*op, (float)*dst, (float)*dstMod, (float)*arg1, (float)*arg1Rep, (float)*arg1Mod, (float)*arg2, (float)*arg2Rep, (float)*arg2Mod, (float)*arg3, (float)*arg3Rep, (float)*arg3Mod);
}


//1195
static void EXEC_glSetFragmentShaderConstantATI(byte *commandbuf)
{

	LOG("glSetFragmentShaderConstantATI()\n");

}


//1196
static void EXEC_glDrawMeshArraysSUN(byte *commandbuf)
{
	GLenum *mode = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLint *first = (GLint*)commandbuf;   commandbuf += sizeof(GLint);
	GLsizei *count = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);
	GLsizei *width = (GLsizei*)commandbuf;   commandbuf += sizeof(GLsizei);

	LOG("glDrawMeshArraysSUN(mode=%0.1f, first=%0.1f, count=%0.1f, width=%0.1f)\n", (float)*mode, (float)*first, (float)*count, (float)*width);
}


//1197
static void EXEC_glPointParameteriNV(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLint *param = (GLint*)commandbuf;   commandbuf += sizeof(GLint);

	LOG("glPointParameteriNV(pname=%0.1f, param=%0.1f)\n", (float)*pname, (float)*param);
}


//1198
static void EXEC_glPointParameterivNV(byte *commandbuf)
{

	LOG("glPointParameterivNV()\n");

}


//1199
static void EXEC_glActiveStencilFaceEXT(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glActiveStencilFaceEXT(face=%0.1f)\n", (float)*face);
}


//1200
static void EXEC_glDrawBuffersATI(byte *commandbuf)
{

	LOG("glDrawBuffersATI()\n");

}


//1201
static void EXEC_glProgramNamedParameter4fNV(byte *commandbuf)
{

	LOG("glProgramNamedParameter4fNV()\n");

}


//1202
static void EXEC_glProgramNamedParameter4dNV(byte *commandbuf)
{

	LOG("glProgramNamedParameter4dNV()\n");

}


//1203
static void EXEC_glProgramNamedParameter4fvNV(byte *commandbuf)
{

	LOG("glProgramNamedParameter4fvNV()\n");

}


//1204
static void EXEC_glProgramNamedParameter4dvNV(byte *commandbuf)
{

	LOG("glProgramNamedParameter4dvNV()\n");

}


//1205
static void EXEC_glGetProgramNamedParameterfvNV(byte *commandbuf)
{

	LOG("glGetProgramNamedParameterfvNV()\n");

}


//1206
static void EXEC_glGetProgramNamedParameterdvNV(byte *commandbuf)
{

	LOG("glGetProgramNamedParameterdvNV()\n");

}


//1207
static void EXEC_glDepthBoundsEXT(byte *commandbuf)
{
	GLclampd *zmin = (GLclampd*)commandbuf;  commandbuf += sizeof(GLclampd);
	GLclampd *zmax = (GLclampd*)commandbuf;  commandbuf += sizeof(GLclampd);

	LOG("glDepthBoundsEXT(zmin=%0.1f, zmax=%0.1f)\n", (float)*zmin, (float)*zmax);
}


//1208
static void EXEC_glBlendEquationSeparateEXT(byte *commandbuf)
{
	GLenum *modeRGB = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLenum *modeA = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	LOG("glBlendEquationSeparateEXT(modeRGB=%0.1f, modeA=%0.1f)\n", (float)*modeRGB, (float)*modeA);
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

	LOG("glBlitFramebufferEXT(srcX0=%0.1f, srcY0=%0.1f, srcX1=%0.1f, srcY1=%0.1f, dstX0=%0.1f, dstY0=%0.1f, dstX1=%0.1f, dstY1=%0.1f, mask=%0.1f, filter=%0.1f)\n", (float)*srcX0, (float)*srcY0, (float)*srcX1, (float)*srcY1, (float)*dstX0, (float)*dstY0, (float)*dstX1, (float)*dstY1, (float)*mask, (float)*filter);
}


//1210
static void EXEC_glBlendEquationSeparateATI(byte *commandbuf)
{
	GLenum *modeRGB = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLenum *modeA = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	LOG("glBlendEquationSeparateATI(modeRGB=%0.1f, modeA=%0.1f)\n", (float)*modeRGB, (float)*modeA);
}


//1211
static void EXEC_glStencilOpSeparateATI(byte *commandbuf)
{
	GLenum *face = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *sfail = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *zfail = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *zpass = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);

	LOG("glStencilOpSeparateATI(face=%0.1f, sfail=%0.1f, zfail=%0.1f, zpass=%0.1f)\n", (float)*face, (float)*sfail, (float)*zfail, (float)*zpass);
}


//1212
static void EXEC_glStencilFuncSeparateATI(byte *commandbuf)
{
	GLenum *frontfunc = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLenum *backfunc = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLint *ref = (GLint*)commandbuf;     commandbuf += sizeof(GLint);
	GLuint *mask = (GLuint*)commandbuf;  commandbuf += sizeof(GLuint);

	LOG("glStencilFuncSeparateATI(frontfunc=%0.1f, backfunc=%0.1f, ref=%0.1f, mask=%0.1f)\n", (float)*frontfunc, (float)*backfunc, (float)*ref, (float)*mask);
}


//1213
static void EXEC_glProgramEnvParameters4fvEXT(byte *commandbuf)
{

	LOG("glProgramEnvParameters4fvEXT()\n");

}


//1214
static void EXEC_glProgramLocalParameters4fvEXT(byte *commandbuf)
{

	LOG("glProgramLocalParameters4fvEXT()\n");

}


//1215
static void EXEC_glGetQueryObjecti64vEXT(byte *commandbuf)
{

	LOG("glGetQueryObjecti64vEXT()\n");

}


//1216
static void EXEC_glGetQueryObjectui64vEXT(byte *commandbuf)
{

	LOG("glGetQueryObjectui64vEXT()\n");

}


//1217
static void EXEC_glBlendFuncSeparateINGR(byte *commandbuf)
{
	GLenum *sfactorRGB = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *dfactorRGB = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
	GLenum *sfactorAlpha = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);
	GLenum *dfactorAlpha = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glBlendFuncSeparateINGR(sfactorRGB=%0.1f, dfactorRGB=%0.1f, sfactorAlpha=%0.1f, dfactorAlpha=%0.1f)\n", (float)*sfactorRGB, (float)*dfactorRGB, (float)*sfactorAlpha, (float)*dfactorAlpha);
}


//1218
static void EXEC_glCreateDebugObjectMESA(byte *commandbuf)
{

	LOG("glCreateDebugObjectMESA()\n");
}


//1219
static void EXEC_glClearDebugLogMESA(byte *commandbuf)
{
	GLhandleARB *obj = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);
	GLenum *logType = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLenum *shaderType = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
#ifndef __APPLE__
	LOG("glClearDebugLogMESA(obj=%0.1f, logType=%0.1f, shaderType=%0.1f)\n", (float)*obj, (float)*logType, (float)*shaderType);
#endif
}


//1220
static void EXEC_glGetDebugLogMESA(byte *commandbuf)
{

	LOG("glGetDebugLogMESA()\n");

}


//1221
static void EXEC_glGetDebugLogLengthMESA(byte *commandbuf)
{
	GLhandleARB *obj = (GLhandleARB*)commandbuf;     commandbuf += sizeof(GLhandleARB);
	GLenum *logType = (GLenum*)commandbuf;   commandbuf += sizeof(GLenum);
	GLenum *shaderType = (GLenum*)commandbuf;    commandbuf += sizeof(GLenum);
#ifndef __APPLE__
	LOG("glGetDebugLogLengthMESA(obj=%0.1f, logType=%0.1f, shaderType=%0.1f)\n", (float)*obj, (float)*logType, (float)*shaderType);
#endif
}


//1222
static void EXEC_glPointParameterfSGIS(byte *commandbuf)
{
	GLenum *pname = (GLenum*)commandbuf;     commandbuf += sizeof(GLenum);
	GLfloat *param = (GLfloat*)commandbuf;   commandbuf += sizeof(GLfloat);

	LOG("glPointParameterfSGIS(pname=%0.1f, param=%0.1f)\n", (float)*pname, (float)*param);
}


//1223
static void EXEC_glPointParameterfvSGIS(byte *commandbuf)
{

	LOG("glPointParameterfvSGIS()\n");

}


//1224
static void EXEC_glIglooInterfaceSGIX(byte *commandbuf)
{

	LOG("glIglooInterfaceSGIX()\n");

}


//1225
static void EXEC_glDeformationMap3dSGIX(byte *commandbuf)
{

	LOG("glDeformationMap3dSGIX()\n");

}


//1226
static void EXEC_glDeformationMap3fSGIX(byte *commandbuf)
{

	LOG("glDeformationMap3fSGIX()\n");

}


//1227
static void EXEC_glDeformSGIX(byte *commandbuf)
{
	GLenum *mask = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glDeformSGIX(mask=%0.1f)\n", (float)*mask);
}


//1228
static void EXEC_glLoadIdentityDeformationMapSGIX(byte *commandbuf)
{
	GLenum *mask = (GLenum*)commandbuf;  commandbuf += sizeof(GLenum);

	LOG("glLoadIdentityDeformationMapSGIX(mask=%0.1f)\n", (float)*mask);
}

bool TextModule::init()
{

	LOG("Loading TextModule\n");

	if(!hasSetup){
		setupPointers();
	}

	LOG("Loaded!\n");

	return true;
}

void setupPointers(){
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

	mFunctions[1498] = EXEC_CGLRepeat;
	mFunctions[1499] = EXEC_CGLSwapBuffers;
	
	hasSetup = true;
}

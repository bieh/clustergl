/*******************************************************************************
	mod_exec helper
*******************************************************************************/

#include "main.h"



extern int displayNumber; //this is the reason the fancier offsets don't work...
extern int currentBuffer;
extern bool glFrustumUsage;
extern bool bezelCompensation;
extern GLenum currentMode;

void handleLoadMatrix(Instruction *iter);
void handlePerspective(Instruction *iter);
void handleOrtho(Instruction *iter);
void handleScissor(Instruction *iter);
void handleViewport(Instruction *iter);

bool ExecModule::handleViewMode(Instruction *iter){
	//We override some methods here for view adjustment
	//TODO: Clean this up some more
	
	if(gConfig->viewMode == VIEWMODE_VIEWPORT){	
		if (iter->id == 305) { //glViewPort
			handleViewport(iter);
			return true;
		}
	}else if(gConfig->viewMode == VIEWMODE_CURVE){
		
		if (iter->id == 305) { //glViewPort
			glViewport(0, 0, 
				gConfig->totalWidth, gConfig->totalHeight);
			return true;
		}
		
		if (iter->id == 290) { //glLoadIdentity
		
			if(currentMode == GL_MODELVIEW){
				glLoadIdentity();
				glRotatef(gConfig->angle, 0, 1, 0);			
				return true;
			}
		}
	}

	/*
	else if (iter->id == 176) {
		handleScissor(iter); //glScissor
	}else if (iter->id == 296) {
		handleOrtho(iter); //glOrtho
	}else if (iter->id == 1539) {
		handlePerspective(iter); //gluPerspective
	}else if (iter->id == 291) {
		handleLoadMatrix(iter); //glLoadMatrixf
	}*/
	return false;
}





/*******************************************************************************
	Special functions we handle seperately
*******************************************************************************/
void handleViewport(Instruction *iter){
/*
	if(!glFrustumUsage){
		//no nothing, will be handled later (in gluPerpespective or glLoadMatrix)
		return
	}
*/
	
	float magic = 1800;
	
	GLint x = *((GLint*)iter->args);
	GLint y = *((GLint*)(iter->args+ sizeof(GLint)));
	GLsizei w = *((GLsizei*)(iter->args+ sizeof(GLint)*2));
	GLsizei h = *((GLsizei*)(iter->args+ sizeof(GLint)*2+ sizeof(GLsizei)));
	
	glViewport(-gConfig->offsetX*gConfig->scaleX,-gConfig->offsetY*gConfig->scaleY, 
				gConfig->totalWidth*gConfig->scaleX, gConfig->totalHeight*gConfig->scaleY);
	//glViewport(x,y,w,h);
	
	
	//if(bezelCompensation){

	//}else{
	//	glViewport((-offsetX*screenWidth)/magic,0, gConfig->totalWidth, 4560*3.0/4.0);
	//}
	
	//LOG("handleViewport(%d,%d,%d,%d)\n",x,y,w,h);
}

void handleScissor(Instruction *iter){
	//read original values from the instruction
	GLint x = *((GLint*)iter->args);
	GLint y = *((GLint*)(iter->args+ sizeof(GLint)));
	GLsizei w = *((GLsizei*)(iter->args+ sizeof(GLint)*2));
	GLsizei h = *((GLsizei*)(iter->args+ sizeof(GLint)*2+ sizeof(GLsizei)));

	//LOG("glScissor values %d %d %d %d\n", x, y, w, h);

	glScissor(0, 0, gConfig->totalWidth, gConfig->totalHeight);
	
	LOG("handleScissor\n");
}

void handleOrtho(Instruction *iter){
	
	//read original values from the instruction
	GLdouble left = *((GLdouble*)iter->args);
	GLdouble right = *((GLdouble*)(iter->args+ sizeof(GLdouble)));
	GLdouble bottom = *((GLdouble*)(iter->args+ sizeof(GLdouble)*2));
	GLdouble top = *((GLdouble*)(iter->args+ sizeof(GLdouble)*3));
	GLdouble nearVal = *((GLdouble*)(iter->args+ sizeof(GLdouble)*4));
	GLdouble farVal = *((GLdouble*)(iter->args+ sizeof(GLdouble)*5));

	//LOG("glOrtho values %lf %lf %lf %lf %lf %lf\n", left, right, bottom, top, nearVal, farVal);

	GLdouble totalWidth 	= 	right - left;
	GLdouble singleWidth 	= 	totalWidth * (gConfig->screenWidth / gConfig->totalWidth);
	GLdouble bezelWidth 	= 	totalWidth * (gConfig->screenGap / gConfig->totalWidth);

	GLdouble startingPoint = left + (displayNumber * (singleWidth + bezelWidth));
	
	//if ratio is correct, do nothing
	if(right/bottom == (double)gConfig->sizeX/gConfig->sizeY || 
		right/top == (double)gConfig->sizeX/gConfig->sizeY){
		glOrtho(startingPoint, 
				startingPoint + singleWidth, 
				bottom, 
				top, 
				nearVal, 
				farVal);
	}
		
	//if ratio incorrect, adjust so things dont get stretched
	else{			
		if(bottom > 0){
			glOrtho(startingPoint, 
					startingPoint + singleWidth, 
					right * gConfig->totalHeight/gConfig->totalWidth, 
					top, 
					nearVal, 
					farVal);
		}else{
			glOrtho(startingPoint, 
					startingPoint + singleWidth, 
					bottom, 
					right * gConfig->totalHeight/gConfig->totalWidth, 
					nearVal, 
					farVal);
			
		}
	}
	
	/*
	else {
						 //if ratio is correct, do nothing
		if(right/bottom == (double) sizeX/sizeY || right/top == (double) sizeX/sizeY)
			glOrtho(left, right, bottom, top, nearVal, farVal);
		else {			 //if ratio incorrect, adjust so things dont get stretched
			if(bottom > 0) glOrtho(left, right, right * sizeY/sizeX, top, nearVal, farVal);
			else glOrtho(left, right ,bottom,  right * sizeY/sizeX, nearVal, farVal);
		}
	}
	*/
	
	LOG("handleOrtho\n");
}

void handlePerspective(Instruction *iter){

	if(!glFrustumUsage) {
#ifdef __APPLE__
		LOG("**** WANTED TO CALL gluPerspective\n");
#else
		gluPerspective(45.0,8880.0/4560.0, 0.9f, 100.0f);
#endif
		return;
	}
	
	//read original values from the instruction
	GLdouble fovy = *((GLdouble*)iter->args);
	GLdouble aspect = *((GLdouble*)(iter->args+ sizeof(GLdouble)));
	GLdouble zNear = *((GLdouble*)(iter->args+ sizeof(GLdouble)*2));
	GLdouble zFar = *((GLdouble*)(iter->args+ sizeof(GLdouble)*3));

	//LOG("gluPerspective values %lf %lf %lf %lf\n", fovy, aspect, zNear, zFar);

	/*      diagram to explain how frustum works (without bezels, ignoring fov)
		(-1,-1)    (-0.6,-1)  (-0.2,-1)  (0.2,-1)   (0.6,-1)  (1,-1)
		~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		|---------||---------||---------||---------||---------|
		|---------||---------||---------||---------||---------|
		|---------||---------||---------||---------||---------|
		|---------||---------||---------||---------||---------|
		|---------||---------||---------||---------||---------|
		|---------||---------||---------||---------||---------|
		|---------||---------||---------||---------||---------|
		|---------||---------||---------||---------||---------|
		|---------||---------||---------||---------||---------|
		~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
		(-1,1)						      (1,1)
	*/

	const GLdouble pi = 3.1415926535897932384626433832795;
	GLdouble fW, fH;
	
	//calculate height, then adjust according to how different the
	//programs aspect ratio and our ratio is
	//fW and fH are the equivalent glFrustum calculations using the gluPerspective values
	fH = tan( (fovy / 360.0) * pi ) * gConfig->scaleY * zNear *
			(1.0/((gConfig->totalWidth/gConfig->totalHeight) / aspect));
	fW = tan( (fovy / 360.0) * pi ) * gConfig->scaleX * zNear * aspect;

	GLdouble totalWidth = fW * 2;
	GLdouble singleWidth = totalWidth * (gConfig->screenWidth / gConfig->totalWidth);
	GLdouble bezelWidth = totalWidth * (gConfig->screenGap / gConfig->totalWidth);
	if(!bezelCompensation) {
		singleWidth = totalWidth * ((gConfig->screenWidth + gConfig->screenGap) / gConfig->totalWidth);
		bezelWidth = 0;					
	}
	GLdouble startingPoint = -fW + (displayNumber * (singleWidth + bezelWidth));

	glFrustum(startingPoint, startingPoint + singleWidth, -fH, fH, zNear, zFar);
	
	/*
	else {
		fH = tan( (fovy / 360.0) * pi ) * scaleY * zNear * (1.0/((sizeX * 1.0/sizeY) / aspect));
		fW = tan( (fovy / 360.0) * pi ) * scaleX * zNear * aspect;

		glFrustum(-fW, fW, -fH, fH, zNear, zFar);

		//gluPerspective(fovy, aspect, zNear, zFar);
	}
	*/
	
	LOG("handlePerspective\n");
}

void handleLoadMatrix(Instruction *iter){
	GLfloat * m = (GLfloat *)iter->buffers[0].buffer;
	GLfloat * mSaved = (GLfloat *) malloc(sizeof(GLfloat) * 16);
	//copy matrix, otherwise CGLRepeat buffers doesn't work
	memcpy(mSaved, m, sizeof(GLfloat) * 16);
	if(currentMode == GL_PROJECTION) {
		//if(symphony) {
			//m[0]= (whatever is is when we are given it) * (proportion that will be seen_
			mSaved[0]= mSaved[0] * (5/((gConfig->screenWidth * 5)/gConfig->totalWidth));
			//m[8] = (left + right)/(left-right) + screenOffset * bezel (from API)
			mSaved[8]=  -(2-(2*gConfig->screenWidth/gConfig->totalWidth))/
				(2*gConfig->screenWidth/gConfig->totalWidth)
				+ displayNumber * (2 * gConfig->totalWidth/(gConfig->screenWidth * 5));
		//}
		glLoadMatrixf(mSaved);
		free(mSaved);
	}
	else {
		glLoadMatrixf(m);
	}
	
	LOG("handleLoadMatrix\n");
}


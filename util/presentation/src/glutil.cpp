#include "main.h"

GLUtil mGLU; //global

/*********************************************
		Draws a box in 3space
**********************************************/
void GLUtil::cube(float x, float y, float z){

	x/=2; y/=2; z/=2;

	glBegin(GL_QUADS);
		// Front Face
		glNormal3f( 0.0f, 0.0f, 1.0f);		// Normal Pointing Towards Viewer
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-x, -y,  z);	// Point 1 (Front)
		glTexCoord2f(1.0f, 0.0f); glVertex3f( x, -y,  z);	// Point 2 (Front)
		glTexCoord2f(1.0f, 1.0f); glVertex3f( x,  y,  z);	// Point 3 (Front)
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-x,  y,  z);	// Point 4 (Front)
		// Back Face
		glNormal3f( 0.0f, 0.0f,-1.0f);		// Normal Pointing Away From Viewer
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-x, -y, -z);	// Point 1 (Back)
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-x,  y, -z);	// Point 2 (Back)
		glTexCoord2f(0.0f, 1.0f); glVertex3f( x,  y, -z);	// Point 3 (Back)
		glTexCoord2f(0.0f, 0.0f); glVertex3f( x, -y, -z);	// Point 4 (Back)
		// Top Face
		glNormal3f( 0.0f, 1.0f, 0.0f);			// Normal Pointing Up
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-x,  y, -z);	// Point 1 (Top)
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-x,  y,  z);	// Point 2 (Top)
		glTexCoord2f(1.0f, 0.0f); glVertex3f( x,  y,  z);	// Point 3 (Top)
		glTexCoord2f(1.0f, 1.0f); glVertex3f( x,  y, -z);	// Point 4 (Top)
		// Bottom Face
		glNormal3f( 0.0f,-1.0f, 0.0f);			// Normal Pointing Down
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-x, -y, -z);	// Point 1 (Bottom)
		glTexCoord2f(0.0f, 1.0f); glVertex3f( x, -y, -z);	// Point 2 (Bottom)
		glTexCoord2f(0.0f, 0.0f); glVertex3f( x, -y,  z);	// Point 3 (Bottom)
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-x, -y,  z);	// Point 4 (Bottom)
		// Right face
		glNormal3f( 1.0f, 0.0f, 0.0f);			// Normal Pointing Right
		glTexCoord2f(1.0f, 0.0f); glVertex3f( x, -y, -z);	// Point 1 (Right)
		glTexCoord2f(1.0f, 1.0f); glVertex3f( x,  y, -z);	// Point 2 (Right)
		glTexCoord2f(0.0f, 1.0f); glVertex3f( x,  y,  z);	// Point 3 (Right)
		glTexCoord2f(0.0f, 0.0f); glVertex3f( x, -y,  z);	// Point 4 (Right)
		// Left Face
		glNormal3f(-1.0f, 0.0f, 0.0f);			// Normal Pointing Left
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-x, -y, -z);	// Point 1 (Left)
		glTexCoord2f(1.0f, 0.0f); glVertex3f(-x, -y,  z);	// Point 2 (Left)
		glTexCoord2f(1.0f, 1.0f); glVertex3f(-x,  y,  z);	// Point 3 (Left)
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-x,  y, -z);	// Point 4 (Left)
	glEnd();								
}

/*********************************************
	Turns the 2D mouse coords into a 3D
	intersection point of the plane
**********************************************/
Vector3 GLUtil::calculateMousePoint(){
	
	//First get the mouse point
	int x, y;
	SDL_GetMouseState(&x, &y); 
		
	//Temp storage
	GLint viewport[4];
	GLdouble modelview[16];
	GLdouble projection[16];
	GLfloat winX, winY, winZ;
	GLdouble posX, posY, posZ;

	//Get the various matrixes to put into gluUnProject
	glGetDoublev( GL_MODELVIEW_MATRIX, modelview );
	glGetDoublev( GL_PROJECTION_MATRIX, projection );
	glGetIntegerv( GL_VIEWPORT, viewport );
	
	winX = (float)x;
	winY = (float)viewport[3] - (float)y; //GL inverts Y, so fix that
			
	//Get the depth
	glReadPixels( x, int(winY), 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ );
	
	//LOG("%f\n", winZ);
			
	//Unproject. This gives us the 3D point that the mouse is hovered over
	//Now we throw some maths at it and it will give us the intersection point 
	gluUnProject( 	winX, winY, winZ, 
					modelview, projection, viewport, 
					&posX, &posY, &posZ	);
	
	return Vector3(posX, posY, posZ);
}

/*********************************************
	Turns a 3D point into a 2D screen pos
**********************************************/
Vector2 GLUtil::utilProject(float x, float y, float z){
	GLdouble pX, pY, pZ;
	
	// arrays to hold matrix information

	GLdouble model_view[16];
	glGetDoublev(GL_MODELVIEW_MATRIX, model_view);

	GLdouble projection[16];
	glGetDoublev(GL_PROJECTION_MATRIX, projection);

	GLint viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	
	// get 3D coordinates based on window coordinates
	gluProject(	x, y, z,
				model_view, projection, viewport,
				&pX, &pY, &pZ);
			
	return Vector2(pX, iScreenY - pY);
}

/*********************************************
				Draws the plane
**********************************************/
void GLUtil::plane(float x, float y, float z){

	x/=2; y/=2; z/=2;
			
	glBegin(GL_QUADS);
		// Front Face
		glNormal3f( 0.0f, 0.0f, 1.0f);	// Normal Pointing Towards Viewer
		glTexCoord2f(0.0f, 0.0f); glVertex3f(-x, -y,  z);	// Point 1 (Front)
		glTexCoord2f(1.0f, 0.0f); glVertex3f( x, -y,  z);	// Point 2 (Front)
		
		//*facepalm*
#ifdef _WINDOWS
		glTexCoord2f(1.0f, 1.0f); glVertex3f( x,  y,  z);	// Point 3 (Front)
		glTexCoord2f(0.0f, 1.0f); glVertex3f(-x,  y,  z);	// Point 4 (Front)		
#else
		glTexCoord2f(1.0f, -1.0f); glVertex3f( x,  y,  z);	// Point 3 (Front)
		glTexCoord2f(0.0f, -1.0f); glVertex3f(-x,  y,  z);	// Point 4 (Front)		
#endif
	glEnd(); // Done Drawing Quads

	glDisable(GL_DEPTH_TEST);
	glDisable(GL_TEXTURE_2D);
	glColor3f(0.25f, 0.25f, 0.25f);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	glBegin(GL_QUADS);
		// Front Face
		glNormal3f( 0.0f, 0.0f, 1.0f);	// Normal Pointing Towards Viewer
		glVertex3f(-x, -y,  z);	// Point 1 (Front)
		glVertex3f( x, -y,  z);	// Point 2 (Front)
		glVertex3f( x,  y,  z);	// Point 3 (Front)
		glVertex3f(-x,  y,  z);	// Point 4 (Front)		
	glEnd(); // Done Drawing Quads
	
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);

	
}

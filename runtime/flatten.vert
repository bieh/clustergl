varying float xpos;
varying float ypos;
varying float zpos;
varying float myVal;

void main(void)
{
	xpos = clamp(gl_Vertex.x, 0.0,1.0);
    	ypos = clamp(gl_Vertex.y, 0.0,1.0);
	zpos = clamp(gl_Vertex.z, 0.0,1.0);
	myVal = 2;
	vec4 v = vec4(gl_Vertex);
	//gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
	gl_Position = gl_ModelViewProjectionMatrix * v;
} 

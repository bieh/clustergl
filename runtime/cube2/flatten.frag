varying float xpos;
varying float ypos;
varying float zpos;
varying float myVal;

void main(void)
{
	gl_FragColor = vec4(xpos, ypos, zpos,1.0);
}

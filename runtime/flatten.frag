varying float xpos;
varying float ypos;
varying float zpos;
varying float myVal;

void main(void)
{
	/*//coloured
	if(xpos > 0.995)
	{
	gl_FragColor = vec3(1.0, 0.0 , 0.0);
	}
	else if(ypos > 0.995)
	{
	gl_FragColor = vec3(1.0, 1.0 , 0.0);
	}
	else if(zpos > 0.995)
	{
	gl_FragColor = vec3(1.0, 0.0 , 1.0);
	}
	else if(xpos < 0.005)
	{
	gl_FragColor = vec3(0.0, 1.0 , 0.0);
	}
	else if(ypos < 0.005)
	{
	gl_FragColor = vec3(0.0, 0.0 , 1.0);
	}
	else if(zpos < 0.005)
	{
	gl_FragColor = vec3(0.0, 1.0 , 1.0);
	}*/
	//blended colour
	gl_FragColor = vec4(xpos, ypos, zpos, 1.0);	
	//grayscale
	//gl_FragColor = vec3(xpos + 0.2, xpos + 0.2, xpos + 0.2);
	//new one
	
	/*float newx, newy, newz;
	newx = xpos;
	newy = ypos;
	newz = zpos;
	if(xpos < 0.5)
	{
	newx = 0.5 + (0.5 - xpos);
	}
	if(ypos < 0.5)
	{
	newy = 0.5 + (0.5 - ypos);
	}
	if(zpos < 0.5)
	{
	newz = 0.5 + (0.5 - zpos);
	}
	float n = (newx * newy * newz) + 0.1;
	n = n * n;
	gl_FragColor = vec3(n, n , n);*/
	//float avg = (xpos + ypos + zpos)/3;
	//gl_FragColor = vec3(avg, avg, avg);
}

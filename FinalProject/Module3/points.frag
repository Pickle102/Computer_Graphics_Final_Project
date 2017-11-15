#version 150

out vec4 fragColor;    // Output fragment color

void main() 
{
	if (dot(gl_PointCoord-0.5, gl_PointCoord-0.5) > 0.25)
		discard;
	else
		fragColor = vec4(gl_PointCoord.st, 0, 1);
}

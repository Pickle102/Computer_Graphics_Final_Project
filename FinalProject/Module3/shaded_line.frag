#version 150

smooth in vec4 color;	// Smooth interpolation of color - input from vertex shader
out vec4 fragColor;

void main()
{
	fragColor = color;
}
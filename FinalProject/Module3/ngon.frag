#version 150

uniform vec4 color;    // Uniform color from the application
out vec4 fragColor;

void main()
{
	fragColor = color;
}
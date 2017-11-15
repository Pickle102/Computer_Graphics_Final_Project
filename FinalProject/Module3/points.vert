#version 150

// Per veretx attributes
in vec2 position;

// Uniform
uniform mat4 ortho;

void main()
{
	gl_Position  = ortho * vec4(position, 0.0, 1.0);
}
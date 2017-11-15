#version 150

in vec2 position;       // Vertex position attribute
in vec4 vertexColor;    // Vertex color attribute

uniform mat4 ortho;		// Uniform transformation matrix from application

smooth out vec4 color;	// Smooth interpolation of color output to fragment shader

void main()
{
	color = vertexColor;
	gl_Position = ortho * vec4(position, 0.0, 1.0);
}

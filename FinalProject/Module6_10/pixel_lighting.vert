#version 150

// Outgoing normal and vertex (interpolated) in world coordinates
smooth out vec3 normal;
smooth out vec3 vertex;
smooth out vec2 texture;

// Incoming vertex and normal attributes
in vec3 vertexPosition;		// Vertex position attribute
in vec3 vertexNormal;		  // Vertex normal attribute
in vec2 texturePosition;  // Texture coordinate

// Uniforms for matrices
uniform mat4 pvm;					  // Composite projection, view, model matrix
uniform mat4 modelMatrix;	  // Modeling  matrix
uniform mat4 normalMatrix;	// Normal transformation matrix

// Simple shader for Phong (per-pixel) shading. The fragment shader will
// do all the work. We need to pass per-vertex normals to the fragment
// shader. We also will transform the vertex into world coordinates so 
// the fragment shader can interpolate world coordinates.
void main()
{
  // Output interpolated texture position
	texture = texturePosition;

	// Transform normal and position to world coords. 
	normal = normalize(vec3(normalMatrix * vec4(vertexNormal, 0.0)));
	vertex = vec3((modelMatrix * vec4(vertexPosition, 1.0)));

	// Convert position to clip coordinates and pass along
	gl_Position = pvm * vec4(vertexPosition, 1.0);
}

#version 150

// Outgoing normal and vertex (interpolated) in world coordinates
smooth out vec3 normal;
smooth out vec3 vertex;
smooth out vec2 texPos;

// Incoming vertex and normal attributes
in vec3 vertexPosition;		// Vertex position attribute
in vec3 vertexNormal;		// Vertex normal attribute
in vec2 texturePosition;     // Texture coordinate

// Uniforms for matrices
uniform mat4 pvm;					// Composite projection, view, model matrix
uniform mat4 projectionMatrix;	    // Projection  matrix
uniform mat4 modelMatrix;			// Modeling  matrix
uniform mat4 viewMatrix;            // View matrix
uniform mat4 normalMatrix;			// Normal transformation matrix

// Enable this texture to be a cylindrical billboard (like a tree)
uniform int enableBillboard;
uniform vec3 cameraUp;
uniform vec3 cameraRight;

out vec4 viewSpace;

// Simple shader for Phong (per-pixel) shading. The fragment shader will
// do all the work. We need to pass per-vertex normals to the fragment
// shader. We also will transform the vertex into world coordinates so 
// the fragment shader can interpolate world coordinates.
void main()
{
	// Output interpolated texture position
	texPos = texturePosition;

	// Transform normal and position to world coords. 
	normal = normalize(vec3(normalMatrix * vec4(vertexNormal, 0.0)));
	vertex = vec3((modelMatrix * vec4(vertexPosition, 1.0)));

	vec3 vertexPos;
	mat4 modelViewMatrix = viewMatrix * modelMatrix;
	if (enableBillboard == 1)
	{
		vertexPos.x = vertexPosition.x - ((cameraRight.x * normal.x) + (cameraUp.x * normal.y));
		vertexPos.y = vertexPosition.y - ((cameraRight.y * normal.x) + (cameraUp.y * normal.y));
		vertexPos.z = vertexPosition.z - ((cameraRight.z * normal.x) + (cameraUp.z * normal.y));

		gl_Position = projectionMatrix * modelViewMatrix * vec4(vertexPos, 1.0);
	}
	else
	{
		//vertexPos = vec4(vertexPosition, 1.0);
		gl_Position = pvm * vec4(vertexPosition, 1.0);
	}

	// Convert position to clip coordinates and pass along
	//gl_Position = pvm * vec4(vertexPosition, 1.0);

	// Send to fragment shader
	viewSpace = viewMatrix * modelMatrix * vec4(vertexPosition, 1.0);
}

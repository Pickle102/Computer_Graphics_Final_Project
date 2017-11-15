#version 150

smooth out vec4 color;				// Color passed to the fragment shader
in vec3 vertexPosition;				// Vertex position attribute
in vec3 vertexNormal;				// Vertex normal attribute
uniform vec3 materialColor;			// Material diffuse color
uniform mat4 pvm;					// Composite projection, view, model matrix
uniform mat4 modelMatrix;		    // Composite modeling matrix
uniform mat4 normalMatrix;			// Normal transformation matrix

void main() 
{
   // Fixed light position in world coordinates. Light is behind the camera in 
   // world coordinates and is hard-coded here for now!
   vec3 lightPosition = vec3(0.0, -100.0, 50.0f);

   // Convert normal and position to world coords. Construct L - from vertex to light
   vec3 N = normalize(vec3(normalMatrix * vec4(vertexNormal, 0.0)));
   vec4 v = modelMatrix * vec4(vertexPosition, 1.0);
   vec3 L = normalize(vec3(lightPosition - vec3(v)));

   // The diffuse shading equation. Intnesity depends on cos of L and N
   color = vec4(materialColor * max(dot(L, N), 0.0), 1.0);

   // Convert position to clip coordinates and pass along
   gl_Position = pvm * vec4(vertexPosition, 1.0);
}
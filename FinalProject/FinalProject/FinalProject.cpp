//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:  Jennifer Olk
//	File:    FinalProject.cpp
//	Purpose: OpenGL and GLUT program to draw polygon mesh objects.
//
//============================================================================

#define _CRT_SECURE_NO_WARNINGS 1 

#include <iostream>
#include <vector>

// Include OpenGL support
#include <GL/gl3w.h>
#include <GL/freeglut.h>

// Include local libraries (geometry first)
#include "geometry/geometry.h"
#include "shader_support/glsl_shader.h"
#include "scene/scene.h"

// Local objects
#include "BilinearPatch.h"
#include "GeodesicDome.h"
#include "unittrough.h"
#include "unit_subdivided_sphere.h"
#include "lighting_shader_node.h"


// externs - methods in ObjectCreation.cpp
extern SceneNode*  ConstructCube(int n, const int position_loc, const int normal_loc, const int texture_loc);
extern TriSurface* ConstructIcosahedron(const int position_loc, const int normal_loc);
extern TriSurface* ConstructOctahedron(const int position_loc, const int normal_loc);
extern TriSurface* ConstructTetrahedron(const int position_loc, const int normal_loc);
extern TriSurface* ConstructDodecahedron(const int position_loc, const int normal_loc);
extern TriSurface* ConstructBuckyball(const int position_loc, const int normal_loc);

// Light types
enum LightType { FIXED_WORLD, MOVING_LIGHT, MINERS_LIGHT };

// Geometric objects - switched based on key commands
enum DrawObjectType { CYLINDER, CONE, TRUNCATED_CONE, TORUS, CUBE, ICOSAHEDRON,
                      OCTAHEDRON, TETRAHEDRON, DODECAHEDRON, GEODESIC_DOME,
                      BILINEAR_PATCH, REV_SURFACE, BUCKYBALL, SPHERE2,
                      TEAPOT, EARTH, TROUGH, A10, BUG
};

// Simple "selector" node to allow selection of one node from an array
// We may want to make this more general later!
class SelectorNode : public SceneNode
{
public:
   SelectorNode(const std::vector<SceneNode*>& nodes)
   {
      m_nodes = nodes;
      m_current = 4;
   }

   virtual void Draw(SceneState& sceneState)
	{
		m_nodes[m_current]->Draw(sceneState);
	}

   virtual void SetCurrent(const uint32_t current)
   {
      if (current < m_nodes.size())
         m_current = current;
   }
protected:
   uint32_t m_current;
   std::vector<SceneNode*> m_nodes;
};


// Default lighting: fixed world coordinate
int LightRotation = 0;
HPoint3 WorldLightPosition;
Matrix4x4 LightTransform;
LightType CurrentLight = FIXED_WORLD;

// Global scene state
//SceneState MySceneState;

// While mouse button is down, the view will be updated
bool TimerActive = false;
bool Animate     = false;
bool Forward     = true;
int MouseX, MouseY;
int RenderWidth  = 640;
int RenderHeight = 480;

// Scene graph elements
SceneNode* SceneRoot;                     // Root of the scene graph
CameraNode* MyCamera;                     // Camera
LightNode* MinersLight;                   // View dependent light
LightNode* WorldLight;                    // World coordiante light (fixed or moving)

char vertex_shader[] = "#version 150\n\
smooth out vec3 normal;\n\
smooth out vec3 vertex;\n\
smooth out vec2 texPos;\n\
in vec3 vertexPosition;\n\
in vec3 vertexNormal;\n\
in vec2 texturePosition;\n\
uniform mat4 pvm;\n\
uniform mat4 modelMatrix;\n\
uniform mat4 normalMatrix;\n\
void main()\n\
{\n\
texPos = texturePosition;\n\
normal = normalize(vec3(normalMatrix * vec4(vertexNormal, 0.0)));\n\
vertex = vec3((modelMatrix * vec4(vertexPosition, 1.0)));\n\
gl_Position = pvm * vec4(vertexPosition, 1.0);\n\
}";


char fragment_shader[] = "#version 150\n\
out vec4 fragColor;\n\
smooth in vec3 normal;\n\
smooth in vec3 vertex;\n\
smooth in vec2 texPos;\n\
uniform vec4   materialAmbient;\n\
uniform	vec4   materialDiffuse;\n\
uniform	vec4   materialSpecular;\n\
uniform	vec4   materialEmission;\n\
uniform	float  materialShininess;\n\
uniform int useTexture;\n\
uniform	sampler2D texImage;\n\
uniform vec4  globalLightAmbient;\n\
uniform vec3  cameraPosition;\n\
uniform int numLights;\n\
const int MAX_LIGHTS = 3;\n\
struct LightSource\n\
{\n\
  int  enabled;\n\
  int  spotlight;\n\
  vec4 position;\n\
  vec4 ambient;\n\
  vec4 diffuse;\n\
  vec4 specular;\n\
  float constantAttenuation;\n\
  float linearAttenuation;\n\
  float quadraticAttenuation;\n\
  float spotCosCutoff; \n\
  float spotExponent;\n\
  vec3  spotDirection;\n\
};\n\
uniform LightSource lights[MAX_LIGHTS];\n\
float calculateAttenuation(in int i, in float distance)\n\
{\n\
  return (1.0 / (lights[i].constantAttenuation +\n\
    (lights[i].linearAttenuation    * distance) +\n\
    (lights[i].quadraticAttenuation * distance * distance)));\n\
}\n\
void directionalLight(in int i, in vec3 N, in vec3 vtx, in vec3 V, inout vec4 ambient,\n\
  inout vec4 diffuse, inout vec4 specular)\n\
{\n\
  ambient += lights[i].ambient;\n\
  vec3 L = lights[i].position.xyz;\n\
  float nDotL = dot(N, L);\n\
  if (nDotL > 0.0)\n\
  {\n\
    diffuse += lights[i].diffuse * nDotL;\n\
    vec3 H = normalize(L + V);\n\
    float nDotH = dot(N, H);\n\
    if (nDotH > 0.0)\n\
      specular += lights[i].specular * pow(nDotH, materialShininess);\n\
  }\n\
}\n\
void pointLight(in int i, in vec3 N, in vec3 vtx, in vec3 V, inout vec4 ambient,\n\
  inout vec4 diffuse, inout vec4 specular)\n\
{\n\
  vec3 tmp = lights[i].position.xyz - vtx;\n\
  float dist = length(tmp);\n\
  vec3 L = tmp * (1.0 / dist);\n\
  float attenuation = calculateAttenuation(i, dist);\n\
  ambient += lights[i].ambient * attenuation;\n\
  float nDotL = dot(N, L);\n\
  if (nDotL > 0.0)\n\
  {\n\
    diffuse += lights[i].diffuse  * attenuation * nDotL;\n\
    vec3 H = normalize(L + V);\n\
    float nDotH = dot(N, H);\n\
    if (nDotH > 0.0)\n\
      specular += lights[i].specular * attenuation * pow(nDotH, materialShininess);\n\
  }\n\
}\n\
void spotLight(in int i, in vec3 N, in vec3 vtx, in vec3 V, inout vec4 ambient,\n\
  inout vec4 diffuse, inout vec4 specular)\n\
{\n\
  vec3 tmp = lights[i].position.xyz - vtx;\n\
  float dist = length(tmp);\n\
  vec3 L = tmp * (1.0 / dist);\n\
  float attenuation = calculateAttenuation(i, dist);\n\
  float nDotL = dot(N, L);\n\
  if (nDotL > 0.0)\n\
  {\n\
    float spotEffect = dot(lights[i].spotDirection, -L);\n\
    if (spotEffect > lights[i].spotCosCutoff)\n\
    {\n\
      attenuation *= pow(spotEffect, lights[i].spotExponent);\n\
      diffuse += lights[i].diffuse  * attenuation * nDotL;\n\
      vec3 H = normalize(L + V);\n\
      float nDotH = dot(N, H);\n\
      if (nDotH > 0.0)\n\
        specular += lights[i].specular * attenuation * pow(nDotH, materialShininess);\n\
    }\n\
    else\n\
      attenuation = 0.0;\n\
  }\n\
  ambient += lights[i].ambient * attenuation;\n\
}\n\
void main()\n\
{\n\
  vec3 n = normalize(normal);\n\
  vec3 V = normalize(cameraPosition - vertex);\n\
  vec4 ambient = vec4(0.0);\n\
  vec4 diffuse = vec4(0.0);\n\
  vec4 specular = vec4(0.0);\n\
  for (int i = 0; i < numLights; i++)\n\
  {\n\
    if (lights[i].enabled != 1)\n\
      continue;\n\
    if (lights[i].position.w == 0.0)\n\
      directionalLight(i, n, vertex, V, ambient, diffuse, specular);\n\
    else if (lights[i].spotlight == 1)\n\
      spotLight(i, n, vertex, V, ambient, diffuse, specular);\n\
    else\n\
      pointLight(i, n, vertex, V, ambient, diffuse, specular);\n\
  }\n\
  vec4 color = materialEmission + globalLightAmbient * materialAmbient +\n\
    (ambient  * materialAmbient) + (diffuse  * materialDiffuse) + (specular * materialSpecular);\n\
  if (useTexture == 1)\n\
  {\n\
    // If a texture is bound, get its texel and modulate lighting and texture color\n\
    vec4 texel = texture2D(texImage, texPos);\n\
    color = vec4(color.rgb * texel.rgb, color.a * texel.a);\n\
  }\n\
  fragColor = clamp(color, 0.0, 1.0);\n\
}";

      
// Simple logging function
void logmsg(const char *message, ...) {
   // Open file if not already opened
   static FILE *lfile = NULL;
   if (lfile == NULL)
      lfile = fopen("PolygonMesh.log", "w");

   va_list arg;
   va_start(arg, message);
   vfprintf(lfile, message, arg);
   putc('\n', lfile);
   fflush(lfile);
   va_end(arg);
}

// Update spotlight given camera position and orientation
void UpdateSpotlight() {
  // Update spotlight/miners light
   Point3 pos = MyCamera->GetPosition();
   MinersLight->SetPosition(HPoint3(pos.x, pos.y, pos.z, 1.0f));
   Vector3 dir = MyCamera->GetViewPlaneNormal() * -1.0f;
   MinersLight->SetSpotlight(dir, 64.0f, 45.0f);
}

/**
 * Updates the view given the mouse position and whether to move 
 * forard or backward
 */
void UpdateView(const int x, const int y, bool forward) {
   const float step = 1.0;
   float dx = 4.0f * ((x - (static_cast<float>(RenderWidth) * 0.5f))  / 
          static_cast<float>(RenderWidth));
   float dy = 4.0f * (((static_cast<float>(RenderHeight) * 0.5f) - y) / 
          static_cast<float>(RenderHeight));
   float dz = (forward) ? step : -step;
   MyCamera->MoveAndTurn(dx * step, dy * step, dz);
   UpdateSpotlight();
}

/**
 * Method to update the moving light or update the camera position.
 */
void AnimateView(int val) {
	bool redisplay = false;
	if (CurrentLight == MOVING_LIGHT) {
    WorldLightPosition = LightTransform * WorldLightPosition;
    WorldLight->SetPosition(WorldLightPosition);
    LightRotation += 5;
    if (LightRotation > 360)
      LightRotation = 0;
    redisplay = true;
  }

	// If mouse button is down, generate another view
  if (Animate) {
    UpdateView(MouseX, MouseY, Forward);
    redisplay = true;
  }
 
  if (redisplay) {
    // Call glutPostRedisplay to force a display refresh
    glutPostRedisplay();

    // Reinitialize the timer if we are animating
    TimerActive = true;
    glutTimerFunc(50, AnimateView, 0);
  }
  else 
    TimerActive = false;
}

SceneNode* ConstructTerrain(TexturedUnitSquareSurface* textured_square)
{
	// Contruct transform nodes for the walls. Perform rotations so the 
	// walls face inwards
	TransformNode* floor_transform = new TransformNode;
	floor_transform->Scale(200.0f, 200.0f, 1.0f);

	// Use a texture for the floor
	PresentationNode* floor_material = new PresentationNode(Color4(0.15f, 0.15f, 0.15f),
		Color4(0.4f, 0.4f, 0.4f), Color4(0.2f, 0.2f, 0.2f), Color4(0.0f, 0.0f, 0.0f), 5.0f);
	floor_material->SetTexture("floor_tiles.jpg", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

	SceneNode* floor = new SceneNode;
	floor->AddChild(floor_material);
	floor_material->AddChild(floor_transform);
	floor_transform->AddChild(textured_square);

	return floor;
}

/**
 * Construct scene including camera, lights, geometry nodes
 */
void ConstructScene() {
  // Construct the lighting shader node
  LightingShaderNode* lightingShader = new LightingShaderNode();
  if (!lightingShader->CreateFromSource(vertex_shader, fragment_shader) ||
      !lightingShader->GetLocations())
    exit(-1);

  int position_loc = lightingShader->GetPositionLoc();
  int normal_loc   = lightingShader->GetNormalLoc();
  int texture_loc  = lightingShader->GetTextureLoc();

  // Initialize the view and set a perspective projection
  MyCamera = new CameraNode;
  MyCamera->SetPosition(Point3(0.0f, -100.0f, 0.0f));
  MyCamera->SetLookAtPt(Point3(0.0f, 0.0f, 0.0f));
  MyCamera->SetViewUp(Vector3(0.0, 0.0, 1.0));
  MyCamera->SetPerspective(50.0, 1.0, 1.0, 300.0);

  // Set world light default position and a light rotation matrix
  WorldLightPosition = { 50.0f, -50.0f, 50.f, 1.0f };
  LightTransform.Rotate(2.0f, 0.0f, 0.0f, 1.0f);

  // Set a white light - use for fixed and moving light
  // Positional light at 45 degree angle to the upper right 
  // front corner
  WorldLight = new LightNode(0);
  WorldLight->SetDiffuse(Color4(1.0f, 1.0f, 1.0f, 1.0f ));
  WorldLight->SetSpecular(Color4(1.0f, 1.0f, 1.0f, 1.0f ));
  WorldLight->SetPosition(WorldLightPosition);
  WorldLight->Enable();

  // Light 1 - Miner's light - use a yellow spotlight
  MinersLight = new LightNode(1);
  MinersLight->SetDiffuse(Color4(1.0f, 1.0f, 0.0f, 1.0f));
  MinersLight->SetSpecular(Color4(1.0f, 1.0f, 0.0f, 1.0f));
  Point3 pos = MyCamera->GetPosition();
  MinersLight->SetPosition(HPoint3(pos.x, pos.y, pos.z, 1.0f));
  Vector3 dir = MyCamera->GetViewPlaneNormal() * -1.0f;
  MinersLight->SetSpotlight(dir, 64.0f, 45.0f);
  MinersLight->Disable();

  TexturedUnitSquareSurface* textured_square = new TexturedUnitSquareSurface(2, 8, position_loc,
	  normal_loc, texture_loc);

  SceneNode *floor = ConstructTerrain(textured_square);

  // Construct scene graph
  SceneRoot = new SceneNode;
  SceneRoot->AddChild(lightingShader);
  lightingShader->AddChild(MyCamera);
  MyCamera->AddChild(WorldLight);
  WorldLight->AddChild(MinersLight);
  MinersLight->AddChild(floor);
}

/**
 * Display callback function
 */
void display() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Draw the scene graph
  SceneState MySceneState;
  MySceneState.Init();
  SceneRoot->Draw(MySceneState);

  // Swap buffers
  glutSwapBuffers();
}

/**
 * Keyboard callback.
 */
void keyboard(unsigned char key, int x, int y) {
   switch (key) {
    // Escape key - exit program
    case 27:   exit(0);   break;

    // Roll the camera by 5 degrees (no need to update spotlight)
    case 'r':  MyCamera->Roll(5);  break;

    // Roll the camera by 5 degrees (clockwise) (no need to update spotlight)
    case 'R':  MyCamera->Roll(-5);  break;

    // Change the pitch of the camera by 5 degrees
    case 'p':  
       MyCamera->Pitch(5);  
       UpdateSpotlight();
       break;

    // Change the pitch of the camera by 5 degrees (clockwise)
    case 'P': 
       MyCamera->Pitch(-5); 
       UpdateSpotlight();
       break;

    // Change the heading of the camera by 5 degrees
    case 'h':  
       MyCamera->Heading(5);
       UpdateSpotlight();
       break;

    // Change the heading of the camera by 5 degrees (clockwise)
    case 'H':  
       MyCamera->Heading(-5);
       UpdateSpotlight();
       break;

    // Reset the view
    case 'i':
      MyCamera->SetPosition(Point3(0.0f, -100.0f, 0.0f));
      MyCamera->SetLookAtPt(Point3(0.0f, 0.0f, 0.0f));
      MyCamera->SetViewUp(Vector3(0.0, 0.0, 1.0));
      UpdateSpotlight();
      break;

    // Change to fixed world light
    case 'w':
      CurrentLight = FIXED_WORLD;
      WorldLight->Enable();
      WorldLightPosition = { 50.0f, -50.0f, 50.f, 1.0f };
      WorldLight->SetPosition(WorldLightPosition);
      MinersLight->Disable();
      break;

    // Moving light
    case 'm':
      CurrentLight = MOVING_LIGHT;
      LightRotation = 0;
      WorldLight->Enable();
      WorldLightPosition = { 50.0f, -50.0f, 50.f, 1.0f };
      WorldLight->SetPosition(WorldLightPosition);
      MinersLight->Disable();

      // Want to start the light moving
      if (!TimerActive)
         glutTimerFunc(10, AnimateView, 0);
      break;

    // Light at view direction
    case 'v':
      CurrentLight = MINERS_LIGHT;
      MinersLight->Enable();
      WorldLight->Disable();
      break;

    case 'b':
      MyCamera->SetPosition(Point3(0.0f, 100.0f, 0.0f));
      MyCamera->SetLookAtPt(Point3(0.0f, 0.0f, 0.0f));
      MyCamera->SetViewUp(Vector3(0.0, 0.0, 1.0));
      UpdateSpotlight();
      break;

    default:
       break;
   }
   glutPostRedisplay();
}

/**
 * Mouse callback (called when a mouse button state changes)
 */
void mouse(int button, int state, int x, int y) {
  // On a left button up event MoveAndTurn the view with forward motion
  if (button == GLUT_LEFT_BUTTON) {
    if (state == GLUT_DOWN) {
      MouseX = x;
      MouseY = y;
      Forward = true;
      Animate = true;
      UpdateView(x, y, true);
      if (!TimerActive)
        glutTimerFunc(10, AnimateView, 0);  
    }
    else {
      Animate = false;
    }
  }

  // On a right button up event MoveAndTurn the view with reverse motion
  if (button == GLUT_RIGHT_BUTTON) {
    if (state == GLUT_DOWN) { 
      MouseX = x;
      MouseY = y;
      Forward = false;
      Animate = true;
      UpdateView(x, y, true);
      if (!TimerActive)
        glutTimerFunc(10, AnimateView, 0);
    }
    else {
      Animate = false;
    }
  }
}

/**
 * Mouse motion callback (called when mouse button is depressed)
 */
void mouseMotion(int x, int y) {
   // Update position used for changing the view and force a new view
   MouseX = x;
   MouseY = y;
}

/**
 * Reshape callback.  Reset the perpective projection so the field of
 * view matches the window's aspect ratio.
 */
void reshape(int width, int height) {
  RenderWidth = width;
  RenderHeight = height;

  // Reset the viewport
  glViewport(0, 0, width, height);

  // Reset the perspective projection to reflect the change of the aspect ratio 
  MyCamera->ChangeAspectRatio(static_cast<float>(width) / static_cast<float>(height));
}

/**
 * Main 
 */
int main(int argc, char** argv) {
  // Print options
  std::cout << "Transforms:" << std::endl;
  std::cout << "x,X - rotate object about x axis" << std::endl;
  std::cout << "z,Z - rotate object about z axis" << std::endl;
  std::cout << "r,R - Change camera roll" << std::endl;
  std::cout << "p,P - Change camera pitch" << std::endl;
  std::cout << "h,H - Change camera heading" << std::endl;
  std::cout << "b   - View back of object"   << std::endl;
  std::cout << "i   - Initialize view"   << std::endl << std::endl;
  std::cout << "Lighting:" << std::endl;
  std::cout << "m - Moving   w - Fixed world position  v - Miner's helmet" << std::endl;

  // Initialize free GLUT
  glutInit(&argc, argv);
  glutInitContextVersion(3, 2);
  glutInitContextProfile(GLUT_CORE_PROFILE);

  // Set up depth buffer and double buffering
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);

  glutInitWindowPosition(100, 100);
  glutInitWindowSize(640, 480);
  glutCreateWindow("Josh and Jennifer's Final Project");
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);
  glutMotionFunc(mouseMotion);
  glutKeyboardFunc(keyboard);

  // Initialize Open 3.2 core profile
  if (gl3wInit()) {
    fprintf(stderr, "gl3wInit: failed to initialize OpenGL\n");
    return -1;
  }
  if (!gl3wIsSupported(3, 2)) {
    fprintf(stderr, "OpenGL 3.2 not supported\n");
    return -1;
  }
  printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

  // Initialize DevIL
  ilInit();

  // Set the clear color to white
  glClearColor(0.3f, 0.3f, 0.5f, 0.0f);

  // Enable the depth buffer
  glEnable(GL_DEPTH_TEST);

  // Enable back face polygon removal
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);

  // Enable multisample anti-aliasing
  glEnable(GL_MULTISAMPLE);

  // Construct the scene.
  ConstructScene();

  glutMainLoop();
  return 0;
}

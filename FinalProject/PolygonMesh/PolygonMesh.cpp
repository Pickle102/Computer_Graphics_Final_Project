//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:  David W. Nesbitt
//	File:    PolygonMesh.cpp
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
TransformNode* ObjectRotation;            // Transform node for rotating the objects
LightNode* MinersLight;                   // View dependent light
LightNode* WorldLight;                    // World coordiante light (fixed or moving)
std::vector<SceneNode*> MaterialList;     // Material Nodes
SelectorNode* MaterialSelector;

// Set of defined object types
std::vector<SceneNode*> GeometryList;
SelectorNode* GeometrySelector;

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

void EnableTextureFiltering() {
  // Turn on filtering for each texture material
  for (uint32_t i = 0; i < MaterialList.size(); i++)
    ((PresentationNode*)(MaterialList[i]))->UpdateTextureFilters(GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
}

void DisableTextureFiltering() {
  // Turn on filtering for each texture material
  for (uint32_t i = 0; i < MaterialList.size(); i++)
    ((PresentationNode*)(MaterialList[i]))->UpdateTextureFilters(GL_NEAREST, GL_NEAREST);
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

/**
 * Build the material list.
 */
void BuildMaterials() {
  // Material values taken from FS Hill. Note that ambient reflection is 
  // lower than diffuse. 

  // Material 0: Black plastic
  PresentationNode* blackPlastic = new PresentationNode();
  blackPlastic->SetMaterialAmbient(Color4(0.0f, 0.0f, 0.0f));
  blackPlastic->SetMaterialDiffuse(Color4(0.01f, 0.01f, 0.01f));
  blackPlastic->SetMaterialSpecular(Color4(0.5f, 0.5f, 0.5f));
  blackPlastic->SetMaterialShininess(32.0f);
  MaterialList.push_back(blackPlastic);

  // Material 1: Brass
  PresentationNode* brass = new PresentationNode();
  brass->SetMaterialAmbient(Color4(0.329412f, 0.223529f, 0.027451f));
  brass->SetMaterialDiffuse(Color4(0.780392f, 0.568627f, 0.113725f));
  brass->SetMaterialSpecular(Color4(0.992157f, 0.941176f, 0.807843f));
  brass->SetMaterialShininess(27.8974f);
  MaterialList.push_back(brass);

  // Material 2: Bronze
  PresentationNode* bronze = new PresentationNode();
  bronze->SetMaterialAmbient(Color4(0.2125f, 0.1275f, 0.054f));
  bronze->SetMaterialDiffuse(Color4(0.714f, 0.4284f, 0.18144f));
  bronze->SetMaterialSpecular(Color4(0.393548f, 0.271906f, 0.166721f));
  bronze->SetMaterialShininess(25.6f);
  MaterialList.push_back(bronze);

  // Material 3: Chrome
  PresentationNode* chrome = new PresentationNode();
  chrome->SetMaterialAmbient(Color4(0.25f, 0.25f, 0.25f));
  chrome->SetMaterialDiffuse(Color4(0.4f, 0.4f, 0.4f));
  chrome->SetMaterialSpecular(Color4(0.774597f, 0.774597f, 0.774597f));
  chrome->SetMaterialShininess(76.8f);
  MaterialList.push_back(chrome);

  // Material 4: Copper
  PresentationNode* copper = new PresentationNode();
  copper->SetMaterialAmbient(Color4(0.19125f, 0.0735f, 0.0225f));
  copper->SetMaterialDiffuse(Color4(0.7038f, 0.27048f, 0.0828f));
  copper->SetMaterialSpecular(Color4(0.256777f, 0.137622f, 0.086014f));
  copper->SetMaterialShininess(12.8f);
  MaterialList.push_back(copper);

  // Material 5: Gold
  PresentationNode* gold = new PresentationNode();
  gold->SetMaterialAmbient(Color4(0.24725f, 0.1995f, 0.0745f));
  gold->SetMaterialDiffuse(Color4(0.75164f, 0.60648f, 0.22648f));
  gold->SetMaterialSpecular(Color4(0.628281f, 0.555802f, 0.366065f));
  gold->SetMaterialShininess(51.2f);
  MaterialList.push_back(gold);

  // Material 6: Pewter
  PresentationNode* pewter = new PresentationNode();
  pewter->SetMaterialAmbient(Color4(0.10588f, 0.058824f, 0.113725f));
  pewter->SetMaterialDiffuse(Color4(0.427451f, 0.470588f, 0.541176f));
  pewter->SetMaterialSpecular(Color4(0.3333f, 0.3333f, 0.521569f));
  pewter->SetMaterialShininess(9.84615f);
  MaterialList.push_back(pewter);

  // Material 7: Silver
  PresentationNode* silver = new PresentationNode();
  silver->SetMaterialAmbient(Color4(0.19225f, 0.19225f, 0.19225f));
  silver->SetMaterialDiffuse(Color4(0.50754f, 0.50754f, 0.50754f));
  silver->SetMaterialSpecular(Color4(0.508273f, 0.508273f, 0.508273f));
  silver->SetMaterialShininess(51.2f);
  MaterialList.push_back(silver);

  // Material 8: Polished Silver
  PresentationNode* polishedSilver = new PresentationNode();
  polishedSilver->SetMaterialAmbient(Color4(0.23125f, 0.23125f, 0.23125f));
  polishedSilver->SetMaterialDiffuse(Color4(0.2775f, 0.2775f, 0.2775f));
  polishedSilver->SetMaterialSpecular(Color4(0.773911f, 0.773911f, 0.773911f));
  polishedSilver->SetMaterialShininess(89.6f);
  MaterialList.push_back(polishedSilver);

  // Material 9: earth texture
  PresentationNode* earthTexture = new PresentationNode();
  earthTexture->SetMaterialAmbient(Color4(0.2f, 0.2f, 0.2f));
  earthTexture->SetMaterialDiffuse(Color4(0.5f, 0.5f, 0.5f));
  earthTexture->SetMaterialSpecular(Color4(0.6f, 0.6f, 0.6f));
  earthTexture->SetMaterialShininess(50);
  earthTexture->SetTexture("earthp2.jpg", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, 
            GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
  MaterialList.push_back(earthTexture);

  // Material 10 ('-'): coke texture
  PresentationNode* cokeTexture = new PresentationNode();
  cokeTexture->SetMaterialAmbient(Color4(0.2f, 0.2f, 0.2f));
  cokeTexture->SetMaterialDiffuse(Color4(0.5f, 0.5f, 0.5f));
  cokeTexture->SetMaterialSpecular(Color4(0.6f, 0.6f, 0.6f));
  cokeTexture->SetMaterialShininess(50);
  cokeTexture->SetTexture("cokecan.jpg", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, 
            GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
  MaterialList.push_back(cokeTexture);

  // Material 11 ('-'): grainy wood texture
  PresentationNode* woodTexture = new PresentationNode();
  woodTexture->SetMaterialAmbient(Color4(0.2f, 0.2f, 0.2f));
  woodTexture->SetMaterialDiffuse(Color4(0.5f, 0.5f, 0.5f));
  woodTexture->SetMaterialSpecular(Color4(0.6f, 0.6f, 0.6f));
  woodTexture->SetMaterialShininess(50);
  woodTexture->SetTexture("grainy_wood.jpg", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
            GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);
  MaterialList.push_back(woodTexture);

  MaterialSelector = new SelectorNode(MaterialList);
  MaterialSelector->SetCurrent(5);
}

/**
 * Build geometry list.
 */
void BuildGeometry(const int position_loc, const int normal_loc, const int texture_loc) {
  // Create a unit circle as a TriSurface (for caps on conic surfaces)
  std::vector<Point3> vlist;
  vlist.push_back(Point3(0.0f, 0.0f, 0.0f));  // Center
  float theta = 0.0f;
  float dtheta = (2.0f * kPi) / 36.0f;
  for (uint32_t i = 0; i <= 36; i++, theta += dtheta)
	  vlist.push_back(Point3(cosf(theta), sinf(theta), 0.0f));
  TriSurface* unitCircle = new TriSurface;
  unitCircle->AddPolygon(vlist);
  unitCircle->CreateVertexBuffers(position_loc, normal_loc);

  // Cylinder with texture coordinates (include scaling)
  TransformNode* cylinderTransform = new TransformNode;
  cylinderTransform->Scale(20.0f, 20.0f, 60.0f);
  TexturedConicSurface* cylinder = new TexturedConicSurface(1.0f, 1.0f, 36, 10, 
              position_loc, normal_loc, texture_loc);
  cylinderTransform->AddChild(cylinder);
  TransformNode* bottomCap = new TransformNode;
  bottomCap->Translate(0.0f, 0.0f, -0.5f);
  bottomCap->RotateX(180.0f);
  bottomCap->AddChild(unitCircle);
  cylinderTransform->AddChild(bottomCap);
  TransformNode* topCap = new TransformNode;
  topCap->Translate(0.0f, 0.0f, 0.5f);
  topCap->AddChild(unitCircle);
  cylinderTransform->AddChild(topCap);
  GeometryList.push_back(cylinderTransform);

  // Cone (include scaling)
  TransformNode* coneTransform = new TransformNode;
  coneTransform->Scale(20.0f, 20.0f, 40.0f);
  TexturedConicSurface* cone = new TexturedConicSurface(1.0f, 0.0f, 36, 10,
              position_loc, normal_loc, texture_loc);
  coneTransform->AddChild(cone);
  TransformNode* coneBottomCap = new TransformNode;
  coneBottomCap->Translate(0.0f, 0.0f, -0.5f);
  coneBottomCap->RotateX(180.0f);
  coneBottomCap->AddChild(unitCircle);
  coneTransform->AddChild(coneBottomCap);
  GeometryList.push_back(coneTransform);

  // Truncated cone (include scaling)
  TransformNode* cone2Transform = new TransformNode;
  cone2Transform->Scale(20.0f, 20.0f, 40.0f);
  TexturedConicSurface* truncatedCone = new TexturedConicSurface(1.0f, 0.5f, 36, 10,
              position_loc, normal_loc, texture_loc);
  cone2Transform->AddChild(truncatedCone);
  TransformNode* cone2BottomCap = new TransformNode;
  cone2BottomCap->Translate(0.0f, 0.0f, -0.5f);
  cone2BottomCap->RotateX(180.0f);
  cone2BottomCap->AddChild(unitCircle);
  cone2Transform->AddChild(cone2BottomCap);
  TransformNode* cone2TopCap = new TransformNode;
  cone2TopCap->Translate(0.0f, 0.0f, 0.5f);
  cone2TopCap->Scale(0.5f, 0.5f, 1.0f);
  cone2TopCap->AddChild(unitCircle);
  cone2Transform->AddChild(cone2TopCap);
  GeometryList.push_back(cone2Transform);

  // Torus (no object scaling needed)
  TexturedTorusSurface* torus = new TexturedTorusSurface(30.0f, 10.0f, 36, 18,
              position_loc, normal_loc, texture_loc);
  GeometryList.push_back(torus);

  // Cube
  SceneNode* cube = ConstructCube(4, position_loc, normal_loc, texture_loc);
  TransformNode* cubeTransform = new TransformNode;
  cubeTransform->Scale(40.0f, 40.0f, 40.0f);
  cubeTransform->AddChild(cube);
  GeometryList.push_back(cubeTransform);

  // Icosahedron
  SceneNode* icosahedron = ConstructIcosahedron(position_loc, normal_loc);
  TransformNode* icoTransform = new TransformNode;
  icoTransform->Scale(30.0f, 30.0f, 30.0f);
  icoTransform->AddChild(icosahedron);
  GeometryList.push_back(icoTransform);

  // Octahedron
  SceneNode* octahedron = ConstructOctahedron(position_loc, normal_loc);
  TransformNode* octaTransform = new TransformNode;
  octaTransform->Scale(40.0f, 40.0f, 40.0f);
  octaTransform->AddChild(octahedron);
  GeometryList.push_back(octaTransform);

  // Tetrahedron
  SceneNode* tetrahedron = ConstructTetrahedron(position_loc, normal_loc);
  TransformNode* tetraTransform = new TransformNode;
  tetraTransform->Scale(20.0f, 20.0f, 20.0f);
  tetraTransform->AddChild(tetrahedron);
  GeometryList.push_back(tetraTransform);

  // Dodecahedron
  SceneNode* dodecahedron = ConstructDodecahedron(position_loc, normal_loc);
  TransformNode* dodecaTransform = new TransformNode;
  dodecaTransform->Scale(40.0f, 40.0f, 40.0f);
  dodecaTransform->AddChild(dodecahedron);
  GeometryList.push_back(dodecaTransform);

  // Geodesic dome
  GeodesicDome* dome = new GeodesicDome(position_loc, normal_loc);
  TransformNode* domeTransform = new TransformNode;
  domeTransform->Scale(30.0f, 30.0f, 30.0f);
  domeTransform->AddChild(dome);
  GeometryList.push_back(domeTransform);

  // Bilinear patch
  Point3 p0(-10.0,  0.0,  0.0);
  Point3 p1( 10.0,  25.0,  0.0);
  Point3 p2(-10.0,  25.0, 20.0);
  Point3 p3( 10.0,  0.0, 20.0);
  BilinearPatch* patch = new BilinearPatch(p0, p1, p2, p3, 10, position_loc, normal_loc);
  TransformNode* patchTransform = new TransformNode;
  patchTransform->Scale(2.0f, 2.0f, 2.0f);
  patchTransform->AddChild(patch);
  GeometryList.push_back(patchTransform);

  // Surface of revolution
  std::vector<Point3> v;
  v.push_back(Point3(0.0f, 0.0f, 0.0f));
  v.push_back(Point3(0.8f, 0.0f, 0.0f));
  v.push_back(Point3(0.9f, 0.0f, 0.2f));
  v.push_back(Point3(0.63f, 0.0f, 0.48f));
  v.push_back(Point3(0.77f, 0.0f, 0.53f));
  v.push_back(Point3(0.83f, 0.0f, 0.64f));
  v.push_back(Point3(0.55f, 0.0f, 0.9f));
  v.push_back(Point3(0.3f, 0.0f, 1.6f));
  v.push_back(Point3(0.23f, 0.0f, 2.3f));
  v.push_back(Point3(0.5f, 0.0f, 2.52f));
  v.push_back(Point3(0.4f, 0.0f, 2.62f));
  v.push_back(Point3(0.6f, 0.0f, 2.74f));
  v.push_back(Point3(0.62f, 0.0f, 2.92f));
  v.push_back(Point3(0.51f, 0.0f, 3.06f));
  v.push_back(Point3(0.3f, 0.0f, 3.28f));
  v.push_back(Point3(0.43f, 0.0f, 3.5f));
  v.push_back(Point3(0.43f, 0.0f, 3.72f));
  v.push_back(Point3(0.33f, 0.0f, 3.95f));
  v.push_back(Point3(0.18f, 0.0f, 4.10f));
  v.push_back(Point3(0.0f, 0.0f, 4.15f));
  TexturedSurfaceOfRevolution* rev = new TexturedSurfaceOfRevolution(v, 36, position_loc, 
              normal_loc, texture_loc);
  TransformNode* revTransform = new TransformNode;
  revTransform->Scale(10.0f, 10.0f, 10.0f);
  revTransform->AddChild(rev);
  GeometryList.push_back(revTransform);

  // Buckyball
  SceneNode* buckyball = ConstructBuckyball(position_loc, normal_loc);
  TransformNode* buckyTransform = new TransformNode;
  buckyTransform->Scale(30.0f, 30.0f, 30.0f);
  buckyTransform->AddChild(buckyball);
  GeometryList.push_back(buckyTransform);

  // Subdivided sphere
  UnitSubdividedSphere* sphere2 = new UnitSubdividedSphere(4, position_loc, normal_loc);
  TransformNode* sphere2Transform = new TransformNode;
  sphere2Transform->Scale(30.0f, 30.0f, 30.0f);
  sphere2Transform->AddChild(sphere2);
  GeometryList.push_back(sphere2Transform);

  // Mesh teapot
  MeshTeapot* teapot1 = new MeshTeapot(3, position_loc, normal_loc);
  TransformNode* teapot1Transform = new TransformNode;
  teapot1Transform->Scale(8.0f, 8.0f, 8.0f);
  teapot1Transform->AddChild(teapot1);
  GeometryList.push_back(teapot1Transform);

  // Sphere using stacks and slices. Enable texture coordinates.
  TexturedSphereSection* earthSphere = new TexturedSphereSection(-90.0f, 90.0f, 36,
                -180.0f, 180.0f, 72, 1.0f, position_loc, normal_loc, texture_loc);
  TransformNode* sphereTransform = new TransformNode;
  sphereTransform->Scale(30.0f, 30.0f, 30.0f);
  sphereTransform->AddChild(earthSphere);
  GeometryList.push_back(sphereTransform);

  // Trough
  UnitTrough* trough = new UnitTrough(18, 10, position_loc, normal_loc);
  TransformNode* trough_transform = new TransformNode;
  trough_transform->Scale(10.0f, 10.0f, 50.0f);
  trough_transform->AddChild(trough);
  GeometryList.push_back(trough_transform);

  // A10 Model
  ModelNode* model_node1 = new ModelNode(position_loc, normal_loc,
          texture_loc, "A10/A10.3ds");
  PresentationNode* model_pres1 = new PresentationNode;
  model_pres1->SetMaterialAmbientAndDiffuse(Color4(1.0f, 1.0f, 1.0f));
  model_pres1->SetMaterialSpecular(Color4(0.2f, 0.2f, 0.2f));
  model_pres1->SetMaterialShininess(70.0f);
  TransformNode* model_transform1 = new TransformNode;
  model_transform1->Scale(3.0f, 3.0f, 3.0f);
  model_transform1->AddChild(model_node1);
  model_pres1->AddChild(model_transform1);
  GeometryList.push_back(model_pres1);

  // Bug Model
  ModelNode* model_node2 = new ModelNode(position_loc, normal_loc,
          texture_loc, "bug/bug.3ds");
  PresentationNode* model_pres2 = new PresentationNode;
  model_pres2->SetMaterialAmbientAndDiffuse(Color4(1.0f, 1.0f, 1.0f));
  model_pres2->SetMaterialSpecular(Color4(0.2f, 0.2f, 0.2f));
  model_pres2->SetMaterialShininess(70.0f);
  TransformNode* model_transform2 = new TransformNode;
  model_transform2->Scale(30.0f, 30.0f, 30.0f);
  model_transform2->AddChild(model_node2);
  model_pres2->AddChild(model_transform2);
  GeometryList.push_back(model_pres2);

  // Add geometry list to the selector
  GeometrySelector = new SelectorNode(GeometryList);
  GeometrySelector->SetCurrent(0);
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

  // Construct the object rotation TransformNode
  ObjectRotation = new TransformNode;

  // Build the material list and initialize the material selector
  BuildMaterials();
   
  // Build the geometry list and initialize the material selector
  BuildGeometry(position_loc, normal_loc, texture_loc);

  // Construct scene graph
  SceneRoot = new SceneNode;
  SceneRoot->AddChild(lightingShader);
  lightingShader->AddChild(MyCamera);
  MyCamera->AddChild(WorldLight);
  WorldLight->AddChild(MinersLight);
  MinersLight->AddChild(MaterialSelector);    

  // Set all material nodes to point to the object rotation (TransformNode)
  for (uint32_t i = 0; i < MaterialList.size(); i++)
    MaterialList[i]->AddChild(ObjectRotation);

  ObjectRotation->AddChild(GeometrySelector);
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

    // Digits 0-8 set the current material
    case '0':  MaterialSelector->SetCurrent(0); break;
    case '1':  MaterialSelector->SetCurrent(1); break;
    case '2':  MaterialSelector->SetCurrent(2); break;
    case '3':  MaterialSelector->SetCurrent(3); break;
    case '4':  MaterialSelector->SetCurrent(4); break;
    case '5':  MaterialSelector->SetCurrent(5); break;
    case '6':  MaterialSelector->SetCurrent(6); break;
    case '7':  MaterialSelector->SetCurrent(7); break;
    case '8':  MaterialSelector->SetCurrent(8); break;
    case '9':  MaterialSelector->SetCurrent(9); break;
    case '-':  MaterialSelector->SetCurrent(10); break;
    case '=':  MaterialSelector->SetCurrent(11); break;

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

    // Cube
    case 'C':  GeometrySelector->SetCurrent((int)CUBE);  break;

    // Octahedron
    case 'O':  GeometrySelector->SetCurrent((int)OCTAHEDRON);  break;

    // Icosahedron
    case 'I':  GeometrySelector->SetCurrent((int)ICOSAHEDRON);  break;

    // Icosahedron
    case 'B':  GeometrySelector->SetCurrent((int)BUCKYBALL);  break;

    // Tetrahedron
    case 'Y':  GeometrySelector->SetCurrent((int)TETRAHEDRON);  break;

    // Dodecahedron
    case 'D':  GeometrySelector->SetCurrent((int)DODECAHEDRON);  break;

    // Earth
    case 'E':  GeometrySelector->SetCurrent((int)EARTH);  break;

    // A10 Model
    case 'F':  GeometrySelector->SetCurrent((int)A10);  break;

    // Bug Model
    case 'M':  GeometrySelector->SetCurrent((int)BUG);  break;

    // Trough
    case 'V':  GeometrySelector->SetCurrent((int)TROUGH);  break;

    // Subdivided sphere
    case 'A':  GeometrySelector->SetCurrent((int)SPHERE2);  break;

    // Torus
    case 'T':  GeometrySelector->SetCurrent((int)TORUS);  break;

    // Cylinder
    case 'L':  GeometrySelector->SetCurrent((int)CYLINDER);  break;

    // Cone
    case 'N':  GeometrySelector->SetCurrent((int)CONE);  break;

    // Teapot
    case 'S':  GeometrySelector->SetCurrent((int)TEAPOT);  break;

    // Truncated cone
    case 'U':  GeometrySelector->SetCurrent((int)TRUNCATED_CONE); break;

    // Surface of Revolution
    case 'Q':  GeometrySelector->SetCurrent((int)REV_SURFACE); break;

    // Bilinear Patch
    case 'J':  GeometrySelector->SetCurrent((int)BILINEAR_PATCH); break;

    // Geodesic dome
    case 'G':  GeometrySelector->SetCurrent((int)GEODESIC_DOME);  break;

    // Increase x rotation
    case 'x':  ObjectRotation->RotateX(10.0f);  break;

    // Decrease x rotation
    case 'X':  ObjectRotation->RotateX(-10.0f);  break;

    // Increase Z rotation
    case 'z':  ObjectRotation->RotateZ(10.0f);  break;

    // Decrease Z rotation
    case 'Z':  ObjectRotation->RotateZ(-10.0f);  break;

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

    case 'f':
      EnableTextureFiltering();
      break;

    case 'n':
      DisableTextureFiltering();
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
  std::cout << "Materials:" << std::endl;
  std::cout << "0 - Black Plastic  1 - Brass   2 - Bronze" << std::endl;
  std::cout << "3 - Chrome         4 - Copper  5 - Gold" << std::endl;
  std::cout << "6 - Pewter         7 - Silver  8 - Polished Silver" << std::endl << std::endl;
  std::cout << "9 - Earth texture  - - Coke Texture  =  - Wood Texture" << std::endl;
  std::cout << "f - Filtering      n - Nearest Texel" << std::endl; 
  std::cout << "Objects:" << std::endl;
  std::cout << "C - Cube     G - Geodesic Sphere O - Octahedron" << std::endl;
  std::cout << "E - Earth    B - Buckyball       I - Icosahedron" << std::endl;
  std::cout << "T - Torus    Y - Tetrahedron     D - Dodecahedron" << std::endl;
  std::cout << "L - Cylinder N - Cone            U - Truncated Cylinder" << std::endl;
  std::cout << "S - Teapot   J - Bilinear Patch  Q - Surface of Revolution" << std::endl;
  std::cout << "A - Sphere2  F - A 10 Model      M - Bug Model" << std::endl;
  std::cout << "V - Trough" << std::endl;

  // Initialize free GLUT
  glutInit(&argc, argv);
  glutInitContextVersion(3, 2);
  glutInitContextProfile(GLUT_CORE_PROFILE);

  // Set up depth buffer and double buffering
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);

  glutInitWindowPosition(100, 100);
  glutInitWindowSize(640, 480);
  glutCreateWindow("Polygon Mesh Objects");
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

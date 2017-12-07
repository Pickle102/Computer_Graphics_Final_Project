//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:  Jennifer Olk, Joshua Griffith
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

const float FRAMES_PER_SEC = 60.0f;

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
LightNode* Spotlight;					  // Keep the spotlight global so we can update its poisition

// Creating a starting camera height constant to easily change the height of the 'player'
const float startingCameraHeight = 5.0f;

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

/**
* Convenience method to add a material, then a transform, then a
* geometry node as a child to a specified parent node.
* @param  parent    Parent scene node.
* @param  material  Presentation node.
* @param  transform Transformation node.
* @param  geometry  Geometry node.
*/
void AddSubTree(SceneNode* parent, SceneNode* material,
	SceneNode* transform, SceneNode* geometry) {
	parent->AddChild(material);
	material->AddChild(transform);
	transform->AddChild(geometry);
}

// Update the spotlight based on camera position change
void UpdateSpotlight() {
	Point3 pos = MyCamera->GetPosition();
	Spotlight->SetPosition(HPoint3(pos.x, pos.y, pos.z, 1.0f));
	Vector3 dir = MyCamera->GetViewPlaneNormal() * -1.0f;
	Spotlight->SetSpotlightDirection(dir);
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

/*
 * In this project, this function is required to animate particle systems by calling into update
 */
void AnimateParticles(int value) 
{
	// Update the scene graph
	SceneState scene_state;
	SceneRoot->Update(scene_state);

	// Set update to specified frames per second
	glutTimerFunc((int)(1000.0f / FRAMES_PER_SEC), AnimateParticles, 0);

	glutPostRedisplay();
}

/**
* Construct the terrain for the scene.
*/
SceneNode* ConstructTrees(TexturedUnitSquareSurface* tree_square)
{
    // Number of trees to generate
    const int NUM_TREES = 2000;

    // (x,y) location constraints for trees
    const int MAX_Y = 2000;
    const int MIN_Y = -2000;
    const int MAX_X = 2000;
    const int MIN_X = -2000;

    // The radius around (0,0) to avoid putting trees
    const int restrictedRadius = 40;

    // Random tree size constraints
    const int MIN_WIDTH = 15;
    const int MAX_WIDTH = 30;
    const int MIN_HEIGHT = 15;
    const int MAX_HEIGHT = 30;

    // Construct the trees
    SceneNode* trees = new SceneNode;

    // Create a tree material with a picture
    PresentationNode* tree_material = new PresentationNode(
        Color4(0.5f, 0.5f, 0.5f), Color4(0.1f, 0.1f, 0.1f),
        Color4(0.1f, 0.1f, 0.1f), Color4(0.0f, 0.0f, 0.0f), 5.0f);
    tree_material->SetTexture("tree5.png", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
        GL_NEAREST, GL_NEAREST);
    tree_material->CreateBillboard();

    // Add the material as the parent of the trees
    trees->AddChild(tree_material);

    // Create NUM_TREES trees
    int randomX = 0;
    int randomY = 0;
    int randomHeight = 0;
    int randomWidth = 0;
    TransformNode* treeFront_transform;
    for (int treeNum = 0; treeNum < NUM_TREES; ++treeNum)
    {
        // Create random x,y
        randomX = rand() % (MAX_X - MIN_X + 1) + MIN_X;
        randomY = rand() % (MAX_Y - MIN_Y + 1) + MIN_Y;

        // Create random width,height
        randomWidth = rand() % (MAX_WIDTH - MIN_WIDTH + 1) + MIN_WIDTH;
        randomHeight = rand() % (MAX_HEIGHT - MIN_HEIGHT + 1) + MIN_HEIGHT;

        // Make sure it's not within the radius around (0,0)
        if ((randomX >= -restrictedRadius) && (randomX <= restrictedRadius) &&
            (randomY >= -restrictedRadius) && (randomY <= restrictedRadius))
        {
            treeNum--;
            continue;
        }

        // Create a new transform (otherwise we'll translate again)
        treeFront_transform = new TransformNode();
        treeFront_transform->Translate(randomX, randomY, randomHeight * 0.4);
        treeFront_transform->RotateX(90.0f);
        treeFront_transform->Scale(randomWidth, randomHeight, 1.0f);

        // Add this tree to the tree material
        tree_material->AddChild(treeFront_transform);
        treeFront_transform->AddChild(tree_square);
    }

    return trees;
}

/**
* Construct lighting for this scene.
* @param  lighting  Pointer to the lighting shader node.
*/
void ConstructLighting(LightingShaderNode* lighting) {
	// Set the global light ambient
	Color4 globalAmbient(0.4f, 0.4f, 0.4f, 1.0f);
	lighting->SetGlobalAmbient(globalAmbient);

	lighting->EnableFog(Color4(0.25f, 0.25f, 0.25f, 1.0f));

	// Light 0 - point light source for fire
	LightNode* light0 = new LightNode(0);
	light0->SetDiffuse(Color4(5.0f, 5.0f, 0.0f, 1.0f));
	light0->SetSpecular(Color4(0.4f, 0.4f, 0.0f, 1.0f));
	light0->SetPosition(HPoint3(0.0f, 0.0f, 3.5f, 1.0f));
	light0->Enable();

	// Light1 - directional light from the ceiling
	LightNode* light1 = new LightNode(1);
	light1->SetDiffuse(Color4(0.4f, 0.4f, 0.4f, 1.0f));
	light1->SetSpecular(Color4(0.4f, 0.4f, 0.4f, 1.0f));
	light1->SetPosition(HPoint3(0.0f, 1.0f, 0.5f, 0.0f));
	light1->Enable();

	// Spotlight - reddish spotlight - we will place at the camera location
	// shining along -VPN
	Spotlight = new LightNode(2);
	Spotlight->SetDiffuse(Color4(0.5f, 0.1f, 0.1f, 1.0f));
	Spotlight->SetSpecular(Color4(0.5f, 0.1f, 0.1f, 1.0f));
	Point3 pos = MyCamera->GetPosition();
	Spotlight->SetPosition(HPoint3(pos.x, pos.y, pos.z, 1.0f));
	Vector3 dir = MyCamera->GetViewPlaneNormal() * -1.0f;
	Spotlight->SetSpotlight(dir, 32.0f, 30.0f);
	Spotlight->Enable();

	// Lights are children of the camera node
	MyCamera->AddChild(light0);
	light0->AddChild(light1);
	light1->AddChild(Spotlight);
}

/**
* Construct ground
* @param  textured_square  Texture to use
* @return Returns a scene node that describes the ground.
*/
SceneNode* ConstructGround(TexturedUnitSquareSurface* textured_square)
{
	// Contruct transform nodes for the ground
	TransformNode* ground_transform = new TransformNode;
	ground_transform->Scale(20000.0f, 20000.0f, 1.0f);

	// Use a texture for the ground
	PresentationNode* ground_material = new PresentationNode(Color4(0.45f, 0.45f, 0.45f),
		Color4(0.4f, 0.4f, 0.4f), Color4(0.2f, 0.2f, 0.2f), Color4(0.0f, 0.0f, 0.0f), 25.0f);
	ground_material->SetTexture("grass_texture_2.png", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

	// Walls. We can group these all under a single presentation node.
	SceneNode* ground = new SceneNode;

	// Add ground to the parent. Use convenience method to add
	// presentation, then transform, then geometry.
	AddSubTree(ground, ground_material, ground_transform, textured_square);

	return ground;
}

/**
* Construct firewood
* @param  box  Geometry node to use for wood
* @return Returns a scene node representing the firewood
*/
SceneNode* ConstructFirewood(SceneNode* box) {
	// Firewood logs / pieces
	TransformNode* piece1 = new TransformNode;
	piece1->Translate(2.5f, 0.0f, 0.0f);
	piece1->RotateY(45.0f);
	piece1->Scale(5.0f, 2.0f, 2.0f);
	TransformNode* piece2 = new TransformNode;
	piece2->Translate(-2.5f, 0.0f, 0.0f);
	piece2->RotateY(-45.0f);
	piece2->Scale(5.0f, 2.0f, 2.0f);
	TransformNode* piece3 = new TransformNode;
	piece3->Translate(0.0f, 2.5f, 0.0f);
	piece3->RotateZ(90.0f);
	piece3->RotateY(45.0f);
	piece3->Scale(5.0f, 2.0f, 2.0f);
	TransformNode* piece4 = new TransformNode;
	piece4->Translate(0.0f, -2.5f, 0.0f);
	piece4->RotateZ(-90.0f);
	piece4->RotateY(45.0f);
	piece4->Scale(5.0f, 2.0f, 2.0f);

	PresentationNode* firewood_material = new PresentationNode(Color4(0.45f, 0.45f, 0.45f),
		Color4(0.4f, 0.4f, 0.4f), Color4(0.2f, 0.2f, 0.2f), Color4(0.0f, 0.0f, 0.0f), 25.0f);
	firewood_material->SetTexture("firewood.jpg", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

	// Create the tree
	SceneNode* firewood = new SceneNode;
	firewood->AddChild(firewood_material);
	firewood_material->AddChild(piece1);
	firewood_material->AddChild(piece2);
	firewood_material->AddChild(piece3);
	firewood_material->AddChild(piece4);
	piece1->AddChild(box);
	piece2->AddChild(box);
	piece3->AddChild(box);
	piece4->AddChild(box);

	return firewood;
}

/**
* Construct a unit tent with outward facing normals.
* @param  textured_triangle  Geometry node to use for front/back
* @param  textured_square  Geometry node to use for sides/bottom
*/
SceneNode* ConstructUnitTent(TexturedUnitTriangleSurface* textured_triangle, TexturedUnitSquareSurface* textured_square) {
	// Contruct transform nodes for the tent faces
	TransformNode* bottom_transform = new TransformNode;
	bottom_transform->Translate(0.0f, 0.0f, -0.5f);
	bottom_transform->RotateX(180.0f);

	TransformNode* back_transform = new TransformNode;
	back_transform->Translate(0.0f, 0.5f, 0.0f);
	back_transform->RotateX(-90.0f);
	back_transform->RotateZ(180.0f);

	TransformNode* front_transform = new TransformNode;
	front_transform->Translate(0.0f, -0.5f, 0.0f);
	front_transform->RotateX(90.0f);

	TransformNode* left_transform = new TransformNode;
	left_transform->Translate(-0.25f, 0.0f, 00.0f);
	left_transform->RotateY(-63.5f);
	left_transform->Scale(1.1f, 1.0f, 1.0f);

	TransformNode* right_transform = new TransformNode;
	right_transform->Translate(0.25f, 0.0f, 0.0f);
	right_transform->RotateY(63.5f);
	right_transform->Scale(1.1f, 1.0f, 1.0f);

	PresentationNode* tent_material = new PresentationNode(Color4(0.25f, 0.25f, 0.25f),
		Color4(0.2f, 0.2f, 0.2f), Color4(0.2f, 0.2f, 0.2f), Color4(0.0f, 0.0f, 0.0f), 50.0f);
	tent_material->SetTexture("redtent.jpg", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

	// Create a SceneNode and add the tent faces
	SceneNode* tent = new SceneNode;
	tent->AddChild(tent_material);
	tent_material->AddChild(back_transform);
	back_transform->AddChild(textured_triangle);
	tent_material->AddChild(left_transform);
	left_transform->AddChild(textured_square);
	tent_material->AddChild(right_transform);
	right_transform->AddChild(textured_square);
	tent_material->AddChild(front_transform);
	front_transform->AddChild(textured_triangle);
	tent_material->AddChild(bottom_transform);
	bottom_transform->AddChild(textured_square);

	return tent;
}

/**
* Construct a unit box with outward facing normals.
* @param  textured_square  Geometry node to use
*/
SceneNode* ConstructTexturedUnitBox(TexturedUnitSquareSurface* textured_square) {
	// Contruct transform nodes for the sides of the box.
	// Perform rotations so the sides face outwards

	// Bottom is rotated 180 degrees so it faces outwards
	TransformNode* bottom_transform = new TransformNode;
	bottom_transform->Translate(0.0f, 0.0f, -0.5f);
	bottom_transform->RotateX(180.0f);

	// Back is rotated -90 degrees about x: (z -> y)
	TransformNode* back_transform = new TransformNode;
	back_transform->Translate(0.0f, 0.5f, 0.0f);
	back_transform->RotateX(-90.0f);

	// Front wall is rotated 90 degrees about x: (y -> z)
	TransformNode* front_transform = new TransformNode;
	front_transform->Translate(0.0f, -0.5f, 0.0f);
	front_transform->RotateX(90.0f);

	// Left wall is rotated -90 about y: (z -> -x)
	TransformNode* left_transform = new TransformNode;
	left_transform->Translate(-0.5f, 0.0f, 00.0f);
	left_transform->RotateY(-90.0f);

	// Right wall is rotated 90 degrees about y: (z -> x)
	TransformNode* right_transform = new TransformNode;
	right_transform->Translate(0.5f, 0.0f, 0.0f);
	right_transform->RotateY(90.0f);

	// Top 
	TransformNode* top_transform = new TransformNode;
	top_transform->Translate(0.0f, 0.0f, 0.50f);

	// Create a SceneNode and add the 6 sides of the box.
	SceneNode* box = new SceneNode;
	box->AddChild(back_transform);
	back_transform->AddChild(textured_square);
	box->AddChild(left_transform);
	left_transform->AddChild(textured_square);
	box->AddChild(right_transform);
	right_transform->AddChild(textured_square);
	box->AddChild(front_transform);
	front_transform->AddChild(textured_square);
	box->AddChild(bottom_transform);
	bottom_transform->AddChild(textured_square);
	box->AddChild(top_transform);
	top_transform->AddChild(textured_square);

	return box;
}

/**
* Construct Sky Box
* @param  unit_square  Geometry node to use
* @param  textured_square  Texture to use
* @return Returns a scene node that describes the sky box.
*/
SceneNode* ConstructSkyBox(UnitSquareSurface* unit_square,
	TexturedUnitSquareSurface* textured_square)
{
	// Back wall is rotated +90 degrees about x: (y -> z)
	TransformNode* backwall_transform = new TransformNode;
	backwall_transform->Translate(0.0f, 10000.0f, 0.0f);
	backwall_transform->RotateX(90.0f);
	backwall_transform->Scale(20000.0f, 20000.0f, 1.0f);

	// Front wall is rotated -90 degrees about x: (z -> y)
	TransformNode* frontwall_transform = new TransformNode;
	frontwall_transform->Translate(0.0f, -10000.0f, 0.0f);
	frontwall_transform->RotateX(-90.0f);
	frontwall_transform->RotateZ(-180.0f);
	frontwall_transform->Scale(20000.0f, 20000.0f, 1.0f);

	// Left wall is rotated 90 degrees about y: (z -> x)
	TransformNode* leftwall_transform = new TransformNode;
	leftwall_transform->Translate(-10000.0f, 0.0f, 0.0f);
	leftwall_transform->RotateY(90.0f);
	leftwall_transform->RotateZ(90.0f);
	leftwall_transform->Scale(20000.0f, 20000.0f, 1.0f);

	// Right wall is rotated -90 about y: (z -> -x)
	TransformNode* rightwall_transform = new TransformNode;
	rightwall_transform->Translate(10000.0f, 0.0f, 0.0f);
	rightwall_transform->RotateY(-90.0f);
	rightwall_transform->RotateZ(-90.0f);
	rightwall_transform->Scale(20000.0f, 20000.0f, 1.0f);

	// Ceiling is rotated 180 about x so it faces inwards
	TransformNode* ceiling_transform = new TransformNode;
	ceiling_transform->Translate(0.0f, 0.0f, 10000.0f);
	ceiling_transform->RotateX(180.0f);
	ceiling_transform->RotateZ(0.0f);
	ceiling_transform->Scale(20500.0f, 20000.0f, 1.0f);

	PresentationNode* backwall_material = new PresentationNode(Color4(0.5f, 0.5f, 0.5f),
		Color4(0.0f, 0.0f, 0.0f), Color4(0.0f, 0.0f, 0.0f), Color4(0.3f, 0.3f, 0.3f), 0.0f);
	backwall_material->SetTexture("skybox/back.png", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

	PresentationNode* leftwall_material = new PresentationNode(Color4(0.5f, 0.5f, 0.5f),
		Color4(0.0f, 0.0f, 0.0f), Color4(0.0f, 0.0f, 0.0f), Color4(0.3f, 0.3f, 0.3f), 0.0f);
	leftwall_material->SetTexture("skybox/left.png", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

	PresentationNode* rightwall_material = new PresentationNode(Color4(0.5f, 0.5f, 0.5f),
		Color4(0.0f, 0.0f, 0.0f), Color4(0.0f, 0.0f, 0.0f), Color4(0.3f, 0.3f, 0.3f), 0.0f);
	rightwall_material->SetTexture("skybox/right.png", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

	PresentationNode* frontwall_material = new PresentationNode(Color4(0.5f, 0.5f, 0.5f),
		Color4(0.0f, 0.0f, 0.0f), Color4(0.0f, 0.0f, 0.0f), Color4(0.3f, 0.3f, 0.3f), 0.0f);
	frontwall_material->SetTexture("skybox/front.png", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

	PresentationNode* ceiling_material = new PresentationNode(Color4(0.5f, 0.5f, 0.5f),
		Color4(0.0f, 0.0f, 0.0f), Color4(0.0f, 0.0f, 0.0f), Color4(0.3f, 0.3f, 0.3f), 0.0f);
	ceiling_material->SetTexture("skybox/ceiling.png", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

	// Scene node for skybox.
	SceneNode* skybox = new SceneNode;

	// Add skyboxes faces. Use convenience method to add
	// presentation, then transform, then geometry
	AddSubTree(skybox, backwall_material, backwall_transform, textured_square);
	AddSubTree(skybox, leftwall_material, leftwall_transform, textured_square);
	AddSubTree(skybox, rightwall_material, rightwall_transform, textured_square);
	AddSubTree(skybox, frontwall_material, frontwall_transform, textured_square);
	AddSubTree(skybox, ceiling_material, ceiling_transform, textured_square);

	return skybox;
}

/**
* ConstructFire
* @param  textured_square  Textured geometry node to use
* @return Returns a scene node that describes the fire
*/
SceneNode* ConstructFire(TexturedUnitSquareSurface* textured_square)
{
	SceneNode* fire = new SceneNode();

	// Fire particle transform
	TransformNode* fire_particle_transform = new TransformNode;
	fire_particle_transform->Translate(0.0f, 0.0f, 2.6f);
	fire_particle_transform->Scale(0.5f, 0.5f, 0.5f);

	// Construct fire particle material
	PresentationNode* fire_particle_material = new PresentationNode(Color4(0.8f, 0.8f, 0.0f),
		Color4(0.0f, 0.0f, 0.0f), Color4(0.0f, 0.0f, 0.0f), Color4(1.0f, 1.0f, 0.0f), 0.0f);
	fire_particle_material->SetTexture("fire.jpg", GL_REPEAT, GL_REPEAT, GL_LINEAR_MIPMAP_LINEAR, GL_LINEAR);

	// Construct graph
	fire->AddChild(fire_particle_transform);
	fire_particle_transform->AddChild(fire_particle_material);

	// Construct a particle firebox
	SceneNode* firebox = ConstructTexturedUnitBox(textured_square);

	// Construct about 50 particles
	for (int i = 0; i < 50; i++)
	{
		ParticleNode* fire_particle_effect = new ParticleNode(FRAMES_PER_SEC);
		fire_particle_material->AddChild(fire_particle_effect);
		fire_particle_effect->AddChild(firebox);
	}

	return fire;
}

/**
 * Construct scene including camera, lights, geometry nodes
 */
void ConstructScene() {

  // Construct the lighting shader node
  LightingShaderNode* lightingShader = new LightingShaderNode();
  if (!lightingShader->Create("phong.vert", "phong.frag") ||
	  !lightingShader->GetLocations())
  {
	  exit(-1);
  }

  int position_loc = lightingShader->GetPositionLoc();
  int normal_loc   = lightingShader->GetNormalLoc();
  int texture_loc  = lightingShader->GetTextureLoc();

    // Initialize the view and set a perspective projection
    MyCamera = new CameraNode;
    MyCamera->SetPosition(Point3(0.0f, -100.0f, startingCameraHeight));
    MyCamera->SetLookAtPt(Point3(0.0f, 0.0f, startingCameraHeight));
    MyCamera->SetViewUp(Vector3(0.0, 0.0, 1.0));

  // Scene is outdoor, so set far clipping to very far
  MyCamera->SetPerspective(50.0, 1.0, 1.0, 25000.0);

  // Set world light default position and a light rotation matrix
  WorldLightPosition = { 50.0f, -50.0f, 50.f, 1.0f };
  LightTransform.Rotate(2.0f, 0.0f, 0.0f, 1.0f);

  // Construct scene lighting - make lighting nodes children of the camera node
  ConstructLighting(lightingShader);

  // Construct subdivided square - subdivided 50x in both x and y
  UnitSquareSurface* unit_square = new UnitSquareSurface(50, position_loc, normal_loc);

  // Construct a textured triangle for general use
  TexturedUnitTriangleSurface* textured_generic_triangle = new TexturedUnitTriangleSurface(1, 1, position_loc,
	  normal_loc, texture_loc);

  // Construct a textured square for the skybox
  TexturedUnitSquareSurface* textured_square_skybox = new TexturedUnitSquareSurface(2, 1, position_loc,
	  normal_loc, texture_loc);

  // Construct the skybox as a child of the root node
  SceneNode* skybox = ConstructSkyBox(unit_square, textured_square_skybox);

  // Construct a textured square for the floor
  TexturedUnitSquareSurface* textured_square = new TexturedUnitSquareSurface(2, 200, position_loc,
	  normal_loc, texture_loc);

  // Construct a textured square for general use
  TexturedUnitSquareSurface* textured_generic_square = new TexturedUnitSquareSurface(2, 1, position_loc,
	  normal_loc, texture_loc);

  // Construct a textured square for the trees
  TexturedUnitSquareSurface* tree_textured_square = new TexturedUnitSquareSurface(1, 1, position_loc,
      normal_loc, texture_loc);
		
  // Construct the ground as a child of the root node
  SceneNode* ground = ConstructGround(textured_square);

  // Construct the trees
  SceneNode* trees = ConstructTrees(tree_textured_square);

  // Fire transform
  TransformNode* fire_transform = new TransformNode;

  // Construct firewood
  TransformNode* firewood_transform = new TransformNode;
  firewood_transform->Translate(0.0f, 0.0f, 1.2f);
  firewood_transform->Scale(0.5f, 0.5f, 0.5f);

  SceneNode* firewood = ConstructFirewood(ConstructTexturedUnitBox(textured_generic_square));

  // Construct tent
  TransformNode* tent_transform = new TransformNode;
  tent_transform->Translate(25.0f, 25.0f, 5.0f);
  tent_transform->RotateZ(-80.0f);
  tent_transform->Scale(15.0f, 15.0f, 10.0f);

  SceneNode* tent = ConstructUnitTent(textured_generic_triangle, textured_generic_square);
 
  // Construct the scene layout
  SceneRoot = new SceneNode;
  SceneRoot->AddChild(lightingShader);
  lightingShader->AddChild(MyCamera);

  // Construct a base node for the rest of the scene, it will be a child
  // of the last light node (so entire scene is under influence of all 
  // lights)
  SceneNode* myscene = new SceneNode;
  Spotlight->AddChild(myscene);

  // Changing where the moon is oriented
  TransformNode* skybox_transform = new TransformNode();
  skybox_transform->RotateZ(90.0f);
  skybox_transform->AddChild(skybox);

  // Add the terrain
  myscene->AddChild(skybox_transform);
  myscene->AddChild(ground);
  myscene->AddChild(trees);

  // Add the fire transform
  myscene->AddChild(fire_transform);

  // Add the firewood
  fire_transform->AddChild(firewood_transform);
  firewood_transform->AddChild(firewood);

  // Add the fire
  fire_transform->AddChild(ConstructFire(textured_generic_square));

  // Add the tent
  myscene->AddChild(tent_transform);
  tent_transform->AddChild(tent);
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

        // Slide right
    case 'd':
        MyCamera->Slide(5.0f, 0.0f, 0.0f);
        UpdateSpotlight();
        glutPostRedisplay();
        break;

        // Slide left
    case 'a':
        MyCamera->Slide(-5.0f, 0.0f, 0.0f);
        UpdateSpotlight();
        glutPostRedisplay();
        break;

        // Move forward
    case 'w':
        MyCamera->Slide(0.0f, 0.0f, -5.0f);
        UpdateSpotlight();
        glutPostRedisplay();
        break;

        // Move backward
    case 's':
        MyCamera->Slide(0.0f, 0.0f, 5.0f);
        UpdateSpotlight();
        glutPostRedisplay();
        break;

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
        MyCamera->SetPosition(Point3(0.0f, -100.0f, startingCameraHeight));
        MyCamera->SetLookAtPt(Point3(0.0f, 0.0f, startingCameraHeight));
        MyCamera->SetViewUp(Vector3(0.0, 0.0, 1.0));
        UpdateSpotlight();
        break;

    case 'b':
        MyCamera->SetPosition(Point3(0.0f, 100.0f, startingCameraHeight));
        MyCamera->SetLookAtPt(Point3(0.0f, 0.0f, startingCameraHeight));
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
    std::cout << "Camera transforms:" << std::endl;

    std::cout << "w - Move camera forward" << std::endl;
    std::cout << "a - Slide camera left" << std::endl;
    std::cout << "s - Move camera backwards" << std::endl;
    std::cout << "d - Slide camera right" << std::endl;
    std::cout << "r,R - Change camera roll" << std::endl;
    std::cout << "p,P - Change camera pitch" << std::endl;
    std::cout << "h,H - Change camera heading" << std::endl;
    std::cout << "b   - View back of object" << std::endl;
    std::cout << "i   - Initialize view" << std::endl << std::endl;

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

  // Set update rate and set initial timer callback
  glutTimerFunc((int)(1000.0f / FRAMES_PER_SEC), AnimateParticles, 0);

  glutMainLoop();
  return 0;
}

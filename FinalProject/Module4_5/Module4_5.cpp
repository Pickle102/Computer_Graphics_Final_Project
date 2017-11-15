//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:  David W. Nesbitt
//	File:    Module4_5.cpp
//	Purpose: OpenGL and GLUT program to draw a simple 3D animation with 
//          collision detection. The scene contains moving spheres inside 
//          a fixed, cube enclosure. A simple scene graph is used to define the 
//          scene. Transform nodes, geometry nodes, and presentation nodes
//          are used.
//
//============================================================================

#include <iostream>
#include <vector>

#include <GL/gl3w.h>
#include <GL/freeglut.h>
#include "geometry/geometry.h"
#include "shader_support/glsl_shader.h"
#include "scene/scene.h"

#include "lighting_shader_node.h"
#include "color_node.h"
#include "unitsquare_node.h"
#include "unitsphere_node.h"
#include "balltransform_node.h"

const uint32_t NUMBER_OF_BALLS = 9;
const uint32_t NUMBER_OF_RED_BALLS = 3;
const uint32_t NUMBER_OF_GREEN_BALLS = 3;
const uint32_t NUMBER_OF_BLUE_BALLS = 3;

const float FRAMES_PER_SEC = 72.0f;

// Root of the scene graph and scene state
SceneNode* SceneRoot;
SceneState MySceneState;

// Bounding planes of the enclosure. Used for intersection testing.
std::vector<Plane> BoundingPlanes;

// Ball transforms. We have this global so we can more easily do intersection testing
BallTransform* BallTransforms[NUMBER_OF_BALLS];

// Simple logging function
void logmsg(const char *message, ...)
{
   // Open file if not already opened
   static FILE *lfile = NULL;
   if (lfile == NULL)
      lfile = fopen("Module4_5.log", "w");

   va_list arg;
   va_start(arg, message);
   vfprintf(lfile, message, arg);
   putc('\n', lfile);
   fflush(lfile);
   va_end(arg);
}

/**
 * Use a timer method to try to do a consistent update rate.
 * Without using a timer, the speed of movement will depend on how fast
 * the program runs (fast movement on a fast PC and slow movement on a
 * slower PC)
 */
void timerFunction(int value) {
  // Initialize all balls to have no intersection
  for (uint32_t i = 0; i < NUMBER_OF_BALLS; i++) {
    BallTransforms[i]->SetIntersectTime(0.0f);
  }

  // Go through all balls and test for intersection with subsequent ball
  for (uint32_t i = 0; i < NUMBER_OF_BALLS; i++) {
    // If intersection with a prior ball is not found, test for intersection with successive balls
    if (BallTransforms[i]->GetIntersectTime() == 0.0f) {
      for (uint32_t j = i + 1; j < NUMBER_OF_BALLS; j++) {
        // If an intersection occurs, break out of loop. We will only worry about a ball intersecting
        // one other in a single frame and won't care much if it is the closest
        if (BallTransforms[i]->IntersectBall(BallTransforms[j])) {
          break;
        }
      }
    }
  }

  // Go through all ball and test for plane intersection on those that do not intersect with another ball
  for (uint32_t i = 0; i < NUMBER_OF_BALLS; i++) {
    // Check for collision with any planes
    if (BallTransforms[i]->GetIntersectTime() == 0.0f) {
      float t = 0.0f;
      float min_t = 1.0f;
      Plane intersecting_plane;
      for (auto plane : BoundingPlanes) {
        t = BallTransforms[i]->IntersectWithPlane(plane);
        if (t < min_t) {
          // Copy the nearest intersection and the plane of intersection
          min_t = t;
          intersecting_plane = plane;
        }
      }
      if (min_t != 1.0f) {
        BallTransforms[i]->SetIntersectTime(min_t);
        BallTransforms[i]->SetIntersectingPlane(intersecting_plane);
      }
    }
  }

  // Update the scene graph
  SceneState scene_state;
  SceneRoot->Update(scene_state);

  // Set update to specified frames per second
  glutTimerFunc((int)(1000.0f / FRAMES_PER_SEC), timerFunction, 0);

  glutPostRedisplay();
}

/**
 Display callback. Clears the prior scene and draws a new one.
 */
void display() {
  // Clear the framebuffer and the depth buffer
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // Init scene state and draw the scene graph
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
  // Escape key
  case 27:
    exit(0);
    break;

  default:
    break;
  }
}

/**
 * Reshape callback. Load a 2-D orthographic projection matrix using the 
 * window width and height so we can directly take window coordinates and 
 * send to OpenGL
 * @param  width  Window width
 * @param  height Window height
 */
void reshape(int width, int height) {
  // Center the world window in the viewport so it does not
  // stretch world window
  float world_aspect = 1.0f;
  float window_aspect = (float)width / (float)height;
  if (window_aspect > world_aspect) {
    // Window is wider than world
    int w = static_cast<int>(height * world_aspect);
    int start_x = static_cast<int>((width - w) * 0.5f);
    glViewport(start_x, 0, w, height);
  } else {
    // Window is taller than world
    int h = static_cast<int>(width / world_aspect);
    int start_y = static_cast<int>((height - h) * 0.5f);
    glViewport(0, start_y, width, h);
  }
}

/**
 * Construct a unit box with outward facing normals.
 * @param  unit_square  Geometry node to use
*/
SceneNode* ConstructUnitBox(UnitSquareSurface1* unit_square) {
  // Contruct transform nodes for the sides of the box.
  // Perform rotations so the sides face outwards

  // Bottom is rotated 180 degrees so it faces outwards
  TransformNode* bottomTransform = new TransformNode;
  bottomTransform->Translate(0.0f, 0.0f, -0.5f);
  bottomTransform->RotateX(180.0f);

  // Back is rotated -90 degrees about x: (z -> y)
  TransformNode* backTransform = new TransformNode;
  backTransform->Translate(0.0f, 0.5f, 0.0f);
  backTransform->RotateX(-90.0f);

  // Front wall is rotated 90 degrees about x: (y -> z)
  TransformNode* frontTransform = new TransformNode;
  frontTransform->Translate(0.0f, -0.5f, 0.0f);
  frontTransform->RotateX(90.0f);

  // Left wall is rotated -90 about y: (z -> -x)
  TransformNode* leftTransform = new TransformNode;
  leftTransform->Translate(-0.5f, 0.0f, 00.0f);
  leftTransform->RotateY(-90.0f);

  // Right wall is rotated 90 degrees about y: (z -> x)
  TransformNode* rightTransform = new TransformNode;
  rightTransform->Translate(0.5f, 0.0f, 0.0f);
  rightTransform->RotateY(90.0f);

  // Top 
  TransformNode* top_transform = new TransformNode;
  top_transform->Translate(0.0f, 0.0f, 0.50f);

  // Create a SceneNode and add the 6 sides of the box.
  SceneNode* box = new SceneNode;
  box->AddChild(backTransform);
  backTransform->AddChild(unit_square);
  box->AddChild(leftTransform);
  leftTransform->AddChild(unit_square);
  box->AddChild(rightTransform);
  rightTransform->AddChild(unit_square);
  box->AddChild(frontTransform);
  frontTransform->AddChild(unit_square);
  box->AddChild(bottomTransform);
  bottomTransform->AddChild(unit_square);
  box->AddChild(top_transform);
  top_transform->AddChild(unit_square);

  return box;
}

/**
* Construct the scene
*/
void ConstructScene() {
  // Shader node
  LightingShaderNode* shader = new LightingShaderNode;
  if (!shader->Create("simple_light.vert", "simple_light.frag") ||
    !shader->GetLocations())
    exit(-1);

  // Form bounding planes (for intersection testing)
  Plane p0(Point3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 1.0f));
  BoundingPlanes.push_back(p0);
  Plane p1(Point3(0.0f, 0.0f, 100.0f), Vector3(0.0f, 0.0f, -1.0f));
  BoundingPlanes.push_back(p1);
  Plane p2(Point3(-50.0f, 0.0f, 50.0f), Vector3(1.0f, 0.0f, 0.0f));
  BoundingPlanes.push_back(p2);
  Plane p3(Point3(50.0f, 0.0f, 50.0f), Vector3(-1.0f, 0.0f, 0.0f));
  BoundingPlanes.push_back(p3);
  Plane p4(Point3(0.0f, 50.0f, 50.0f), Vector3(0.0f, -1.0f, 0.0f));
  BoundingPlanes.push_back(p4);
  Plane p5(Point3(0.0f, -50.0f, 50.0f), Vector3(0.0f, 1.0f, 0.0f));
  BoundingPlanes.push_back(p5);

  UnitSquareSurface1* unit_square = new UnitSquareSurface1(shader->GetPositionLoc(), shader->GetNormalLoc());
  UnitSphere* unit_sphere = new UnitSphere(18, 36, shader->GetPositionLoc(), shader->GetNormalLoc());

  // Contruct transform nodes for the walls. Perform rotations so the 
  // walls face inwards
  TransformNode* floorTransform = new TransformNode;
  floorTransform->Scale(100.0f, 100.0f, 1.0f);

  // Back wall is rotated +90 degrees about x: (y -> z)
  TransformNode* backWallTransform = new TransformNode;
  backWallTransform->Translate(0.0f, 50.0f, 50.0f);
  backWallTransform->RotateX(90.0f);
  backWallTransform->Scale(100.0f, 100.0f, 1.0f);

  // Left wall is rotated 90 degrees about y: (z -> x)
  TransformNode* leftWallTransform = new TransformNode;
  leftWallTransform->Translate(-50.0f, 0.0f, 50.0f);
  leftWallTransform->RotateY(90.0f);
  leftWallTransform->Scale(100.0f, 100.0f, 1.0f);

  // Right wall is rotated -90 about y: (z -> -x)
  TransformNode* rightWallTransform = new TransformNode;
  rightWallTransform->Translate(50.0f, 0.0f, 50.0f);
  rightWallTransform->RotateY(-90.0f);
  rightWallTransform->Scale(100.0f, 100.0f, 1.0f);

  // Rotate ceiling by 180 so it faces downward
  TransformNode* ceilingTransform = new TransformNode;
  ceilingTransform->Translate(0.0f, 0.0f, 100.0f);
  ceilingTransform->RotateX(180.0f);
  ceilingTransform->Scale(100.0f, 100.0f, 1.0f);

  // Construct color nodes for the walls, floor, and ceiling
  ColorNode* floorColor = new ColorNode(Color4(0.6f, 0.5f, 0.2f));
  ColorNode* backWallColor = new ColorNode(Color4(0.9f, 0.7f, 0.5f));
  ColorNode* wallColor = new ColorNode(Color4(1.0f, 1.0f, 1.0f));
  ColorNode* ceilingColor = new ColorNode(Color4(0.1f, 0.4f, 1.0f));

  // Construct a unit box with outward normals as a scene graph node
  // using relative transforms
  SceneNode* unit_box = ConstructUnitBox(unit_square);

  // Set a purple color for the box
  ColorNode* box_color = new ColorNode(Color4(0.5f, 0.0f, 0.5f));

  // Construct a transform node to position, size, and orient the box
  TransformNode* box_transform = new TransformNode();
  box_transform->Translate(25.0f, 25.0f, 10.1f);
  box_transform->RotateZ(45.0f);
  box_transform->Scale(40.0f, 20.0f, 20.0f);

  // Construct transform nodes for the balls. Each one has a 
  // random initial position with x,y values between -40 and 40
  // and z values between 25 and 75. A random radius between 3 and 
  // 7 units, a random initial direction vectors (unit length) and 
  // random velocity between 5 and 15 units per second is used.
  for (uint32_t i = 0; i < NUMBER_OF_BALLS; i++) {
    BallTransforms[i] = new BallTransform(FRAMES_PER_SEC);
  }

  // Construct color nodes for the balls
  ColorNode* redBall = new ColorNode(Color4(1.0f, 0.0f, 0.0f));
  ColorNode* greenBall = new ColorNode(Color4(0.0f, 1.0f, 0.0f));
  ColorNode* blueBall = new ColorNode(Color4(0.0f, 0.0f, 1.0f));

  // Construct the scene layout
  SceneRoot = new SceneNode;
  SceneRoot->AddChild(shader);

  // Walls
  shader->AddChild(backWallColor);
  backWallColor->AddChild(backWallTransform);
  backWallTransform->AddChild(unit_square);
  shader->AddChild(wallColor);
  wallColor->AddChild(leftWallTransform);
  leftWallTransform->AddChild(unit_square);
  wallColor->AddChild(rightWallTransform);
  rightWallTransform->AddChild(unit_square);
  shader->AddChild(floorColor);
  floorColor->AddChild(floorTransform);
  floorTransform->AddChild(unit_square);
  shader->AddChild(ceilingColor);
  ceilingColor->AddChild(ceilingTransform);
  ceilingTransform->AddChild(unit_square);

  // Comment out the box for Module 5
//  shader->AddChild(box_color);
//  box_color->AddChild(box_transform);
//  box_transform->AddChild(unit_box);

  // Add a unit sphere child node to each ball transform node 
  for (uint32_t i = 0; i < NUMBER_OF_BALLS; i++) {
    BallTransforms[i]->AddChild(unit_sphere);
  }

  // Add ball transforms under each ball presentation node
  uint32_t j = 0;
  shader->AddChild(redBall);
  for (uint32_t i = 0; i < NUMBER_OF_RED_BALLS; i++) {
    redBall->AddChild(BallTransforms[j++]);
  }
  shader->AddChild(greenBall);
  for (uint32_t i = 0; i < NUMBER_OF_GREEN_BALLS; i++) {
    greenBall->AddChild(BallTransforms[j++]);
  }
  shader->AddChild(blueBall);
  for (uint32_t i = 0; i < NUMBER_OF_BLUE_BALLS; i++) {
    blueBall->AddChild(BallTransforms[j++]);
  }
}

/**
 * Main 
 */
int main(int argc, char** argv) {
  // Initialize free GLUT
  glutInit(&argc, argv);
  glutInitContextVersion(3, 2);
  glutInitContextProfile(GLUT_CORE_PROFILE);

  // Double buffer with depth buffer and MSAA
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_DEPTH | GLUT_MULTISAMPLE);
  glutInitWindowPosition(100, 100); 
  glutInitWindowSize(640, 480);
  glutCreateWindow("Simple Animation by David Nesbitt");
 
  // Add GLUT callbacks
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
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

  // Set the clear color to black. Any part of the window outside the
  // viewport should appear black
  glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

  // Construct scene.
  ConstructScene();

  // Enable depth testing
  glEnable(GL_DEPTH_TEST);

  // Enable back face polygon removal
  glFrontFace(GL_CCW);
  glCullFace(GL_BACK);
  glEnable(GL_CULL_FACE);

  // Set a fixed perspective projection. fov = 70, aspect = 1.0, near = 1.0 far = 200.
  // We are hard-coding viewing and projection matrices since they do not change in
  // this application.
  Matrix4x4 projection;
  projection.m00() = 1.428f;
  projection.m11() = 1.428f;
  projection.m22() = -1.010f;
  projection.m23() = -2.010f;
  projection.m32() = -1.0f;
  projection.m33() =  0.0f;

  // Set a fixed camera outside the center of the front wall (imagine
  // it being a window) looking parallel to the floor
  Matrix4x4 view;
  view.m00() = 1.0f;
  view.m11() = 0.0f;
  view.m12() = 1.0f;
  view.m13() = -50.0f;
  view.m21() = -1.0f;
  view.m22() =  0.0f;
  view.m23() = -90.0f;
  view.m33() = 1.0f;

  // Set the composite projection and viewing matrix
  // These remain fixed.
  MySceneState.pv = projection * view;

  // Set update rate and set initial timer callback
  glutTimerFunc((int)(1000.0f / FRAMES_PER_SEC), timerFunction, 0);

  glutMainLoop();
  return 0;
}
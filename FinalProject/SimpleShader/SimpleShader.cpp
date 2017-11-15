//============================================================================
//	Johns Hopkins University Whiting School of Engineering
//	605.461 Principles of Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:  David W. Nesbitt
//	File:    GetStarted.cpp
//	Purpose: OpenGL shader program to draw a set of "wide lines" and 
//           interactively draw points based on mouse clicks.
//
//============================================================================

#include <iostream>
#include <vector>

#include <GL/gl3w.h>
#include <GL/freeglut.h>
#include "geometry/geometry.h"
#include "shader_support/glsl_shader.h"
#include "scene/scene.h"

#include "line_shader_node.h"
#include "line_node.h"

// Root of the scene graph
SceneNode* SceneRoot;

// Scene state
SceneState MySceneState;

// Simple logging function
void logmsg(const char *message, ...) {
  // Open file if not already opened
  static FILE *lfile = NULL;
  if (lfile == NULL) {
    lfile = fopen("SimpleShader.log", "w");
  }

  va_list arg;
  va_start(arg, message);
  vfprintf(lfile, message, arg);
  putc('\n', lfile);
  fflush(lfile);
  va_end(arg);
}

/**
 * Reshape callback. Load a 2-D orthographic projection matrix using the 
 * window width and height so we can directly take window coordinates and 
 * send to OpenGL. Note that this callback will be called when the window
 * is opened.
 * @param  width  Window width
 * @param  height Window height
 */
void reshape(int width, int height) {
  // Set a simple 4x4 matrix (use an array until we develop the Matrix4x4 class)
  MySceneState.ortho[0]  = 2.0f / static_cast<float>(width);
  MySceneState.ortho[1]  = 0.0f;
  MySceneState.ortho[2]  = 0.0f;
  MySceneState.ortho[3]  = 0.0f;
  MySceneState.ortho[4]  = 0.0f;
  MySceneState.ortho[5] = -2.0f / static_cast<float>(height);
  MySceneState.ortho[6]  = 0.0f;
  MySceneState.ortho[7]  = 0.0f;
  MySceneState.ortho[8]  = 0.0f;
  MySceneState.ortho[9]  = 0.0f;
  MySceneState.ortho[10] = 1.0f;
  MySceneState.ortho[11] = 0.0f;
  MySceneState.ortho[12] = -1.0f;
  MySceneState.ortho[13] = 1.0f;
  MySceneState.ortho[14] = 0.0f;
  MySceneState.ortho[15] = 1.0f;

  // Update the viewport
  glViewport(0, 0, width, height);
}

/**
 * Display callback function
 */
void display(void) {
  // Clear the framebuffer
  glClear(GL_COLOR_BUFFER_BIT);
   
  SceneRoot->Draw(MySceneState);
  CheckError("After Draw");  

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
 * Create the scene.
 */
void CreateScene() {
  SceneRoot = new SceneNode;

  // Create shader and get uniform locations
  LineShaderNode* lineShader = new LineShaderNode();
  if (!lineShader->Create("lines.vert", "lines.frag") ||
    !lineShader->GetLocations()) {
    exit(-1);
  }

  // Create a line node using the following set of vertices
  std::vector<Point2> pts = {
    { 20.0f, 50.0f },
    { 100.0f, 120.0f },
    { 170.0f, 160.0f },
    { 220.0f, 180.0f },
    { 380.0f, 250.0f },
    { 400.0f, 350.0f },
    { 320.0f, 450.0f },
    { 120.0f, 400.0f }
  };
  LineNode* lines = new LineNode(pts, Color4(0.1f, 0.7f, 0.1f, 1.0f), lineShader->GetPositionLoc());

  // Create scene graph
  SceneRoot->AddChild(lineShader);
  lineShader->AddChild(lines);
}

/**
 * Main 
 */
int main(int argc, char** argv) {
  std::cout << "Keyboard Controls:" << std::endl;
  std::cout << "ESC - Exit program" << std::endl;

  // Initialize free GLUT
  glutInit(&argc, argv);
  glutInitContextVersion(3, 2);
  glutInitContextProfile(GLUT_CORE_PROFILE);

  // Initialize display mode and window
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_MULTISAMPLE);
  glutInitWindowPosition(100, 100);
  glutInitWindowSize(640, 480);

  // Create the window and callback methods
  glutCreateWindow("SimpleShader by David Nesbitt");
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutKeyboardFunc(keyboard);

  // Initialize Open 3.2 core profile
  if (gl3wInit()) {
    std::cout << "gl3wInit: failed to initialize OpenGL" << std::endl;
    return -1;
  }
  if (!gl3wIsSupported(3, 2)) {
    std::cout << "OpenGL 3.2 not supported" << std::endl;
    return -1;
  }
  printf("OpenGL %s, GLSL %s\n", glGetString(GL_VERSION), glGetString(GL_SHADING_LANGUAGE_VERSION));

  // Set OpenGL defaults
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  // Create the scene
  CreateScene();
  CheckError("CreateScene");

  // Main GLUT loop
  glutMainLoop();

  return 0;
}
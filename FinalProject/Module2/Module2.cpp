//============================================================================
//	Johns Hopkins University Whiting School of Engineering
//	605.461 Principles of Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:  David W. Nesbitt
//	File:    Module2.cpp
//	Purpose: OpenGL shader program to draw points or lines based on mouse
//           clicks.
//
//============================================================================

#include <iostream>
#include <vector>

#include <GL/gl3w.h>
#include <GL/freeglut.h>
#include "geometry/geometry.h"
#include "shader_support/glsl_shader.h"
#include "scene/scene.h"

#include "point_shader_node.h"
#include "point_node.h"
#include "line_shader_node.h"
#include "line_node.h"

// Root of the scene graph
SceneNode* SceneRoot;

// Scene state
SceneState MySceneState;

// PointNode - global so we can add points dynamically
PointNode* Points;

// PointShaderNode - global so we can get the point location
PointShaderNode* PointShader;

// LineNode
LineNode* Lines;

// LineShaderNode - global so we can get the point location
LineShaderNode* LineShader;

// Simple logging function
void logmsg(const char *message, ...) {
  // Open file if not already opened
  static FILE *lfile = NULL;
  if (lfile == NULL) {
    lfile = fopen("Module2.log", "w");
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
  // Force a display update with glutPostRedisplay on a valid key input
  switch (key) {
    // Escape key
  case 27:
    exit(0);
    break;

  // Alter point size and line width if a numeric key is pressed
  case '1':
    glLineWidth(1.0f);
    glPointSize(2.0f);
    glutPostRedisplay();
    break;
  case '2':
    glLineWidth(2.0f);
    glPointSize(4.0f);
    glutPostRedisplay();
    break;
  case '3':
    glLineWidth(3.0f);
    glPointSize(6.0f);
    glutPostRedisplay();
    break;
  case '4':
    glLineWidth(4.0f);
    glPointSize(8.0f);
    glutPostRedisplay();
    break;
  case '5':
    glLineWidth(5.0f);
    glPointSize(10.0f);
    glutPostRedisplay();
    break;
  case '6':
    glLineWidth(6.0f);
    glPointSize(12.0f);
    glutPostRedisplay();
    break;
  case '7':
    glLineWidth(7.0f);
    glPointSize(14.0f);
    glutPostRedisplay();
    break;
  case '8':
    glLineWidth(8.0f);
    glPointSize(16.0f);
    glutPostRedisplay();
    break;
  case '9':
    glLineWidth(9.0f);
    glPointSize(18.0f);
    glutPostRedisplay();
    break;
  default:
    break;
  }
}

/**
 * Mouse callback (called when a mouse button state changes).
 * @param  button  Button 
 * @param  x       Screen x position
 * @param  y       Screen y position
 */
void mouse(int button, int state, int x, int y) {
  // On a button down event add a point to both the points and lines
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    // Add a point to display
    Points->Add((float)x, (float)y, PointShader->GetPositionLoc());

    // Add a point to the polyline
    Lines->Add((float)x, (float)y, LineShader->GetPositionLoc());

    // Force a redisplay
    glutPostRedisplay();
  }
}


/**
 * Create the scene.
 */
void CreateScene() {
  // Create a scene graph with 2 shaders (one for offset lines and one 
  // for points) and 2 geometry nodes
  LineShader = new LineShaderNode();
  if (!LineShader->Create("lines.vert", "lines.frag") ||
    !LineShader->GetLocations())
    exit(-1);

  Lines = new LineNode(Color4(0.1f, 0.1f, 6.1f, 1.0f));

  // Create the point shader node
  PointShader = new PointShaderNode();
  if (!PointShader->Create("points.vert", "points.frag") ||
      !PointShader->GetLocations())
    exit(-1);

  // Create the node that manages the points
  Points = new PointNode();

  // Create scene graph
  SceneRoot = new SceneNode;
  SceneRoot->AddChild(LineShader);
  LineShader->AddChild(Lines);
  SceneRoot->AddChild(PointShader);
  PointShader->AddChild(Points);
}

/**
 * Main - entry point for GetStarted GLUT application.
 */
int main(int argc, char** argv) {
  std::cout << "Keyboard Controls:" << std::endl;
  std::cout << "1-9 : Alter line width and point size" << std::endl;
  std::cout << "ESC - Exit program" << std::endl;

  // Initialize free GLUT
  glutInit(&argc, argv);
  glutInitContextVersion(3, 2);
  glutInitContextProfile(GLUT_CORE_PROFILE);

  // Initialize display mode and window
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
  glutInitWindowPosition(200, 200);
  glutInitWindowSize(800, 600);

  // Create the window and register callback methods
  glutCreateWindow("Module2 by David Nesbitt");
  glutDisplayFunc(display);
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);
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

  // Set the clear color to white
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  // Set a default point size and line width
  glPointSize(8.0f);
  glLineWidth(4.0f);

  // Setting so gl_PointCoord has origin at lower left.
  glPointParameteri(GL_POINT_SPRITE_COORD_ORIGIN, GL_LOWER_LEFT);

  // Create the scene
  CreateScene();
  CheckError("CreateScene");

  // Main GLUT loop
  glutMainLoop();
  return 0;
}
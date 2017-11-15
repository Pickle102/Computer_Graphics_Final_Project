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

#include "ngon_shader_node.h"
#include "ngon_node.h"
#include "color_blending_node.h"
#include "line_shader_node.h"
#include "drag_line_node.h"
#include "point_shader_node.h"
#include "point_node.h"

// Root of the scene graph
SceneNode* SceneRoot;

// Scene state
SceneState MySceneState;

// NGons
NGonNode* Octagon;
NGonNode* Circle;
NGonNode* Hexagon;

// Draggable line
DragLineNode* DragLine;

// Are we drawing a line with the mouse? This global flag allows us to 
// follow mouse motion events and draw if the left button is down.
bool DrawLine = false;

// Current line being drawn (while dragging the mouse)
LineSegment2 CurrentLine;

// Intersection points
PointNode* IntersectionPoints;

// Inverse transformation from screen window coordinates (mouse event positions) 
// into world coordinates.
Matrix4x4 Inverse;

// Simple logging function
void logmsg(const char *message, ...) {
  // Open file if not already opened
  static FILE *lfile = NULL;
  if (lfile == NULL) {
    lfile = fopen("Module3.log", "w");
  }

  va_list arg;
  va_start(arg, message);
  vfprintf(lfile, message, arg);
  putc('\n', lfile);
  fflush(lfile);
  va_end(arg);
}

/**
 * Reshape callback. Load a 2-D orthographic projection matrix. Use a world
 * window with width or height of 10 units along the smallest of the screen
 * window dimensions. Set a viewport to draw into the entire window.
 * Note that this callback will be called when the window is opened.
 * @param  width  Window width
 * @param  height Window height
 */
void reshape(int width, int height) {
  // Set world window boundaries so the world window has dimensions of 10 
  // in the smallest screen window dimension. Match the other dimension to 
  // the screen window aspect.
  float w = static_cast<float>(width);
  float h = static_cast<float>(height);
  float aspect = w / h;
  float wl, wr, wt, wb;
  if (width < height) {
    wl = -5.0f;
    wr =  5.0f;
    wb = -5.0f / aspect;
    wt =  5.0f / aspect;
  }
  else {
    wl = -5.0f * aspect;
    wr =  5.0f * aspect;
    wb = -5.0f;
    wt =  5.0f;
  }

  // Create an orthographic transformation to transform the world window 
  // into clip coordinates with vl = -1, vr = 1, vt = 1, and vb = -1.
  // Set as a 4x4 matrix with the scaling factors A,B and translation C,D
  MySceneState.ortho_matrix.m00() = 2.0f / (wr - wl);  // A
  MySceneState.ortho_matrix.m03() = -(wr + wl) / (wr - wl);
  MySceneState.ortho_matrix.m11() = 2.0f / (wt - wb);
  MySceneState.ortho_matrix.m13() = -(wt + wb) / (wt - wb);

  // Set the Inverse transformation from screen coordinates into world coordinates
  Inverse.m00() = (wr - wl) / w;              // A
  Inverse.m03() = wl;                         // C
  Inverse.m11() = (wt - wb) / -h;             // B
  Inverse.m13() = wb - (Inverse.m11() * h);   // D

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

  // Enable/disable MSAA
  case 'M':
    glEnable(GL_MULTISAMPLE);
    glutPostRedisplay();
    break;
  case 'm':
    glDisable(GL_MULTISAMPLE);
    glutPostRedisplay();
    break;
  default:
    break;
  }
}

/**
 * Mouse callback (called when a mouse button state changes). Starts a 
 * new draggable line when the left button is down. When left button up 
 * the line is cleared.
 */
void mouse(int button, int state, int x, int y) {
  // On a button down event add a point as the start of a line
  if (button == GLUT_LEFT_BUTTON && state == GLUT_DOWN) {
    // Set the current line endpoints (both) to the mouse position and
    // set flag indicating we are dragging a line
    HPoint3 world = Inverse * Point3(static_cast<float>(x), static_cast<float>(y), 0.0f);
    DrawLine = true;
    CurrentLine.a.Set(world.x, world.y);
    CurrentLine.b.Set(world.x, world.y);
    DragLine->ReplacePoint0(CurrentLine.a);

    // Force a redisplay
    glutPostRedisplay();
  }

  // On a button up event clear the draggable line and intersection points
  if (button == GLUT_LEFT_BUTTON && state == GLUT_UP) {
    // No longer dragging a line - clear it
    DrawLine = false;
    DragLine->Clear();
    IntersectionPoints->Clear();

    // Force a redisplay
    glutPostRedisplay();
  }
}

// Test if 2 points are equal within tolerance
bool Equal(const Point2& p0, const Point2& p1) {
  return (fabs(p1.x - p0.x) < kEpsilon &&
          fabs(p1.y - p0.y) < kEpsilon);
}

/**
* Mouse motion callback (called when mouse button is depressed)
*/
void mouseMotion(int x, int y) {
  // Set the current line endpoints to the mouse position
  if (DrawLine) {
    // Update the current line VBO
    HPoint3 world = Inverse * Point3(x, y, 0.0f);
    CurrentLine.b.Set(world.x, world.y);
    DragLine->ReplacePoint1(CurrentLine.b);

    // Compute intersection points with n-gons
    std::vector<Point2> int_pts;
    LineSegment2 clip;
    if (CurrentLine.ClipToPolygon(Hexagon->get_vertex_list(), clip)) {
      if (!Equal(clip.a, CurrentLine.a)) {
        int_pts.push_back(clip.a);
      }
      if (!Equal(clip.b, CurrentLine.b)) {
        int_pts.push_back(clip.b);
      }
    }
    if (CurrentLine.ClipToPolygon(Circle->get_vertex_list(), clip)) {
      if (!Equal(clip.a, CurrentLine.a)) {
        int_pts.push_back(clip.a);
      }
      if (!Equal(clip.b, CurrentLine.b)) {
        int_pts.push_back(clip.b);
      }
    }
    if (CurrentLine.ClipToPolygon(Octagon->get_vertex_list(), clip)) {
      if (!Equal(clip.a, CurrentLine.a)) {
        int_pts.push_back(clip.a);
      }
      if (!Equal(clip.b, CurrentLine.b)) {
        int_pts.push_back(clip.b);
      }
    }
    IntersectionPoints->Update(int_pts);

    glutPostRedisplay();
  }
}

/**
 * Create the scene.
 */
void CreateScene() {
  // Shader for n-gons
  NGonShaderNode* ngon_shader = new NGonShaderNode();
  if (!ngon_shader->Create("ngon.vert", "ngon.frag") ||
      !ngon_shader->GetLocations())
    exit(-1);

  // Shader for draggable smooth shaded line
  LineShaderNode* line_shader = new LineShaderNode();
  if (!line_shader->Create("shaded_line.vert", "shaded_line.frag") ||
      !line_shader->GetLocations()) {
    exit(-1);
  }

  // Shader for intersection points
  PointShaderNode* point_shader = new PointShaderNode();
  if (!point_shader->Create("points.vert", "points.frag") ||
      !point_shader->GetLocations()) {
    exit(-1);
  }

  // Create an octagon centered at (2.5, 2.5) with radius = 2
  Octagon = new NGonNode(Point2(2.5f, 2.5f), 8, 2.0f, 
              ngon_shader->GetPositionLoc());

  // Create a circle approximation with 72 sides centered at (0,0) 
  // with radius 4.5
  Circle = new NGonNode(Point2(0.0f, 0.0f), 72, 4.5f, 
              ngon_shader->GetPositionLoc());

  // Create a hexagon centered at (-2,-2) with radius = 3
  Hexagon = new NGonNode(Point2(-2.0f, -2.0f), 6, 3.0f,
              ngon_shader->GetPositionLoc());

  // Circle is red and not blended - it is the "background" color and is
  // drawn first so the other 2 filled objects blend with its color
  ColorBlendingNode* circle_color = new ColorBlendingNode(
            Color4(0.75f, 0.0f, 0.0f, 1.0f), false);

  // Octagon is green and 50% opaque
  ColorBlendingNode* octagon_color = new ColorBlendingNode(
        Color4(0.0f, 1.0f, 0.0f, 0.75f), true);

  // Hexagon is blue and 25% opaque (75 % transparent)
  ColorBlendingNode* hexagon_color = new ColorBlendingNode(
        Color4(0.0f, 0.0f, 1.0f, 0.25f), true);

  // Create a node for managing current line. Use a width = 4 for current lines
  Color4 drag_color0(0.8f, 0.1f, 0.1f, 1.0f);      // Color at the start of the line
  Color4 drag_color1(0.1f, 0.8f, 0.1f, 1.0f);      // Color at the end of the line
  DragLine = new DragLineNode(drag_color0, drag_color1, 4.0f, 
            line_shader->GetPositionLoc(), line_shader->GetColorLoc());

  // Intersection points - capacity = 6 (max. intersections with n-gons)
  IntersectionPoints = new PointNode(6, point_shader->GetPositionLoc());

  // Create scene graph
  SceneRoot = new SceneNode;

  // Add n-gons with appropriate color/presentation nodes
  SceneRoot->AddChild(ngon_shader);
  ngon_shader->AddChild(circle_color);
  circle_color->AddChild(Circle);
  ngon_shader->AddChild(octagon_color);
  octagon_color->AddChild(Octagon);
  ngon_shader->AddChild(hexagon_color);
  hexagon_color->AddChild(Hexagon);

  // Add draggable line
  SceneRoot->AddChild(line_shader);
  line_shader->AddChild(DragLine);

  // Add intersection points
  SceneRoot->AddChild(point_shader);
  point_shader->AddChild(IntersectionPoints);
}

/**
 * Main - entry point for GetStarted GLUT application.
 */
int main(int argc, char** argv) {
  std::cout << "Keyboard Controls:" << std::endl;
  std::cout << "M : Enable MSAA    m : Disable MSAA" << std::endl;
  std::cout << "ESC - Exit program" << std::endl;

  // Initialize free GLUT
  glutInit(&argc, argv);
  glutInitContextVersion(3, 2);
  glutInitContextProfile(GLUT_CORE_PROFILE);

  // Initialize display mode and window
  glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE | GLUT_MULTISAMPLE);
  glutInitWindowPosition(200, 200);
  glutInitWindowSize(800, 800);

  // Create the window and register callback methods
  glutCreateWindow("Module3 by David Nesbitt");
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

  // Set the clear color to black
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

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
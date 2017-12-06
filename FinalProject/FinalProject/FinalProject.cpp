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
enum DrawObjectType {
    CYLINDER, CONE, TRUNCATED_CONE, TORUS, CUBE, ICOSAHEDRON,
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
bool Animate = false;
bool Forward = true;
int MouseX, MouseY;
int RenderWidth = 640;
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
    float dx = 4.0f * ((x - (static_cast<float>(RenderWidth) * 0.5f)) /
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
* Construct the terrain for the scene.
*/
SceneNode* ConstructTreeModel(float x, float y, TexturedUnitSquareSurface* tree_square)
{
    // Create a tree material with a picture
    PresentationNode* tree_material = new PresentationNode(
        Color4(0.5f, 0.5f, 0.5f), Color4(0.75f, 0.75f, 0.75f),
        Color4(0.1f, 0.1f, 0.1f), Color4(0.0f, 0.0f, 0.0f), 5.0f);
    tree_material->SetTexture("tree5.png", GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE,
        GL_NEAREST, GL_NEAREST);
    tree_material->CreateBillboard();

    TransformNode* treeFront_transform = new TransformNode;
    treeFront_transform->Translate(x, y, 6.0f);
    treeFront_transform->RotateX(90.0f);
    treeFront_transform->Scale(15.0f, 15.0f, 1.0f);

    TransformNode* treeRighttransform = new TransformNode;
    treeRighttransform->Translate(x, y, 6.0f);
    treeRighttransform->RotateX(90.0f);
    treeRighttransform->RotateY(90.0f);
    treeRighttransform->Scale(15.0f, 15.0f, 1.0f);

    // Return the constructed floor
    SceneNode* tree = new SceneNode;

    tree->AddChild(tree_material);
    tree_material->AddChild(treeFront_transform);
    treeFront_transform->AddChild(tree_square);

    tree->AddChild(tree_material);
    tree_material->AddChild(treeRighttransform);
    treeRighttransform->AddChild(tree_square);

    return tree;
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

    // Light 0 - point light source in back right corner
    LightNode* light0 = new LightNode(0);
    light0->SetDiffuse(Color4(0.5f, 0.5f, 0.5f, 1.0f));
    light0->SetSpecular(Color4(0.5f, 0.5f, 0.5f, 1.0f));
    light0->SetPosition(HPoint3(90.0f, 90.0f, 30.f, 1.0f));
    light0->Enable();

    // Light1 - directional light from the ceiling
    LightNode* light1 = new LightNode(1);
    light1->SetDiffuse(Color4(0.4f, 0.4f, 0.4f, 1.0f));
    light1->SetSpecular(Color4(0.4f, 0.4f, 0.4f, 1.0f));
    light1->SetPosition(HPoint3(1.0f, 0.0f, 0.5f, 0.0f));
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
    MyCamera->AddChild(light1);
    //light0->AddChild(light1);
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
    int normal_loc = lightingShader->GetNormalLoc();
    int texture_loc = lightingShader->GetTextureLoc();

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

    /********TEST***************/
    // To be replaced by trees!
    ConicSurface* test_obj = new ConicSurface(0.5f, 0.5f, 18, 4,
        position_loc, normal_loc);;

    // Construct scene lighting - make lighting nodes children of the camera node
    ConstructLighting(lightingShader);

    // Construct subdivided square - subdivided 50x in both x and y
    UnitSquareSurface* unit_square = new UnitSquareSurface(50, position_loc, normal_loc);

    // Construct a textured square for the skybox
    TexturedUnitSquareSurface* textured_square_skybox = new TexturedUnitSquareSurface(2, 1, position_loc,
        normal_loc, texture_loc);

    // Construct the skybox as a child of the root node
    SceneNode* skybox = ConstructSkyBox(unit_square, textured_square_skybox);

    // Construct a textured square for the floor
    TexturedUnitSquareSurface* ground_textured_square = new TexturedUnitSquareSurface(2, 200, position_loc,
        normal_loc, texture_loc);

    // Construct a textured square for the trees
    TexturedUnitSquareSurface* tree_textured_square = new TexturedUnitSquareSurface(1, 1, position_loc,
        normal_loc, texture_loc);

    // Construct the ground as a child of the root node
    SceneNode* ground = ConstructGround(ground_textured_square);

    // Construct the trees
    SceneNode* trees = ConstructTreeModel(0.0f, 0.0f, tree_textured_square);

    // Construct the scene layout
    SceneRoot = new SceneNode;
    SceneRoot->AddChild(lightingShader);
    lightingShader->AddChild(MyCamera);

    // Construct a base node for the rest of the scene, it will be a child
    // of the last light node (so entire scene is under influence of all 
    // lights)
    SceneNode* myscene = new SceneNode;
    Spotlight->AddChild(myscene);

    // Add the terrain
    myscene->AddChild(skybox);
    myscene->AddChild(ground);
    myscene->AddChild(trees);
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
    //glFrontFace(GL_CCW);
    //glCullFace(GL_BACK);
    //glEnable(GL_CULL_FACE);

    // Enable multisample anti-aliasing
    glEnable(GL_MULTISAMPLE);

    // Construct the scene.
    ConstructScene();

    glutMainLoop();
    return 0;
}

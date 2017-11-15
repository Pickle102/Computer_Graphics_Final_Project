//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:  David W. Nesbitt
//	File:    ObjectCreation.cpp
//	Purpose: Construct polyhedra and assorted objects
//
//============================================================================

// Disable Visual C++ warnings for fopen and sprintf
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

/**
 * Form a vertex list for a pentagon face
 */
std::vector<Point3> FormPentagon(const Point3& v0, const Point3& v1, 
               const Point3& v2, const Point3& v3, const Point3& v4) {
   return{ v0, v1, v2, v3, v4 };
}

/**
 * Find the dual (center) of a triangle face.
 * @param  v0   Vertex 0 of the triangle.
 * @param  v1   Vertex 0 of the triangle.
 * @param  v2   Vertex 0 of the triangle.
 * @return Returns the center of the triangle.
 */
Point3 dual(const Point3& v0, const Point3& v1, const Point3& v2) {
   return Point3((v0.x + v1.x + v2.x) * 0.33333f,
                 (v0.y + v1.y + v2.y) * 0.33333f,
                 (v0.z + v1.z + v2.z) * 0.33333f);
}


/**
 * Construct cube. Create a flat surface (subdivided by 5). Create
 * TransformNodes to transform for each face and add the flat surface
 * as a child of each Transform node.
 * @param  n  Number of subdivisions for faces of the cube
 */
SceneNode* ConstructCube(int n, const int position_loc, const int normal_loc, 
                          const int texture_loc) {
   // Create flat surface
   TexturedUnitSquareSurface* square = new TexturedUnitSquareSurface(5, 1.0f,
              position_loc, normal_loc, texture_loc);

   // Top
   TransformNode* top = new TransformNode;
   top->Translate(0.0f, 0.0f, 0.5f);
   top->AddChild(square);

   // Bottom
   TransformNode* bottom = new TransformNode;
   bottom->Translate(0.0f, 0.0f, -0.5f);
   bottom->RotateX(180.0f);
   bottom->AddChild(square);

   // Left
   TransformNode* left = new TransformNode;
   left->Translate(-0.5f, 0.0f, 0.0f);
   left->RotateY(-90.0f);
   left->AddChild(square);

   // Right
   TransformNode* right = new TransformNode;
   right->Translate(0.5f, 0.0f, 0.0f);
   right->RotateY(90.0f);
   right->AddChild(square);

   // Back
   TransformNode* back = new TransformNode;
   back->Translate(0.0f, 0.5f, 0.0f);
   back->RotateX(-90.0f);
   back->AddChild(square);

   // Front
   TransformNode* front = new TransformNode;
   front->Translate(0.0f, -0.5f, 0.0f);
   front->RotateX(90.0f);
   front->AddChild(square);

   SceneNode* cube = new SceneNode;
   cube->AddChild(top);
   cube->AddChild(back);
   cube->AddChild(front);
   cube->AddChild(right);
   cube->AddChild(left);
   cube->AddChild(bottom);
   return cube;
}

/**
 * Construct a tetrahedron
 */
TriSurface* ConstructTetrahedron(const int position_loc, const int normal_loc) {
   // Vertices of the tetrahedron
   Point3 v0( 1.0f,  1.0f,  1.0f);
   Point3 v1( 1.0f, -1.0f, -1.0f);
   Point3 v2(-1.0f, -1.0f,  1.0f);
   Point3 v3(-1.0f,  1.0f, -1.0f);

   // Create faces as children of a base scene node
   TriSurface* tetrahedron = new TriSurface();
   tetrahedron->AddPolygon(FormTriangle(v2, v3, v1));
   tetrahedron->AddPolygon(FormTriangle(v2, v0, v3));
   tetrahedron->AddPolygon(FormTriangle(v0, v1, v3));
   tetrahedron->AddPolygon(FormTriangle(v1, v0, v2));
   tetrahedron->CreateVertexBuffers(position_loc, normal_loc);
   return tetrahedron;
}

/**
 * Construct an octahedron
 */
TriSurface* ConstructOctahedron(const int position_loc, const int normal_loc) {
   // 6 vertices of the octahedron
   Point3 v0( 0.0f,  0.0f,  0.5f);
   Point3 v1( 0.0f,  0.0f, -0.5f);
   Point3 v2(-0.5f,  0.0f,  0.0f);
   Point3 v3( 0.5f,  0.0f,  0.0f);
   Point3 v4( 0.0f,  0.5f,  0.0f);
   Point3 v5( 0.0f, -0.5f,  0.0f);

   // Create faces as children of a base scene node
   TriSurface* octahedron = new TriSurface;
   octahedron->AddPolygon(FormTriangle(v0, v3, v4));
   octahedron->AddPolygon(FormTriangle(v0, v5, v3));
   octahedron->AddPolygon(FormTriangle(v0, v2, v5));
   octahedron->AddPolygon(FormTriangle(v0, v4, v2));
   octahedron->AddPolygon(FormTriangle(v4, v3, v1));
   octahedron->AddPolygon(FormTriangle(v3, v5, v1));
   octahedron->AddPolygon(FormTriangle(v5, v2, v1));
   octahedron->AddPolygon(FormTriangle(v2, v4, v1));
   octahedron->CreateVertexBuffers(position_loc, normal_loc);
   return octahedron;
}

/**
 * Construct an icosahedron
 */
TriSurface* ConstructIcosahedron(const int position_loc, const int normal_loc) {
   // Vertices of the icosahedron
   float t = 0.618f;
   Point3 v0( 0.0f,  1.0f,  t);
   Point3 v1( 0.0f,  1.0f, -t);
   Point3 v2( 1.0f,   t,  0.0f);
   Point3 v3( 1.0f,  -t,  0.0f);
   Point3 v4( 0.0f, -1.0f,  -t);
   Point3 v5( 0.0f, -1.0f,   t);
   Point3 v6(  t, 0.0f,  1.0f);
   Point3 v7( -t, 0.0f,  1.0f);
   Point3 v8(  t, 0.0f,  -1.0f);
   Point3 v9( -t, 0.0f,  -1.0f);
   Point3 v10(-1.0f,  t,  0.0f);
   Point3 v11(-1.0f, -t,  0.0f);

   // Create faces as children of a base scene node
   TriSurface* icosahedron = new TriSurface;
   icosahedron->AddPolygon(FormTriangle(v0, v6, v2));
   icosahedron->AddPolygon(FormTriangle(v6, v3, v2));
   icosahedron->AddPolygon(FormTriangle(v6, v5, v3));
   icosahedron->AddPolygon(FormTriangle(v7, v5, v6));
   icosahedron->AddPolygon(FormTriangle(v7, v6, v0));
   icosahedron->AddPolygon(FormTriangle(v2, v3, v8));
   icosahedron->AddPolygon(FormTriangle(v1, v2, v8));
   icosahedron->AddPolygon(FormTriangle(v0, v2, v1));
   icosahedron->AddPolygon(FormTriangle(v10, v0, v1));
   icosahedron->AddPolygon(FormTriangle(v10, v1, v9));
   icosahedron->AddPolygon(FormTriangle(v1, v8, v9));
   icosahedron->AddPolygon(FormTriangle(v3, v4, v8));
   icosahedron->AddPolygon(FormTriangle(v5, v4, v3));
   icosahedron->AddPolygon(FormTriangle(v11, v4, v5));   
   icosahedron->AddPolygon(FormTriangle(v11, v7, v10));
   icosahedron->AddPolygon(FormTriangle(v7, v0, v10));
   icosahedron->AddPolygon(FormTriangle(v9, v4, v11));
   icosahedron->AddPolygon(FormTriangle(v9, v8, v4));
   icosahedron->AddPolygon(FormTriangle(v11, v5, v7));
   icosahedron->AddPolygon(FormTriangle(v11, v10, v9));
   icosahedron->CreateVertexBuffers(position_loc, normal_loc);
   return icosahedron;
}

/**
 * Construct dodecahedron
 */
TriSurface* ConstructDodecahedron(const int position_loc, const int normal_loc) {
   // Vertices of the icosahedron
   float t = 0.618f;
   Point3 iv0( 0.0f,  1.0f,  t);
   Point3 iv1( 0.0f,  1.0f, -t);
   Point3 iv2( 1.0f,   t,  0.0f);
   Point3 iv3( 1.0f,  -t,  0.0f);
   Point3 iv4( 0.0f, -1.0f,  -t);
   Point3 iv5( 0.0f, -1.0f,   t);
   Point3 iv6(  t, 0.0f,  1.0f);
   Point3 iv7( -t, 0.0f,  1.0f);
   Point3 iv8(  t, 0.0f,  -1.0f);
   Point3 iv9( -t, 0.0f,  -1.0f);
   Point3 iv10(-1.0f,  t,  0.0f);
   Point3 iv11(-1.0f, -t,  0.0f);

   // Vertices of the dodecahecdron are formed by the dual of the icosahedron
   Point3 v0  = dual(iv0, iv6, iv2);
   Point3 v1  = dual(iv6, iv3, iv2);
   Point3 v2  = dual(iv6, iv5, iv3);
   Point3 v3  = dual(iv7, iv5, iv6);
   Point3 v4  = dual(iv7, iv6, iv0);
   Point3 v5  = dual(iv2, iv3, iv8);
   Point3 v6  = dual(iv1, iv2, iv8);
   Point3 v7  = dual(iv0, iv2, iv1);
   Point3 v8  = dual(iv10, iv0, iv1);
   Point3 v9  = dual(iv10, iv1, iv9);
   Point3 v10 = dual(iv1, iv8, iv9);
   Point3 v11 = dual(iv3, iv4, iv8);
   Point3 v12 = dual(iv5, iv4, iv3);
   Point3 v13 = dual(iv11, iv4, iv5);
   Point3 v14 = dual(iv11, iv7, iv10);
   Point3 v15 = dual(iv7, iv0, iv10);
   Point3 v16 = dual(iv9, iv4, iv11);
   Point3 v17 = dual(iv9, iv8, iv4);
   Point3 v18 = dual(iv11, iv5, iv7);
   Point3 v19 = dual(iv11, iv10, iv9);

   // Form pentagonal faces
   TriSurface* dodecahedron = new TriSurface;
   dodecahedron->AddPolygon(FormPentagon(v0, v7, v8, v15, v4));
   dodecahedron->AddPolygon(FormPentagon(v6, v10, v9, v8, v7));
   dodecahedron->AddPolygon(FormPentagon(v5, v6, v7, v0, v1));
   dodecahedron->AddPolygon(FormPentagon(v12, v11, v5, v1, v2));
   dodecahedron->AddPolygon(FormPentagon(v13, v16, v17, v11, v12));
   dodecahedron->AddPolygon(FormPentagon(v2, v3, v18, v13, v12));
   dodecahedron->AddPolygon(FormPentagon(v1, v0, v4, v3, v2));
   dodecahedron->AddPolygon(FormPentagon(v3, v4, v15, v14, v18));
   dodecahedron->AddPolygon(FormPentagon(v11, v17, v10, v6, v5));
   dodecahedron->AddPolygon(FormPentagon(v16, v19, v9, v10, v17));
   dodecahedron->AddPolygon(FormPentagon(v15, v8, v9, v19, v14));
   dodecahedron->AddPolygon(FormPentagon(v18, v14, v19, v16, v13));
   dodecahedron->CreateVertexBuffers(position_loc, normal_loc);
   return dodecahedron;
}

/**
 * Form a pentagon for the buckyball
 */
std::vector<Point3> buckyPentagon(const Point3& v0, const Point3& v1, 
                                  const Point3& v2, const Point3& v3, 
                                  const Point3& v4, const Point3& v5) {
   std::vector<Point3> vlist;
   vlist.push_back(v0.AffineCombination(0.666f, 0.334f, v1));
   vlist.push_back(v0.AffineCombination(0.666f, 0.334f, v2));
   vlist.push_back(v0.AffineCombination(0.666f, 0.334f, v3));
   vlist.push_back(v0.AffineCombination(0.666f, 0.334f, v4));
   vlist.push_back(v0.AffineCombination(0.666f, 0.334f, v5));
   vlist.push_back(v0.AffineCombination(0.666f, 0.334f, v1));
   return vlist;
}

/**
 * Subdivide each edge into 3 equal pieces.  Connect to form a hexagon.
 */
std::vector<Point3> divideFace(const Point3& v0, const Point3& v1, const Point3& v2) {
   std::vector<Point3> vlist;
   vlist.push_back(v0.AffineCombination(0.666f, 0.334f, v1));
   vlist.push_back(v0.AffineCombination(0.334f, 0.666f, v1));
   vlist.push_back(v1.AffineCombination(0.666f, 0.334f, v2));
   vlist.push_back(v1.AffineCombination(0.334f, 0.666f, v2));
   vlist.push_back(v2.AffineCombination(0.666f, 0.334f, v0));
   vlist.push_back(v2.AffineCombination(0.334f, 0.666f, v0));
   return vlist;
}

/**
 * Construct buckyball
 */
TriSurface* ConstructBuckyball(const int position_loc, const int normal_loc) {
   // Vertices of the icosahedron
   float t = 0.618f;
   Point3 v0( 0.0f,  1.0f,  t);
   Point3 v1( 0.0f,  1.0f, -t);
   Point3 v2( 1.0f,   t,  0.0f);
   Point3 v3( 1.0f,  -t,  0.0f);
   Point3 v4( 0.0f, -1.0f,  -t);
   Point3 v5( 0.0f, -1.0f,   t);
   Point3 v6(  t, 0.0f,  1.0f);
   Point3 v7( -t, 0.0f,  1.0f);
   Point3 v8(  t, 0.0f,  -1.0f);
   Point3 v9( -t, 0.0f,  -1.0f);
   Point3 v10(-1.0f,  t,  0.0f);
   Point3 v11(-1.0f, -t,  0.0f);

   // Replace each triangular face with a hexagon
   TriSurface* buckyball = new TriSurface;
   buckyball->AddPolygon(divideFace(v0, v6, v2));
   buckyball->AddPolygon(divideFace(v6, v3, v2));
   buckyball->AddPolygon(divideFace(v6, v5, v3));
   buckyball->AddPolygon(divideFace(v7, v5, v6));
   buckyball->AddPolygon(divideFace(v7, v6, v0));
   buckyball->AddPolygon(divideFace(v2, v3, v8));
   buckyball->AddPolygon(divideFace(v1, v2, v8));
   buckyball->AddPolygon(divideFace(v0, v2, v1));
   buckyball->AddPolygon(divideFace(v10, v0, v1));
   buckyball->AddPolygon(divideFace(v10, v1, v9));
   buckyball->AddPolygon(divideFace(v1, v8, v9));
   buckyball->AddPolygon(divideFace(v3, v4, v8));
   buckyball->AddPolygon(divideFace(v5, v4, v3));
   buckyball->AddPolygon(divideFace(v11, v4, v5));
   buckyball->AddPolygon(divideFace(v11, v7, v10));
   buckyball->AddPolygon(divideFace(v7, v0, v10));
   buckyball->AddPolygon(divideFace(v9, v4, v11));
   buckyball->AddPolygon(divideFace(v9, v8, v4));
   buckyball->AddPolygon(divideFace(v11, v5, v7));
   buckyball->AddPolygon(divideFace(v11, v10, v9));

   // Add pentagon surfaces
   buckyball->AddPolygon(buckyPentagon(v0, v6, v2, v1, v10, v7));
   buckyball->AddPolygon(buckyPentagon(v1, v2, v8, v9, v10, v0));
   buckyball->AddPolygon(buckyPentagon(v2, v3, v8, v1, v0, v6));
   buckyball->AddPolygon(buckyPentagon(v3, v5, v4, v8, v2, v6));
   buckyball->AddPolygon(buckyPentagon(v4, v3, v5, v11, v9, v8));
   buckyball->AddPolygon(buckyPentagon(v5, v6, v7, v11, v4, v3));
   buckyball->AddPolygon(buckyPentagon(v6, v3, v2, v0, v7, v5));
   buckyball->AddPolygon(buckyPentagon(v7, v11, v5, v6, v0, v10));
   buckyball->AddPolygon(buckyPentagon(v8, v4, v9, v1, v2, v3));
   buckyball->AddPolygon(buckyPentagon(v9, v8, v4, v11, v10, v1));
   buckyball->AddPolygon(buckyPentagon(v10, v0, v1, v9, v11, v7));
   buckyball->AddPolygon(buckyPentagon(v11, v5, v7, v10, v9, v4));
   buckyball->CreateVertexBuffers(position_loc, normal_loc);
   return buckyball;
}

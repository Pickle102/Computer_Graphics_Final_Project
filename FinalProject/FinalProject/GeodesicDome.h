//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	David W. Nesbitt
//	File:    GeodesicDome.h
//	Purpose: Scene graph geometry node representing a geodesic dome.
//
//============================================================================

#ifndef __GEODESICDOME_H
#define __GEODESICDOME_H

#include "geometry/geometry.h"

class GeodesicDome : public TriSurface
{
public:
  /**
   * Creates a scene node representation of a geodesic dome.
   */
  GeodesicDome(const int position_loc, const int normal_loc) {
    // Vertices
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
    Point3 v9 ( -t, 0.0f,  -1.0f);
    Point3 v10(-1.0f,  t,  0.0f);
    Point3 v11(-1.0f, -t,  0.0f);

    // Triangle faces
    SubdivideFace(v0, v6, v2);
    SubdivideFace(v6, v3, v2);
    SubdivideFace(v6, v5, v3);
    SubdivideFace(v7, v5, v6);
    SubdivideFace(v7, v6, v0);
    SubdivideFace(v2, v3, v8);
    SubdivideFace(v1, v2, v8);
    SubdivideFace(v0, v2, v1);
    SubdivideFace(v10, v0, v1);
    SubdivideFace(v10, v1, v9);
    SubdivideFace(v1, v8, v9);
    SubdivideFace(v3, v4, v8);
    SubdivideFace(v5, v4, v3);
    SubdivideFace(v11, v4, v5);
    SubdivideFace(v11, v7, v10);
    SubdivideFace(v7, v0, v10);
    SubdivideFace(v9, v4, v11);
    SubdivideFace(v9, v8, v4);
    SubdivideFace(v11, v5, v7);
    SubdivideFace(v11, v10, v9);

    CreateVertexBuffers(position_loc, normal_loc);
  }
	
private:
  // Subdivide a face of the geodesic dome
  void SubdivideFace(const Point3& v0_in, const Point3& v1_in, const Point3& v2_in) {
    Point3 v0 = v0_in;
    Point3 v1 = v1_in;
    Point3 v2 = v2_in;
    Point3 v3 = v0.AffineCombination(0.666f, 0.334f, v1);
    Point3 v4 = v0.AffineCombination(0.334f, 0.666f, v1);
    Point3 v5 = v0.AffineCombination(0.666f, 0.334f, v2);
    Point3 v6 = v0.AffineCombination(0.334f, 0.666f, v2);
    Point3 v7 = v1.AffineCombination(0.666f, 0.334f, v2);
    Point3 v8 = v1.AffineCombination(0.334f, 0.666f, v2);
    Point3 v9 = Point3((v0.x + v1.x + v2.x) * 0.333333f,
                       (v0.y + v1.y + v2.y) * 0.333333f,
                       (v0.z + v1.z + v2.z) * 0.333333f);

    Normalize(v0);
    Normalize(v1);
    Normalize(v2);
    Normalize(v3);
    Normalize(v4);
    Normalize(v5);
    Normalize(v6);
    Normalize(v7);
    Normalize(v8);
    Normalize(v9);

    // Add faces
    AddPolygon(FormTriangle(v0, v3, v5));
    AddPolygon(FormTriangle(v3, v4, v9));
    AddPolygon(FormTriangle(v3, v9, v5));
    AddPolygon(FormTriangle(v5, v9, v6));
    AddPolygon(FormTriangle(v4, v1, v7));
    AddPolygon(FormTriangle(v4, v7, v9));
    AddPolygon(FormTriangle(v9, v7, v8));
    AddPolygon(FormTriangle(v9, v8, v6));
    AddPolygon(FormTriangle(v6, v8, v2));
  }

  // Place a point on a unit sphere
  void Normalize(Point3& p) {
    float inv = FastInvSqrt(p.x * p.x  + p.y * p.y + p.z * p.z);
    p.x *= inv;
    p.y *= inv;
    p.z *= inv;
  }
};

#endif
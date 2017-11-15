//============================================================================
//	Johns Hopkins University Engineering for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:  David W. Nesbitt
//	File:    geometry.h
//	Purpose: Geometric types used in the lab.  Includes
//			   coordinate and vector structures.
//          Student should include "geometry.h" to get all 
//          class definitions included in proper order.
//
//============================================================================

#ifndef __GEOMETRY_H__
#define __GEOMETRY_H__

#include <stdint.h>
#include <vector>

// TODO - Visual Studio 2013 does not support constexpr
#ifdef _WIN32
const float kPi = 3.14159265f;
const float kEpsilon   = 0.000001f;
const float kDegPerRad = (180.0f / kPi);  // Radians to degrees conversion
const float kRadPerDeg = (kPi / 180.0f);  // Degrees to radians conversion
#else
constexpr float kPi        = 3.14159265f;
constexpr float kEpsilon   = 0.000001f;
constexpr float kDegPerRad = (180.0f / kPi);  // Radians to degrees conversion
constexpr float kRadPerDeg = (kPi / 180.0f);  // Degrees to radians conversion
#endif

/**
 * Degrees to radians conversion
 * @param   d   Angle in degrees.
 * @return  Returns the angle in radians.
 */
inline float DegreesToRadians(const float d) {
  return d * kRadPerDeg;
}

/**
 * Radians to degrees conversion
 * @param   r   Angle in radians.
 * @return  Returns the angle in degrees.
 */
inline float RadiansToDegrees(const float r) {
  return r * kDegPerRad;
}

/**
 * Get a random number between 0 and 1.
 * TODO - use new C++ 11 random number methods
 * return  Returns a random floating point number betwen 0 and 1.
 */
inline float rand01() {
  return (float)rand() / (float)RAND_MAX;
}

/** 
 * Fast inverse sqrt method. Originally used in Quake III 
 * @param  x  Value to find inverse sqrt for
 * @return  Returns 1/sqrtf(x)
 */
inline float FastInvSqrt(float x) {
   float xhalf = 0.5f * x;
   int i = *(int*)&x;            // get bits for floating value
   i = 0x5f3759df - (i>>1);      // give initial guess y0
   x = *(float*)&i;              // convert bits back to float
   return x*(1.5f - xhalf*x*x);  // newton step
   // x *= 1.5f - xhalf*x*x;     // repeating step increases accuracy
}

// Include individual geometry files
#include "geometry/hpoint2.h"
#include "geometry/point2.h"
#include "geometry/hpoint3.h"
#include "geometry/point3.h"
#include "geometry/vector2.h"
#include "geometry/vector3.h"
#include "geometry/segment2.h"
#include "geometry/segment3.h"
#include "geometry/plane.h"
#include "geometry/aabb.h"
#include "geometry/boundingsphere.h"
#include "geometry/ray3.h"
#include "geometry/noise.h"
#include "geometry/matrix.h"

/**
 * Structure to hold a vertex position and normal
 */
struct VertexAndNormal {
  Point3  vertex;
  Vector3 normal;
	
	VertexAndNormal() {
    vertex.Set(0.0f, 0.0f, 0.0f);
    normal.Set(0.0f, 0.0f, 0.0f);
	}
	VertexAndNormal(const Point3& v) {
    vertex = v;
    normal.Set(0.0f, 0.0f, 0.0f);
	}
};

/**
* Structure to hold a vertex position, normal, and texture coordinate
*/
struct PNTVertex {
  Point3  vertex;
  Vector3 normal;
  float   s, t;

  PNTVertex() :
    vertex({ 0.0f, 0.0f, 0.0f }),
    normal({ 0.0f, 0.0f, 0.0f }),
    s(0.0f),
    t(0.0f) {
  }
  PNTVertex(const Point3& v) :
    vertex(v),
    normal({ 0.0f, 0.0f, 0.0f }),
    s(0.0f),
    t(0.0f) {
  }
  PNTVertex(const Point3& v, const Vector3& n, float s_in, float t_in) :
    vertex(v),
    normal(n),
    s(s_in),
    t(t_in) {
  }
};

// Define the operators in Point2 that allow a vector to be added
// to and subtracted from a point. Descriptions are in the Point2
// structure.
inline Point2 Point2::operator + (const Vector2& w) const {
  return Point2(x + w.x, y + w.y);
}
inline Point2 Point2::operator - (const Vector2 &w) const {
  return Point2(x - w.x, y - w.y);
}
inline Vector2 Point2::operator - (const Point2& p) const {
  return Vector2(x - p.x, y - p.y);
}

/**
 * Overloading: allows float * Vector2
 */
inline Vector2 operator *(float s, const Vector2 &v) {
  return Vector2(v.x * s, v.y * s);
}

// Define the operators in Point3 that allow a vector to be added
// to and subtracted from a point. Descriptions are in the Point3
// structure.
inline Point3 Point3::operator + (const Vector3& w) const {
  return Point3(x + w.x, y + w.y, z + w.z);
}
inline Point3 Point3::operator - (const Vector3 &w) const {
  return Point3(x - w.x, y - w.y, z - w.z);
}
inline Vector3 Point3::operator - (const Point3& p) const {
  return Vector3(x - p.x, y - p.y, z - p.z);
}

/**
 * Overloading: allows float * Vector3
 */
inline Vector3 operator *(float s, const Vector3 &v) {
  return Vector3(v.x * s, v.y * s, v.z * s);
}

// Convert homogeneous point to a cartesian representation
inline Point3 HPoint3::ToCartesian() const {
  if (w == 1.0f) {
    return Point3(x, y, z);
  } else {
    // Perform division through by w
    float d = (fabs(w) > kEpsilon) ? (1.0f / w) : 1.0f;
    return Point3(x * d, y * d, z * d);
  }
}

// Convert homogeneous point to a cartesian representation
inline Point2 HPoint2::ToCartesian() const {
  if (w == 1.0f) {
    return Point2(x, y);
  } else {
    // Perform division through by w
    float d = (fabs(w) > kEpsilon) ? (1.0f / w) : 1.0f;
    return Point2(x * d, y * d);
  }
}

/**
 * Test if a point is inside a 3D polygon. Uses the normal to the
 * polygon to project to one of the 2D axis planes and solves in that
 * space. This method assumes that the point is on the plane of the 
 * polygon (generally found by intersecting a ray with a plane). 
 * @param   polygon  Polygon to test
 * @param   n        Normal to the polygon
 * @return  Returns true if the point is inside the polygon, false if not.
*/
inline bool Point3::IsInPolygon(const std::vector<Point3>& polygon, 
                                const Vector3& n) const {
  if (fabs(n.x) >= fabs(n.y) && fabs(n.x) >= fabs(n.z))
    return IsInPolygonYZ(polygon);   // Drop the x component
  else if (fabs(n.y) >= fabs(n.x) && fabs(n.y) >= fabs(n.z))
    return IsInPolygonXZ(polygon);   // Drop the y component
  else
    return IsInPolygonXY(polygon);   // Drop the z component
}

#endif

//============================================================================
//	Johns Hopkins University Whiting School of Engineering
//	605.461 Principles of Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:  David W. Nesbitt
//	File:    VectorTest.cpp
//	Purpose: Test the vector library.
//
//============================================================================

#include <stdio.h>
#include <stdarg.h>

#include "geometry/geometry.h"

// Simple logging function
void logmsg(const char *message, ...) {
  // Open file if not already opened
  static FILE *lfile = NULL;
  if (lfile == NULL) {
    lfile = fopen("VectorTest.log", "w");
  }

  va_list arg;
  va_start(arg, message);
  vfprintf(lfile, message, arg);
  putc('\n', lfile);
  fflush(lfile);
  va_end(arg);
}

/**
 * Main method. Entry point for test application.
 */
int main(int argc, char* argv[]) {
  logmsg("Vector Test");

  // ------------------ 2D Point and Vector Tests --------------------/
  logmsg("\n2-D Point and Vector Operations\n");

  // Construct some 2D points
  Point2 a( 5.0f,  0.0f);
  Point2 b(-2.0f,  4.0f);
  Point2 c( 1.0f, -5.0f);
  logmsg("Point a (%.2f, %.2f)", a.x, a.y);
  logmsg("Point b (%.2f, %.2f)", b.x, b.y);
  logmsg("Point c (%.2f, %.2f)", c.x, c.y);

  // Midpoint between a and b
  Point2 m = a.MidPoint(b);
  logmsg("Mid point between a and b is (%.2f, %.2f)", m.x, m.y);

  // Point 1/4 of the way from a to b
  Point2 q = a.AffineCombination(0.75f, 0.25f, b);
  logmsg("Point q (1/4 between a and b) is (%.2f, %.2f)", q.x, q.y);

  // Construct a vector from point a to point b
  Vector2 v(a, b);
  logmsg("Vector v (%.2f, %.2f)", v.x, v.y);

  // Get the length of v
  logmsg("Norm of v = %.2f", v.Norm());

  // Create a unit vector from v
  Vector2 u = v.Normalize();
  logmsg("Unit vector of v = (%.2f, %.2f)", u.x, u.y);
  logmsg("Note: v is now a unit vector (result of Normalize())");

  // Create a vector w from point b to point c
  Vector2 w = c - b;
  logmsg("Vector w (%.2f, %.2f)", w.x, w.y);
  w.Normalize();
  logmsg("Unit vector of w = (%.2f, %.2f)", w.x, w.y);

  // Create vectors t0 and t1 that are in the opposite direction of w 
  // and have a length of 4. (Tests both * operators)
  Vector2 t0 = w * -4.0f;
  logmsg("Vector t0 (%.2f, %.2f)", t0.x, t0.y);
  Vector2 t1 = -4.0f * w;
  logmsg("Vector t1 (%.2f, %.2f)", t1.x, t1.y);

  // Add vector v and vector w
  Vector2 wv = v + w;
  logmsg("Vector v + w = (%.2f, %.2f)", wv.x, wv.y);

  // Dot product of v and w
  float dot = v.Dot(w);
  logmsg("Dot Product of v and w is %f", dot);

  // Find the angle in degrees between v and w
  logmsg("Angle between v and w = %.2f degrees", RadiansToDegrees(v.AngleBetween(w)));

  // Add vector t0 to point c
  Point2 d = c + t0;
  logmsg("Point d: (%.2f %.2f)", d.x, d.y);

  // Find the projection of vector w onto vector v
  Vector2 wOnv = w.Projection(v);
  logmsg("Projection of w onto v is (%.2f %.2f)", wOnv.x, wOnv.y);

  // Find perpendicular 2-D vectors
  Vector2 v2p1 = wOnv.GetPerpendicular(true);
  logmsg("Clockwise perpendicular to wOnV is (%.2f %.2f)", v2p1.x, v2p1.y);
  Vector2 v2p2 = wOnv.GetPerpendicular(false);
  logmsg("Counter-Clockwise perpendicular to wOnV is (%.2f %.2f)", v2p2.x, v2p2.y);

  // Find the reflection of vector t0 off the x axis
  Vector2 n1(0.0f, 1.0f);
  Vector2 r = t0.Reflect(n1);
  logmsg("Reflection of t0 off the x-axis is %f %f", r.x, r.y);

  // ------------------ 3-D Point and Vector Tests --------------------/
  logmsg("\n3-D Point and Vector Operations\n");

  // Construct three points 
  Point3 a3( 0.0f,  -2.0f,  0.0f);
  Point3 b3(-4.0f,  2.0f,  5.0f);
  Point3 c3( 2.0f, -1.0f, -3.0f);
  logmsg("Point a3 (%.2f, %.2f, %.2f)", a3.x, a3.y, a3.z);
  logmsg("Point b3 (%.2f, %.2f, %.2f)", b3.x, b3.y, b3.z);
  logmsg("Point c3 (%.2f, %.2f, %.2f)", c3.x, c3.y, c3.z);

  // Midpoint between a3 and b3
  Point3 mid_pt = a3.MidPoint(b3);
  logmsg("Mid point between a3 and b3 is (%.2f, %.2f, %.2f)", mid_pt.x, mid_pt.y, mid_pt.z);

  // Point 1/4 of the way from a3 to b3
  Point3 q3 = a3.AffineCombination(0.75f, 0.25f, b3);
  logmsg("Point q3 (1/4 between a3 and b3) is (%.2f, %.2f, %.2f)", q3.x, q3.y, q3.z);

  // Construct a vector from point a3 to point b3
  Vector3 v3(a3, b3);
  logmsg("Vector v3 (%.2f, %.2f, %.2f)", v3.x, v3.y, v3.z);

  // Get the length of v3
  logmsg("Norm of v = %.2f", v3.Norm());

  // Create a unit vector from v2
  Vector3 u3 = v3.Normalize();
  logmsg("Unit vector of v3 = (%.2f, %.2f, %.2f)", u3.x, u3.y, u3.z);
  logmsg("Note: v3 is now a unit vector (result of Normalize())");

  // Create a vector t that is in the opposite direction of u 
  // and has a length of 10. (Tests both * operators)
  Vector3 t3 = u3 * -10.0f;
  logmsg("Vector t3 (%.2f, %.2f, %.2f)", t3.x, t3.y, t3.z);

  Vector3 t31 = -10.0f * u3;
  logmsg("Vector t31 (%.2f, %.2f, %.2f)", t31.x, t31.y, t31.z);

  // Create a vector from Point a3 to point c3
  Vector3 w3 = c3 - a3;
  logmsg("Vector w3 (%.2f, %.2f, %.2f)", w3.x, w3.y, w3.z);

  // Add vector v and vector w
  Vector3 wv3 = v3 + w3;
  logmsg("Vector v3 + w3 = (%.2f, %.2f, %.2f)", wv3.x, wv3.y, wv3.z);

  // Dot product of v3 and w3
  float dot3 = v3.Dot(w3);
  logmsg("Dot Product of v3 and w3 is %f", dot3);

  // Find the angle in degrees between v and w
  logmsg("Angle between v3 and w3 = %.2f degrees", RadiansToDegrees(v3.AngleBetween(w3)));

  // Add vector t to point c
  Point3 d3 = c3 + t3;
  logmsg("Point d3: (%.2f %.2f %.2f)", d3.x, d3.y, d3.z);

  // Find the projection of vector w onto vector v
  Vector3 w3Onv3 = w3.Projection(v3);
  logmsg("Projection of w3 onto v3 is (%.2f %.2f %.2f)", w3Onv3.x, w3Onv3.y, w3Onv3.z);

  // ------------------- Line segment tests ------------------------//

  logmsg("\n2-D Line Segment Operations\n");

  // Construct a line segment from point a2 to point b2
  Point2 a2(5.0f, 3.0f);
  Point2 b2(-3.0f, 10.0f);
  LineSegment2 seg2(a2, b2);

  // Find the distance of point c2a from line segment seg2
  Point2 c2a(-5.0f, 12.0f);
  Point2 closest2;
  float dist = seg2.Distance(c2a, closest2);
  logmsg("Distance of point c2a from line segment ab is %.2f", dist);
  logmsg("Closest Point is (%.2f, %.2f, %.2f)", closest2.x, closest2.y);

  // Find the distance of point c2b from line segment seg2
  Point2 c2b(3.0f, 0.0f);
  dist = seg2.Distance(c2b, closest2);
  logmsg("Distance of point c2b from line segment ab is %.2f", dist);
  logmsg("Closest Point is (%.2f, %.2f, %.2f)", closest2.x, closest2.y);

  // Find the distance of point c2c from line segment seg2
  Point2 c2c(12.0f, 1.0f);
  dist = seg2.Distance(c2c, closest2);
  logmsg("Distance of point c2c from line segment ab is %.2f", dist);
  logmsg("Closest Point is (%.2f, %.2f, %.2f)", closest2.x, closest2.y);

  // Construct other 2-D line segments and test for intersection
  Point2 intersect_pt;
  LineSegment2 seg2b({ -10.0f, 12.0f }, { 4.0f, 11.0f });
  if (seg2b.Intersect(seg2, intersect_pt)) {
    logmsg("seg2b intersects seg2 at point: (%.2f, %.2f, %.2f)", intersect_pt.x, intersect_pt.y);
  }
  else {
    logmsg("seg2b does not intersect seg2");
  }
  LineSegment2 seg2c({ -3.0f, 8.0f }, { 5.0f, 1.0f });
  if (seg2c.Intersect(seg2, intersect_pt)) {
    logmsg("seg2c intersects seg2 at point: (%.2f, %.2f, %.2f)", intersect_pt.x, intersect_pt.y);
  }
  else {
    logmsg("seg2c does not intersect seg2");
  }
  LineSegment2 seg2d({ 0.0f, 0.0f }, { 7.0f, 7.0f });
  if (seg2d.Intersect(seg2, intersect_pt)) {
    logmsg("seg2d intersects seg2 at point: (%.2f, %.2f, %.2f)", intersect_pt.x, intersect_pt.y);
  }
  else {
    logmsg("seg2d does not intersect seg2");
  }
 
  // ------------ Line segment 3-D tests ---------------//
  logmsg("\n3-D Line Segment Operations\n");

  // Construct a line segment in 3-D
  LineSegment3 seg3({ 2.0f, 4.0f, -5.0f }, { -5.0f, 1.0f, 6.0f });

  // Find the distance of point c3a from line segment seg3
  Point3 closest3;
  Point3 c3a(12.0f, 5.0f, 10.0f);
  dist = seg3.Distance(c3a, closest3);
  logmsg("Distance of point c3a from line segment seg3 is %.2f", dist);
  logmsg("Closest Point is (%.2f, %.2f, %.2f)", closest3.x, closest3.y, closest3.z);

  // Find the distance of point c3b from line segment seg3
  Point3 c3b(5.0f, 2.0f, 0.0f);
  dist = seg3.Distance(c3b, closest3);
  logmsg("Distance of point c3b from line segment seg3 is %.2f", dist);
  logmsg("Closest Point is (%.2f, %.2f, %.2f)", closest3.x, closest3.y, closest3.z);

  // -------------------------- Plane tests ------------------- //

  // Construct 3 points (not all in x,y plane!)
  Point3 m3(-5.0f, -3.0f, 5.0f);
  Point3 n3(3.0f, -5.0f, 1.0f);
  Point3 o3(4.0f, 4.0f, -3.0f);
  logmsg("\nPlane Test\n");
  logmsg("Point m3 (%.2f, %.2f, %.2f)", m3.x, m3.y, m3.z);
  logmsg("Point n3 (%.2f, %.2f, %.2f)", n3.x, n3.y, n3.z);
  logmsg("Point o3 (%.2f, %.2f, %.2f)", o3.x, o3.y, o3.z);

  // Get the cross product of vector mn and mo
  Vector3 perp = (n3 - m3).Cross(o3 - m3);
  logmsg("Cross Product of mn and mo = %.2f %.2f %.2f", perp.x, perp.y, perp.z);

  // Construct a plane through m3,n1,o3
  Plane plane(m3, n3, o3);
  logmsg("Plane through points m3,n3,o3 = %.2f %.2f %.2f %.2f",
    plane.a, plane.b, plane.c, plane.d);

  plane.Normalize();
  logmsg("Normalized Plane = %.2f %.2f %.2f %.2f", plane.a, plane.b, plane.c, plane.d);

  // Construct plane with point m3 and perp
  Plane plane2(m3, perp);
  plane2.Normalize();
  logmsg("Normalized Plane with point m and normal perp = %.2f %.2f %.2f %.2f",
          plane.a, plane.b, plane.c, plane.d);

  // Check if point n3 is on the plane
  float s = plane.Solve(n3);
  if (s > kEpsilon)
    logmsg("Point n3 is in front of the plane: s = %.2f", s);
  else if (s < -kEpsilon)
    logmsg("Point n3 is behind the plane: s = %.2f", s);
  else
    logmsg("Point n3 is on the plane");

  // Check point k against the plane
  Point3 k(5.0, 2.0, 7.0f);
  logmsg("Point k (%.2f, %.2f, %.2f)", k.x, k.y, k.z);
  s = plane.Solve(k);
  if (s > kEpsilon)
    logmsg("Point k is in front of the plane: s = %.2f", s);
  else if (s < -kEpsilon)
    logmsg("Point k is behind the plane: s = %.2f", s);
  else
    logmsg("Point k is on the plane");

  // Construct a vector from between 2 points
  Vector3 v3a({ 0.0f, -2.0f, 0.0f }, { -4.0f, 2.0f, 5.0f });
  logmsg("Vector v3a (%.2f, %.2f, %.2f)", v3.x, v3.y, v3.z);

  // Reflect vector v3 off the plane
  Vector3 r3 = v3a.Reflect(plane.GetNormal());
  logmsg("Reflection of v3a off the plane = %.2f, %.2f, %.2f", r3.x, r3.y, r3.z);

  // --------------------- Student Written Expressions ------------------- //
  logmsg("\nStudent Written Expressions");

  // Write an expression point and vector operations (not using AffineCombination or MidPoint)
  // to find the point (m34) that is 3/4 of the distance along a line between
  // m3 = (1.0f, 2.0f, 3.0f) and m2 = (5.0f, -5.0f, 0.0f) 
  Point3 m1(1.0f, 2.0f, 3.0f);
  Point3 m2(5.0f, -5.0f, 0.0f);
  Point3 m34 = m1 + (m2 - m1) * 0.75f;
  logmsg("m34 = (%.2f %.2f %.2f)", m34.x, m34.y, m34.z);


  // ---------- Ray to Sphere Intersection Tests (Module 5) ----------------//

  // Create a bounding sphere centered at (2,2,0) with radius 4.0f
  logmsg("\nRay Tests\n");
  Point3 origin(2.0f, 2.0f, 0.0f);
  BoundingSphere sphere(origin, 4.0f);

  // Create a unit length ray starting at (-10, 10, 0) in the +x direction and test intersection
  Ray3 ray1(Point3(-10.0f, 10.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f));
  float t = ray1.Intersect(sphere);
  if (t != 0.0f) {
    Point3 intersect = ray1.Intersect(t);
    logmsg("   Ray1 intersects sphere at %f %f %f, t = %f", intersect.x, intersect.y, intersect.z, t);
  }
  else {
    logmsg("   Ray1 does not intersect sphere");
  }

  // Create a unit length ray starting at (4, 8, 0) in the +x direction and test intersection
  Ray3 ray2(Point3(4.0f, 8.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f));
  t = ray2.Intersect(sphere);
  if (t != 0.0f) {
    Point3 intersect = ray2.Intersect(t);
    logmsg("   Ray2 intersects sphere at %f %f %f, t = %f", intersect.x, intersect.y, intersect.z, t);
  }
  else {
    logmsg("   Ray2 does not intersect sphere");
  }

  // Create a unit length ray starting at (-10, 4, 0) in the +x direction and test intersection
  Ray3 ray3(Point3(-10.0f, 4.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f));
  t = ray3.Intersect(sphere);
  if (t != 0.0f) {
    Point3 intersect = ray3.Intersect(t);
    logmsg("   Ray3 intersects sphere at %f %f %f, t = %f", intersect.x, intersect.y, intersect.z, t);
  }
  else {
    logmsg("   Ray3 does not intersect sphere");
  }

  // Create a unit length ray starting at (4, 4, 0) in the +y direction and test intersection

  Ray3 ray4(Point3(4.0f, 4.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
  t = ray4.Intersect(sphere);
  if (t != 0.0f) {
    Point3 intersect = ray4.Intersect(t);
    logmsg("   Ray4 intersects sphere at %f %f %f, t = %f", intersect.x, intersect.y, intersect.z, t);
  }
  else {
    logmsg("   Ray4 does not intersect sphere");
  }
  return 1;
}
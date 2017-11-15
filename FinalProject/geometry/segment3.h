//============================================================================
//	Johns Hopkins University Engineering for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:  David W. Nesbitt
//	File:    Segment.h
//	Purpose: 3D Line segment class.
//          Student should  include "geometry.h" to get all class definitions
//          included in proper order.
//
//============================================================================

#ifndef __SEGMENT3_H__
#define __SEGMENT3_H__

#include <vector>

/**
 * Line segment in 3-D. Data members are the 2 endpoints of the segment: a and b.
 */
struct LineSegment3 {
  Point3 a;
  Point3 b;

  /**
   * Constructor given 2 points.
   * @param   p1    First point of the segment.
   * @param   p2    Second point of the segment.
   */
  LineSegment3(const Point3& p1, const Point3& p2)
    : a(p1),
      b(p2) {
  }

  /**
   * Finds the distance of a specified point from the line segment 
   * and the closest point on the segement to the specified point.
   * @param   p         Test point.
   * @param   closest   (Return) Closest point on the segment to point p.
   * @return  Returns the distance from p to the closest point on
   *          the segment.
   */
  float Distance(const Point3& p, Point3& closest) const {
    // Construct vectors v (ab) and w (ap)
    Vector3 v(a, b);
    Vector3 w(a, p);

    // Numerator of the component of w onto v. If <= 0 then a 
    // is the closest point. By separating into the numerator
    // and denominator of the component we avoid a division unless
    // it is necessary.
    float n = w.Dot(v);
    if (n <= 0.0f) {
      closest = a;
      return w.Norm();
    }

    // Get the denominator of the component.  If the component >= 1
    // (d <= n) then point b is the closest point
    float d = v.Dot(v);
    if (d <= n) {
      closest = b;
      return Vector3(b, p).Norm();
    }

    // Closest point is along the segment. The point is the projection of
    // w onto v.
    closest = a + v * (n / d);
    return Vector3(closest, p).Norm();
  }
};

#endif

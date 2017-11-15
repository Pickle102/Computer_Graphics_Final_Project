//============================================================================
//	Johns Hopkins University Engineering for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:  David W. Nesbitt
//	File:    Segment.h
//	Purpose: 2D Line segment class. Includes clipping code. 
//          Student should  include "geometry.h" to get all class definitions
//          included in proper order.
//
//============================================================================

#ifndef __SEGMENT2_H__
#define __SEGMENT2_H__

#include <vector>

/**
 * Rectangle. 
 * TODO - add namespace so we don't conflict with Windows Rectangle
 */
struct CRectangle {
  float left;
  float right;
  float bottom;
  float top;
};

// Clipping code words for Cohen-Sutherland line segment clipping
union ClipCode {
  uint8_t code;
  struct {
    uint8_t left : 1;
    uint8_t right : 1;
    uint8_t bottom : 1;
    uint8_t top : 1;
  } m_bits;

  // Constructor. Set code to 0;
  ClipCode(void) : code(0) { }
};

/**
 * Line segment in 2D. Data members are the 2 endpoints of the segment: a and b.
 */
struct LineSegment2 {
  Point2 a;
  Point2 b;

  /**
  * Default constructor.
  */
  LineSegment2()
    : a{ 0.0f, 0.0f },
      b{ 0.0f, 0.0f } {
  }

  /**
   * Constructor given 2 points.
   * @param   p1    First point of the segment.
   * @param   p2    Second point of the segment.
   */
  LineSegment2(const Point2& p1, const Point2& p2) 
    : a(p1),
      b(p2) {
  }

  /**
   * Finds the distance of a specified point from the line segment 
   * and the closest point on the segement to the specified point.
   * @param   p        Test point.
   * @param   closest  (Return) Closest point on the segment to point p.
   * @return  Returns the distance from p to the closest point on
   *          the segment.
   */
  float Distance(const Point2& p, Point2& closest) const {
    // Construct vectors v (ab) and w (ap)
    Vector2 v(a, b);
    Vector2 w(a, p);

    // Numerator of the component of w onto v. If <= 0 then a 
    // is the closest point. By separat_ing into the numerator
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
      return Vector2(b, p).Norm();
    }

    // Closest point is along the segment. The point is the projection of
    // w onto v.
    closest = a + v * (n / d);
    return Vector2(closest, p).Norm();
  }

  /**
   * Determines if the current segment intersects the specified segment.
   * If an intersect occurs the intersect_pt is determined.  Note: the
   * case where the lines overlap is not considered. Consider any parallel
   * line segment case to be no intersect (return false).
   * @param   segment        Segment to determine intersection with.
   * @param   intersect_pt   (OUT) Intersection point.
   * @return   Returns true if an intersection exists, false if not.
   */
  bool Intersect(const LineSegment2& segment, Point2& intersect_pt) {
    // Construct vectors
    Vector2 v = b - a;
    Vector2 w = segment.b - segment.a;

    // Set 2D perpendicular vector to w
    Vector2 wp(w.y, -w.x);

    // Check if denominator will be 0 (lines are parallel)
    float wp_dot_v = wp.Dot(v);
    if (wp_dot_v == 0.0f) {
      return false;
    }

    // Solve for the parameter t. Early exit if no intersect.
    Vector2 c = segment.a - a;
    float t = wp.Dot(c) / wp_dot_v;
    if (t < 0.0f || t > 1.0f) {
      return false;
    }

    // Solve for the parameter u
    Vector2 vp(v.y, -v.x);
    float u = vp.Dot(c) / wp_dot_v;
    if (u < 0.0f || u > 1.0f) {
      return false;
    }

    // An intersect occurs.  Set the intersect point and return true
    intersect_pt = a + v * t;
    return true;
  }

  /**
  * Clips the line segment to a specified convex polygon.
  * @param  poly  A counter-clockwise oriented polygon.
  * @param  clip_segment   (OUT) the clipped segment.
  * @return  Returns true if any part of hte segment remains.
  */
  bool ClipToPolygon(const std::vector<Point2>& poly, 
                     LineSegment2& clip_segment) const {
    // Initialize the candidate interval
    float t_out = 1.0f;
    float t_in = 0.0f;

    // Ray for this line segment
    Vector2 c = b - a;

    float t_hit;
    Vector2 n;
    Vector2 w;
    auto pt1 = poly.end() - 1;
    auto pt2 = poly.begin();
    for (; pt2 != poly.end(); pt1 = pt2, pt2++) {
      // Set an outward facing normal (polygon is assumed to be CCW)
      n.Set(pt2->y - pt1->y, pt1->x - pt2->x);

      // Get the dot product of the normal to this polygon edge
      // with the ray
      float n_dot_c = n.Dot(c);

      // Check for parallel line
      if (std::abs(n_dot_c) < kEpsilon) {
        continue;
      }

      w = *pt1 - a;
      t_hit = n.Dot(w) / n_dot_c;

      // Ray is exit_ing P
      if (n_dot_c > 0)
        t_out = (t_out < t_hit) ? t_out : t_hit;
      else
        t_in = (t_in > t_hit) ? t_in : t_hit;

      // Early out
      if (t_in > t_out) {
        // Set clip segment to 0s
        clip_segment.a.Set(0.0f, 0.0f);
        clip_segment.b.Set(0.0f, 0.0f);
        return false;
      }
    }

    // If candidate interval is not empty then set the clip segment
    clip_segment.a = a + c * t_in;
    clip_segment.b = a + c * t_out;
    return true;
  }

  /**
  * Clips the line segment to a specified rectangle.
  * @param  poly  A counter-clockwise oriented polygon.
  * @returns   Returns the clipped segment.
  */
  bool ClipToRectangle(const CRectangle& r, LineSegment2& clip) const {
    ClipCode p1, p2;
    clip.a = a;
    clip.b = b;
    while (true) {
      // Form code words for the two points      
      p1.m_bits.left = (clip.a.x < r.left);
      p1.m_bits.right = (clip.a.x > r.right);
      p1.m_bits.bottom = (clip.a.y < r.bottom);
      p1.m_bits.top = (clip.a.y > r.top);
      p2.m_bits.left = (clip.b.x < r.left);
      p2.m_bits.right = (clip.b.x > r.right);
      p2.m_bits.bottom = (clip.b.y < r.bottom);
      p2.m_bits.top = (clip.b.y > r.top);

      // Trivial accept - both points are inside, done
      if (p1.code == 0 && p2.code == 0)
        return true;

      // Trivial reject. Both points are outside
      if ((p1.m_bits.left == 1 && p2.m_bits.left == 1) ||
        (p1.m_bits.right == 1 && p2.m_bits.right == 1) ||
        (p1.m_bits.bottom == 1 && p2.m_bits.bottom == 1) ||
        (p1.m_bits.top == 1 && p2.m_bits.top == 1)) {
        clip.a.Set(0.0f, 0.0f);
        clip.b.Set(0.0f, 0.0f);
        return false;
      }

      if (p1.code != 0) {
        if (p1.m_bits.left) {
          clip.a.y += (r.left - clip.a.x) * (clip.a.y - clip.b.y) /
            (clip.a.x - clip.b.x);
          clip.a.x = r.left;
        }
        else if (p1.m_bits.right) {
          clip.a.y += (r.right - clip.a.x) * (clip.a.y - clip.b.y) /
            (clip.a.x - clip.b.x);
          clip.a.x = r.right;
        }
        else if (p1.m_bits.bottom) {
          clip.a.x += (clip.a.y - r.bottom) * (clip.b.x - clip.a.x) /
            (clip.a.y - clip.b.y);
          clip.a.y = r.bottom;
        }
        else if (p1.m_bits.top) {
          clip.a.x += (clip.a.y - r.top) * (clip.b.x - clip.a.x) /
            (clip.a.y - clip.b.y);
          clip.a.y = r.top;
        }
      }
      else {
        if (p2.m_bits.left) {
          clip.b.y += (r.left - clip.b.x) * (clip.b.y - clip.a.y) /
            (clip.b.x - clip.a.x);
          clip.b.x = r.left;
        }
        else if (p2.m_bits.right) {
          clip.b.y += (r.right - clip.b.x) * (clip.b.y - clip.a.y) /
            (clip.b.x - clip.a.x);
          clip.b.x = r.right;
        }
        else if (p2.m_bits.bottom) {
          clip.b.x += (clip.b.y - r.bottom) * (clip.a.x - clip.b.x) /
            (clip.b.y - clip.a.y);
          clip.b.y = r.bottom;
        }
        else if (p2.m_bits.top) {
          clip.b.x += (clip.b.y - r.top) * (clip.a.x - clip.b.x) /
            (clip.b.y - clip.a.y);
          clip.b.y = r.top;
        }
      }
    }
  }
};

#endif

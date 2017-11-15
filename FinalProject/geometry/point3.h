//============================================================================
//	Johns Hopkins University Engineering for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:  David W. Nesbitt
//	File:    Point3.h
//	Purpose: Structures to hold 3D coordinates and homogenous coordinates
//
//============================================================================

#ifndef __POINT3_H__
#define __POINT3_H__

// Forward references
struct Vector3;

/**
 * 3D Point (cartesian)
 */
struct Point3 {
  float x;
  float y; 
  float z;

	/**
   * Default constructor
   */
  Point3() 
    : x(0.0f),
      y(0.0f), 
      z(0.0f) { 
  }

  /**
   * Constructor with initial values for x,y,z.
   * @param   ix   x coordinate position.
   * @param   iy   y coordinate position.
   * @param   iz   z coordinate position.
   */
  Point3(const float ix, const float iy, const float iz)
    : x(ix),
      y(iy),
      z(iz) { 
  }

  /**
   * Copy constructor.
   * @param   p   Point to copy to the new point.
   */
  Point3(const Point3& p) 
    : x(p.x),
      y(p.y), 
      z(p.z) { 
  }

  /**
   * Convert a homogeneous coordinate into a cartesian coordinate.
   * @param   p   Homogenous point
   */
  Point3(const HPoint3& p) { 
    *this = p.ToCartesian();
  }

  /**
   * Assignment operator
   * @param   p   Point to assign to this point.
   * @return   Returns the address of this point.
   */
  Point3& operator = (const Point3& p) {
    x = p.x;
    y = p.y;
    z = p.z;
    return *this;
  }

  /**
   * Set the coordinate components to the specified values.
   * @param   ix   x coordinate position.
   * @param   iy   y coordinate position.
   * @param   iz   z coordinate position.
   */
  void Set(const float ix, const float iy, const float iz) {
    x = ix;
    y = iy;
    z = iz;
  }

  /**
   * Equality operator.
   * @param   p  Point to compare to the current point.
   * @return  Returns true if two points are equal, false otherwise.
   */
  bool operator == (const Point3& p) const {
    return (x == p.x && y == p.y && z == p.z);
  }

  /**
   * Affine combination of this point with another point. 2 scalars are provided
   * (a0 and a1) and the must add to 1.
   * @param  a0  Scalar for this point
   * @param  a1  Scalar for p1
   * @param  p1  Point 1
   */
  Point3 AffineCombination(const float a0, const float a1, const Point3& p1) const {
    return Point3(a0*x + a1*p1.x, a0*y + a1*p1.y, a0*z + a1*p1.z);
  }

  /**
   * Gets the midpoint on a line segment between this point and point p1.
   * @param  p1  Point
   * @return  Returns the midpoint between this point and p1.
   */
  Point3 MidPoint(const Point3& p1) {
    return Point3(0.5f*x + 0.5f*p1.x, 0.5f*y + 0.5f*p1.y, 0.5f*z + 0.5f * p1.z);
  }

  /**
	 * Test if a point is inside a 3D polygon. Uses the normal to the
	 * polygon to project to one of the 2D axis planes and solves in that
	 * space. This method assumes that the point is on the plane of the 
	 * polygon (generally found by intersecting a ray with a plane). 
    * Function defined in geometry.h
	 * @param   polygon  Polygon to test
	 * @param   n        Normal to the polygon
	 * @return  Returns true if the point is inside the polygon, false if not.
	 */
	inline bool IsInPolygon(const std::vector<Point3>& polygon, const Vector3& n) const;

  // The following methods are defined in geometry.h

  /**
   * Add a vector to the current point.
   * @param   v  Vector to add to the current point.
   * @return  Returns a new point: the result of the current point
   *          plus the specified vector.
   */
  Point3 operator + (const Vector3& v) const;

  /**
   * Subtract a vector from the current point.
   * @param   v  Vector to subtract from the current point.
   * @return  Returns a new point: the result of the current point
   *          minus the specified vector.
   */
	Point3 operator - (const Vector3& v) const;
   
  /**
   * Subtraction of a point from the current point.
   * @param   Point to subtract from the current point.
   * @return  Returns a vector.
   */
	Vector3 operator - (const Point3& p) const;

protected:
  // Test if point is inside polygon: drop the z component when making the
  // tests. Shoots a test ray along +x axis.  The strategy is to compare
  // polygon vertex y values to the test point y and quickly discard edges
  // entirely to one side of the test ray.
	bool IsInPolygonXY(const std::vector<Point3>& polygon) const {
		bool inside = false;
		auto pt1 = polygon.end() - 1;
		auto pt2 = polygon.begin();
		bool y1 = (pt1->y >= y);     //  Is vertex above or below test point
		bool y2;
		for ( ; pt2 != polygon.end(); pt1 = pt2, pt2++) {
			// Check if endpoints straddle ray. If so, +x ray could intersect
			// this edge.
			y2 = (pt2->y >= y);
			if (y1 != y2) {
				if (((pt2->y - y) * (pt1->x - pt2->x)) >= 
					 ((pt2->x - x) * (pt1->y - pt2->y)) == y2)
					inside = !inside;
			}
			y1 = y2;
		}
		return inside;
	}

	// Test if point is inside polygon: drop the y component when making the
	// tests. Shoots a test ray along +x axis.  The strategy is to compare
	// polygon vertex z values to the test point z and quickly discard edges
	// entirely to one side of the test ray.
	bool IsInPolygonXZ(const std::vector<Point3>& polygon) const {
		bool inside = false;
		auto pt1 = polygon.end() - 1;
		auto pt2 = polygon.begin();
		bool z1 = (pt1->z >= z);     //  Is vertex above or below test point
		bool z2;
		for ( ; pt2 != polygon.end(); pt1 = pt2, pt2++) {
			// Check if endpoints straddle ray. If so, +x ray could intersect
			// this edge.
			z2 = (pt2->z >= z);
			if (z1 != z2) {
				if (((pt2->z - z) * (pt1->x - pt2->x)) >= 
					((pt2->x - x) * (pt1->z - pt2->z)) == z2)
				inside = !inside;
			}
			z1 = z2;
		}
		return inside;
	}

	// Test if point is inside polygon: drop the x component when making the
	// tests. Shoots a test ray along +y axis.  The strategy is to compare
	// polygon vertex z values to the test point z and quickly discard edges
	// entirely to one side of the test ray.
	bool IsInPolygonYZ(const std::vector<Point3>& polygon) const {
		bool inside = false;
		auto pt1 = polygon.end() - 1;
		auto pt2 = polygon.begin();
		bool z1 = (pt1->z >= z);     //  Is vertex above or below test point
		bool z2;
		for ( ; pt2 != polygon.end(); pt1 = pt2, pt2++)
		{
			// Check if endpoints straddle ray. If so, +y ray could intersect
			// this edge.
			z2 = (pt2->z >= z);
			if (z1 != z2) {
				if (((pt2->z - z) * (pt1->y - pt2->y)) >= 
					((pt2->y - y) * (pt1->z - pt2->z)) == z2)
					inside = !inside;
			}
			z1 = z2;
		}
		return inside;
	}
};

#endif

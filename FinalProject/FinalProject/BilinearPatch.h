//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	David W. Nesbitt
//	File:    BilinearPatch.h
//	Purpose: Scene graph geometry node representing a bilinear patch.
//
//============================================================================

#ifndef __BILINEARPATCH_H
#define __BILINEARPATCH_H

#include "geometry/geometry.h"

class BilinearPatch : public TriSurface {
public:
  /**
   * Creates a scene node representation of a bilinear patch.
   */
	BilinearPatch(const Point3& p0, const Point3& p1,
                const Point3& p2, const Point3& p3, int n,
                const int position_loc, const int normal_loc) {
    float a1, a2;
    Point3 v0 = p0;
    Point3 v1 = p2;
    Point3 v2, v3;
    for (int i = 1; i <= n; i++) {
      a1 = static_cast<float>(i) / static_cast<float>(n);
      a2 = 1.0f - a1;
      v2 = p0.AffineCombination(a2, a1, p1);
      v3 = p2.AffineCombination(a2, a1, p3);

      // Add as 2 triangles
      Add(v1, v0, v2);
      Add(v1, v2, v3);

      v0 = v2;
      v1 = v3;
    }
    End(position_loc, normal_loc);
  }
	
private:
   // Make default constructor private to force use of the constructor
	// with number of subdivisions.
	BilinearPatch() { } 
};

#endif
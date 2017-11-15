//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:  David W. Nesbitt
//	File:    torus.h
//	Purpose: Scene graph geometry node representing a torus.
//
//============================================================================

#ifndef __TORUS_H
#define __TORUS_H

#include "geometry/geometry.h"

class TorusSurface : public TriSurface {
public:
	/**
   * Creates a torus with two radii specified: the ring radius and the radius
   * of the swept circle (tube).  The number of divisions around the ring and
   * the number of divisions around the tube are specified.  Scaling of the
   * torus can be performed but will it will scale both the ring and tube
   * radius.
   * @param   ringradius  Radius of the ring
   * @param   tuberadius  Radius of the circle swept about the ring
   * @param   nring       Number of divisions around the ring
   * @param   ntube       Number of divisions around the tube
   */
  TorusSurface(const float ringradius, const float tuberadius, 
               const int nring, const int ntube, const int position_loc, 
               const int normal_loc) {
    // Use <= so we wrap around to make the last vertices meet the first
    int i, j;
    float v, phi, theta; 
    float dphi   = (2.0f * kPi) / (float)ntube;
    float dtheta = (2.0f * kPi) / (float)nring;
    VertexAndNormal vtx;
    for (phi = 0.0f, j = 0; j <= ntube; j++, phi += dphi) {
      for (theta = 0.0f, i = 0; i <= nring; i++, theta += dtheta) {
        // Compute vertex
        v = (ringradius + tuberadius * cos(phi));
        vtx.vertex.Set(v * cosf(theta), v * sinf(theta), tuberadius * sinf(phi));

        // Compute normal. It is the cross product of the two tangents (one  
        // with respect to the ring rotation and one with repect to the tube
        // rotation). These are found by taking the derivative of the 
        // parametric equation with respect to theta and phi.
        Vector3 tan1(-sinf(theta), cosf(theta), 0.0f);
        Vector3 tan2(cosf(theta) * (-sinf(phi)), sinf(theta) * (-sinf(phi)), cosf(phi));
        vtx.normal = (tan1.Cross(tan2)).Normalize();
        vertices.push_back(vtx);
      }
    }

    // Construct face list and create VBOs. There are ntube+1 rows (outer for 
    // loop) and nring+1 columns (inner for loop).
    ConstructRowColFaceList(ntube + 1, nring + 1);
    CreateVertexBuffers(position_loc, normal_loc);
  }
	
private:

	// Make default constructor private to force use of the constructor
	// with arguments.
	TorusSurface() { };
};

class TexturedTorusSurface : public TexturedTriSurface {
public:
  /**
  * Creates a torus with two radii specified: the ring radius and the radius
  * of the swept circle (tube).  The number of divisions around the ring and
  * the number of divisions around the tube are specified.  Scaling of the
  * torus can be performed but will it will scale both the ring and tube
  * radius.
  * @param   ringradius  Radius of the ring
  * @param   tuberadius  Radius of the circle swept about the ring
  * @param   nring       Number of divisions around the ring
  * @param   ntube       Number of divisions around the tube
  */
  TexturedTorusSurface(const float ringradius, const float tuberadius,
                  const int nring, const int ntube, const int position_loc,
                  const int normal_loc, const int texture_loc)  {
    // Use <= so we wrap around to make the last vertices meet the first
    int i, j;
    float v, phi, theta;
    float dphi = (2.0f * kPi) / (float)ntube;
    float dtheta = (2.0f * kPi) / (float)nring;
    Vector3 tan1;               // Tangent vector with respect to ring
    Vector3 tan2;               // Tangent vector with respect to tube
    float ds = 1.0f / (float)nring;
    float dt = 1.0f / (float)ntube;
    PNTVertex vtx;
    for (vtx.t = 0.0f, phi = 0.0f, j = 0; j <= ntube; j++, phi += dphi, vtx.t += dt) {
      for (vtx.s = 0.0f, theta = 0.0f, i = 0; i <= nring; i++, theta += dtheta, vtx.s += ds) {
        // Compute vertex
        v = (ringradius + tuberadius * cos(phi));
        vtx.vertex.Set(v * cosf(theta), v * sinf(theta), tuberadius * sinf(phi));

        // Compute normal. It is the cross product of the two tangents (one with respect to the ring 
        // rotation and one with repect to the tube rotation). These are found by taking the derivative
        // of the parametric equation with respect to theta and phi.
        tan1.Set(-sinf(theta), cosf(theta), 0.0f);
        tan2.Set(cosf(theta) * (-sinf(phi)), sinf(theta) * (-sinf(phi)), cosf(phi));
        vtx.normal = (tan1.Cross(tan2)).Normalize();
        vertices.push_back(vtx);
      }
    }

    // Construct face list.  There are ntube+1 rows and nring+1 columns. Create VBOs
    ConstructRowColFaceList(ntube + 1, nring + 1);
    CreateVertexBuffers(position_loc, normal_loc, texture_loc);
  }

private:
  // Make default constructor private to force use of the constructor
  // with arguments.
  TexturedTorusSurface() { };
};




#endif
//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	 David W. Nesbitt
//	File:    ConicSurface.h
//	Purpose: Scene graph geometry node representing a conic surface.
//
//============================================================================

#ifndef __CONICSURFACE_H
#define __CONICSURFACE_H

/**
 * Conic surface geometry node.
 */
class ConicSurface : public TriSurface {
public:
	/**
   * Creates a conic surface (cone, cylinder) with specified bottom radius,
   * top radius, number of sides (how many times the top/bottom circles
   * are divided), and number of stacks (subdivisions in z).  z values range 
   * from -0.5 to 0.5 (unit height). If either the bottom or top radius is 0,
   * a cone outer surface is created. If bottom_radius and top_radius are  
   * equal, a cylinder surface is created. End caps are not included!
   * @param   bottom_radius  Radius of bottom
   * @param   top_radius     Radius of top
   * @param   nsides         Number of sides (divisions of the top/bottom)
   * @param   nstacks        Number of height divisions
   */
	ConicSurface(const float bottom_radius,  const float top_radius, 
               const uint32_t nsides, const uint32_t nstacks,
               const int position_loc, const int normal_loc) {
    // Fail if top and bottom radius are both 0
    if (bottom_radius <= 0.0f && top_radius <= 0.0f)
      return;
      
    // There are nstacks+1 rows in the vertex list and nsides columns
    nrows = nstacks + 1;
    ncols = nsides + 1;

    // Set a rotation matrix for the normals
    Matrix4x4 m;
    m.RotateZ(360.0f / static_cast<float>(nsides));

    // Create a normal at theta = 0 perpendicular to vector along side. Note 
    // that if we use a 2D vector in the x,z plane to represent the side 
    // vector then we just swap vertices and negate to find a perpendicular
    Vector3 n(1.0f, 0.0f, (bottom_radius - top_radius));
    n.Normalize();

    // Create a vertex list. We change radius linearly from the top
    // radius to bottom radius
    uint32_t i, j;
    float z, r;
    float dz = 1.0f / nstacks;
    float dr = (bottom_radius - top_radius) / nstacks;
    float theta = 0.0f;
    float dtheta = (2.0f * kPi) / static_cast<float>(nsides);
    float cosTheta, sinTheta;
    VertexAndNormal vtx;
    for (i = 0; i < nsides; i++, theta += dtheta) {
      // Compute sin,cos to use in the loop below
      cosTheta = cosf(theta);
      sinTheta = sinf(theta);

      // Set the normal for this side
      vtx.normal = n;

      // Form vertices along the side for this angle increment
      // Iterate from top to bottom so we create ccw triangles
      for (j = 0, z = 0.5f, r = top_radius; j <= nstacks; 
                     j++, z -= dz, r += dr) {
        vtx.vertex.Set(r * cosTheta, r * sinTheta, z);
        vertices.push_back(vtx);
      }

      // Rotate the normal
      n = m * n;
    }

    // Copy the first column of vertices
    for (uint32_t i = 0; i < nrows; i++) {
      vertices.push_back(vertices[i]);
    }

    // Construct the face list and create VBOs
    ConstructRowColFaceList(ncols, nrows);
    CreateVertexBuffers(position_loc, normal_loc);
	}
	
private:
  uint32_t nrows;
  uint32_t ncols;

	// Make default constructor private to force use of the constructor
	// with number of subdivisions.
	ConicSurface() { };
};

/**
* Conic surface geometry node with texture coordinates
*/
class TexturedConicSurface : public TexturedTriSurface {
public:
  /**
  * Creates a conic surface (cone, cylinder) with specified bottom radius,
  * top radius, number of sides (how many times the top/bottom circles
  * are divided), and number of stacks (subdivisions in z).  z values range
  * from -0.5 to 0.5 (unit height). If either the bottom or top radius is 0,
  * a cone outer surface is created. If bottom_radius and top_radius are equal,
  * a cylinder surface is created. End caps are not included!
  *
  * @param   bottom_radius   Radius of bottom
  * @param   top_radius      Radius of top
  * @param   nsides          Number of sides (divisions of the top/bottom)
  * @param   nstacks        Number of height divisions
  */
  TexturedConicSurface(const float bottom_radius, const float top_radius,
    const uint32_t nsides, const uint32_t nstacks,
    const int position_loc, const int normal_loc,
    const int texture_loc) {
    // Fail if top and bottom radius are both 0
    if (bottom_radius <= 0.0f && top_radius <= 0.0f)
      return;

    // There are nstacks+1 rows in the vertex list and nsides columns
    nrows = nstacks + 1;
    ncols = nsides + 1;

    // Set a rotation matrix for the normals
    Matrix4x4 m;
    m.RotateZ(360.0f / static_cast<float>(nsides));

    // Create a normal at theta = 0 perpendicular to vector along side. Note 
    // that if we use a 2D vector in the x,z plane to represent the side 
    // vector then we just swap vertices and negate to find a perpendicular
    Vector3 n(1.0f, 0.0f, (bottom_radius - top_radius));
    n.Normalize();

    // Create a vertex list. We change radius linearly from the top
    // radius to bottom radius
    uint32_t i, j;
    float s, t;
    float z, r;
    float dz = 1.0f / nstacks;
    float dr = (bottom_radius - top_radius) / nstacks;
    float theta = 0.0f;
    float dtheta = (2.0f * kPi) / static_cast<float>(nsides);
    float cosTheta, sinTheta;
    float ds = 1.0f / static_cast<float>(nsides);
    float dt = 1.0f / static_cast<float>(nstacks);
    PNTVertex vtx;
    for (i = 0, s = 0.0f; i < nsides; i++, theta += dtheta, s += ds) {
      // Compute sin,cos to use in the loop below
      cosTheta = cosf(theta);
      sinTheta = sinf(theta);

      // Set the normal and texture s for this side
      vtx.s = s;
      vtx.normal = n;

      // Form vertices along the side for this angle increment
      // Iterate from top to bottom so we create ccw triangles
      for (j = 0, z = 0.5f, t = 1.0f, r = top_radius; j <= nstacks;
        j++, z -= dz, r += dr, t -= dt) {
        vtx.vertex.Set(r * cosTheta, r * sinTheta, z);
        vtx.t = t;
        vertices.push_back(vtx);
      }

      // Rotate the normal
      n = m * n;
    }

    // Copy the first column of vertices
    for (uint32_t i = 0; i < nrows; i++) {
      vtx = vertices[i];
      vtx.s = 1.0f;
      vertices.push_back(vertices[i]);
    }

    // Construct the face list and create VBOs
    ConstructRowColFaceList(ncols, nrows);
    CreateVertexBuffers(position_loc, normal_loc, texture_loc);
  }

private:
  uint32_t nrows;
  uint32_t ncols;

  // Make default constructor private to force use of the constructor
  // with number of subdivisions.
  TexturedConicSurface() { };
};

#endif
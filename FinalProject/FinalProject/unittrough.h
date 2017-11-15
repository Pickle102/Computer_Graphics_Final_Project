//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	David W. Nesbitt
//	File:    unittrough.h
//	Purpose: Scene graph geometry node representing a circular trough
//           (inside of a half cyclinder).
//
//============================================================================

#ifndef __UNITTROUGH_H
#define __UNITTROUGH_H

/**
 * Conic surface geometry node.
 */
class UnitTrough : public TriSurface {
public:
	/**
   * Creates a conic surface (cone, cylinder) with specified bottom radius,
   * top radius, number of sides (how many times the top/bottom circles
   * are divided), and number of stacks (subdivisions in z).  z values range 
   * from -0.5 to 0.5 (unit height). If either the bottom or top radius is 0,
   * a cone outer surface is created. If bottom_radius and top_radius are equal, 
   * a cylinder surface is created. End caps are not included!
   * @param   bottom_radius  Radius of bottom
   * @param   top_radius     Radius of top
   * @param   nsides         Number of sides (divisions of the top/bottom)
   * @param   nstacks        Number of height divisions
   */
  UnitTrough(const uint32_t nsides, const uint32_t nstacks,
             const int position_loc, const int normal_loc) {
    // Set a rotation matrix for the normals (only rotatiing 180 degrees)
    Matrix4x4 m;
    m.RotateZ(180.0f / static_cast<float>(nsides));

    // Create a normal at theta = 0 pointing inwards.
    Vector3 n(-1.0f, 0.0f, 0.0f);

    // Create a vertex list. Note that normals can be computed based
    // on the difference in bottom and top radius. Note that we 
    // change radius linearly from the bottom radius to top radius
    uint32_t i, j;
    float z;
    float dz = 1.0f / nstacks;
    float theta = 0.0f;
    float dtheta = (kPi) / static_cast<float>(nsides);
    VertexAndNormal vtx;
    for (i = 0; i <= nsides; i++, theta += dtheta) {
      // Compute sin,cos to use in the loop below
      float cos_theta = cosf(theta);
      float sin_theta = sinf(theta);

      // Set the normal for this side
      vtx.normal = n;

      // Form vertices along the side for this angle increment
      for (j = 0, z = -0.5f; j <= nstacks; j++, z += dz) {
        vtx.vertex.Set(cos_theta, sin_theta, z);
        vertices.push_back(vtx);
      }

      // Rotate the normal for next angle
      n = m * n;
    }

    // Form triangle strip face indexes.
    // Note: there are n+1 rows and n+1 columns.
    nrows = nstacks + 1;
    ncols = nsides  + 1;
    for (uint32_t row = 0; row < nrows - 1; row++) {
      // Repeat the first face index (unless the 1st row)
      if (row > 0) {
        faces.push_back(GetIndex(row, 0));
      }

      // Iterate along columns - alternate between rows
      for (uint32_t col = 0; col < ncols; col++) {
        faces.push_back(GetIndex(row, col));
        faces.push_back(GetIndex(row+1, col));
      }

      // Repeat the last index along the rowm (except for last row)
      if (row < nrows - 2) {
        faces.push_back(GetIndex(row + 1, ncols - 1));
      }
    }
    printf("Face Size = %d\n", faces.size());
    // Create vertex buffers
    CreateVertexBuffers(position_loc, normal_loc);
	}

  /**
  * Draw this geometry node.
  */
  void Draw(SceneState& scene_state) {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLE_STRIP, (GLsizei)face_count, GL_UNSIGNED_SHORT, (void*)0);
    glBindVertexArray(0);
  }
	
private:
   uint32_t nrows;
   uint32_t ncols;

	// Make default constructor private to force use of the constructor
	// with number of subdivisions.
	UnitTrough() { };

  // Convenience method to get the index into the vertex list given the
  // "row" and "column" of the subdivision/grid
  uint32_t GetIndex(uint32_t row, uint32_t col) const {
    return (col*nrows) + row;
  }
};

#endif
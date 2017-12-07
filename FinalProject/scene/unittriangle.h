//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	 Joshua Griffith
//	File:    unittriangle.h
//	Purpose: Scene graph geometry node representing a subdivided unit triangle.
//
//============================================================================

#ifndef __UNITTRIANGLE_H
#define __UNITTRIANGLE_H

class UnitTriangleSurface : public TriSurface {
public:
	/**
	 * Creates a unit triangle with n subdivisions. 
	 * Note that this is the intention -- this currently only 
	 * supports a single triangle (3 vertices)
   * @param  n   Number of subdivisions in x and y
	 */
	UnitTriangleSurface(uint32_t n, const int position_loc, const int normal_loc) {
    // Only allow 250 subdivision (so it creates less that 65K vertices)
    if (n > 250)
      n = 250;
		
    // Normal is 0,0,1. z = 0 so all vertices lie in x,y plane.
    // Having issues with roundoff when n = 40,50 - so compare with some tolerance
    VertexAndNormal vtx;
    vtx.normal = { 0.0f, 0.0f, 1.0f };
    vtx.vertex.z = 0.0f;
    float spacing = 1.0f / n;

	// Currently, only allows for 3 vertex triangle
	vtx.vertex.x = -0.5f;
	vtx.vertex.y = -0.5f;
	vertices.push_back(vtx);

	vtx.vertex.x = 0.5f;
	vtx.vertex.y = -0.5f;
	vertices.push_back(vtx);

	vtx.vertex.x = 0.0f;
	vtx.vertex.y = 0.5f;
	vertices.push_back(vtx);
		
	// Construct face list and create vertex buffer objects
	ConstructTriangleFaceList(n + 1, n + 1);
	CreateVertexBuffers(position_loc, normal_loc);
	}

	/**
	* Form triangle face indexes for a surface constructed using a double loop - one can be considered
	* rows of the surface and the other can be considered columns of the surface. Assumes the vertex
	* list is populated in row order.
	* @param  nrows  Number of rows
	* @param  ncols  Number of columns
	*/
	void ConstructTriangleFaceList(const uint32_t nrows, const uint32_t ncols) {
		for (uint32_t row = 0; row < nrows - 1; row++) {
			for (uint32_t col = 0; col < ncols - 1; col++) {
				// GL_TRIANGLES draws independent triangles for each set of 3 vertices
				faces.push_back(GetIndex(row + 1, col, ncols));
				faces.push_back(GetIndex(row, col, ncols));
				faces.push_back(GetIndex(row, col + 1, ncols));
			}
		}
	}

private:
	// Make default constructor private to force use of the constructor
	// with number of subdivisions.
	UnitTriangleSurface() { };
};

class TexturedUnitTriangleSurface : public TexturedTriSurface
{
public:
  /**
  * Creates a unit length and width "flat surface".  The surface is composed of
  * triangles such that the unit length/width surface is divided into n
  * equal paritions in both x and y. Constructs a vertex list and face list
  * for the surface.
  * @param  n   Number of subdivisions in x and y
  */
  TexturedUnitTriangleSurface(unsigned int n, float texture_scale, const int position_loc,
                            const int normal_loc, const int texture_loc) {
    // Only allow 250 subdivision (so it creates less that 65K vertices)
    if (n > 250)
      n = 250;

    // Normal is 0,0,1. z = 0 so all vertices lie in x,y plane.
    // Having issues with roundoff when n = 40,50 - so compare with some tolerance
    // Store in column order.
    float ds = texture_scale / n;
    float dt = texture_scale / n;
    PNTVertex vtx;
    vtx.normal.Set(0.0f, 0.0f, 1.0f);
    vtx.vertex.z = 0.0f;
    float spacing = 1.0f / n;

	// Currently, only allows for 3 vertex triangle
	vtx.vertex.x = -0.5f;
	vtx.vertex.y = -0.5f;
	vertices.push_back(vtx);

	vtx.vertex.x = 0.5f;
	vtx.vertex.y = -0.5f;
	vertices.push_back(vtx);

	vtx.vertex.x = 0.0f;
	vtx.vertex.y = 0.5f;
	vertices.push_back(vtx);


    // Construct face list and create vertex buffer objects
    ConstructTriangleFaceList(n + 1, n + 1);
    CreateVertexBuffers(position_loc, normal_loc, texture_loc);
  }

  /**
  * Form triangle face indexes for a surface constructed using a double loop - one can be considered
  * rows of the surface and the other can be considered columns of the surface. Assumes the vertex
  * list is populated in row order.
  * @param  nrows  Number of rows
  * @param  ncols  Number of columns
  */
  void ConstructTriangleFaceList(const uint32_t nrows, const uint32_t ncols) {
	  for (uint32_t row = 0; row < nrows - 1; row++) {
		  for (uint32_t col = 0; col < ncols - 1; col++) {
			  // GL_TRIANGLES draws independent triangles for each set of 3 vertices
			  faces.push_back(GetIndex(row + 1, col, ncols));
			  faces.push_back(GetIndex(row, col, ncols));
			  faces.push_back(GetIndex(row, col + 1, ncols));
		  }
	  }
  }

private:
  // Make default constructor private to force use of the constructor
  // with number of subdivisions.
  TexturedUnitTriangleSurface() { };
};


#endif
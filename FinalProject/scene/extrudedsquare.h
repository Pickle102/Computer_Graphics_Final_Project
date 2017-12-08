//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	 Joshua Griffith
//	File:    extrudedsquare.h
//	Purpose: Scene graph geometry node representing a unit square extruded about a path
//
//============================================================================

#ifndef __EXTRUDEDSQUARE_H
#define __EXTRUDEDSQUARE_H

class ExtrudedSquare : public TriSurface {
public:
	/**
	 * Creates a unit length and width "flat surface".  The surface is composed of
	 * triangles such that the unit length/width surface is divided into n
	 * equal paritions in both x and y. Constructs a vertex list and face list
	 * for the surface.
   * @param  n   Number of subdivisions in x and y
	 */
	ExtrudedSquare(uint32_t n, const int position_loc, const int normal_loc) {

		Vector3 direction = Vector3(1.0, 0.0, 0.0);
		float length = 5.0f;

    // Only allow 250 subdivision (so it creates less that 65K vertices)
    if (n > 250)
      n = 250;
		
    // Normal is 0,0,1. z = 0 so all vertices lie in x,y plane.
    // Having issues with roundoff when n = 40,50 - so compare with some tolerance
    VertexAndNormal vtx;
    vtx.normal = { 0.0f, 0.0f, 1.0f };
    vtx.vertex.z = 0.0f;
    float spacing = 1.0f / n;
    for (vtx.vertex.y = -0.5; vtx.vertex.y <= 0.5f + kEpsilon; vtx.vertex.y += spacing) {
      for (vtx.vertex.x = -0.5; vtx.vertex.x <= 0.5f + kEpsilon; vtx.vertex.x += spacing) {
        vertices.push_back(vtx);
      }
    }
		
    // Construct the face list and create VBOs
    ConstructRowColFaceList(n + 1, n + 1);
    CreateVertexBuffers(position_loc, normal_loc);
	}
	
private:
	// Make default constructor private to force use of the constructor
	// with number of subdivisions.
	ExtrudedSquare() { };
};

class TexturedExtrudedSquare : public TexturedTriSurface
{
public:
  /**
  * Creates a unit length and width "flat surface".  The surface is composed of
  * triangles such that the unit length/width surface is divided into n
  * equal paritions in both x and y. Constructs a vertex list and face list
  * for the surface.
  * @param  n   Number of subdivisions in x and y
  */
	TexturedExtrudedSquare(unsigned int n, float texture_scale, const int position_loc,
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
    for (vtx.vertex.y = -0.5, vtx.t = 0.0f; vtx.vertex.y <= 0.5f + kEpsilon;
      vtx.vertex.y += spacing, vtx.t += dt) {
      for (vtx.vertex.x = -0.5, vtx.s = 0.0f; vtx.vertex.x <= 0.5f + kEpsilon;
        vtx.vertex.x += spacing, vtx.s += ds) {
        vertices.push_back(vtx);
      }
    }

    // Construct face list and create vertex buffer objects
    ConstructRowColFaceList(n + 1, n + 1);
    CreateVertexBuffers(position_loc, normal_loc, texture_loc);
  }

private:
  // Make default constructor private to force use of the constructor
  // with number of subdivisions.
	TexturedExtrudedSquare() { };
};


#endif
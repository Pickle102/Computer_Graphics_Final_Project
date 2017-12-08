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
	 * 
	 * Extrudes the geometry node about a predefined path
   * @param  n   Number of subdivisions in x and y
	 */
	ExtrudedSquare(const int position_loc, const int normal_loc) {

		std::vector< std::tuple<Vector3, float> > path;
		
		// Construct a path to extrude about. Essentially a list of sequential vectors with lengths.
		path.push_back(std::tuple<Vector3, float>(Vector3(0.0, 2.5, 1.0), 2.5f));
		path.push_back( std::tuple<Vector3, float>(Vector3(0.0, 5.0, 1.0), 5.0f) );
		path.push_back(std::tuple<Vector3, float>(Vector3(0.0, 5.0, 1.0), 5.0f));
		path.push_back(std::tuple<Vector3, float>(Vector3(0.0, 2.5, 1.0), 7.5f));
		path.push_back(std::tuple<Vector3, float>(Vector3(0.0, 0.0, 1.0), 10.0f));
		path.push_back(std::tuple<Vector3, float>(Vector3(0.0, 0.0, 1.0), 7.0f));
		path.push_back(std::tuple<Vector3, float>(Vector3(0.0, -7.0, 1.0), 7.0f));
		path.push_back(std::tuple<Vector3, float>(Vector3(0.0, -7.0, 1.0), 3.0f));
		path.push_back(std::tuple<Vector3, float>(Vector3(0.0, 0.0, 1.0), 3.0f));
		path.push_back(std::tuple<Vector3, float>(Vector3(0.0, 0.0, 1.0), 0.0f));

		
    // Normal is 0,0,1. z = 0 so all vertices lie in x,y plane.
    // Having issues with roundoff when n = 40,50 - so compare with some tolerance
    VertexAndNormal vtx;
    vtx.normal = { 0.0f, 0.0f, 1.0f };
    vtx.vertex.z = 0.0f;
    float spacing = 1.0f;
    for (vtx.vertex.y = -0.5; vtx.vertex.y <= 0.5f + kEpsilon; vtx.vertex.y += spacing) {
      for (vtx.vertex.x = -0.5; vtx.vertex.x <= 0.5f + kEpsilon; vtx.vertex.x += spacing) {
        vertices.push_back(vtx);
      }
    }

	int currentIndex = 0;
	int lastFaceIndex = currentIndex;
	CreateSquareFaceList(currentIndex);

	for (int i = 0; i < path.size(); i++)
	{
		int faceIndex = 0;

		// Unpack tuple
		Vector3 direction = std::get<0>(path[i]);
		float length = std::get<1>(path[i]);
		
		// Set front face vertices normal to direction
		vtx.normal = std::get<0>(path[i]);
		vtx.vertex.z = length;

		vtx.vertex.x = -0.5f + direction.x;
		vtx.vertex.y = -0.5f + direction.y;
		vertices.push_back(vtx);

		vtx.vertex.x = 0.5f + direction.x;
		vtx.vertex.y = -0.5f + direction.y;
		vertices.push_back(vtx);

		vtx.vertex.x = -0.5f + direction.x;
		vtx.vertex.y = 0.5f + direction.y;
		vertices.push_back(vtx);

		vtx.vertex.x = 0.5f + direction.x;
		vtx.vertex.y = 0.5f + direction.y;
		vertices.push_back(vtx);

		currentIndex += 4;
		faceIndex = currentIndex;
		CreateSquareFaceList(currentIndex);

		// Need to set normals correctly here -- currently defaulting to the direction 
		for (int j = 0; j < 4; j++)
		{
			if (j != 3)
			{
				vtx.vertex.x = vertices[lastFaceIndex + j].vertex.x;
				vtx.vertex.y = vertices[lastFaceIndex + j].vertex.y;
				vtx.vertex.z = vertices[lastFaceIndex + j].vertex.z;
				vertices.push_back(vtx);

				vtx.vertex.x = vertices[faceIndex + j].vertex.x;
				vtx.vertex.y = vertices[faceIndex + j].vertex.y;
				vtx.vertex.z = vertices[faceIndex + j].vertex.z;
				vertices.push_back(vtx);
			}
			else
			{
				vtx.vertex.x = vertices[lastFaceIndex].vertex.x;
				vtx.vertex.y = vertices[lastFaceIndex].vertex.y;
				vtx.vertex.z = vertices[lastFaceIndex].vertex.z;
				vertices.push_back(vtx);

				vtx.vertex.x = vertices[faceIndex].vertex.x;
				vtx.vertex.y = vertices[faceIndex].vertex.y;
				vtx.vertex.z = vertices[faceIndex].vertex.z;
				vertices.push_back(vtx);
			}

			if (j % 2 == 0)
			{
				vtx.vertex.x = vertices[lastFaceIndex + 1 + j].vertex.x;
				vtx.vertex.y = vertices[lastFaceIndex + 1 + j].vertex.y;
				vtx.vertex.z = vertices[lastFaceIndex + 1 + j].vertex.z;
				vertices.push_back(vtx);

				vtx.vertex.x = vertices[faceIndex + 1 + j].vertex.x;
				vtx.vertex.y = vertices[faceIndex + 1 + j].vertex.y;
				vtx.vertex.z = vertices[faceIndex + 1 + j].vertex.z;
				vertices.push_back(vtx);
			}
			else if (j != 3)
			{
				vtx.vertex.x = vertices[lastFaceIndex + 2 + j].vertex.x;
				vtx.vertex.y = vertices[lastFaceIndex + 2 + j].vertex.y;
				vtx.vertex.z = vertices[lastFaceIndex + 2 + j].vertex.z;
				vertices.push_back(vtx);

				vtx.vertex.x = vertices[faceIndex + 2 + j].vertex.x;
				vtx.vertex.y = vertices[faceIndex + 2 + j].vertex.y;
				vtx.vertex.z = vertices[faceIndex + 2 + j].vertex.z;
				vertices.push_back(vtx);
			}
			else
			{
				vtx.vertex.x = vertices[lastFaceIndex + 2].vertex.x;
				vtx.vertex.y = vertices[lastFaceIndex + 2].vertex.y;
				vtx.vertex.z = vertices[lastFaceIndex + 2].vertex.z;
				vertices.push_back(vtx);

				vtx.vertex.x = vertices[faceIndex + 2].vertex.x;
				vtx.vertex.y = vertices[faceIndex + 2].vertex.y;
				vtx.vertex.z = vertices[faceIndex + 2].vertex.z;
				vertices.push_back(vtx);
			}

			currentIndex += 4;

			if (j < 2)
			{
				CreateSquareFaceListAlt(currentIndex);
			}
			else
			{
				CreateSquareFaceList(currentIndex);
			}
		}

		lastFaceIndex = faceIndex;
	}
		
    // Create VBOs
    CreateVertexBuffers(position_loc, normal_loc);
	}

private:
	// Make default constructor private to force use of the constructor
	// with number of subdivisions.
	ExtrudedSquare() { };

	void CreateSquareFaceList(int startIndex)
	{
		faces.push_back(startIndex);
		faces.push_back(startIndex + 1);
		faces.push_back(startIndex + 2);

		faces.push_back(startIndex + 1);
		faces.push_back(startIndex + 3);
		faces.push_back(startIndex + 2);
	}

	void CreateSquareFaceListAlt(int startIndex)
	{
		faces.push_back(startIndex);
		faces.push_back(startIndex + 2);
		faces.push_back(startIndex + 1);

		faces.push_back(startIndex + 1);
		faces.push_back(startIndex + 2);
		faces.push_back(startIndex + 3);
	}
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
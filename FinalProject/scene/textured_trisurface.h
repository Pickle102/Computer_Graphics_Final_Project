//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	  David W. Nesbitt
//	File:     textured_trisurface.h
//	Purpose:  Scene graph geometry node indicating a triangle based
//            surface with texture coordinates.
//
//============================================================================

#ifndef __TEXTUREDTRISURFACE_H
#define __TEXTUREDTRISURFACE_H

/**
 * Textured triangle mesh surface.
 */
class TexturedTriSurface : public GeometryNode {
public:
	/**
	 * Constructor. 
	 */
	TexturedTriSurface() {
    vao        = 0;
    vbo        = 0;
    facebuffer = 0;
  }
	
	/**
	 * Destructor.
	 */
	~TexturedTriSurface() {
    // Delete vertex buffer objects
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &facebuffer);
    glDeleteVertexArrays(1, &vao);
  }
	
  /**
  * Draw this geometry node.
  */
  void Draw(SceneState& scene_state) {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, (GLsizei)face_count, GL_UNSIGNED_SHORT, (void*)0);
    glBindVertexArray(0);

    // Disable texture vertex attribute 
    glDisableVertexAttribArray(scene_state.texture_loc);
  }
	
	/**
	 * Construct triangle surface by passing in vertex list and face list
    * @param  vertexList  List of vertices (position and normal)
    * @param  faceList    Index list for triangles
    * @param  position_loc Location of the vertex position attribute
    * @param  normal_loc   Location of the vertex normal attribute
    *@param   texture_loc  Location of the vertex texture attribute
	 */
	void Construct(std::vector<PNTVertex>& vertexList, std::vector<uint16_t> faceList,
                 const int position_loc, const int normal_loc, const int texture_loc) {
		vertices = vertexList;
		faces   = faceList;

      // Create the vertex and face buffers
	   CreateVertexBuffers(position_loc, normal_loc, texture_loc);
	}

  /**
  * Marks the end of a triangle mesh. Calculates the vertex normals.
  */
  void End(const int position_loc, const int normal_loc, const int texture_loc) {
    // Iterate through the face list and calculate the normals for each 
    // face and add the normal to each vertex in the face list. This 
    // assumes the vertex normals are initilaized to 0 (in constructor
    // of VertexAndNormal)
    uint32_t v0, v1, v2;
    Vector3 e1, e2, face_normal;
    auto face_vertex = faces.begin();
    while (face_vertex != faces.end()) {
      // Get the vertices of the face (assumes ccw order)
      v0 = *face_vertex++;
      v1 = *face_vertex++;
      v2 = *face_vertex++;

      // Calculate surface normal. Normalize it since cross products
      // do not ensure unit length normals. We need to make sure normals
      // are same length so averaging works properly
      e1.Set(vertices[v0].vertex, vertices[v1].vertex);
      e2.Set(vertices[v0].vertex, vertices[v2].vertex);
      face_normal = (e1.Cross(e2)).Normalize();

      // Add the face normal to the vertex normals of the triangle
      vertices[v0].normal += face_normal;
      vertices[v1].normal += face_normal;
      vertices[v2].normal += face_normal;
    }

    // Normalize the vertex normals - this essentially averages the 
    // adjoining face normals.
    for (auto v : vertices) {
      v.normal.Normalize();
    }

    // Create the vertex and face buffers
    CreateVertexBuffers(position_loc, normal_loc, texture_loc);
  }

  /**
  * Form triangle face indexes for a surface constructed using a double loop - one can be considered
  * rows of the surface and the other can be considered columns of the surface. Assumes the vertex
  * list is populated in row order.
  * @param  nrows  Number of rows
  * @param  ncols  Number of columns
  */
  void ConstructRowColFaceList(const uint32_t nrows, const uint32_t ncols) {
    for (uint32_t row = 0; row < nrows - 1; row++) {
      for (uint32_t col = 0; col < ncols - 1; col++) {
        // Divide each square into 2 triangles - make sure they are ccw.
        // GL_TRIANGLES draws independent triangles for each set of 3 vertices
        faces.push_back(GetIndex(row + 1, col, ncols));
        faces.push_back(GetIndex(row, col, ncols));
        faces.push_back(GetIndex(row, col + 1, ncols));

        faces.push_back(GetIndex(row + 1, col, ncols));
        faces.push_back(GetIndex(row, col + 1, ncols));
        faces.push_back(GetIndex(row + 1, col + 1, ncols));
      }
    }
  }

  // Convenience method to get the index into the vertex list given the
  // "row" and "column" of the subdivision/grid
  uint16_t GetIndex(uint32_t row, uint32_t col, uint32_t ncols) const {
    return static_cast<uint16_t>((row*ncols) + col);
  }

  /**
   * Creates vertex buffers for this object.
   */
  void CreateVertexBuffers(const int position_loc, const int normal_loc, const int texture_loc) {
     // Generate vertex buffers for the vertex list and the face list
     glGenBuffers(1, &vbo);
     glGenBuffers(1, &facebuffer);

     // Bind the vertex list to the vertex buffer object
     glBindBuffer(GL_ARRAY_BUFFER, vbo);
     glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(PNTVertex),
       (void*)&vertices[0], GL_STATIC_DRAW);

     // Bind the face list to the vertex buffer object
     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facebuffer);
     glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(uint16_t),
       (void*)&faces[0], GL_STATIC_DRAW);

     // Copy the face list count for use in Draw (then we can clear the vector)
     face_count = faces.size();

     // We could clear any local memory as it is now in the VBO. However there may be
     // cases where we want to keep it (e.g. collision detection, picking) so I am not
     // going to do that here.

     // Allocate a VAO, enable it and set the vertex attribute arrays and pointers
     glGenVertexArrays(1, &vao);
     glBindVertexArray(vao);

     // Bind the vertex buffer, set the vertex position attribute and the vertex normal attribute
     glBindBuffer(GL_ARRAY_BUFFER, vbo);
     glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(PNTVertex), (void*)0);
     glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, sizeof(PNTVertex), (void*)(sizeof(Point3)));
     glVertexAttribPointer(texture_loc, 2, GL_FLOAT, GL_FALSE, sizeof(PNTVertex),
       (void*)(sizeof(Point3) + sizeof(Vector3)));
     glEnableVertexAttribArray(position_loc);
     glEnableVertexAttribArray(normal_loc);
     glEnableVertexAttribArray(texture_loc);

     // Bind the face list buffer and draw. Note the use of 0 offset in glDrawElements
     glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facebuffer);

     // Make sure changes to this VAO are local
     glBindVertexArray(0);
   }
	
protected:
  // Vertex buffer support
  uint32_t face_count;
  GLuint   vao;
  GLuint   vbo;
  GLuint   facebuffer;

  // Vertex and normal list
  std::vector<PNTVertex> vertices;
	
  // Use uint16_t for face list indexes (OpenGL ES compatible)
  std::vector<uint16_t> faces;
};


#endif
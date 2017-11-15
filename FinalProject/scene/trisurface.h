//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	  David W. Nesbitt
//	File:     TransformNode.h
//	Purpose:  Scene graph geometry node indicating a triangle mesh based
//            surface. Uses indexed vertex arrays and per vertex normals.
//
//============================================================================

#ifndef __TRISURFACE_H
#define __TRISURFACE_H

/**
* Triangle mesh surface. Uses indexed vertex arrays. Stores
* vertices as VertexAndNormal.
*/
class TriSurface : public GeometryNode {
public:
  /**
   * Constructor. 
   */
  TriSurface() 
      : GeometryNode() {
    vao = 0;
    vbo = 0;
    facebuffer = 0;
  }
	
  /**
   * Destructor.
   */
  ~TriSurface()  {
    // Delete vertex buffer objects
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &facebuffer);
    glDeleteVertexArrays(1, &vao);
  }
	
  /**
   * Draw this geometry node.
   */
  virtual void Draw(SceneState& scene_state) {
    glBindVertexArray(vao);
    glDrawElements(GL_TRIANGLES, (GLsizei)face_count, GL_UNSIGNED_SHORT, (void*)0);
    glBindVertexArray(0);
  }
	
  /**
   * Construct triangle surface by passing in vertex list and face list
   * @param  v  List of vertices (position and normal)
   * @param  f    Index list for triangles
   */
  void Construct(std::vector<VertexAndNormal>& v, std::vector<uint16_t>& f) {
    vertices = v;
    faces    = f;
  }

  /**
   * Adds the vertices of the triangle to the vertex list. Accounts for
   * shared vertices by checking if the vertex is already in the list.
   * Manages the face list as well - stores indexes into the vertex list
   * for this triangle. This method allows triangles to be added one at 
   * a time to a curved surface. Call End() when done to get vertex normal
   * assigned. Vertices should be oriented counter-clockwise.
   * @param  v0  First vertex in the triangle
   * @param  v1  Second vertex in the triangle
   * @param  v2  Last vertex in the triangle
   */
  void Add(const Point3& v0, const Point3& v1, const Point3& v2) {
    faces.push_back(AddVertex(v0));
    faces.push_back(AddVertex(v1));
    faces.push_back(AddVertex(v2));
  }

  /**
  * Adds the vertices of a convex polygon to the vertex list. Forms faces
  * as if a triangle fan was constructed.
  * @param  vlist  List of vertices in the triangle (ccw orientation)
  */
  void AddPolygon(std::vector<Point3>& vlist) {
    // Form the normal
    VertexAndNormal vertex;
    Vector3 e1(vlist[0], vlist[1]);
    Vector3 e2(vlist[0], vlist[2]);
    vertex.normal = (e1.Cross(e2)).Normalize();

    // Add vertices to the vertex list (with the face normal).
    // Save the current index so the face list is properly constructed
    uint32_t curr_vertex = vertices.size();
    std::vector<Point3>::iterator v = vlist.begin();
    for (auto v : vlist) {
      vertex.vertex = v;
      vertices.push_back(vertex);
    }

    // Form face list as if this was a triangle fan (always use the 1st vertex)
    for (uint32_t i = 2, n = vlist.size(); i < n; i++) {
      faces.push_back(curr_vertex);
      faces.push_back(curr_vertex + i - 1);
      faces.push_back(curr_vertex + i);
    }
  }

  /**
   * Marks the end of a triangle mesh. Calculates the vertex normals.
   */
  void End(const int position_loc, const int normal_loc) {
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
    CreateVertexBuffers(position_loc, normal_loc);
  }

  /**
  * Creates vertex buffers for this object.
  */
  void CreateVertexBuffers(const int position_loc, const int normal_loc) {
    // Generate vertex buffers for the vertex list and the face list
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &facebuffer);

    // Bind the vertex list to the vertex buffer object
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(VertexAndNormal),
      (void*)&vertices[0], GL_STATIC_DRAW);

    // Bind the face list to the vertex buffer object
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facebuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, faces.size() * sizeof(uint16_t),
      (void*)&faces[0], GL_STATIC_DRAW);

    // Copy the face list count for use in Draw
    face_count = faces.size();

    // Allocate a VAO, enable it and set the vertex attribute arrays and pointers
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Bind the vertex buffer, set the vertex position attribute and the vertex normal attribute
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAndNormal), (void*)0);
    glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAndNormal), (void*)(sizeof(Point3)));
    glEnableVertexAttribArray(position_loc);
    glEnableVertexAttribArray(normal_loc);

    // Bind the face list buffer and draw.
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, facebuffer);

    // Make sure changes to this VAO are local
    glBindVertexArray(0);

    // We could clear any local memory as it is now in the VBO. However there may be
    // cases where we want to keep it (e.g. collision detection, picking) so I am not
    // going to do that here.
  }
	
protected:
  // Vertex buffer support
  uint32_t face_count;
  GLuint vao;
  GLuint vbo;
  GLuint facebuffer;

  // Vertex and normal list
  std::vector<VertexAndNormal> vertices;
	
  // Use uint16_t for face list indexes (OpenGL ES compatible)
  std::vector<uint16_t> faces;

  /**
   * Form triangle face indexes for a surface constructed using a double loop -
   * one can be considered rows of the surface and the other can be considered 
   * columns of the surface. Assumes the vertex list is populated in "row" 
   * order.
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
   * Adds a vertex to the surface vertex list.  Returns the index into the
	 * vertex list.  If the vertex is already in the list it does not
	 * replicate it.
   * @param  v_in  Vertex
   */
	uint32_t AddVertex(const Point3& v_in) {
    // Check if vertex is in the list. This is just a brute force method.
    // Efficiency can be improved but we only use this at startup
    uint32_t index = 0;
    for (auto v : vertices) {
      if (v_in == v.vertex) {
        return index;
      }
      index++;
    }
		
		// Not in the list, add it. Make sure the vertex normal is initialized 
		// to (0,0,0)
		VertexAndNormal vertex(v_in);
		vertices.push_back(vertex);
		return (uint32_t)vertices.size() - 1;
	}
};


#endif
//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	David W. Nesbitt
//	File:    UnitSphere.h
//	Purpose:	Simple geometry node that defines a unit sphere and draws
//          it with glutSolidSphere.
//
//============================================================================

#ifndef __UNITSPHERE_H
#define __UNITSPHERE_H

/**
 * Unit sphere geometry node.
 */
class UnitSphere: public GeometryNode {
public:
  /**
   * Constructor.
   * @param  nlat  Number of latitude stacks.
   * @param  nlng  Number of longitude slices.
   * @param  position_loc  Location of position attribute in the vertex shader.
   * @param  normal_loc    Location of normal attribute in the vertex shader.
   */
  UnitSphere(const uint32_t nlat, const uint32_t nlng, 
             const int position_loc, const int normal_loc)
      : GeometryNode() {
    // Convert to radians
    float minlat_r = DegreesToRadians(-90.0f);
    float maxlat_r = DegreesToRadians(90.0f);
    float minlng_r = DegreesToRadians(-180.0f);
    float maxlng_r = DegreesToRadians(180.0f);

    // Compute delta lat,lng
    float dLat = (maxlat_r - minlat_r) / static_cast<float>(nlat);
    float dLng = (maxlng_r - minlng_r) / static_cast<float>(nlng);
 
    // Create vertex list. Since vertex position and normal are equal we
    // can use just Point3 in the VBO.
    std::vector<Point3> vertex_list;
    for (float lat1 = minlat_r, lat2 = lat1 + dLat; lat2 <= maxlat_r + kEpsilon;
                lat1 = lat2, lat2 += dLat) {
      float cos_lat1 = cosf(lat1);
      float sin_lat1 = sinf(lat1);
      float cos_lat2 = cosf(lat2);
      float sin_lat2 = sinf(lat2);

      // Iterate over longitude, connecting a vertex on the next latitude row with one
      // on the current latitude row. Since we wrap around to the same longitude we do
      // not need to repeat vertices to create degenerate triangles to connect each row.
      for (float lng = minlng_r; lng <= maxlng_r + kEpsilon; lng += dLng) {
        float cos_lng = cosf(lng);
        float sin_lng = sinf(lng);

        // Add a vertex on the upper latitude
        vertex_list.emplace_back(cos_lng * cos_lat2, sin_lng * cos_lat2, sin_lat2);

        // Add a vertex on the lower latitude
        vertex_list.emplace_back(cos_lng * cos_lat1, sin_lng * cos_lat1, sin_lat1);
      }
    }
    vertex_count = vertex_list.size();

    // Create a buffer object and load the data
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(Point3), 
            (GLvoid*)&vertex_list[0].x, GL_STATIC_DRAW);
	
    // Allocate a VAO, enable it
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Bind the VBO, set vertex attribute pointers for position and normal.
    // Enable the arrays.
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(position_loc);
    glVertexAttribPointer(normal_loc,   3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(normal_loc);

    // Make sure changes to this VAO are local
    glBindVertexArray(0);

  }
	
  /**
   * Destructor
   */
  virtual ~UnitSphere() { }

  /**
   * Draw the ball / sphere. Note that geometry nodes are at the
   * leaves of the tree, so no children exist.
   */
  virtual void Draw(SceneState& sceneState) {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_count);
    glBindVertexArray(0);
  }

protected:
   GLuint vao;            // Vertex Array Object
   GLuint vbo;            // Vertex Buffer Object
   uint32_t vertex_count; // Number of vertices in the sphere
};

#endif
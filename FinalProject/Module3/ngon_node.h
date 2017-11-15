//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	David W. Nesbitt
//	File:    NGonNode.h
//	Purpose: Simple geometry node that draws an n-gon as a triangle fan.
//
//============================================================================

#ifndef __NGONNODE_H
#define __NGONNODE_H

#include <vector>
#include "scene/scene.h"

/**
 * Geometry node for an n-gon drawn as a triangle fan.
 */
class NGonNode: public GeometryNode {
public:
  /**
   * Constructor.
   * @param  center  Center of the n-gon
   * @param  nsides  Number of sides.     
   * @param  radius  Radius.
   * @param  position_loc  Shader location for vertex position.
   */
  NGonNode(const Point2& center, const int nsides, const float radius,
           const int position_loc) {
    // Create the vertex list. The first vertex is the center, then rotate
    // find points on a circle of specified radius to create the specified
    // number of sides;
    vertex_list.push_back(center);
    float angle = 0.0f;
    float da = (2.0f * kPi) / static_cast<float>(nsides);
    for (int i = 0; i < nsides; i++, angle += da) {
      vertex_list.push_back({ center.x + (radius * cosf(angle)),
                              center.y + (radius * sinf(angle)) });
    }

    // Close the shape
    vertex_list.push_back({ center.x + radius, center.y });
   
    // Add the points to a VBO.
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_list.size() * sizeof(Point2),
                 (GLvoid*)&vertex_list[0], GL_STATIC_DRAW);
    nverts = vertex_list.size();

    // VAO
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(position_loc);
    glVertexAttribPointer(position_loc, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

    // Remvoe the first vertex and last vertex in the list (for use in 
    // intersection computations with the outer boundary of the ngon
    vertex_list.erase(vertex_list.begin());
    vertex_list.pop_back();
  }

  /**
   * Destructor. Delete VBO and VAO.
   */
  virtual ~NGonNode() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
  }

	/**
	 * Draw the lines
   * @param  scene_state  Current scene state.
	 */
	virtual void Draw(SceneState& scene_state) {
    // Bind the VAO and draw the line
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_FAN, 0, nverts);
    glBindVertexArray(0);
    CheckError("End of n-gon:");
  }

  /**
   * Get the vertex list for intersection computations. Removes the
   * first vertex.
   * @return  Returns a const address of the vertex vector.
   */
  const std::vector<Point2>& get_vertex_list() const {
    return vertex_list;
  }

protected:
  int nverts;                       // Number of vertices in the VBO.
  GLuint vbo;                       // VBO
  GLuint vao;                       // Vertex Array Object
  std::vector<Point2> vertex_list;  // Vertex list (excluding first vertex)
};

#endif
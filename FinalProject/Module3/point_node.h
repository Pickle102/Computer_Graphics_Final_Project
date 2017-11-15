//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	David W. Nesbitt
//	File:    PointNode.h
//	Purpose:	Simple geometry node that draws points.
//
//============================================================================

#ifndef __POINTNODE_H
#define __POINTNODE_H

#include <vector>
#include "scene/scene.h"

/**
 * Geometry node for adding and drawing points.
 */
class PointNode: public GeometryNode {
public:
  /**
   * Constructor.
   * @param  capacity      Maximum number of vertices in the VBO.
   * @param  position_loc  Shader vertex position attribute location.
   */
  PointNode(const int capacity, const int position_loc) { 
    // Set up a buffer for dynamic draw with the specified capacity.
    pos_loc = position_loc;
    std::vector<Point2> pts(capacity);
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, capacity * sizeof(Point2), (GLvoid*)&pts[0], GL_DYNAMIC_DRAW);

    // Allocate a VAO, enable it and set the vertex attribute arrays and pointers
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(position_loc, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
    glEnableVertexAttribArray(position_loc);

    // Initial vertex count is 0
    vertex_count = 0;
  }
	
  /**
   * Destructor
   */
  virtual ~PointNode() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
  }

  /**
  * Adds vertices to the list. Reloads the point VBO.
  * @param  vlist  Point vertex positions.
  * @param  position_loc  Position location (shader vertex attribute)
  */
  virtual void Update(std::vector<Point2>& vlist) {
    // Update the vertex buffer object.
    if (vlist.size() > 0) {
      glBindBuffer(GL_ARRAY_BUFFER, vbo);
      glBufferSubData(GL_ARRAY_BUFFER, 0, vlist.size() * sizeof(Point2),
             (GLvoid*)&vlist[0]);
    }
    vertex_count = vlist.size();
  }

  /**
   * Clears the vertices (sets vertex_count to 0).
   */
  void Clear() {
    vertex_count = 0;
  }

	/**
	 * Draw the set of points.
   * @param  scene_state  Current scene state.
	 */
	virtual void Draw(SceneState& scene_state) {
    if (vertex_count > 0) {
      // Bind the VAO and draw the points
      glBindVertexArray(vao);
      glDrawArrays(GL_POINTS, 0, vertex_count);
      glBindVertexArray(0);
    }
	}

protected:
  int vertex_count;   // Current number of vertices
  int pos_loc;        // Shader vertex position attribute.
  GLuint vbo;         // VBO
  GLuint vao;         // Vertex Array Object
};

#endif
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
   */
  PointNode() {
    // Create a buffer object and a vertex array object
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);
  }
	
  /**
   * Destructor
   */
  virtual ~PointNode() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
  }

  /**
  * Adds a point to the list. Reloads the point VBO.
  * @param  x   Screen x location
  * @param  y   Screen y location
  * @param  position_loc  Position location (shader vertex attribute)
  */
  virtual void Add(const float x, const float y, int position_loc) {
    vertex_list.emplace_back(Point2(x, y));

    // Add the points to the VBO. Reloads entire VBO.
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_list.size() * sizeof(Point2), 
                (GLvoid*)&vertex_list[0], GL_DYNAMIC_DRAW);

    // Update the VAO
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(position_loc);
    glVertexAttribPointer(position_loc, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
  }

	/**
	 * Draw the set of points.
   * @param  scene_state  Current scene state.
	 */
	virtual void Draw(SceneState& scene_state) {
    if (vertex_list.size() > 0) {
      // Bind the VAO and draw the points
      glBindVertexArray(vao);
      glDrawArrays(GL_POINTS, 0, vertex_list.size());
      glBindVertexArray(0);
      CheckError("End of Points:");
    }
	}

protected:
  GLuint vbo;                       // VBO
  GLuint vao;                       // Vertex Array Object
  std::vector<Point2> vertex_list;  // Vertex list
};

#endif
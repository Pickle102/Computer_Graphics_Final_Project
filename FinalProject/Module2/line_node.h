//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	David W. Nesbitt
//	File:    LineNode.h
//	Purpose:	Simple geometry node that draws a connected line.
//
//============================================================================

#ifndef __LINENODE_H
#define __LINENODE_H

#include <vector>
#include "scene/scene.h"

/**
 * Unit sphere geometry node.
 */
class LineNode: public GeometryNode {
public:
  /**
   * Constructor.
   * @param  c     Color for the line.
   */
  LineNode(const Color4& c) {
    // Copy the color
    color = c;

    // Create a buffer object and a vertex array object
    glGenBuffers(1, &vbo);
    glGenVertexArrays(1, &vao);
  }

  /**
   * Destructor. Delete VBO and VAO.
   */
  virtual ~LineNode() { 
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
	 * Draw the lines
   * @param  scene_state  Current scene state.
	 */
	virtual void Draw(SceneState& scene_state) {
    // Draw line strip if at least 2 points
    if (vertex_list.size() > 1) {
      // Set the color
      glUniform4f(scene_state.color_loc, color.r, color.g,
                  color.b, color.a);

      // Bind the VAO and draw the line
      glBindVertexArray(vao);
      glDrawArrays(GL_LINE_STRIP, 0, vertex_list.size());
      glBindVertexArray(0);
      CheckError("End of Lines:");
    }
  }

protected:
  Color4 color;                     // Color of the line
  GLuint vbo;                       // VBO
  GLuint vao;                       // Vertex Array Object
  std::vector<Point2> vertex_list;  // Vertex list
};

#endif
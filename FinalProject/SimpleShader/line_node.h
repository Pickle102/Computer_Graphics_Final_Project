//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	 David W. Nesbitt
//	File:    line_node.h
//	Purpose: Simple geometry node that draws a connected line.
//
//============================================================================

#ifndef __LINENODE_H
#define __LINENODE_H

#include <vector>
#include "scene/scene.h"

/**
 * Simple scene graph geometry node to store and draw lines.
 */
class LineNode: public GeometryNode {
public:
  /**
   * Constructor.
  */
  LineNode(std::vector<Point2>& pts, const Color4& c, 
           const int position_loc) {
    // Copy the color and set the number of vertices
    color = c;
    vertex_count = pts.size();

    // Create vertex buffer object
    glGenBuffers(1, &vbo);

    // Add the points to the buffer
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(Point2),  
                 (GLvoid*)&pts.front(), GL_STATIC_DRAW);

    // Allocate a VAO, enable it and set the vertex attribute arrays and pointers
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glEnableVertexAttribArray(position_loc);
    glVertexAttribPointer(position_loc, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);
  }
	
  /**
  * Destructor
  */
  virtual ~LineNode() {
    glDeleteBuffers(1, &vbo);
    glDeleteVertexArrays(1, &vao);
  }

  /**
   * Draw the lines
   * @param  scene_state  Current scene state.
   */
  virtual void Draw(SceneState& scene_state) {
    // Set the color
    glUniform4f(scene_state.color_loc, color.r, color.g, 
                color.b, color.a);

    // Bind the VAO and draw the lines
    glBindVertexArray(vao);
    glDrawArrays(GL_LINE_STRIP, 0, vertex_count);
    glBindVertexArray(0);
  }
protected:
  uint32_t vertex_count;    // Number of vertices
  GLuint vbo;               // VBO for positions
  GLuint vao;               // Vertex Array Object
  Color4 color;             // Color to draw the lines
};

#endif
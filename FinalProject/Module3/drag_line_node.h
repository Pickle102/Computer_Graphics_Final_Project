//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	David W. Nesbitt
//	File:   DragDragLineNode.h
//	Purpose:	Simple geometry node that draws a draggable line.
//
//============================================================================

#ifndef __DragLineNode_H
#define __DragLineNode_H

#include <vector>
#include "scene/scene.h"

struct PositionAndColor {
  Point2 position;
  Color4 color;
};

/**
 * Draggable line node.
 */
class DragLineNode: public GeometryNode
{
public:
   /**
    * Constructor.
    * @param  color1        Color at the start of the line
    * @param  color2        Color at the end of the line
    * @param  w             Line width
    * @param  position_loc  Location of vertex position attribute
    * @param  color_loc     Location of vertex color attribute
    */
	DragLineNode(const Color4& color1, const Color4& color2, const float w, 
               const int position_loc, const int color_loc) {
    width    = w;
    capacity = 2;
    start_vertex.color = color1;
    end_vertex.color = color2;

    // Create vertex buffer object
    glGenBuffers(1, &vbo);
    
    // Load dummy data into the vertex position VBO
    std::vector<LineSegment2> pts(capacity);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, capacity * 2 * sizeof(PositionAndColor), (GLvoid*)&pts[0], GL_DYNAMIC_DRAW);
      
    // Allocate a VAO, enable it and set the vertex attribute arrays and pointers
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Enable vertex attribute array and pointer so they are bound to the VAO
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(position_loc, 2, GL_FLOAT, GL_FALSE, sizeof(PositionAndColor), (void*)0);
    glEnableVertexAttribArray(position_loc);

    // Enable vertex attribute array and pointer so they are bound to the VAO
    glVertexAttribPointer(color_loc, 4, GL_FLOAT, GL_FALSE, sizeof(PositionAndColor), (void*)(sizeof(Point2)));
    glEnableVertexAttribArray(color_loc);

    // Make sure changes to this VAO are local
    glBindVertexArray(0);
	}

  /**
   * Set the line width when drawing the VBO.
   * @param  w  Line width.
   */
  void SetWidth(const float w)  {
    width = w;
  }

  /**
   * Clears the draggable line (sets draw to false).
   */
  void Clear() {
    draw = false;
  }

  /**
   * Replace the starting vertex of the draggable line.
   * @param pt  Position to set as start of the line segment
   */
  void ReplacePoint0(const Point2& pt) {
    // Replace the first vertex and color in the VBO.
    start_vertex.position = pt;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, sizeof(PositionAndColor), (GLvoid*)&start_vertex);
    draw = false;
  }

  /**
   * Replace the starting vertex of the draggable line.
   * @param pt  Position to set as start of the line segment
   */
  void ReplacePoint1(const Point2& pt) {
    // Replace the first vertex and color in the VBO.
    end_vertex.position = pt;
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferSubData(GL_ARRAY_BUFFER, sizeof(PositionAndColor), sizeof(PositionAndColor), (GLvoid*)&end_vertex);
    draw = true;
  }

  /**
   * Draw the lines
   * @param  scene_state  Current scene state.
   */
  virtual void Draw(SceneState& scene_state) {
    if (draw) {
      // Line width
      CheckError("DragLineNode - before width");
      glLineWidth(width);
      CheckError("DragLineNode - after width");

      // Draw - the count in glDrawArrays is the number of vertices in the list
      glBindVertexArray(vao);
      glDrawArrays(GL_LINES, 0, 2);
      glBindVertexArray(0);
    }
  }

protected:
  bool draw;
  GLuint vao;                     // Vertex Array Object
  GLuint vbo;                     // VBO for positions
  float  width;                   // Line width
  uint32_t capacity;              // Maximum number of lines
  PositionAndColor start_vertex;  // Start vertex (position and color)
  PositionAndColor end_vertex;    // End vertex (position and color)
};

#endif
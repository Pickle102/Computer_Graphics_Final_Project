//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	David W. Nesbitt
//	File:    UnitSquareSurface.h
//	Purpose:	Simple geometry node that defines a unit square.
//
//============================================================================

#ifndef __UNITSQUARESURFACE_H
#define __UNITSQUARESURFACE_H

#include <vector>
#include "scene/scene.h"

/**
 * Unit square geometry node.
 */
class UnitSquareSurface1: public GeometryNode {
public:
  /**
   * Constructor. Construct the vertex list for a triangle strip
   * representing a unit square in the x,y plane. Make sure the 
   * vertices alternate top to bottom in y. All normals are (0,0,1).
   */
  UnitSquareSurface1(const int position_loc, const int normal_loc) 
      : GeometryNode() {
    std::vector<VertexAndNormal> vertex_list;
    VertexAndNormal vtx;
    vtx.normal.x = 0.0f;
    vtx.normal.y = 0.0f;
    vtx.normal.z = 1.0f;
    vtx.vertex.x = -0.5f;
    vtx.vertex.y = 0.5f;
    vtx.vertex.z = 0.0f;
    vertex_list.push_back(vtx);
    vtx.vertex.x = -0.5f;
    vtx.vertex.y = -0.5f;
    vertex_list.push_back(vtx);
    vtx.vertex.x = 0.5f;
    vtx.vertex.y = 0.5f;
    vertex_list.push_back(vtx);
    vtx.vertex.x = 0.5f;
    vtx.vertex.y = -0.5f;
    vertex_list.push_back(vtx);
    vertex_count = vertex_list.size();

    // Create a buffer object and load the data
    glGenBuffers(1, &vbo);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertex_count * sizeof(VertexAndNormal), 
              (GLvoid*)&vertex_list[0].vertex.x, GL_STATIC_DRAW);
	
    // Allocate a VAO, enable it and set the vertex attribute arrays and pointers
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    // Bind the VBO, set vertex attribute pointers for position and normal (using stride and offset).
    // Enable the arrays.
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glVertexAttribPointer(position_loc, 3, GL_FLOAT, GL_FALSE, sizeof(VertexAndNormal), (void*)0);
    glEnableVertexAttribArray(position_loc);
    glVertexAttribPointer(normal_loc,   3, GL_FLOAT, GL_FALSE, sizeof(VertexAndNormal), (void*)(sizeof(Point3)));
    glEnableVertexAttribArray(normal_loc);

    // Make sure changes to this VAO are local
    glBindVertexArray(0);
  }

   /**
    * Destructor
    */
	virtual ~UnitSquareSurface1() { }

  /**
   * Draw a unit square. Note that geometry nodes are at the
   * leaves of the tree, so no children exist.
   */
  virtual void Draw(SceneState& scene_state) {
    glBindVertexArray(vao);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, vertex_count);
    glBindVertexArray(0);
  }

protected:
   GLuint vao;            // Vertex Array Object
   GLuint vbo;            // Vertex Buffer Object
   uint32_t vertex_count; // Number of vertices in the square
};

#endif
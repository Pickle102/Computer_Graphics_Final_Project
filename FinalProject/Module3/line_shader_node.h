//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	David W. Nesbitt
//	File:    LineShaderNode.h
//	Purpose:	Derived class to handle an offset line shader and its uniforms and 
//          attribute locations.
//
//============================================================================

#ifndef __LINESHADERNODE_H
#define __LINESHADERNODE_H

#include <vector>
#include "scene/scene.h"

/**
 * Offset line shader node.
 */
class LineShaderNode: public ShaderNode {
public:
  /**
   * Gets uniform and attribute locations.
   */
  bool GetLocations() {
    orthomatrix_loc = glGetUniformLocation(shader_program.GetProgram(), "ortho");
    if (orthomatrix_loc < 0) {
      std::cout << "LineShaderNode: Error getting ortho matrix location" << std::endl;
      return false;
    }
    position_loc = glGetAttribLocation(shader_program.GetProgram(), "position");
    if (position_loc < 0) {
      std::cout << "LineShaderNode: Error getting vertex position location" << std::endl;
      return false;
    }
    vertexcolor_loc = glGetAttribLocation(shader_program.GetProgram(), "vertexColor");
    if (vertexcolor_loc < 0) {
      std::cout << "LineShaderNode: Error getting vertex color location" << std::endl;
      return false;
    }
    return true;
  }

  /**
   * Draw method for this shader - enable the program and set up uniforms
   * and vertex attribute locations
   * @param  sceneState   Current scene state.
   */
  virtual void Draw(SceneState& scene_state) {
    // Enable this program
    shader_program.Use();

    // Set scene state locations to ones needed for this program
    scene_state.orthomatrix_loc = orthomatrix_loc;
    scene_state.position_loc = position_loc;
    scene_state.vertexcolor_loc = vertexcolor_loc;

    // Set the matrix
    glUniformMatrix4fv(orthomatrix_loc, 1, GL_FALSE, scene_state.ortho_matrix.Get());

    // Draw all children
    SceneNode::Draw(scene_state);
	}

  /**
   * Get the location of the vertex position attribute.
   * @return  Returns the vertex position attribute location.
   */
  int GetPositionLoc() const {
    return position_loc;
  }

  /**
   * Get the location of the vertex position attribute.
   * @return  Returns the vertex position attribute location.
   */
  int GetColorLoc() const {
    return vertexcolor_loc;
  }

protected:
  // Uniform and attribute locations
  GLint orthomatrix_loc;
  GLint width_loc;
  GLint position_loc;
  GLint vertexcolor_loc;
};

#endif
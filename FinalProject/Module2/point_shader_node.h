//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	David W. Nesbitt
//	File:    PointShaderNode.h
//	Purpose:	Derived class to handle a point shader and its uniforms and 
//          attribute locations.
//
//============================================================================

#ifndef __POINTSHADERNODE_H
#define __POINTSHADERNODE_H

#include <vector>
#include "scene/scene.h"

/**
 * Point shader node.
 */
class PointShaderNode: public ShaderNode {
public:
  /**
   * Gets uniform and attribute locations.
   */
  bool GetLocations() {
    orthomatrix_loc = glGetUniformLocation(shader_program.GetProgram(), "ortho");
    if (orthomatrix_loc < 0) {
      std::cout << "Error getting ortho matrix location" << std::endl;
      return false;
    }
    position_loc = glGetAttribLocation(shader_program.GetProgram(), "position");
    if (position_loc < 0) {
      std::cout << "Error getting vertex position location" << std::endl;
      return false;
    }
    return true;
  }

  /**
	 * Draw method for this shader - enable the program and set up uniforms
    * and vertex attribute locations
    * @param  scene_state   Current scene state.
	 */
	virtual void Draw(SceneState& scene_state) {
    // Enable this program
    shader_program.Use();

    // Set scene state locations to ones needed for this program
    scene_state.orthomatrix_loc = orthomatrix_loc;
    scene_state.position_loc = position_loc;

    // Set the matrix
    glUniformMatrix4fv(orthomatrix_loc, 1, GL_FALSE, scene_state.ortho);

    // Draw all children
    SceneNode::Draw(scene_state);
	}

  /**
   * Get the vertex position attribute location
   * @return  Returns the position attribute location.
   */
  int GetPositionLoc() const {
    return position_loc;
  }

protected:
   // Uniform and attribute locations used by this shader
   GLint orthomatrix_loc;
   GLint position_loc;
};

#endif
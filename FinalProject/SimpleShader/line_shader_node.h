//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt

//	Author:	  David W. Nesbitt
//	File:     LineShaderNode.h
//	Purpose:	Derived class to handle an offset line shader and its uniforms and 
//          attribute locations.
//============================================================================

#ifndef __LINESHADERNODE_H
#define __LINESHADERNODE_H

#include "scene/scene.h"

/**
 * Simple line shader node.
 */
class LineShaderNode: public ShaderNode {
public:
  /**
   * Gets uniform and attribute locations.
   */
  bool GetLocations()  {
  position_loc = glGetAttribLocation(shader_program.GetProgram(), "position");
    if (position_loc < 0) {
      printf("Error getting vertex position location\n");
      return false;
    }
    orthomatrix_loc = glGetUniformLocation(shader_program.GetProgram(), "ortho");
    if (orthomatrix_loc < 0) {
      printf("Error getting ortho matrix location\n");
      return false;
    }
    color_loc = glGetUniformLocation(shader_program.GetProgram(), "color");
    if (color_loc < 0) {
      printf("Error getting color location\n");
      return false;
    }
    return true;
  }

	/**
    * Draw method for this shader - enable the program and set up uniforms
    * and vertex attribute locations
    * @param  scenestate   Current scene state.
	 */
	virtual void Draw(SceneState& scenestate) {
    // Enable this program
    shader_program.Use();

    // Set scene state locations to ones needed for this program
    scenestate.position_loc    = position_loc;
    scenestate.orthomatrix_loc = orthomatrix_loc;
    scenestate.color_loc       = color_loc;

    // Set the matrix
    glUniformMatrix4fv(orthomatrix_loc, 1, GL_FALSE, scenestate.ortho);

    // Draw all children
    SceneNode::Draw(scenestate);
	}

   /**
    * Get the location of the vertex position attribute.
    * @return  Returns the vertex position attribute location.
    */
   int GetPositionLoc() const {
      return position_loc;
   }

protected:
   // Uniform and attribute locations
   GLint orthomatrix_loc;
   GLint color_loc;
   GLint position_loc;
};

#endif
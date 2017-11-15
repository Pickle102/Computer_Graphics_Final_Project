//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	David W. Nesbitt
//	File:    LightingShaderNode.h
//	Purpose: Derived class to handle the lighting shader program.
//
//============================================================================

#ifndef __LIGHTINGSHADERNODE_H
#define __LIGHTINGSHADERNODE_H

#include <vector>

/**
 * Simple lighting shader node.
 */
class LightingShaderNode: public ShaderNode {
public:
  /**
   * Gets uniform and attribute locations.
   */
  bool GetLocations() {
    position_loc = glGetAttribLocation(shader_program.GetProgram(), "vertexPosition");
    if (position_loc < 0) {
      std::cout << "Error getting vertexPosition location" << std::endl;
      return false;
    }
    vertexnormal_loc = glGetAttribLocation(shader_program.GetProgram(), "vertexNormal");
    if (vertexnormal_loc < 0) {
      std::cout << "Error getting vertexNormal location" << std::endl;
      return false;
    }
    materialcolor_loc = glGetUniformLocation(shader_program.GetProgram(), "materialColor");
    if (materialcolor_loc < 0) {
      std::cout << "Error getting materialColor location" << std::endl;
      return false;
    }
    pvm_loc = glGetUniformLocation(shader_program.GetProgram(), "pvm");
    if (pvm_loc < 0) {
      std::cout << "Error getting pvm location" << std::endl;
      return false;
    }
    modelmatrix_loc = glGetUniformLocation(shader_program.GetProgram(), "modelMatrix");
    if (modelmatrix_loc < 0) {
    std::cout <<"Error getting modelViewMatrix location" << std::endl;
    return false;
    }

    normalmatrix_loc = glGetUniformLocation(shader_program.GetProgram(), "normalMatrix");
    if (normalmatrix_loc < 0) {
      std::cout << "Error getting normalMatrix location" << std::endl;
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
    scene_state.position_loc = position_loc;
    scene_state.normal_loc = vertexnormal_loc;
    scene_state.materialdiffuse_loc = materialcolor_loc;
    scene_state.pvm_loc = pvm_loc;
    scene_state.modelmatrix_loc = modelmatrix_loc;
    scene_state.normalmatrix_loc = normalmatrix_loc;

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
   * Get the location of the vertex normal attribute.
   * @return  Returns the vertex normal attribute location.
   */
  int GetNormalLoc() const {
    return vertexnormal_loc;
  }

protected:
  // Uniform and attribute locations:
  GLint position_loc;	        // Vertex position attribute location
  GLint vertexnormal_loc;		  // Vertex normal attribute location
  GLint materialcolor_loc;	  // Material diffuse color location
  GLint pvm_loc;				      // Composite projection, view, model matrix location
  GLint modelmatrix_loc;	    // Modeling composite matrix location
  GLint normalmatrix_loc;		  // Normal transformation matrix location
};

#endif
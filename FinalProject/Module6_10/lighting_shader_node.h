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
    texture_loc = glGetAttribLocation(shader_program.GetProgram(), "texturePosition");
    if (texture_loc < 0) {
      std::cout << "LightingShaderNode: Error getting vertex texture location" << std::endl;
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

    // Populate camera position uniform location in scene state
    cameraposition_loc = glGetUniformLocation(shader_program.GetProgram(), "cameraPosition");

    // Set the number of lights to 2 for now
    light_count = 3;
    lightcount_loc = glGetUniformLocation(shader_program.GetProgram(), "numLights");
    if (lightcount_loc < 0) {
      std::cout << "LightingShaderNode: Error getting lightcount_loc location" << std::endl;
    }

    // Get light uniforms
    char name[128];
    for (int i = 0; i < light_count; i++) {
      sprintf(name, "lights[%d].enabled", i);
      lights[i].enabled = glGetUniformLocation(shader_program.GetProgram(), name);
      sprintf(name, "lights[%d].spotlight", i);
      lights[i].spotlight = glGetUniformLocation(shader_program.GetProgram(), name);
      sprintf(name, "lights[%d].position", i);
      lights[i].position = glGetUniformLocation(shader_program.GetProgram(), name);
      sprintf(name, "lights[%d].ambient", i);
      lights[i].ambient = glGetUniformLocation(shader_program.GetProgram(), name);
      sprintf(name, "lights[%d].diffuse", i);
      lights[i].diffuse = glGetUniformLocation(shader_program.GetProgram(), name);
      sprintf(name, "lights[%d].specular", i);
      lights[i].specular = glGetUniformLocation(shader_program.GetProgram(), name);
      sprintf(name, "lights[%d].constantAttenuation", i);
      lights[i].att_constant = glGetUniformLocation(shader_program.GetProgram(), name);
      sprintf(name, "lights[%d].linearAttenuation", i);
      lights[i].att_linear = glGetUniformLocation(shader_program.GetProgram(), name);
      sprintf(name, "lights[%d].quadraticAttenuation", i);
      lights[i].att_quadratic = glGetUniformLocation(shader_program.GetProgram(), name);
      sprintf(name, "lights[%d].spotCosCutoff", i);
      lights[i].spot_cutoffcos = glGetUniformLocation(shader_program.GetProgram(), name);
      sprintf(name, "lights[%d].spotExponent", i);
      lights[i].spot_exponent = glGetUniformLocation(shader_program.GetProgram(), name);
      sprintf(name, "lights[%d].spotDirection", i);
      lights[i].spot_direction = glGetUniformLocation(shader_program.GetProgram(), name);
    }

    globalambient_loc = glGetUniformLocation(shader_program.GetProgram(), "globalLightAmbient");
    if (globalambient_loc < 0) {
      std::cout << "LightingShaderNode: Error getting global ambient location" << std::endl;
    }
   
    // TODO - may want to check for errors - however any uniforms that are not yet
    // used will be "optimized out" during the compile and can return loc < 0

    // Populate material uniform locations in scene state 
    materialambient_loc = glGetUniformLocation(shader_program.GetProgram(), "materialAmbient");
    materialdiffuse_loc = glGetUniformLocation(shader_program.GetProgram(), "materialDiffuse");
    materialspecular_loc = glGetUniformLocation(shader_program.GetProgram(), "materialSpecular");
    materialemission_loc = glGetUniformLocation(shader_program.GetProgram(), "materialEmission");
    materialshininess_loc = glGetUniformLocation(shader_program.GetProgram(), "materialShininess");

    // Populate texture locations
    usetexture_loc = glGetUniformLocation(shader_program.GetProgram(), "useTexture");
    textureunit_loc = glGetUniformLocation(shader_program.GetProgram(), "texImage");
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
    scene_state.texture_loc = texture_loc;
    scene_state.pvm_loc = pvm_loc;
    scene_state.modelmatrix_loc = modelmatrix_loc;
    scene_state.normalmatrix_loc = normalmatrix_loc;
    scene_state.cameraposition_loc = cameraposition_loc;

    // Set material uniform location
    scene_state.materialambient_loc = materialambient_loc;
    scene_state.materialdiffuse_loc = materialdiffuse_loc;
    scene_state.materialspecular_loc = materialspecular_loc;
    scene_state.materialemission_loc = materialemission_loc;
    scene_state.materialshininess_loc = materialshininess_loc;

    // Set texture uniform locations
    scene_state.usetexture_loc = usetexture_loc;
    scene_state.textureunit_loc = textureunit_loc;

    // Set the light locations
    scene_state.lightcount_loc = lightcount_loc;
    for (int i = 0; i < light_count; i++) {
      scene_state.lights[i] = lights[i];
    }

    // Draw all children
    SceneNode::Draw(scene_state);
	}

  /**
   * Set the global ambient lighting property. This sets uniforms in the
   * shader directly.
   * @param  global_ambient  Color/intensity of global ambient lighting.
   */
  void SetGlobalAmbient(const Color4& global_ambient) {
    shader_program.Use();
    glUniform4fv(globalambient_loc, 1, &global_ambient.r);
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

  /**
   * Get the location of the vertex texture attribute.
   * @return  Returns the vertex texture attribute location.
   */
  int GetTextureLoc() const {
    return texture_loc;
  }

protected:
  // Uniform and attribute locations:
  GLint position_loc;	        // Vertex position attribute location
  GLint vertexnormal_loc;		  // Vertex normal attribute location
  GLint texture_loc;          // Vertex texture attribute location
  GLint pvm_loc;				      // Composite projection, view, model matrix location
  GLint modelmatrix_loc;	    // Modeling composite matrix location
  GLint normalmatrix_loc;		  // Normal transformation matrix location
  GLint cameraposition_loc;   // Camera position uniform location

  // Material uniform locations
  GLint materialambient_loc;  // Material ambient location
  GLint materialdiffuse_loc;  // Material diffuse location
  GLint materialspecular_loc; // Material specular location
  GLint materialemission_loc; // Material emission location
  GLint materialshininess_loc; // Material shininess location

  // Texture uniform locations
  GLint usetexture_loc;       // Texture use flag location
  GLint textureunit_loc;      // Texture unit location
  
  // Lighting uniforms
  int light_count;            // Number of lights
  GLint lightcount_loc;       // Light count uniform locations
  GLint globalambient_loc;    // Global ambient uniform location
  LightUniforms lights[2];    // Light source uniform locations
};

#endif
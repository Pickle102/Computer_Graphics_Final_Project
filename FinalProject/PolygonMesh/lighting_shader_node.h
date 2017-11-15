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

#ifndef __LIGHTINGSHADERNODE_H
#define __LIGHTINGSHADERNODE_H

#include "scene/scene.h"

/**
 * Lighting shader node.
 */
class LightingShaderNode: public ShaderNode {
public:
   /**
    * Gets uniform and attribute locations.
    */
   bool GetLocations() {
    position_loc = glGetAttribLocation(shader_program.GetProgram(), "vertexPosition");
    if (position_loc < 0) {
      std::cout << "LightingShaderNode: Error getting vertex position location" << std::endl;
      return false;
    }
    normal_loc = glGetAttribLocation(shader_program.GetProgram(), "vertexNormal");
    if (normal_loc < 0) {
      std::cout << "LightingShaderNode: Error getting vertex normal location" << std::endl;
      return false;
    }
    texture_loc = glGetAttribLocation(shader_program.GetProgram(), "texturePosition");
    if (texture_loc < 0) {
      std::cout << "LightingShaderNode: Error getting vertex texture location" << std::endl;
      return false;
    }

    globalambient_loc = glGetUniformLocation(shader_program.GetProgram(), "globalLightAmbient");
    if (globalambient_loc < 0) {
      std::cout << "LightingShaderNode: Error getting global ambient location" << std::endl;
      return false;
    }

    // Set the number of lights to 3
    light_count = 3;
    lightcount_loc = glGetUniformLocation(shader_program.GetProgram(), "numLights");
    if (lightcount_loc < 0) {
      std::cout << "LightingShaderNode: Error getting numLights Loc location" << std::endl;
      return false;
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
      
    // Populate matrix uniform locations in scene state
    pvm_loc = glGetUniformLocation(shader_program.GetProgram(), "pvm");
    modelmatrix_loc = glGetUniformLocation(shader_program.GetProgram(), "modelMatrix");
    normalmatrix_loc = glGetUniformLocation(shader_program.GetProgram(), "normalMatrix");
  
    // Populate material uniform locations in scene state 
    materialambient_loc   = glGetUniformLocation(shader_program.GetProgram(), "materialAmbient");
    materialdiffuse_loc   = glGetUniformLocation(shader_program.GetProgram(), "materialDiffuse");
    materialspecular_loc  = glGetUniformLocation(shader_program.GetProgram(), "materialSpecular");
    materialemission_loc  = glGetUniformLocation(shader_program.GetProgram(), "materialEmission");
    materialshininess_loc = glGetUniformLocation(shader_program.GetProgram(), "materialShininess");

    // Populate texture locations
    usetexture_loc = glGetUniformLocation(shader_program.GetProgram(), "useTexture");
    textureunit_loc = glGetUniformLocation(shader_program.GetProgram(), "texImage");

    // Populate camera position uniform location in scene state
    cameraposition_loc = glGetUniformLocation(shader_program.GetProgram(), "cameraPosition");
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
    scene_state.lightcount_loc = lightcount_loc;
    scene_state.position_loc = position_loc;
    scene_state.normal_loc = normal_loc;
    scene_state.cameraposition_loc = cameraposition_loc;
    scene_state.pvm_loc = pvm_loc;
    scene_state.modelmatrix_loc = modelmatrix_loc;
    scene_state.normalmatrix_loc = normalmatrix_loc;
    scene_state.materialambient_loc = materialambient_loc;
    scene_state.materialdiffuse_loc = materialdiffuse_loc;
    scene_state.materialspecular_loc = materialspecular_loc;
    scene_state.materialemission_loc = materialemission_loc;
    scene_state.materialshininess_loc = materialshininess_loc;
    scene_state.usetexture_loc = usetexture_loc;
    scene_state.textureunit_loc = textureunit_loc;

    // Set the light locations
    for (int i = 0; i < light_count; i++) {
      scene_state.lights[i] = lights[i];
    }

    // Draw all children
    SceneNode::Draw(scene_state);
  }

  /**
   * Set the lighting
   */
  void SetGlobalAmbient(const Color4& global_ambient) {
    shader_program.Use();
    glUniform4fv(globalambient_loc, 1, &global_ambient.r);
  }


  /**
   * Set light (used for Lighting and Viewing HW)
   */
  void SetLight(const unsigned int n, const HPoint3& position, const Color4& ambient,
                const Color4& diffuse, const Color4& specular) {
    shader_program.Use();
    glUniform1i(lights[n].enabled, 1);
    glUniform4fv(lights[n].position, 1, &position.x);
    glUniform4fv(lights[n].ambient, 1, &ambient.r);
    glUniform4fv(lights[n].diffuse, 1, &diffuse.r);
    glUniform4fv(lights[n].specular, 1, &specular.r);
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
  int GetNormalLoc() const {
    return normal_loc;
  }

  /**
   * Get the location of the vertex texture attribute.
   * @return  Returns the vertex texture attribute location.
   */
  int GetTextureLoc() const {
    return texture_loc;
  }

protected:
   // Uniform and attribute locations
   GLint position_loc;
   GLint normal_loc;
   GLint texture_loc;
   GLint pvm_loc;
   GLint modelmatrix_loc;
   GLint normalmatrix_loc;
   GLint materialambient_loc;
   GLint materialdiffuse_loc;
   GLint materialspecular_loc;
   GLint materialemission_loc;
   GLint materialshininess_loc;
   GLint cameraposition_loc;
   GLint globalambient_loc;
   GLint usetexture_loc;
   GLint textureunit_loc;

   int light_count;
   GLint lightcount_loc;
   LightUniforms lights[3];
};

#endif
//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	 David W. Nesbitt
//	File:    lightnode.h
//	Purpose: Scene graph light node.
//
//============================================================================

#ifndef __LIGHTNODE_H_
#define __LIGHTNODE_H_

/**
 * Light node. Sets individual light source properties.
 */
class LightNode : public SceneNode {
public:
	/**
	 * Constructor given the index (light number).
   * @param  idx  Light index.
	 */
	LightNode(const uint32_t idx) {
    node_type = SCENE_LIGHT;
    index = idx;

    // Set default attenuation to 1 and spotlight to false
    enabled = false;
    atten0 = 1.0f;
    atten1 = 0.0f;
    atten2 = 0.0f;
    is_spotlight = false;
		
    // Note: color constructors default rgb to 0 and alpha to 1
	}
	
	/** 
	 * Enable this light source
	 */
	void Enable() {
		enabled = true;
	}
	
	/** 
	 * Disable this light source
	 */
	void Disable() {
		enabled = false;
	}
		
	/**
	 * Set ambient light illumination.
   * @param  c  Ambient light color/intensity
	 */
	void SetAmbient(const Color4& c) {
		ambient = c;
	}
	
	/**
	 * Set diffuse light illumination.
   * @param  c  Diffuse light color/intensity
	 */
	void SetDiffuse(const Color4& c) {
		diffuse = c;
	}
	
	/**
	 * Set specular light illumination.
   * @param  c  Specular light color/intensity
	 */
	void SetSpecular(const Color4& c) {
		specular = c;
	}
	
	/**
	 * Set the light position. Uses a homogeneous coordinate. If w = 0 the light is 
	 * directional. Position is set within the Draw method.
   * @param  pos Light position.
	 */
	void SetPosition(const HPoint3& pos) {
		position = pos;

    // If directional light - make sure the position represents a unit vector
    if (position.w == 0.0f) {
      Vector3 L(position.x, position.y, position.z);
      L.Normalize();
      position.x = L.x;
      position.y = L.y;
      position.z = L.z;
    }
	}

  /**
   * Set spotlight parameters.
   * @param  dir     Spotlight direction vector.
   * @param  exp     Spotlight exponent.
   * @param  cutoff  Spotlight cutoff angle in degrees.
   */
  void SetSpotlight(const Vector3& dir, const float exp,
                    const float cutoff) {
    spot_direction = dir;
    spot_exponent  = exp;
    spot_cutoffcos = cosf(DegreesToRadians(cutoff));
    is_spotlight   = true;
  }

  /**
   * Set the spotlight direction,
   * @param  dir  Set/update the spotlight direction vector.
   */
  void SetSpotlightDirection(const Vector3& dir) {
    spot_direction = dir;
  }
	
  /**
   * Disable spotlight (turns back into point light source).
   */
  void TurnOffSpotlight() {
    is_spotlight = false;
  }

  /**
   * Set attenuation factors.
   * @param  constant  Constant factor.
   * @param  linear    Linear factor.
   * @param  quadratic Quadratic factor.
   */
  void SetAttenuation(const float constant, const float linear, 
                      const float quadratic) {
    atten0 = constant;
    atten1 = linear;
    atten2 = quadratic;
  }

	/**
	 * Draw. Sets the light properties if enabled. Note that only position
   * is set within the Draw method - since it needs to be transformed by
   * the current matrix. 
   * @param  scene_state  Current scene state.
	 */
	void Draw(SceneState& scene_state) {
    glUniform1i(scene_state.lights[index].enabled, static_cast<int>(enabled));
		if (enabled){
      glUniform1i(scene_state.lights[index].spotlight, static_cast<int>(is_spotlight));
      glUniform4fv(scene_state.lights[index].position, 1, &position.x);
      glUniform4fv(scene_state.lights[index].ambient, 1, &ambient.r);
      glUniform4fv(scene_state.lights[index].diffuse, 1, &diffuse.r);
      glUniform4fv(scene_state.lights[index].specular, 1, &specular.r);
      glUniform1f(scene_state.lights[index].att_constant, atten0);
      glUniform1f(scene_state.lights[index].att_linear, atten1);
      glUniform1f(scene_state.lights[index].att_quadratic, atten2);
      if (is_spotlight) {
        // Note we use cos of the spotlight cutoff angle so we don't have
        // to compute cos in the shader
        glUniform1f(scene_state.lights[index].spot_cutoffcos, spot_cutoffcos);
        glUniform3fv(scene_state.lights[index].spot_direction, 1, &spot_direction.x);
        glUniform1f(scene_state.lights[index].spot_exponent, spot_exponent);
      }

      // Track the maximum light index that is enabled
      if (index >= (uint32_t)scene_state.max_enabled_light) {
        glUniform1i(scene_state.lightcount_loc, index + 1);
        scene_state.max_enabled_light = index;
      }
    }

    // Draw children of this node
    SceneNode::Draw(scene_state);

    // To be proper we should disable this light so it does not impact any nodes that 
    // are not descended from this node
    glUniform1i(scene_state.lights[index].enabled, 0);
	}
	
protected:
  bool     enabled;
  bool     is_spotlight;
  uint32_t index;
  Color4   ambient;
  Color4   diffuse;
  Color4   specular;
  Vector3  spot_direction;
  float    spot_cutoffcos;
  float    spot_exponent;
  float    atten0;
  float    atten1; 
  float    atten2;
	
  // Light position as a homogeneous coordinate. If w = 0 the light is directional
  HPoint3  position;
};

#endif
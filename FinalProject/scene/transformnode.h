//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	David W. Nesbitt
//	File:    TransformNode.h
//	Purpose:	Scene graph transformation node.
//
//============================================================================

#ifndef __TRANSFORMNODE_H
#define __TRANSFORMNODE_H

#include "geometry/geometry.h"

/**
 * Transform node. Applies a transformation. This class allows OpenGL style 
 * transforms applied to the scene graph.
 */
class TransformNode : public SceneNode {
public:
  /**
   * Constructor.
   */
  TransformNode() {
    node_type = SCENE_TRANSFORM;
    reference_count = 0;
    LoadIdentity();
    scaleX = 1;
    scaleY = 1;
  }

  /**
   * Destructor.
   */
  virtual ~TransformNode() { }

  /**
   * Set the identity matrix
   */
  void LoadIdentity() {
    model_matrix.SetIdentity();
    scaleX = 1;
    scaleY = 1;
  }

  /**
   * Apply a translation
   * @param  x  x translation
   * @param  y  y translation
   * @param  z  z translation
   */
  void Translate(const float x, const float y, const float z) {
    model_matrix.Translate(x, y, z);
  }

  /**
  * Apply a general axis rotation.
  * @param  deg  Degrees counter-clockwise rotation.
  * @param  v    Rotation axis
  */
  void Rotate(const float deg, Vector3& v) {
    model_matrix.Rotate(deg, v.x, v.y, v.z);
  }

  /**
   * Apply rotation about the x axis.
   * @param  deg  Degrees counter-clockwise rotation.
   */
  void RotateX(const float deg) {
    model_matrix.RotateX(deg);
  }

  /**
   * Apply rotation about the y axis.
   * @param  deg  Degrees counter-clockwise rotation.
   */
  void RotateY(const float deg) {
    model_matrix.RotateY(deg);
  }

  /**
   * Apply rotation about the z axis.
   * @param  deg  Degrees counter-clockwise rotation.
   */
  void RotateZ(const float deg) {
    model_matrix.RotateZ(deg);
  }

  /**
   * Apply a scaling.
   * @param  x  x scaling factor
   * @param  y  y scaling factor
   * @param  z  z scaling factor
   */
  void Scale(const float x, const float y, const float z) {
    model_matrix.Scale(x, y, z);
    scaleX *= x;
    scaleY *= y;
  }

	/**
	 * Draw this transformation node and its children
   * @param  scene_state   Current scene state
	 */
  virtual void Draw(SceneState& scene_state) {
    // Copy current transforms onto stack
    scene_state.PushTransforms();

    // Apply this modeling transform to the current modeling matrix. 
    // Note the postmultiply - this allows hierarchical transformations
    // in the scene
    scene_state.model_matrix *= model_matrix;
    glUniformMatrix4fv(scene_state.modelmatrix_loc, 1, GL_FALSE, scene_state.model_matrix.Get());

    // Set the normal transform matrix (transpose of the inverse of the model matrix).
    // This transforms normals into view coordinates
    Matrix4x4 normal_matrix = scene_state.model_matrix.GetInverse().Transpose();
    glUniformMatrix4fv(scene_state.normalmatrix_loc, 1, GL_FALSE, normal_matrix.Get());

    // Set the composite projection, view, modeling matrix
    Matrix4x4 pvm = scene_state.pv * scene_state.model_matrix;
    glUniformMatrix4fv(scene_state.pvm_loc, 1, GL_FALSE, pvm.Get());

    // Set scale uniforms
    glUniform1f(scene_state.scaley_loc, scaleX);
    glUniform1f(scene_state.scalex_loc, scaleX);

    // Draw all children
    SceneNode::Draw(scene_state);

    // Pop matrix stack to revert to prior matrices
    scene_state.PopTransforms();
	}

protected:
  Matrix4x4 model_matrix;   // Local modeling transformation

  // Current scale, for billboard fix in the shader
  float scaleX;
  float scaleY;
};

#endif

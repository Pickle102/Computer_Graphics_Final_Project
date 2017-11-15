//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	David W. Nesbitt
//	File:    CameraNode.h
//	Purpose: Provide support for setting the view and updating the view.
//
//============================================================================

#ifndef __CAMERA_H
#define __CAMERA_H

#include "geometry/geometry.h"

enum ProjectionType 	{ PERSPECTIVE, ORTHOGRAPHIC };

/**
* Camera node class.
*/
class CameraNode : public SceneNode {
public:
  /**
   * Constructor.
   */
  CameraNode() {
    node_type = SCENE_CAMERA;
    reference_count = 0;

    // Perspective settings
    fov = 50.0f;
    aspect = 1.0f;
    near_clip = 1.0f;
    far_clip = 1000.0f;

    // Initial view settings
    lpt = Point3(0.0f, 0.0f, 0.0f);
    vrp = Point3(0.0f, 0.0f, 1.0f);
    v = Vector3(0.0f, 1.0f, 0.0f);
  }

  /**
   * Draw the scene node and its children. The base class just draws the
   * children. Derived classes can use this (SceneNode::Draw()) to draw
   * all children without having to duplicate this code.
   */
  void Draw(SceneState& scene_state) {
    // Copy the current composite projection and viewing matrix to the scene state
    scene_state.pv = projection * view;

    // Set the shader PVM matrix - this will allow drawing children without a TransformNode
    glUniformMatrix4fv(scene_state.pvm_loc, 1, GL_FALSE, scene_state.pv.Get());

    // Set the camera position
    glUniform3fv(scene_state.cameraposition_loc, 1, &vrp.x);
 
    // Draw children
    SceneNode::Draw(scene_state);
  }

  /**
   * Sets the view reference point (camera position)
   *	@param	vp		View reference point.
   */
  void SetPosition(const Point3& vp) {
    vrp = vp;
    LookAt();
  }

  /**
   * Gets the view reference point (camera position)
   * @return		Returns the view reference point.
   */
  Point3 GetPosition() const {
    return vrp;
  }

  /**
   * Sets the lookat point.
   * @param	lp		Lookat point.
   */
  void SetLookAtPt(const Point3& lp) {
    lpt = lp;
    LookAt();
  }

  /**
   * Gets the lookat point.
   * @return	Returns the lookat point.
   */
  Point3 GetLookAtPt() const {
    return lpt;
  }

  /**
   * Sets the view up direction.
   * @param	vup		View up direction.
   */
  void SetViewUp(const Vector3& vup) {
    v = vup;
    LookAt();
  }

  /**
   * Gets the view plane normal (vz axis).
   * @return	Returns the view plane normal (vz)
   */
  Vector3 GetViewPlaneNormal() const {
    return n;
  }

  /**
   *	Gets the view right axis (vx axis).
   * @return	Returns the view right axis (vx)
   */
  Vector3 GetViewRight() const {
    return u;
  }

  /**
   * Gets the view up axis (vy axis).
   * @return	Returns the view up axis (vy)
   */
  Vector3 GetViewUp() const {
    return v;
  }

  /**
   * Roll the camera by the specified degrees about the VPN.
   * Rotation about n. Note that the lookat point does not change
   * @param  degrees   Rotation angle (degrees)
   */
  void Roll(const float degrees) {
    // Method below is described in FS Hill and lecture notes. Just 
    // change the two axes and reset the view matrix
    float radians = DegreesToRadians(degrees);
    float c = cosf(radians);
    float s = sinf(radians);

    Vector3 utmp = u;
    u.Set(c * utmp.x + s * v.x,
          c * utmp.y + s * v.y,
          c * utmp.z + s * v.z);
    v.Set(-s * utmp.x + c * v.x,
          -s * utmp.y + c * v.y,
          -s * utmp.z + c * v.z);
    SetViewMatrix();
  }

  /**
   * Change the pitch of the camera by the specified degrees.
   * This is a rotation about u
   * @param  degrees   Rotation angle (degrees)
   */
  void Pitch(const float degrees) {
    // Method below is described in FS Hill and lecture notes. Just 
    // change the two axes and reset the view matrix
    float radians = DegreesToRadians(degrees);
    float c = cosf(radians);
    float s = sinf(radians);
    Vector3 vtmp = v;
    v.Set(c * vtmp.x + s * n.x,
          c * vtmp.y + s * n.y,
          c * vtmp.z + s * n.z);
    n.Set(-s * vtmp.x + c * n.x,
          -s * vtmp.y + c * n.y,
          -s * vtmp.z + c * n.z);

    SetViewMatrix();

    // Reset the lookat (keep the same distance)
    Vector3 v1 = vrp - lpt;
    float d = v1.Norm();
    lpt = vrp - n * d;
  }

  /**
   * Change the heading of the camera by the specified degrees.
   * This is a rotation about v
   * @param  degrees   Rotation angle (degrees)
   */
  void Heading(const float degrees)  {
    // Method below is described in FS Hill and lecture notes. Just 
    // change the two axes and reset the view matrix
    float radians = DegreesToRadians(degrees);
    float c = cosf(radians);
    float s = sinf(radians);
    Vector3 utmp = u;
    u.Set(c * utmp.x - s * n.x,
          c * utmp.y - s * n.y,
          c * utmp.z - s * n.z);
    n.Set(s * utmp.x + c * n.x,
          s * utmp.y + c * n.y,
          s * utmp.z + c * n.z);

    SetViewMatrix();

    // Reset the lookat (keep the same distance)
    Vector3 v1 = vrp - lpt;
    float d = v1.Norm();
    lpt = vrp - n * d;
  }

  /**
   * Move and Turn.  Move the LookAt point by the specified vector.
   * The VRP is then moved so that it is the same distance from the
   * LookAt point as previous.  This effectively moves and turns.
   *
   * NOTE: a different method would involve a rotation using pitch and
   * heading and then move forward or backwards along the new VPN. I figure
   * that method is more common and wanted to present the method below
   * as an alternate way to approach the problem
   */
  void MoveAndTurn(const float right, const float up, const float forward) {
    // Construct a vector from current lookat to VRP
    Vector3 v1 = vrp - lpt;
    float d = v1.Norm();

    // Move the LookAt point by specified distances along the 3 view axes.
    Vector3 dir = u * right + v * up + n * -forward;
    lpt = lpt + dir;

    // Construct vector from new LookAt to current VRP
    Vector3 v2 = vrp - lpt;

    // Find the projection of v1 onto v2. Keep same distance as previous
    Vector3 v3 = v1.Projection(v2);
    v3.Normalize();
    v3 *= d;

    // New VRP is the new lookat point plus v3
    vrp = lpt + v3;
    LookAt();
  }

  /**
   * Slide the camera in the specified lengths along the view axes.
   * @param  x   Amount to move along u
   * @param  y   Amount to move along v
   * @param  z   Amount to move along n
   */
  void Slide(const float x, const float y, const float z) {
    Vector3 mv = u * x + v * y + n * z;
    vrp = vrp + mv;
    lpt = lpt + mv;
    LookAt();
  }

  /**
  * Gets the current matrix (used to store modeling transforms).
  *	@return	Returns the current modeling/viewing composite matrix.
  */
  Matrix4x4 GetViewMatrix() const {
    return view;
  }

  /**
   * Sets a symmetric perspective projection
   * @param  fv  Field of view angle y (degrees)
   * @param  asp Aspect ratio (width / height)
   * @param  n   Near plane distance (must be positive)
   * @param  f   Far plane distance (must be positive)
   */
  void SetPerspective(const float fv, const float asp,
                      const float n, const float f) {
    fov = fv;
    aspect = asp;
    near_clip = n;
    far_clip = f;
    SetPerspective();
  }

  /**
   * Change the field of view.
   * @param  fov  Field of view angle y (degrees)
   */
  void ChangeFieldOfView(const float fv) {
    fov = fv;
    SetPerspective();
  }

  /**
   * Change the aspect ratio.
   * @param  asp  Aspect ratio (width / height)
   */
  void ChangeAspectRatio(const float asp) {
    aspect = asp;
    SetPerspective();
  }

  /**
  * Change the near_clip and far_clip clipping planes.
  * @param  n  Near plane distance (must be positive)
  * @param  f  Far plane distance (must be positive)
  */
  void ChangeClippingPlanes(const float n, const float f) {
    near_clip = n;
    far_clip  = f;
    SetPerspective();
  }

private:
  // Perspective projection parameters
  float   fov;          // Field of view in degrees
  float   aspect;       // Aspect ratio (width / height)
  float   near_clip;    // Near clipping plane distance
  float   far_clip;     // Far clipping plane distance

  Point3  vrp;		      // View point (eye)
  Point3  lpt;		      // Lookat point

  // View axes
  Vector3  n;		      // View plane normal
  Vector3  u;		      // View right axis
  Vector3  v;		      // View up axis

  // Matrices
  Matrix4x4 view;	      // Viewing matrix
  Matrix4x4 projection; // Projection matrix

  // Sets the view axes
  void LookAt() {
    // Set the VPN, which is the vector vp - vc
    n = vrp - lpt;
    n.Normalize();

    // Get the view right axis from UP cross VPN
    u = v.Cross(n);
    u.Normalize();

    // Calculate the view up axis from VPN cross VRT. 
    // (No need to normalize here since VPN and VRT are orthogonal
    // and unit length)
    v = n.Cross(u);

    // Set the view matrix
    SetViewMatrix();
  }

  // Sets the persective projection matrix
  void SetPerspective() {
    // Get the dimensions at the near_clip clipping plane
    float h = near_clip * tanf(DegreesToRadians(fov * 0.5f));
    float w = aspect * h;

    // Set the elements of the perspective projection matrix
    projection.m00() = near_clip / w;
    projection.m01() = 0.0f;
    projection.m02() = 0.0f;
    projection.m03() = 0.0f;
    projection.m10() = 0.0f;
    projection.m11() = near_clip / h;
    projection.m12() = 0.0f;
    projection.m13() = 0.0f;
    projection.m20() = 0.0f;
    projection.m21() = 0.0f;
    projection.m22() = -(far_clip + near_clip) / (far_clip - near_clip);
    projection.m23() = -(2.0f * far_clip * near_clip) / (far_clip - near_clip);
    projection.m30() = 0.0f;
    projection.m31() = 0.0f;
    projection.m32() = -1.0f;
    projection.m33() = 0.0f;
  }

  // Create viewing transformation matrix by composing the translation 
  // matrix with the rotation matrix given by the view coordinate axes
  void SetViewMatrix() {
    // Set the view matrix using the view axes and the translation
    float x = -vrp.x;
    float y = -vrp.y;
    float z = -vrp.z;
    view.m00() = u.x;
    view.m01() = u.y;
    view.m02() = u.z;
    view.m03() = u.x * x + u.y * y + u.z * z;
    view.m10() = v.x;
    view.m11() = v.y;
    view.m12() = v.z;
    view.m13() = v.x * x + v.y * y + v.z * z;
    view.m20() = n.x;
    view.m21() = n.y;
    view.m22() = n.z;
    view.m23() = n.x * x + n.y * y + n.z * z;
    view.m30() = 0.0f;
    view.m31() = 0.0f;
    view.m32() = 0.0f;
    view.m33() = 1.0f;
  }
};

#endif

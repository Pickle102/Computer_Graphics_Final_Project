//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	David W. Nesbitt
//	File:    BallTransformNode.h
//	Purpose:	Transform node that controls the movement of a ball within
//          a confined space.
//
//============================================================================

#ifndef __BALLTRANSFORMNODE_H
#define __BALLTRANSFORMNODE_H

// TODO - use C++ 11 random number generation

/**
 * Ball transform node.
 */
class BallTransform: public TransformNode {
public:
  /**
   * Constructor given the number of frames per second.
   * @param  fps  Frames per second.
   */
  BallTransform(const float fps) {
    // Set a random initial position with x,y values between 
    // -40 and 40 and z between 25 and 75
    position.Set(getRandom(-40.0f, 40.0f), getRandom(-40.0f, 40.0f),
                  getRandom(25.0f, 75.0f));

    // Set a random radius between 3 and 7
    radius = getRandom(3.0f, 7.0f);

    // Set a random initial direction
    direction.Set(getRand01(), getRand01(), getRand01());
    direction.Normalize();
      
    // Set a random speed between 5 and 15 units per second. Convert it to 
    // speed per frame.
    speed = getRandom(5.0f, 15.0f) / fps;
    setTransform();
  }

  /**
   * Update the scene node and its children
   */
  virtual void Update(SceneState& scene_state) {
    // Check if intersection occured
    if (intersection_time != 0.0f && intersection_time < 1.0f) {
      // An intersect occured move along the current direction by parameter t, 
      // reflect the direction about the normal to the intersected plane, then move 
      // the position along the new direction by the remaining distance.
      position  = position + direction * (speed * intersection_time);
      direction = direction.Reflect(intersecting_plane.GetNormal());
      position  = position + direction * (speed * (1.0f - intersection_time));
    } else {
      // No intersection - move origin along direction vector
      position = position + (direction * speed);
    }

    // Update this node's transformations
    setTransform();       

    // Update all children
    SceneNode::Update(scene_state);
  }

   // Use the base class Draw method

  /**
   * Get the current position
   * @return  Returns the current posiiton / center of the sphere.
   */
  const Point3& GetPosition() const {
    return position;
  }

  /**
   * Gets the ball radius
   * @return   Returns the radius.
   */
  float GetRadius() const {
    return radius;
  }

  /**
   * Get the direction of travel (a unit vector).
   * @return  Returns the direction of travel.
   */
  const Vector3& GetDirection() const {
    return direction;
  }

  /**
   * Gets the speed of travel.
   * @return   Returns the speed of travel
   */
  float GetSpeed() const {
    return speed;
  }

  /**
   * Special method to set intersect time (for simple collision detection)
   * @param  t  Time until intersect (0.0 - no intersect)
   */
  void SetIntersectTime(const float t) {
    intersection_time = t;
  }

  /**
   * Get the time until nearest intersection
   * @return  Returns the time to nearest intersection (0.0 if none occurs)
   */
  float GetIntersectTime() const {
    return intersection_time;
  }

  /**
   * Set the plane of intersection (use this in collision response)
   * @param  plane  Plane of intersection
   */
  void SetIntersectingPlane(const Plane& plane) {
   intersecting_plane = plane;
  }

  /**
   * Intersection the ray with the plane. Use a radius r to indicate a 
   * sphere. The ray indicates the movement of the sphere. The return 
   * value indicates the time along the ray where an intersect of a 
   * sphere moving along the ray. A return value > 1.0 indicates no
   * intersection occurs.
   * @plane  plane  Plane to test intersection against
   */
  float IntersectWithPlane(Plane& plane) {
    // Find the signed distance of sphere at start and end of the 
    // time interval. Note that speed indicates the distance moved
    // per frame
    float dc = plane.Solve(position);
    float de = plane.Solve(position + direction * speed);

    // No intersect if both dc and de are > r
    if (dc > radius && de > radius) {
      return 100.0f;
    }

    // Intersect occurs when sphere first touches plane
    return (dc - radius) / (dc - de);
  }

  /**
   * Test if this moving ball intersect another moving ball
   * @param  ball  Other moving ball to test intersection against
   */
  bool IntersectBall(BallTransform* ball) {
    // Test if 2 balls already are intersecting. This can occur if an 
    // intersection is missed in a prior frame due to not checking all
    // possible intersections. Since ray-sphere intersection does not return
    // a value < 0 it will not detect this case.
    Vector3 vb = ball->GetPosition() - GetPosition();
    float vbn = vb.Norm();
    if (vbn < (ball->GetRadius() + radius)) {
      // Set the time of intersection at 0 and the plane of intersection 
      // (only need the normal so a reflection can occur)
      vb *= (1.0f / vbn);                        // Normalize vb      
      ball->SetIntersectTime(0.0f);
      ball->SetIntersectingPlane(Plane(ball->GetPosition(), vb));
      SetIntersectTime(0.0f);
      SetIntersectingPlane(Plane(GetPosition(), vb * -1.0f));
      return true;
    }

    // Create a ray at the position of this ball with direction 
    // vector = difference of the 2 velocity vectors. Get the length of v 
    // (note that the ray to sphere intersection requires a unit length ray
    // direction)
    Vector3 v = direction * speed - (ball->GetDirection() * ball->GetSpeed());
    float l   = v.Norm();
    Ray3 ray(position, v * (1.0f / l));

    // Construct a bounding sphere at the center of the other ball with radius equal to
    // sum of the 2 balls
    BoundingSphere sphere(ball->GetPosition(), ball->GetRadius() + radius);

    float t = ray.Intersect(sphere);
    if (t > kEpsilon && t < l + kEpsilon) {
      // Find the centers of both balls when intersection occurs - convert t into proper units
      t *= 1.0f / l;
      Point3 c1 = position + (direction * (speed * t));
      Point3 c2 = ball->GetPosition() + (ball->GetDirection() * (ball->GetSpeed() * t));

      // Find the intersect point - it is along a vector between the 2 sphere centers
      Vector3 d = (c2 - c1).Normalize();
      Point3 intPt = c1 + d * radius;

      // The plane of intersection is at the intersection point with normal along the vector between
      // the centers (normal will be opposite direction for each ball)
        
      // Set the time of intersection and the plane of intersection for both balls
      ball->SetIntersectTime(t);
      ball->SetIntersectingPlane(Plane(intPt, d));
      SetIntersectTime(t);
      SetIntersectingPlane(Plane(intPt, d * -1.0f));
      return true;
    }
    else {
      return false;
    }
  }

protected:
  float     radius;            // Radius
  float     speed;             // Speed - units per frame
  Point3    position;          // Current position
  Vector3   direction;         // Direction vector (unit length)

  // Plane where an intersection occurs
  Plane     intersecting_plane;

  // Time of interesection (0.0 if no intersection occurs)
  float     intersection_time;

  // Set the default constructor to private to force use of the 
  // one with arguments
  BallTransform() {}

  // Sets the transformation matrix
  void setTransform() {
    model_matrix.SetIdentity();
    model_matrix.Translate(position.x, position.y, position.z);
    model_matrix.Scale(radius, radius, radius);
  }

  // Create a random value between a specified minv and maxv.
  float getRandom(const float minv, const float maxv) {
    return minv + ((maxv - minv) * (float)rand() / (float)RAND_MAX);
  } 

  float getRand01() const {
    return (float)rand() / (float)RAND_MAX;
  }
};

#endif
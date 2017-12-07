//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	Joshua Griffith
//	File:    ParticleNode.h
//	Purpose:	Node for particle effect system
//
//============================================================================

#ifndef __PARTICLE_NODE_H
#define __PARTICLE_NODE_H

/**
 * Particle node base class. Stores and draws particles.
 * This currently is tuned to a fire particle system -- not generalized
 */
class ParticleNode : public TransformNode {
public:
	float     _speed;         
	Point3    _position;        
	Vector3   _direction;       
	float _size;
	float _age = 0.0f;
	float _lifeTime = 0.0f;
	float _fps;
  

  /**
   * Constructor
   * @param fps the frames per second
   */
  ParticleNode(float fps) 
  {
	  _fps = fps;
	  InitializeParticle(fps);
  }

  /*
   * This reinitializes a particle to some starting value, rather 
   * than destroying / recreating the particle
   * @param fps the frames per second
   */
  void InitializeParticle(float fps)
  {
	  _age = 0.0f;

	  // Set a random initial position with x,y values between 
	  // -40 and 40 and z between 25 and 75
	  _position.Set(getRandom(-1.0f, 1.0f), getRandom(-1.0f, 1.0f), getRandom(0.0f, 0.2f));

	  // Set a random initial direction
	  _direction.Set(getRandom(-0.3f, 0.3f), getRandom(-0.3f, 0.3f), 1.0f);
	  _direction.Normalize();

	  _size = getRandom(0.1f, 1.0f);

	  if (_size > 0.5f)
	  {
		  _speed = getRandom(5.0f, 7.0f) / fps;
		  _lifeTime = getRandom(72.0f * 0.2, 72.0f * 0.6);
	  }
	  else if (_size < 0.2)
	  {
		  _speed = getRandom(8.0f, 10.0f) / fps;
		  _lifeTime = getRandom(72.0f * 0.4, 72.0f * 1.0);
	  }
	  else
	  {
		  _speed = getRandom(6.5f, 8.0f) / fps;
		  _lifeTime = getRandom(72.0f * 0.4, 72.0f * 1.0);
	  }

	  setTransform(true);
  }

  /**
   * Destructor
   */
  virtual ~ParticleNode() { }

  /**
  * Update the particle node 
  * @param  scene_state  Current scene state
  */
  virtual void Update(SceneState& sceneState) 
  {
	  _age++;

	  if (_age > _lifeTime)
	  {
		  std::cout << "Reinit Particle" << std::endl;
		  InitializeParticle(_fps);
	  }
	  
	  _position = _position + (_direction * _speed);

	  setTransform(false);

	  // Update children of this node
	  SceneNode::Update(sceneState);
  }

  // Create a random value between a specified minv and maxv.
  float getRandom(const float minv, const float maxv) {
	  return minv + ((maxv - minv) * (float)rand() / (float)RAND_MAX);
  }

  // Get a random number
  float getRand01() const {
	  return (float)rand() / (float)RAND_MAX;
  }

  // Sets the transformation matrix
  void setTransform(bool setIdentity) {
	  
	  model_matrix.SetIdentity();
	  model_matrix.Translate(_position.x, _position.y, _position.z);
	  model_matrix.Scale(_size, _size, _size);
  }
};

#endif

//============================================================================
//	Johns Hopkins University Engineering for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:  David W. Nesbitt
//	File:    Noise.h
//	Purpose: Noise generation methods.
//============================================================================

#ifndef __NOISE_H__
#define __NOISE_H__

// NOTE - not required until 605.767!

/**
 * Noise generation methods
 */
class Noise
{
public:
  /**
   * Constructor
   */
  Noise() {
  }

  /**
   * Finds the noise at a specific 3D position. Linearly interpolates 
   * lattice noise.
   * @param  p       Position
   * @param  scale   Scale
   * @return  Returns linearly interpolated noise value.
   */
  float noise(const Point3& p, const float scale) {
    // Complete in 605.767
    return 1.0f;
  }

  /**
   * Find turbelence value.
   * @param  scale   Scale
   * @param  p       Position
   * @return  Returns a turbulence value
   */
  float turbulence(float scale, const Point3& p) {
    // Complete in 605.767
    return 1.0f;
  }

private:
};

#endif


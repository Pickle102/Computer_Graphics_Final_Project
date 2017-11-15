//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	David W. Nesbitt
//	File:    SphereSection.h
//	Purpose: Scene graph geometry node representing a section of a 
//           sphere.
//
//============================================================================

#ifndef __SPHERESECTION_H
#define __SPHERESECTION_H

#include "geometry/geometry.h"

class SphereSection : public TriSurface {
public:
	/**
    * Creates a section of a sphere with bounds given by minlat, maxlat and
    * minlng, maxlng. The number of subdivisions of each is also given. Can be 
    * used to construct a full sphere. This method uses a single vertex at the
    * north and south pole and avoids creating extra triangles for polar regions.
    * @param   minlat   Minimum latitude
    * @param   maxlat   Maximum latitude
    * @param   nlat     Number of divisions of latitude
    * @param   minlng   Minimum longitude
    * @param   maxlng   Maximum longitude
    * @param   nlng     Number of divisions of longitude
    * @param   radius   Radius of the sphere 
    */
  SphereSection(const float minlat, const float maxlat, const uint32_t nlat,
        const float minlng, const float maxlng, const uint32_t nlng,
        const float radius, const int position_loc, const int normal_loc) {
    // Convert to radians
    float minlat_rad = DegreesToRadians(minlat);
    float maxlat_rad = DegreesToRadians(maxlat);
    float minlng_rad = DegreesToRadians(minlng);
    float maxlng_rad = DegreesToRadians(maxlng);

    // Create a vertex list with unit length normals
    float lng = minlng_rad;
    float coslat, coslng, sinlng;
    float dLat = (maxlat_rad - minlat_rad) / static_cast<float>(nlat);
    float dLng = (maxlng_rad - minlng_rad) / static_cast<float>(nlng);
    VertexAndNormal vtx;
    for (float lng = minlng_rad; lng < maxlng_rad + kEpsilon; lng += dLng) {
      coslng = cosf(lng);
      sinlng = sinf(lng);
      for (float lat = maxlat_rad; lat >= minlat_rad - kEpsilon; lat -= dLat) {
        coslat = cosf(lat);
        vtx.normal.x = coslng * coslat;
        vtx.normal.y = sinlng * coslat;
        vtx.normal.z = sinf(lat);
        vtx.vertex.x = radius * vtx.normal.x;
        vtx.vertex.y = radius * vtx.normal.y;
        vtx.vertex.z = radius * vtx.normal.z;
        vertices.push_back(vtx);
      }
    }

    // Copy the first column of vertices
    for (uint32_t i = 0; i <= nlat + 1; i++) {
      vertices.push_back(vertices[i]);
    }

    // Construct face list.  There are nlat+1 rows and nlng+1 columns. Create VBOs
    ConstructRowColFaceList(nlng + 1, nlat + 1);
    CreateVertexBuffers(position_loc, normal_loc);
	}
	
private:
	// Make default constructor private to force use of the constructor
	// with number of subdivisions.
	SphereSection() { };
};

class TexturedSphereSection : public TexturedTriSurface {
public:
  /**
  * Creates a section of a sphere with bounds given by minlat, maxlat and
  * minlng, maxlng. The number of subdivisions of each is also given. Can be
  * used to construct a full sphere. This method uses a single vertex at the
  * north and south pole and avoids creating extra triangles for polar regions.
  * @param   minlat   Minimum latitude
  * @param   maxlat   Maximum latitude
  * @param   nlat     Number of divisions of latitude
  * @param   minlng   Minimum longitude
  * @param   maxlng   Maximum longitude
  * @param   nlng     Number of divisions of longitude
  * @param   radius   Radius of the sphere
  */
  TexturedSphereSection(const float minlat, const float maxlat, const uint32_t nlat,
      const float minlng, const float maxlng, const uint32_t nlng,
      const float radius, const int position_loc, const int normal_loc,
      const int texture_loc) {
    // Convert to radians
    float minlat_rad = DegreesToRadians(minlat);
    float maxlat_rad = DegreesToRadians(maxlat);
    float minlng_rad = DegreesToRadians(minlng);
    float maxlng_rad = DegreesToRadians(maxlng);

    // Create a vertex list with unit length normals
    float lng = minlng_rad;
    float coslat, coslng, sinlng;
    float dLat = (maxlat_rad - minlat_rad) / static_cast<float>(nlat);
    float dLng = (maxlng_rad - minlng_rad) / static_cast<float>(nlng);
    float ds = 1.0f / static_cast<float>(nlng);
    float dt = 1.0f / static_cast<float>(nlat);
    PNTVertex vtx;
    for (float lng = minlng_rad, s = 0.0f; lng < maxlng_rad + kEpsilon; lng += dLng, s += ds) {
      coslng = cosf(lng);
      sinlng = sinf(lng);
      for (float lat = maxlat_rad, t = 1.0f; lat >= minlat_rad - kEpsilon; lat -= dLat, t -= dt) {
        coslat = cosf(lat);
        vtx.normal.x = coslng * coslat;
        vtx.normal.y = sinlng * coslat;
        vtx.normal.z = sinf(lat);
        vtx.vertex.x = radius * vtx.normal.x;
        vtx.vertex.y = radius * vtx.normal.y;
        vtx.vertex.z = radius * vtx.normal.z;
        vtx.s = s;
        vtx.t = t;
        vertices.push_back(vtx);
      }
    }

    // Construct face list and create VBOs.  There are nlat+1 rows and 
    // nlng+1 columns.
    ConstructRowColFaceList(nlng + 1, nlat + 1);
    CreateVertexBuffers(position_loc, normal_loc, texture_loc);
  }

private:

  // Make default constructor private to force use of the constructor
  // with number of subdivisions.
  TexturedSphereSection() { };
};


#endif
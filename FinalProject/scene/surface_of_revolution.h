//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	 David W. Nesbitt
//	File:    SurfaceOfRevolution.h
//	Purpose: Scene graph geometry node representing a surface or revolution.
//
//============================================================================

#ifndef __SURFACEOFREVOLUTION_H
#define __SURFACEOFREVOLUTION_H

#include "geometry/geometry.h"

class SurfaceOfRevolution : public TriSurface {
public:
	/**
    * Construct a surface of revolution given a list of points
    * where x and z are defined. x is the distance from the axis
    * and z is the height along the axis. Rotation around the z axis is
    * performed. Profile edge must be defined with increasing z.
    * @param  v   Edge/polyline to revolve
    * @param  n   Number of angular subdivisions
    */
  SurfaceOfRevolution(std::vector<Point3>& v, uint32_t n,
    const int position_loc, const int normal_loc) {
    // Set number of rows and columns
    nrows = (uint32_t)v.size();
    ncols = n + 1;

    // Add vertices to the vertex list, compute normals
    Vector3 normal, prev_normal;
    VertexAndNormal vtx;
    auto vtx1 = v.begin();
    auto vtx2 = vtx1 + 1;
    for (uint32_t i = 0; vtx2 != v.end(); vtx1++, vtx2++, i++) {
      normal = { vtx2->z - vtx1->z, 0.0f, vtx1->x - vtx2->x };
      normal.Normalize();
      vtx.vertex = { vtx1->x, vtx1->y, vtx1->z };
      if (i == 0) {
        // Use normal for the first edge vertex
        vtx.normal = normal;
      }
      else {
        // Average normals of successive edges
        vtx.normal = (prev_normal + normal).Normalize();
      }
      vertices.push_back(vtx);

      // Copy normal for use in averaging
      prev_normal = normal;
    }

    // Store last vertex
    vtx.vertex = { vtx1->x, vtx1->y, vtx1->z };
    vtx.normal = normal;
    vertices.push_back(vtx);

    // Reverse the vertex list so we go from top to bottom so 
    // ConstructRowColFaceList forms ccw triangles
    std::reverse(vertices.begin(), vertices.end());

    // Create a rotation matrix
    Matrix4x4 m;
    m.RotateZ(360.0f / static_cast<float>(n));

    // Rotate the prior "edge" vertices
    uint32_t index = 0;  // Index to the prior edge at this row
    for (uint32_t i = 0; i < n; i++) {
      for (uint32_t j = 0; j < nrows; j++) {
        // Rotate the vertex and the normal
        vtx.vertex = m * vertices[index].vertex;
        vtx.normal = m * vertices[index].normal;
        vertices.push_back(vtx);
        index++;
      }
    }

    // Copy the first column of vertices
    for (uint32_t i = 0; i < nrows; i++) {
      vertices.push_back(vertices[i]);
    }
 
    // Construct the face list and create VBOs
    ConstructRowColFaceList(ncols, nrows);
    CreateVertexBuffers(position_loc, normal_loc);
  }
	
private:
  uint32_t nrows;
  uint32_t ncols;

   // Make default constructor private to force use of the constructor
	// with number of subdivisions.
	SurfaceOfRevolution() { };
};

class TexturedSurfaceOfRevolution : public TexturedTriSurface
{
public:
  /**
  * Construct a surface of revolution given a list of points
  * where x and z are defined. x is the distance from the axis
  * and z is the height laong the axis. Rotation around the z axis is
  * performed.
  * @param  v   Edge/polyline to revolve
  * @param  n   Number of angular subdivisions
  */
  TexturedSurfaceOfRevolution(std::vector<Point3>& v, uint32_t n, const int position_loc,
                const int normal_loc, const int texture_loc) {
    // Set number of rows and columns
    nrows = (uint32_t)v.size();
    ncols = n + 1;

    // Attempt to normalize spacing in t by "delta" between points on the profile edge
    float total_length = 0.0f;
    std::vector<float> accumulated_length;
    accumulated_length.push_back(0.0f);
    for (uint32_t i = 0; i < nrows - 1; i++) {
      total_length += (v[i + 1] - v[i]).Norm();
      accumulated_length.push_back(total_length);
    }

    // Add vertices to the vertex list, compute normals
    Vector3 normal, prev_normal;
    PNTVertex vtx;
    auto vtx1 = v.begin();
    auto vtx2 = vtx1 + 1;
    for (uint32_t i = 0; vtx2 != v.end(); vtx1++, vtx2++, i++) {
      normal = { vtx2->z - vtx1->z, 0.0f, vtx1->x - vtx2->x };
      normal.Normalize();
      vtx.vertex = { vtx1->x, vtx1->y, vtx1->z };
      vtx.s = 0.0f;
      vtx.t = accumulated_length[i] / total_length;
      if (i == 0) {
        // Use normal for the first edge vertex
        vtx.normal = normal;
      }
      else {
        // Average normals of successive edges
        vtx.normal = (prev_normal + normal).Normalize();
      }
      vertices.push_back(vtx);

      // Copy normal for use in averaging
      prev_normal = normal;
    }

    // Store last vertex
    vtx.vertex = { vtx1->x, vtx1->y, vtx1->z };
    vtx.t = 1.0f;
    vtx.normal = normal;
    vertices.push_back(vtx);

    // Reverse the vertex list so we go from top to bottom so 
    // ConstructRowColFaceList forms ccw triangles
    std::reverse(vertices.begin(), vertices.end());

    // Create a rotation matrix
    Matrix4x4 m;
    m.RotateZ(360.0f / static_cast<float>(n));

    // Rotate the prior "edge" vertices
    float ds = 1.0f / static_cast<float>(n);
    float s = ds;
    uint32_t index = 0;  // Index to the prior edge at this row
    for (uint32_t i = 0; i < n; i++, s += ds) {
      vtx.s = s;
      for (uint32_t j = 0; j < nrows; j++) {
        // Rotate the vertex and the normal
        vtx.vertex = m * vertices[index].vertex;
        vtx.normal = m * vertices[index].normal;
        vtx.t = vertices[index].t;
        vertices.push_back(vtx);
        index++;
      }
    }

    // Copy the first column of vertices
    for (uint32_t i = 0; i < nrows; i++) {
      vertices.push_back(vertices[i]);
    }

    // Construct the face list and create VBOs
    ConstructRowColFaceList(ncols, nrows);
    CreateVertexBuffers(position_loc, normal_loc, texture_loc);
  }

private:
  uint32_t nrows;
  uint32_t ncols;

  // Make default constructor private to force use of the constructor
  // with number of subdivisions.
  TexturedSurfaceOfRevolution() { };
};


#endif

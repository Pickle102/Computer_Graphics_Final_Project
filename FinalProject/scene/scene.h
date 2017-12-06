//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	David W. Nesbitt
//	File:    Scene.h
//	Purpose:	Scene graph support.
//
//============================================================================

#ifndef __SCENE_H
#define __SCENE_H

#include <vector>

enum SceneNodeType { SCENE_BASE, SCENE_PRESENTATION, 
                     SCENE_TRANSFORM, SCENE_GEOMETRY,
                     SCENE_SHADER, SCENE_CAMERA, SCENE_LIGHT };

inline void CheckError(const char* str) {
  GLenum err = glGetError();
  if (err != GL_NO_ERROR)
    printf("%s: OpenGL Error: %d\n", str, err);
}

// Convenience method to form a vertex list from a list of triangle vertices
inline std::vector<Point3> FormTriangle(const Point3& v0, const Point3& v1,
  const Point3& v2) {
  return{ v0, v1, v2 };
}

// Include other scene files
#include <stdint.h>
#include "scene/color3.h"
#include "scene/color4.h"
#include "scene/scenestate.h"
#include "scene/scenenode.h"
#include "scene/transformnode.h"
#include "scene/presentationnode.h"
#include "scene/lightnode.h"
#include "scene/geometrynode.h"
#include "scene/shadernode.h"
#include "scene/cameranode.h"
#include "scene/trisurface.h"
#include "scene/textured_trisurface.h"
#include "scene/meshteapot.h"
#include "scene/conic.h"
#include "scene/unitsquare.h"
#include "scene/spheresection.h"
#include "scene/surface_of_revolution.h"
#include "scene/torus.h"
#include "scene/modelnode.h"
#include "scene/unittriangle.h"

#endif

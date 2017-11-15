//============================================================================
//	Johns Hopkins University Whiting School of Engineering
//	605.461 Principles of Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:  David W. Nesbitt
//	File:    VectorTest.cpp
//	Purpose: Test the vector library.
//
//============================================================================

#include <stdio.h>
#include <stdarg.h>

#include "geometry/geometry.h"

// Simple logging function
void logmsg(const char *message, ...) {
  // Open file if not already opened
  static FILE *lfile = NULL;
  if (lfile == NULL) {
   lfile = fopen("MatrixTest.log", "w");
  }

  va_list arg;
  va_start(arg, message);
  vfprintf(lfile, message, arg);
  putc('\n', lfile);
  fflush(lfile);
  va_end(arg);
}

/**
 * Main - entry point for test application.
 */
int main(int argc, char* argv[]) {
  logmsg("Matrix Test");

  Matrix4x4 I;
  I.SetIdentity();
  I.Log("Identity Matrix");
   
  // Set a scaling matrix
  Matrix4x4 S;
  S.m00() = 10.0f;	
  S.m11() = 10.0f;
  S.m22() = 10.0f;
  S.Log("Scaling Matrix");

  // Set a translation matrix
  Matrix4x4 T;
  T.m03() = -5.0f;	
  T.m13() = 10.0f;
  T.m23() = 15.0f;
  T.Log("Translation Matrix");

  // Set a rotation matrix about z
  Matrix4x4 R;
  R.m00() = cosf(DegreesToRadians(45.0f));
  R.m11() = R.m00();
  R.m10() = sinf(DegreesToRadians(45.0f));
  R.m01() = -R.m10();
  R.Log("Rotation (z) Matrix");

  // Perform composite
  Matrix4x4 C = T * R * S;
  C.Log("Composite TRS");

  // Transform Cartesian point 1,1,1 (homogeneous factor assumed to be 1)
  Point3 P0(1.0f, 1.0f, 1.0f);
  HPoint3 P1 = C * P0;
  logmsg("Transformed Cartesian Point is %f %f %f %f", P1.x, P1.y, P1.z, P1.w);

  // Transform point 1,1,1,1 (homogeneous point)
  HPoint3 P(1.0f, 1.0f, 1.0f, 1.0f);
  P1 = C * P;
  logmsg("Transformed Homogeneous Point is %f %f %f %f", P1.x, P1.y, P1.z, P1.w);

  // Show how the order matters
  Matrix4x4 C2 = S * R * T;
  C2.Log("Composite SRT");
  HPoint3 P2 = C2 * P;
  logmsg("Transformed Point is %f %f %f %f", P2.x, P2.y, P2.z, P2.w);

  // Test the local Translate, Rotate, Scale methods. Should match C
  logmsg("\nTest Local Translate, Rotate, Scale");
  logmsg("Should match Composite TRS Above");
  Matrix4x4 M;
  M.Translate(-5.0f, 10.0f, 15.0f);
  M.RotateZ(45.f);
  M.Scale(10.0f, 10.0f, 10.0f);
  M.Log("After TRS: Composite matrix is");

  // New composite matrix
  C.SetIdentity();
  C.Translate(-10.0f, 20.0f, 3.0f);
  C.RotateX(60.f);                 // 60 degrees about x axis
  C.Scale(10.0f, 10.0f, 10.0f);
  C.Log("New Composite TRS");

  // Transform point 1,1,0
  Point3 P3(1.0f, 1.0f, 0.0f);
  P1 = C * P3;
  logmsg("Transformed Point is %f %f %f %f", P1.x, P1.y, P1.z, P1.w);

  // Transform the vector (1, 0, 0)
  Vector3 normal(1.0f, 0.0f, 0.0f);
  Vector3 transformed_normal = C * normal;
  logmsg("Transformed Normal is %f %f %f", transformed_normal.x,
          transformed_normal.y, transformed_normal.z); 

  // Get the transpose of C (this method leaves the contents of C unchanged)
  Matrix4x4 D = C.GetTranspose();
  D.Log("D = Transpose of C");

  // Transpose C
  C.Transpose();
  C.Log("Transpose of C");

  // Get the inverse of C
  Matrix4x4 CInverse = C.GetInverse();
  CInverse.Log("Inverse of C");

  // Verify the Inverse
  Matrix4x4 VerifyInverse = C * CInverse;
  VerifyInverse.Log("Matrix times its Inverse (Should be Identity Matrix)");

  // Test the ability to get the matrix and set it
  logmsg("Verify Set and Get Methods: Result should match rotation matrix");
  const float* mat = R.Get();
  R.Set(mat);
  R.Log("Rotation matrix after Get and Set");

  Matrix4x4 test;
  test.RotateX(45.0f);
  test.RotateY(60.0f);
  test.Scale(10.0f, 10.0f, 10.0f);
  test.Translate(50.0f, -5.0f, 10.0f);
  test.RotateY(-60.0f);
  test.RotateX(-45.0f);
  test.Log("Composite2:");

  // Test rotation about a general axis
  Matrix4x4 general;
  general.Rotate(45.0f, 1.0f, 1.0f, 0.0f);
  general.Log("Rotation about a general axis");

  // Transform a ray
  Ray3 ray1(Point3(5.0f, 0.0f, 0.0f), Vector3(1.0f, 0.0f, 0.0f));
  Ray3 tr = R * ray1;
  logmsg("Transformed Ray Origin is %f %f %f  Ray Direction = %f %f %f",
          tr.o.x, tr.o.y, tr.o.z, tr.d.x, tr.d.y, tr.d.z);

  return 1;
}
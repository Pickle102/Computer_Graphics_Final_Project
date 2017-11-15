//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:  David W. Nesbitt
//	File:    GLSL vertex shader class
//	Purpose: Support for GLSL vertex shaders. Supports loading, compiling, 
//           linking, attaching, etc.
//
//============================================================================

#ifndef __GLSLVERTEXSHADER_H__
#define __GLSLVERTEXSHADER_H__

#include <stdio.h>
#include <stdarg.h>

/**
 * GLSL vertex shader
 */
class GLSLVertexShader : public GLSLShader
{
public:
   GLSLVertexShader()  { }
   ~GLSLVertexShader() { }

  /**
   * Create vertex shader from source code chararacter array.
   * @param  vertex_source  Source code for the vertex shader.
   * @return  Returns true if successful, false if not.
   */
  bool CreateFromSource(const char* vertex_source) {
    vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertex_shader, 1, &vertex_source, NULL);
    glCompileShader(vertex_shader);
    if (!CheckCompileStatus(vertex_shader)) {
      std::cout << "Vertex shader compile failed." << std::endl;
      LogCompileError(vertex_shader);
      std::cout << "Vertex Shader Source = " << std::endl;
      std::cout << vertex_source << std::endl;
      return false;
    }
    return true;
  }

  /**
   * Create vertex shader from source code file.
   * @param  fname File name for the source code for the vertex shader.
   * @return  Returns true if successful, false if not.
   */
  bool Create(const char* fname) {
    char* source = ReadShaderSource(fname);
    bool success = CreateFromSource(source);
    delete [] source;
    return success;
  }

  /**
   * Get the vertex shader handle.
   * @return Returns a handle to the vertex shader.
   */
  GLuint Get() const {
    return vertex_shader;
  }

protected:
  GLuint vertex_shader;
};

#endif

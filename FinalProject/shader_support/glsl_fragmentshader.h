//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:  David W. Nesbitt
//	File:    GLSL fragment shader class
//	Purpose: Support for GLSL fragment shaders. Supports loading, compiling, 
//           linking, attaching, etc.
//============================================================================

#ifndef __GLSLFRAGMENTSHADER_H__
#define __GLSLFRAGMENTSHADER_H__

#include <stdio.h>
#include <stdarg.h>

/**
 * GLSL fragment shader
 */
class GLSLFragmentShader : public GLSLShader {
public:
  GLSLFragmentShader()  { }
  ~GLSLFragmentShader() { }

  /**
   * Create fragment shader from source code chararacter array.
   * @param  fragment_source  Source code for the fragment shader.
   * @return  Returns true if successful, false if not.
   */
  bool CreateFromSource(const char* fragment_source) {
    bool success = true;
    fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragment_shader, 1, &fragment_source, NULL);
    glCompileShader(fragment_shader);
    if (!CheckCompileStatus(fragment_shader)) {
      std::cout << "Fragment shader compile failed." << std::endl;
      LogCompileError(fragment_shader);
      std::cout << "Fragment Shader Source = " << std::endl;
      std::cout << fragment_source << std::endl;
      success = false;
    }
    return success;
  }

  /**
   * Create fragment shader from source code file.
   * @param  fname File name for the source code for the fragment shader.
   * @return  Returns true if successful, false if not.
   */
  bool Create(const char* fname) {
    char* source = ReadShaderSource(fname);
    bool success = CreateFromSource(source);
    delete [] source;
    return success;
  }

  /**
   * Get the fragment shader handle.
   * @return Returns a handle to the fragment shader.
   */
  GLuint Get() const {
    return fragment_shader;
  }

protected:
   GLuint fragment_shader;
};

#endif

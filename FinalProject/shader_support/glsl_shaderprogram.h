//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:  David W. Nesbitt
//	File:    GLSL shader program class
//	Purpose: Support for GLSL shader programs
//============================================================================

#ifndef __GLSLSHADERPROGRAM_H__
#define __GLSLSHADERPROGRAM_H__

#include <stdio.h>
#include <stdarg.h>

/**
 * GLSL shader program
 */
class GLSLShaderProgram {
public:
  GLSLShaderProgram()
    : shader_program(0) {
  }
  ~GLSLShaderProgram() { }

  /**
   * Create a shader program
   */
  void Create() {
    shader_program = glCreateProgram();
  }

  /**
   * Attach the specified shaders.
   */
  bool AttachShaders(GLuint vertex_shader, GLuint fragment_shader) {
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);
    if (!CheckLinkStatus()) {
      std::cout << "Shader link failed" << std::endl;
      LogLinkError();
      return false;
    }
    return true;
  }

  /**
   * Get the shader program handle
   * @return  Returns the handle to the shader program.
   */
  GLuint GetProgram() const {
    return shader_program;
  }

  /**
   * Use this shader program.
   */
  void Use() {
    glUseProgram(shader_program);
  }

protected:
   GLuint shader_program;

  /**
   * Checks the link status.
   * @return  Returns true if hte linker was successful, false if an error occured.
   */
  bool CheckLinkStatus() {
    int param = 0;
    glGetProgramiv(shader_program, GL_LINK_STATUS, &param);
    return (param == GL_TRUE);
  }

  /**
  * Logs a shader program link error to the console window.
  */
  void LogLinkError() {
    GLint    len = 0; 
    GLsizei slen = 0;
    glGetProgramiv(shader_program, GL_INFO_LOG_LENGTH , &len); 
    if (len > 1) {
      GLchar* linklog = (GLchar*)new GLchar*[len];
      glGetProgramInfoLog(shader_program, len, &slen, linklog);
      std::cout << "Program Link Log:\n" << linklog << std::endl;
      delete[] linklog;
    }
  }
};

#endif

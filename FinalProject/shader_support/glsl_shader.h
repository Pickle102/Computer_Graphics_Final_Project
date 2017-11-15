//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:  David W. Nesbitt
//	File:    Base GLSL shader class
//	Purpose: Support for loadng a shader from a file and compiling it
//
//============================================================================

#ifndef __GLSLSHADER_H__
#define __GLSLSHADER_H__

#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>

#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#define _lseek lseek
#define _open open
#define _close close
#endif

/**
 * Base shader class. Loads from file. Checks compile status.
 */
class GLSLShader {
public:
  GLSLShader()  { }
  ~GLSLShader() { }

  /**
   * Check the compile status of a shader.
   * @return  Returns true if the compile status is success.
   */
  bool CheckCompileStatus(GLuint shader) {
    int param = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &param);
    return (param == GL_TRUE);
  }

protected:
  // Utility to read a shader source file
  char* ReadShaderSource(const char* filename)  {
    if (filename == 0) {
      std::cout << "NULL filename for shader...exiting" << std::endl;
      exit(-1);
    }

    // Get file size
    std::string fname = filename;
    int f = _open(fname.c_str(), O_RDONLY);
    if (f < 0)  {
      // Try the parent directory
      fname.insert(0, "../");
      f = _open(fname.c_str(), O_RDONLY);
      if (f < 0) {
        printf("Could not open shader file %s. Also not in parent directory\n", filename);
        exit(-1);
      }
    }
    int count = _lseek(f, 0, SEEK_END);
    _close(f);

    char* content = NULL;
    FILE* fp = fopen(fname.c_str(), "rt");
    if (fp != NULL) {
      if (count > 0) {
        content = (char *)new char[count + 1];
        count = (int)fread(content, sizeof(char), count, fp);
        content[count] = '\0';
      }
      fclose(fp);
    }
    else {
      std::cout << "Could not read file " << filename << std::endl;
    }
    return content;
  } 

  /**
   * Logs a shader compile error
   */
  void LogCompileError(GLuint shader) {
    GLint    len = 0; 
    GLsizei slen = 0;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH , &len); 
    if (len > 1) {
      GLchar* compilerlog = (GLchar*)new GLchar*[len];
      glGetShaderInfoLog(shader, len, &slen, compilerlog);
      std::cout << "compiler log:\n" << compilerlog << std::endl;
      delete[] compilerlog;
    }
  }
};

// Include derived classes so application only add this one file
#include "shader_support/glsl_fragmentshader.h"
#include "shader_support/glsl_vertexshader.h"
#include "shader_support/glsl_shaderprogram.h"

#endif

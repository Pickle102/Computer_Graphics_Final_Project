//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:	David W. Nesbitt
//	File:    ShaderNode.h
//	Purpose:	Scene graph shader node.
//
//============================================================================

#ifndef __SHADERNODE_H
#define __SHADERNODE_H

/**
 * Shader node. Enables a shader program. The program is loaded with
 * different constructor methods. Derived shader node classes should 
 * provide specialization to control uniforms and attributes.
 */
class ShaderNode : public SceneNode {
public:
  /**
   * Constructor.
   */
  ShaderNode() { 
    node_type = SCENE_SHADER;
    reference_count = 0;
  }

  /**
   * Destructor.
   */
  virtual ~ShaderNode() { }

  /**
   * Create a shader program given a filename for the vertex shader and a filename
   * for the fragment shader.
   * @param  vertexShaderFilename    Vertex shader file name
   * @param  fragmentShaderFilename  Fragment shader file name
   * @return  Returns true if successful, false if compile or link errors occur.
   */
  bool Create(const char* vertexShaderFilename, const char* fragmentShaderFilename) {
    // Create and compile the vertex shader
    if (!vertex_shader.Create(vertexShaderFilename)) {
      std::cout << "Vertex Shader compile failed" << std::endl;
      return false;
    }

    // Create and compile the fragment shader
    if (!fragment_shader.Create(fragmentShaderFilename)) {
      std::cout << "Fragment Shader compile failed" << std::endl;
      return false;
    }

    shader_program.Create();
    if (!shader_program.AttachShaders(vertex_shader.Get(), fragment_shader.Get())) {
      std::cout << "Shader program link failed" << std::endl;
      return false;
    }
    return true;
  }

  /**
   * Create a shader program given source char array for the vertex shader and source
   * for the fragment shader.
   * @param  vertexShaderSource    Vertex shader source (char array)
   * @param  fragmentShaderSource  Fragment shader source (char array)
   * @return  Returns true if successful, false if compile or link errors occur.
   */
  bool CreateFromSource(const char* vertexShaderSource, const char* fragmentShaderSource) {
    // Create and compile the vertex shader
    if (!vertex_shader.CreateFromSource(vertexShaderSource)) {
      std::cout << "Vertex Shader compile failed" << std::endl;
      return false;
    }

    // Create and compile the fragment shader
    if (!fragment_shader.CreateFromSource(fragmentShaderSource)) {
      std::cout << "Fragment Shader compile failed" << std::endl;
      return false;
    }

    shader_program.Create();
    if (!shader_program.AttachShaders(vertex_shader.Get(), fragment_shader.Get())) {
      std::cout << "Shader program link failed" << std::endl;
      return false; 
    }
    return true;
  }

  // Derived classes must add this to set all internal uniforms and attribute locations
  virtual bool GetLocations() = 0;

protected:
 GLSLVertexShader   vertex_shader;
 GLSLFragmentShader fragment_shader;
 GLSLShaderProgram  shader_program;
};

#endif

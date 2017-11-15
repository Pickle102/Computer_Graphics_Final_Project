//============================================================================
//	Johns Hopkins University Engineering Programs for Professionals
//	605.467 Computer Graphics and 605.767 Applied Computer Graphics
//	Instructor:	David W. Nesbitt
//
//	Author:  David W. Nesbitt
//	File:    UnitSphere.h
//	Purpose: Scene graph geometry node representing a unit sphere. Constructed
//          using subdivision of a tetrahedron.
//
//============================================================================

#pragma once

#include "scene/scene.h"

// Assimp include files. These three are usually needed.
#include "assimp/Importer.hpp"
#include "assimp/PostProcess.h"
#include "assimp/Scene.h"

// DevIL
#include "IL/il.h"

#include <math.h>
#include <fstream>
#include <map>
#include <string>
#include <vector>

// Note - this does not handle node hierarchy and transformations
// It does handle multiple meshes and textures.

// Information to render each assimp node
struct ModelMesh {
  bool has_texture;
  GLuint texture_id;
  int numFaces;
  GLuint vao;
  GLuint position_vbo;
  GLuint normal_vbo;
  GLuint texture_vbo;
};

/**
 * Node that loads a model using Assimp.
 */
class ModelNode : public SceneNode {
public:
  /**
   * Constructor.  Loads the given model.  (Implements a modified version
   * of: http://www.opengl-tutorial.org/beginners-tutorials/tutorial-7-model-loading/)
   * Modified code from Brian Foster - student in 2014. Updates to fix destructor
   * and use texture mapping along with Phong shading shaders.
   * @param filename : Model file path/name
   */
  ModelNode(const int position_loc, const int normal_loc, const int texture_loc, 
            const std::string& filename) {
      ImportModelFromFile(filename);
      GenVAOsAndUniformBuffer(scene, position_loc, normal_loc, texture_loc);
   }

  ~ModelNode() {
    for (uint32_t n = 0; n < meshes.size(); ++n) {
      // Delete vertex buffer objects, VAO, and texture objects
      if (meshes[n].position_vbo > 0)
        glDeleteBuffers(1, &meshes[n].position_vbo);
      if (meshes[n].normal_vbo > 0)
        glDeleteBuffers(1, &meshes[n].normal_vbo);
      if (meshes[n].texture_vbo > 0)
        glDeleteBuffers(1, &meshes[n].texture_vbo);
      glDeleteVertexArrays(1, &meshes[n].vao);
      if (meshes[n].has_texture) {
        glDeleteTextures(1, &meshes[n].texture_id);
      }
    }
  }

  /**
   * Draw this model node.
   * @param  scene_state   Current scene state
   */
  void Draw(SceneState& scene_state) {
    // Draw all meshes assigned to this node
    for (uint32_t n = 0; n < meshes.size(); ++n) {
      if (meshes[n].has_texture) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, meshes[n].texture_id);
        glUniform1i(scene_state.usetexture_loc, 1);   // Tell shader we are using textures
        glUniform1i(scene_state.textureunit_loc, 0);  // Texture unit 0
      }
      else {
        glUniform1i(scene_state.usetexture_loc, 0);
      }
      glBindVertexArray(meshes[n].vao);
      glDrawElements(GL_TRIANGLES, meshes[n].numFaces * 3, GL_UNSIGNED_INT, 0);
    }
  }

protected:
  std::vector<ModelMesh> meshes;
  const aiScene* scene;
  Assimp::Importer importer;
  std::string model_filename;
  std::string model_directory;

  /**
  * Import the model into a Assimp scene
  */
  void ImportModelFromFile(const std::string& filename) {
    // Check if file exists (look in parent directory under model subdir)
    std::string full_path = "../model/" + filename;
    std::ifstream fin(full_path.c_str());
    if (!fin.fail()) {
      fin.close();
    } else {
      // Try up 2 levels
      full_path = "../../model/" + filename;
      std::ifstream fin(full_path.c_str());
      if (!fin.fail()) {
        fin.close();
      } else {
        std::cout << "Couldn't open file: " << full_path << std::endl;
        std::cout << importer.GetErrorString() << std::endl;
        system("pause");
        exit(1);
      }
    }
    model_filename = full_path;
    model_directory = GetFilePath(full_path);

    scene = importer.ReadFile(model_filename, aiProcessPreset_TargetRealtime_Quality);

    // If the import failed, report it
    if (!scene) {
      std::cout << importer.GetErrorString() << std::endl;
      system("pause");
      exit(1);
    }
//    std::cout << "Import of scene " << filename << " succeeded." << std::endl;

    // Now we can access the file's contents. Everything will be cleaned up
    // by the importer destructor
  }

  /**
   * Load the model from a Assimp scene into VBOs.
   */
  void GenVAOsAndUniformBuffer(const aiScene *sc, const int vertexLoc, 
                          const int normal_loc, const int texture_loc) {
    ModelMesh model_mesh;
    GLuint buffer;

    // For each mesh
    for (uint32_t n = 0; n < sc->mNumMeshes; ++n) {
      const aiMesh* mesh = sc->mMeshes[n];

      // create array with faces. Convert from Assimp format to array
      uint32_t* faceArray = (uint32_t *)malloc(sizeof(uint32_t) * mesh->mNumFaces * 3);
      uint32_t faceIndex = 0;
      for (uint32_t t = 0; t < mesh->mNumFaces; ++t) {
        const aiFace* face = &mesh->mFaces[t];
        memcpy(&faceArray[faceIndex], face->mIndices, 3 * sizeof(uint32_t));
        faceIndex += 3;
      }
      model_mesh.numFaces = sc->mMeshes[n]->mNumFaces;

      // Generate Vertex Array Object for mesh
      glGenVertexArrays(1, &(model_mesh.vao));
      glBindVertexArray(model_mesh.vao);

      // Buffer for faces
      glGenBuffers(1, &buffer);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, buffer);
      glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * mesh->mNumFaces * 3, faceArray, GL_STATIC_DRAW);

      // buffer for vertex positions
      if (mesh->HasPositions()) {
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->mNumVertices, mesh->mVertices, GL_STATIC_DRAW);
        glEnableVertexAttribArray(vertexLoc);
        glVertexAttribPointer(vertexLoc, 3, GL_FLOAT, 0, 0, 0);
      }

      // buffer for vertex normals
      if (mesh->HasNormals()) {
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 3 * mesh->mNumVertices, mesh->mNormals, GL_STATIC_DRAW);
        glEnableVertexAttribArray(normal_loc);
        glVertexAttribPointer(normal_loc, 3, GL_FLOAT, 0, 0, 0);
      }

      // buffer for vertex texture coordinates
      if (mesh->HasTextureCoords(0)) {
        float* texCoords = (float *)malloc(sizeof(float) * 2 * mesh->mNumVertices);
        for (uint32_t k = 0; k < mesh->mNumVertices; ++k) {
            texCoords[k * 2] = mesh->mTextureCoords[0][k].x;
            texCoords[k * 2 + 1] = mesh->mTextureCoords[0][k].y;
        }

        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 2 * mesh->mNumVertices, texCoords, GL_STATIC_DRAW);
        glEnableVertexAttribArray(texture_loc);
        glVertexAttribPointer(texture_loc, 2, GL_FLOAT, 0, 0, 0);
      }

      // unbind buffers
      glBindVertexArray(0);
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

      // Create material uniform buffer
      aiMaterial *mtl = sc->mMaterials[mesh->mMaterialIndex];
      aiString texPath;	// contains filename of texture
      if (AI_SUCCESS == mtl->GetTexture(aiTextureType_DIFFUSE, 0, &texPath)) {
        std::string texFilename(texPath.data);
        if (!fileExists(texFilename)) {
            texFilename = model_directory;
            texFilename += "/";
            texFilename += texPath.data;
        }

        // Load the image file
        // Bind a DevIL image
        ILuint id;
        ilGenImages(1, &id);
        ilBindImage(id);
        ILuint err = ilGetError();
        if (err) {
          printf("Error binding image. %s %d\n", texFilename.c_str(), err);
          system("pause");
          exit(1);
        }

        // Load image using lower left origin. Convert to RGBA
        ilOriginFunc(IL_ORIGIN_LOWER_LEFT);
        ilEnable(IL_ORIGIN_SET);
        ilLoadImage(texFilename.c_str());
        ilConvertImage(IL_RGBA, IL_UNSIGNED_BYTE);
        err = ilGetError();
        if (err) {
          printf("Error loading texture. %s %d\n", texFilename.c_str(), err);
          system("pause");
          exit(1);
        }

        // Get image dimensions and data
        int w = ilGetInteger(IL_IMAGE_WIDTH);
        int h = ilGetInteger(IL_IMAGE_HEIGHT);
        unsigned char* data = ilGetData();
        if (ilGetError() != IL_NO_ERROR) {
            printf("Error getting image data\n");
            system("pause");
            exit(1);
        }

        // Open GL stuff to generate texture ID, bind texture, load image data, generate mipmaps, set wrapping and filter modes...
        glGenTextures(1, &model_mesh.texture_id);
        glBindTexture(GL_TEXTURE_2D, model_mesh.texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        // Set wrap, repeat mode
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        model_mesh.has_texture = true;
      }
      else {
        model_mesh.has_texture = false;
      }
      meshes.push_back(model_mesh);
    }
  }

  std::string GetFilePath(const std::string& str) {
    size_t found = str.find_last_of("/\\");
    return str.substr(0, found);
  }

  bool fileExists(const std::string& name) {
    std::ifstream f(name.c_str());
    if (f.good()) {
      f.close();
      return true;
    } else {
      f.close();
      return false;
    }
  }
};
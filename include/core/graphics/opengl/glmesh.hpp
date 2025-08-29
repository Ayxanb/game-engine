#pragma once

#include <core/graphics/mesh.hpp>
#include <core/graphics/opengl/opengl.hpp>

namespace Engine {

struct MeshInfo::OpenGL final : public MeshInfo {
  GLuint vao = 0;
  GLuint vbo = 0;
  GLuint ibo = 0;
};

class MeshManager::OpenGL final : public MeshManager {

public:
  OpenGL() noexcept = default;
  ~OpenGL() noexcept {
    for (std::unique_ptr<MeshInfo> &mesh_data : meshes) {
      auto gl_mesh_data = static_cast<MeshInfo::OpenGL *>(mesh_data.get());
      glDeleteBuffers(1, &gl_mesh_data->vbo);
      glDeleteBuffers(1, &gl_mesh_data->ibo);
      glDeleteVertexArrays(1, &gl_mesh_data->vao);
    }
  };

  Mesh::Handle addMesh(Mesh &) override;
  void uploadPending() override;
};

}; /* namespace Engine */
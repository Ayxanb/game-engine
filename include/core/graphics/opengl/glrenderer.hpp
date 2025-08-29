#pragma once

#include <cstddef>

#include <core/graphics/renderer.hpp>
#include <core/graphics/opengl/opengl.hpp>
#include <core/graphics/opengl/glshader.hpp>

namespace Engine {


// class Renderer::OpenGL final : public Renderer {

//   /*********************************************************************************************************/

//   struct MeshInfo {
//     uint32_t vao = 0;
//     uint32_t vbo = 0;
//     uint32_t ibo = 0;
//     uint32_t index_count = 0;
//   };

//   class MeshManager {
//     std::vector<MeshInfo> meshes;

//   public:
//     MeshManager() noexcept = default;
//     ~MeshManager() noexcept {
//       for (MeshInfo &mesh_data : meshes) {
//         glDeleteBuffers(1, &mesh_data.vbo);
//         glDeleteBuffers(1, &mesh_data.ibo);
//         glDeleteVertexArrays(1, &mesh_data.vao);
//       }
//     };

//     Mesh::Handle upload(Mesh &);
//     MeshInfo &get(Mesh::Handle handle) { return meshes.at(handle); }
//   };

//   class UniformBufferManager {
//     struct BufferData {
//       GLuint buffer = 0;
//       size_t size_in_bytes = 0;
//     };

//     std::unordered_map<UniformBufferType, BufferData> uniform_buffers;

//   public:
//     UniformBufferManager() noexcept = default;
//     ~UniformBufferManager() noexcept {
//       for (auto &[_, buffer_data] : uniform_buffers) {
//         if (!buffer_data.buffer) continue;
//         glDeleteBuffers(1, &buffer_data.buffer);
//       }
//       uniform_buffers.clear();
//     }

//     bool create(UniformBufferType, size_t);
//     bool update(UniformBufferType, const void *, size_t, size_t) const;
//     void bindUBO(UniformBufferType, GLuint) const;
//   };

//   /*********************************************************************************************************/

//   std::unique_ptr<GraphicsAPI::OpenGL> opengl;
//   std::unique_ptr<MeshManager> mesh_manager;
//   UniformBufferManager ub_manager;
//   std::vector<Shader::OpenGL> pipelines;

// public:
//   OpenGL(Engine::Window *window) noexcept
//     : Renderer(window), 
//       opengl(std::make_unique<GraphicsAPI::OpenGL>()),
//       mesh_manager(std::make_unique<MeshManager>()),
//       ub_manager(),
//       pipelines() {}
//   ~OpenGL() noexcept override = default;

//   bool init(Config::Renderer &) override;

//   bool bindPipeline(uint32_t handle) override {
//     if(handle >= pipelines.size()) return false;
//     pipelines.at(handle).bind();
//     return true; 
//   }
//   Mesh::Handle addMesh(Mesh &mesh) override {
//     return mesh_manager->upload(mesh);
//   };

// };

} /* namespace Engine */

#pragma once

#include <memory>
#include <cstdint>

#include <core/config.hpp>
#include <core/graphics/mesh.hpp>
#include <core/graphics/shader.hpp>
#include <core/graphics/buffer.hpp>
#include <core/graphics/graphics_api.hpp>

namespace Engine {

class Window;

/**
 * @class Renderer
 * @brief Responsible for managing meshes, pipelines, uniform buffers.
 */
class Renderer {
  std::unique_ptr<GraphicsAPI> graphics_api;
  std::unique_ptr<MeshManager> mesh_manager;
  std::vector<std::unique_ptr<Pipeline>> pipelines;
  std::unique_ptr<UniformBufferManager> ub_manager;

protected:
  Engine::Window *window = nullptr; /**< Associated window pointer */

public:
  explicit Renderer(Engine::Window *_window) noexcept : window(_window) {}
  ~Renderer() = default;

  /** Initialize renderer with configuration */
  bool init(Config::Renderer &);

  /** Called at the start of each frame */
  bool beginFrame();

  /** Called at the end of each frame */
  bool endFrame();

  bool render(Mesh::Handle);

  /** Bind a shader by ID */
  bool bindPipeline(uint32_t);

  /** Add a mesh to the renderer and return its handle */
  __forceinline Mesh::Handle addMesh(Mesh &mesh) const {
    return mesh_manager->addMesh(mesh);
  }

  /**
   * @brief Update a uniform buffer object
   * @tparam T Type of the UBO
   * @param type Uniform buffer type
   * @param ubo Reference to UBO data
   * @param offset_in_bytes Byte offset into buffer (default 0)
   * @return true if update succeeded
   */
  template <typename T>
  __forceinline bool updateUniformBuffer(UniformBufferType type, const T &ubo, size_t offset_in_bytes = 0) const {
    return graphics_api->updateUBO(type, &ubo, sizeof(T), offset_in_bytes);
  }

private:

  friend class Instance; /**< Engine instance can access private members */
};

} /* namespace Engine */
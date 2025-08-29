#pragma once
#include <glm/glm.hpp>
#include <span>
#include <vector>
#include <optional>

#include <util/file_utils.hpp>

namespace Engine {

class Mesh {
public:

  struct Vertex {
    glm::vec3 position;
    glm::vec3 color;
    glm::vec2 uv;
    glm::vec3 normal;
  };

  using Handle = uint32_t;
  using Index = uint32_t;
  static constexpr Handle InvalidHandle = std::numeric_limits<Handle>::max();

  Mesh() noexcept = default;
  ~Mesh() noexcept = default;

  inline std::span<Vertex> getVerticesView() { return std::span<Mesh::Vertex>{vertices}; }
  inline std::span<Index> getIndicesView() { return std::span<Mesh::Index>{indices}; }

  static std::optional<Mesh> fromOBJ(File::Path);

private:
  std::vector<Vertex> vertices;
  std::vector<Index> indices;
};

struct MeshInfo {
  std::vector<Mesh::Vertex> cpu_vertices;
  std::vector<Mesh::Index>  cpu_indices;
  bool gpu_uploaded        = false;
  bool alive              = true;

  struct OpenGL;
  struct Vulkan;
};

class MeshManager {
protected:
  std::vector<std::unique_ptr<MeshInfo>> meshes;

public:
  MeshManager() noexcept = default;
  virtual ~MeshManager() noexcept = default;

  inline MeshInfo &get(Mesh::Handle handle) {
    assert(handle < meshes.size() && "handle >= meshes.size()");
    return *meshes.at(handle);
  }

  virtual Mesh::Handle addMesh(Mesh &) = 0;
  virtual void uploadPending() = 0;

  class OpenGL;
  class Vulkan;
};

} /* namespace Engine */

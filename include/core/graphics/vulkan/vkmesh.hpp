#pragma once

#include <cassert>
#include <core/graphics/mesh.hpp>
#include <core/graphics/vulkan/vulkan.hpp>

namespace Engine {

struct MeshInfo::Vulkan final : public MeshInfo {
  VkBuffer vertex_buffer   = VK_NULL_HANDLE;
  VkBuffer index_buffer    = VK_NULL_HANDLE;

  VkDeviceMemory vertex_memory   = VK_NULL_HANDLE;
  VkDeviceMemory index_memory   = VK_NULL_HANDLE;
};

class MeshManager::Vulkan final : public MeshManager {
  GraphicsAPI::Vulkan *vulkan;

public:
  explicit Vulkan(GraphicsAPI::Vulkan *_vulkan) noexcept : vulkan(_vulkan) {}
  ~Vulkan() noexcept;

  Mesh::Handle addMesh(Mesh &) override;
  void uploadPending() override;

};


}; /* namespace Engine */
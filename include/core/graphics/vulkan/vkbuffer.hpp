#pragma once

#include <unordered_map>

#include <core/graphics/buffer.hpp>
#include <core/graphics/vulkan/vulkan.hpp>

namespace Engine {
struct BufferFrame {
  size_t size = 0;
  void *mapped = nullptr;
  VkBuffer buffer = VK_NULL_HANDLE;
  VkDeviceMemory memory = VK_NULL_HANDLE;
};

struct UniformBuffer {
  uint32_t size = 0;
  std::vector<BufferFrame> frames;
  UniformBufferType type = UniformBufferType::None;
};

class UniformBufferManager::Vulkan final : public UniformBufferManager {
  GraphicsAPI::Vulkan *vulkan;
  std::unordered_map<UniformBufferType, UniformBuffer> uniform_buffers;

public:
  Vulkan(GraphicsAPI::Vulkan *_vulkan) noexcept : vulkan(_vulkan) {}
  ~Vulkan() noexcept;

  bool create(UniformBufferType, size_t) override;
  bool update(UniformBufferType, uint32_t, const void *, size_t, size_t) const override;
  VkDescriptorBufferInfo getDescriptorBufferInfo(UniformBufferType, uint32_t) const;

private:
  bool createBufferForFrame(BufferFrame &, uint32_t) const;
};


}; /* namespace Engine */
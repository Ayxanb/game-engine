#pragma once

#include <string_view>

namespace Engine {

enum class UniformBufferType : uint8_t {
  Camera,
  None,
  Count,
};

static inline std::string_view uniformBufferTypeToString(UniformBufferType type) {
  switch (type) {
  case UniformBufferType::None: return std::string_view{ "None" };
  case UniformBufferType::Camera: return std::string_view{ "Camera" };

  case UniformBufferType::Count:
  }
  return std::string_view{ "Unknown" };
}

struct UniformBufferManager {  
  class OpenGL;
  class Vulkan;

  UniformBufferManager() noexcept = default;
  virtual ~UniformBufferManager() noexcept = default;

  virtual bool create(UniformBufferType, size_t) = 0;
  virtual bool update(UniformBufferType, uint32_t, const void *, size_t, size_t) const = 0;

};
  
}; /* namespace Engine */
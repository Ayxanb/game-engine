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

// struct UniformBufferManager {  
//   class OpenGL;
//   class Vulkan;
// };
  
}; /* namespace Engine */
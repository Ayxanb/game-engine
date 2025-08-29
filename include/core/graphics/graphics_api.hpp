#pragma once

#include <cstdint>
#include <glm/vec3.hpp>

#include <core/graphics/mesh.hpp>
#include <core/graphics/buffer.hpp>

namespace Engine {
class Window;

struct DrawInfo {
  DrawInfo() noexcept = default;
  virtual ~DrawInfo() noexcept = default;

  uint32_t index_count = 0;

  struct OpenGL;
  struct Vulkan;
};

class GraphicsAPI {
  uint32_t current_image_index = 0;
  uint32_t current_frame_index = 0;

public:
  struct OpenGL;
  struct Vulkan;
  
  enum class Backend {
    OpenGL,
    Vulkan,
  };

  GraphicsAPI() noexcept = default;
  virtual ~GraphicsAPI() = default;

  inline uint32_t getCurrentImageIndex() const { return current_image_index; }
  inline uint32_t getCurrentFrameIndex() const { return current_frame_index; }

  virtual bool init(Window *) = 0;
  virtual void enableVsync() = 0;
  virtual bool beginFrame() = 0;
  virtual bool endFrame(Window *) = 0;
  virtual bool drawIndexed(DrawInfo &) = 0;
  virtual bool updateUBO(UniformBufferType, const void *, size_t, size_t) = 0;
  virtual void setClearColor(glm::vec3, float = 1.0f) = 0;

  static void applyWindowHints(GraphicsAPI::Backend); 
  
};

} /* namespace Engine */

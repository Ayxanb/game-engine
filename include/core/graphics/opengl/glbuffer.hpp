#pragma once

#include <unordered_map>

#include <core/graphics/buffer.hpp>
#include <core/graphics/opengl/opengl.hpp>

namespace Engine {
  
struct BufferData {
  GLuint buffer = 0;
  size_t size_in_bytes = 0;
};
  
class UniformBufferManager::OpenGL final : public UniformBufferManager {
  std::unordered_map<UniformBufferType, BufferData> uniform_buffers;

public:
  OpenGL() noexcept = default;
  ~OpenGL() noexcept {
    for (auto &[_, buffer_data] : uniform_buffers) {
      if (!buffer_data.buffer)
        continue;
      glDeleteBuffers(1, &buffer_data.buffer);
    }

    uniform_buffers.clear();
  }

  bool create(UniformBufferType, size_t) override;
  bool update(UniformBufferType, uint32_t, const void *, size_t, size_t) const override;

  void bind(UniformBufferType, uint32_t) const;

};

}; /* namespace Engine */
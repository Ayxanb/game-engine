#pragma once

#include <glad/glad.h>

#include <core/platform/window.hpp>
#include <core/graphics/graphics_api.hpp>

namespace Engine {

struct DrawInfo::OpenGL : DrawInfo {
  GLuint vao;
};

struct GraphicsAPI::OpenGL final : public GraphicsAPI {
  OpenGL() noexcept = default;
  ~OpenGL() noexcept = default;

  bool init(Window *) override;
  void enableVsync() override;
  bool beginFrame() override;
  bool endFrame(Window *) override;
  bool drawIndexed(DrawInfo &) override;
  bool updateUBO(UniformBufferType, const void *, size_t, size_t) override;
  void setClearColor(glm::vec3 rgb, float a) override {
    glClearColor(rgb.r, rgb.g, rgb.b, a);
  };

};

} /* namespace Engine */

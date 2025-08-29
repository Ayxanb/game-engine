#pragma once

#include <string>
#include <unordered_map>

#include <util/file_utils.hpp>
#include <core/graphics/shader.hpp>
#include <core/graphics/opengl/opengl.hpp>

#include <glm/mat4x4.hpp>
#include <glm/gtc/type_ptr.hpp>

namespace Engine {

class Pipeline::OpenGL final : public Pipeline {
  ShaderStages stages;
  GLuint program = 0;
  std::unordered_map<std::string, GLint> uniform_locations;

public:
  OpenGL() noexcept = default;
  ~OpenGL() noexcept override;

  bool create(ShaderStages) override;
  void bind(uint32_t) override;

private:
  GLuint compileShader(GLenum, File::Path);
  GLint getUniformLocation(std::string);
};

} /* namespace Engine */

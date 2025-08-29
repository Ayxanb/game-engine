#pragma once

#include <util/file_utils.hpp>

namespace Engine {

class Shader {
public:

  class OpenGL;
  class Vulkan;

  struct StagePaths {
    File::Path vertex {};
    File::Path fragment {};
    File::Path geometry {};
    File::Path compute {};
  };

  Shader() noexcept = default;
  virtual ~Shader() noexcept = default;

  virtual bool create(StagePaths) = 0;

};

class Pipeline {
public:

  struct ShaderStages {
    File::Path vertex {};
    File::Path fragment {};
    File::Path geometry {};
    File::Path compute {};
  };

  Pipeline() noexcept = default;
  virtual ~Pipeline() noexcept = default;

  virtual bool create(ShaderStages) = 0;
  virtual void bind(uint32_t) = 0;

  class OpenGL;
  class Vulkan;
};


};
#include <core/graphics/graphics_api.hpp>
#include <core/graphics/opengl/opengl.hpp>
#include <core/graphics/vulkan/vulkan.hpp>

#include <core/logging.hpp>

namespace Engine {

void GraphicsAPI::applyWindowHints(GraphicsAPI::Backend backend) {
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);

  switch (backend) {
  case GraphicsAPI::Backend::OpenGL:
    glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_API);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    break;

  case GraphicsAPI::Backend::Vulkan:
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    break;
  }
}

}; /* namespace Engine */
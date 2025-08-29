#include <core/graphics/vulkan/vksurface.hpp>
#include <core/graphics/vulkan/vkinstance.hpp>
#include <core/platform/window.hpp>

#include <core/logging.hpp>

namespace Engine {
using Vulkan = GraphicsAPI::Vulkan;

Vulkan::SurfaceManager::~SurfaceManager() noexcept {
  if (surface != VK_NULL_HANDLE) {
    vkDestroySurfaceKHR(instance, surface, nullptr);
    surface = VK_NULL_HANDLE;
    LOG_INFO("[GraphicsAPI::Vulkan::SurfaceManager]: surface destroyed");
  }
}

bool Vulkan::SurfaceManager::init(Vulkan::InstanceManager *instance_manager, Engine::Window *window) {
  instance = instance_manager->getInstance();

  if (glfwCreateWindowSurface(instance, window->getNativeHandle(), VK_NULL_HANDLE, &surface) != VK_SUCCESS) {
    LOG_ERROR("[GraphicsAPI::Vulkan::SurfaceManager]: failed to create window surface");
    return false;
  }

  LOG_INFO("[GraphicsAPI::Vulkan::SurfaceManager]: surface created successfully");
  return true;
}

} /* namespace Engine */
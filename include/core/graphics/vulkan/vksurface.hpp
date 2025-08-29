#pragma once

#include <core/graphics/vulkan/vulkan.hpp>

namespace Engine {
class Window;

struct GraphicsAPI::Vulkan::SurfaceManager {
private:
  VkInstance instance = VK_NULL_HANDLE; /* acquired from InstanceManager */
  VkSurfaceKHR surface = VK_NULL_HANDLE;

public:
  SurfaceManager() noexcept = default;
  ~SurfaceManager() noexcept;

  /*
   * Initializes the Vulkan surface for a given window.
   * Returns true on success, false on failure.
   */
  bool init(Vulkan::InstanceManager* instance_manager, Engine::Window* window);

  VkSurfaceKHR& getSurface() { return surface; }
};

} /* namespace Engine */

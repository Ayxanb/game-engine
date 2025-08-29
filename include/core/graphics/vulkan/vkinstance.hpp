#pragma once

#include <vector>
#include <string_view>

#include <core/graphics/vulkan/vulkan.hpp>

namespace Engine {

class Window;

struct GraphicsAPI::Vulkan::InstanceManager {
  InstanceManager() noexcept = default;
  ~InstanceManager() noexcept;

  bool init(Window *, std::string_view, bool);
  VkInstance &getInstance() { return instance; }

private:
  VkInstance instance = VK_NULL_HANDLE;
  VkDebugUtilsMessengerEXT debug_messenger = VK_NULL_HANDLE;
  bool validation_enabled = false;
  Window *window = nullptr;

  bool createInstance(std::string_view);
  bool setupDebugMessenger();

  std::vector<const char*> getRequiredExtensions() const;
  bool checkValidationLayerSupport() const;

  /* Static helper functions for debug messenger */
  static VkResult createDebugUtilsMessengerExt(
    VkInstance,
    const VkDebugUtilsMessengerCreateInfoEXT *,
    const VkAllocationCallbacks *,
    VkDebugUtilsMessengerEXT *
  );

  static void destroyDebugUtilsMessenger(
    VkInstance,
    VkDebugUtilsMessengerEXT,
    const VkAllocationCallbacks *
  );

  static void populateDebugMessengerCreateInfo(
    VkDebugUtilsMessengerCreateInfoEXT &
  );
};

} /* namespace Engine */

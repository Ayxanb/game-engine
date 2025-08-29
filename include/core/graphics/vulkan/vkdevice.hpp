#pragma once

#include <optional>
#include <cassert>

#include <core/graphics/vulkan/vulkan.hpp>

namespace Engine {

struct GraphicsAPI::Vulkan::DeviceManager {
  DeviceManager() noexcept = default;
  ~DeviceManager() noexcept;

  static std::string_view deviceTypeToString(VkPhysicalDeviceType);

  /*
   * Initializes the device manager with instance and surface managers.
   * Returns false on failure.
   */
  bool init(InstanceManager *, SurfaceManager *);

  VkDevice getDevice() { return device; }
  VkPhysicalDevice getPhysicalDevice() { return physical_device; }

  inline VkQueue getGraphicsQueue() { 
    assert(graphics_queue != VK_NULL_HANDLE && "Graphics queue not initialized");
    return graphics_queue; 
  }
  inline VkQueue getPresentQueue()  {
    assert(present_queue != VK_NULL_HANDLE && "Present queue not initialized");
    return present_queue;
  }

  inline uint32_t getGraphicsQueueFamily() const { return graphics_queue_family.value(); }
  inline uint32_t getPresentQueueFamily()  const { return present_queue_family.value(); }

  inline void waitIdle() const { vkDeviceWaitIdle(device); }
  inline VkResult waitForFences(const std::vector<VkFence> &fences) const {
    return vkWaitForFences(
      device,
      fences.size(),
      fences.data(),
      VK_TRUE,
      std::numeric_limits<uint64_t>::max()
    );
  }
  inline VkResult resetFences(const std::vector<VkFence> &fences) {
    return vkResetFences(device, fences.size(), fences.data());
  }

private:
  VkDevice device = VK_NULL_HANDLE;
  VkPhysicalDevice physical_device = VK_NULL_HANDLE;

  VkQueue graphics_queue = VK_NULL_HANDLE;
  VkQueue present_queue  = VK_NULL_HANDLE;

  VkInstance instance     = VK_NULL_HANDLE;
  VkSurfaceKHR surface    = VK_NULL_HANDLE;

  std::optional<uint32_t> graphics_queue_family;
  std::optional<uint32_t> present_queue_family;

  /*
   * Selects the first suitable physical device.
   * Returns true on success, false otherwise.
   */
  bool pickPhysicalDevice();

  /*
   * Creates a logical device and retrieves queue handles.
   * Returns true on success, false otherwise.
   */
  bool createLogicalDevice();

  /*
   * Finds queue families that support graphics and present operations.
   * Returns true on success, false otherwise.
   */
  bool findQueueFamilies(VkPhysicalDevice) ;

  /*
   * Checks if the device supports the required queues.
   */
  inline bool isDeviceSuitable(VkPhysicalDevice device) {
    return findQueueFamilies(device);
  }

  /*
   * Checks if all required device extensions are available.
   */
  bool checkDeviceExtensionSupport(VkPhysicalDevice, std::span<const char*>) const;

};

} /* namespace Engine */

#pragma once

#include <cstdint>
#include <span>
#include <vector>
#include <array>
#include <memory>
#include <cassert>

#include <vulkan/vulkan.h>
#include <core/graphics/graphics_api.hpp>

namespace Engine {
class Window;

struct DrawInfo::Vulkan : DrawInfo {
  VkBuffer vertex_buffer   = VK_NULL_HANDLE;
  VkBuffer index_buffer    = VK_NULL_HANDLE;
  VkCommandBuffer command_buffer = VK_NULL_HANDLE;
};

/*
 * Vulkan backend for GraphicsAPI.
 * Manages Vulkan initialization, frame lifecycle, and resource management.
 */
struct GraphicsAPI::Vulkan final : public GraphicsAPI {
  static constexpr inline uint32_t MAX_FRAMES_IN_FLIGHT = 3;

  struct InstanceManager;
  struct DeviceManager;
  struct SurfaceManager;
  struct DescriptorManager;

  Vulkan() noexcept;
  ~Vulkan() noexcept;

  /* === Initialization === */
  bool init(Engine::Window *) override;
  void enableVsync() override;
  bool beginFrame() override;
  bool endFrame(Window *) override;
  bool drawIndexed(DrawInfo &) override;
  bool updateUBO(UniformBufferType, const void *, size_t, size_t) override;
  void setClearColor(glm::vec3 rgb, float a) override {
    clear_color.color.float32[0] = rgb.r;
    clear_color.color.float32[1] = rgb.g;
    clear_color.color.float32[2] = rgb.g;
    clear_color.color.float32[3] = a;
  };

  /* === Managers Access === */
  inline InstanceManager &getInstanceManager() {
    assert(instance_manager != nullptr && "InstanceManager is not initialized");
    return *instance_manager;
  }
  inline DeviceManager &getDeviceManager() {
    assert(device_manager != nullptr && "DeviceManager is not initialized");
    return *device_manager;
  }
  inline SurfaceManager &getSurfaceManager() {
    assert(surface_manager != nullptr && "SurfaceManager is not initialized");
    return *surface_manager;
  }
  inline DescriptorManager &getDescriptorManager() {
    assert(descriptor_manager != nullptr && "DescriptorManager is not initialized");
    return *descriptor_manager;
  }

  /* === Vulkan Objects Access === */
  inline VkSwapchainKHR getSwapchain() const { return swapchain; }
  inline VkCommandPool getCommandPool() const { return command_pool; }
  inline VkRenderPass getRenderPass() const { return render_pass; }
  inline VkExtent2D getSwapchainExtent() const { return swapchain_extent; }

  inline VkCommandBuffer getCommandBuffer(uint32_t index) const { return command_buffers[index]; }
  inline VkSemaphore getImageAvailableSemaphore(uint32_t index) const { return image_available_semaphores[index]; }
  inline VkSemaphore getRenderFinishedSemaphore(uint32_t index) const { return render_finished_semaphores[index]; }
  inline VkFramebuffer getFramebufferForImage(uint32_t index) const { return frame_buffers[index]; }
  inline VkFence getFence(uint32_t index) const { return in_flight_fences[index]; }

  /* === Command Buffer Utilities === */
  VkResult submitQueue(VkQueue, const std::vector<VkSubmitInfo> &, uint32_t);

  bool createBuffer(
    void *,
    uint32_t,
    VkBufferUsageFlags,
    VkBuffer &,
    VkDeviceMemory &
  );

  bool createRawBuffer(
    size_t,
    VkBufferUsageFlags,
    VkMemoryPropertyFlags,
    VkBuffer &,
    VkDeviceMemory &
  );

  VkCommandBuffer beginSingleTimeCommands() const;
  bool endSingleTimeCommands(VkCommandBuffer) const;
  uint32_t findMemoryType(uint32_t, VkMemoryPropertyFlags) const;

private:
  /* === Vulkan Initialization Helpers === */
  bool createSwapchain();
  bool createImageviews();
  bool createRenderpass();
  bool createFramebuffers();
  bool createCommandPool();
  bool createCommandBuffers();
  bool createSyncObjects();

  VkSurfaceFormatKHR &chooseSwapSurfaceFormat(std::span<VkSurfaceFormatKHR>);
  VkPresentModeKHR choosePresentMode(std::span<VkPresentModeKHR>);
  VkExtent2D chooseSwapExtent(VkSurfaceCapabilitiesKHR&);

  /* === Vulkan Resource Managers === */
  std::unique_ptr<InstanceManager> instance_manager;
  std::unique_ptr<SurfaceManager> surface_manager;
  std::unique_ptr<DeviceManager> device_manager;
  std::unique_ptr<DescriptorManager> descriptor_manager;

  /* === Swapchain & Images === */
  VkSwapchainKHR swapchain = VK_NULL_HANDLE;
  std::vector<VkImage> swapchain_images;
  std::vector<VkImageView> swapchain_image_views;
  VkFormat swapchain_image_format;
  VkExtent2D swapchain_extent;

  VkRenderPass render_pass = VK_NULL_HANDLE;
  std::vector<VkFramebuffer> frame_buffers;

  VkCommandPool command_pool = VK_NULL_HANDLE;
  std::vector<VkCommandBuffer> command_buffers;

  std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> image_available_semaphores;
  std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> render_finished_semaphores;
  std::array<VkFence, MAX_FRAMES_IN_FLIGHT> in_flight_fences;
  std::vector<VkFence> in_flight_images;
  VkClearValue clear_color {};

};

} /* namespace Engine */

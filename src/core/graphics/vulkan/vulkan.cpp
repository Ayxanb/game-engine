#include <core/logging.hpp>
#include <core/platform/window.hpp>
#include <core/graphics/vulkan/vkmesh.hpp>
#include <core/graphics/vulkan/vulkan.hpp>
#include <core/graphics/vulkan/vkshader.hpp>
#include <core/graphics/vulkan/vkdevice.hpp>
#include <core/graphics/vulkan/vksurface.hpp>
#include <core/graphics/vulkan/vkinstance.hpp>
#include <core/graphics/vulkan/descriptor_manager.hpp>

#include <cstdint>
#include <string_view>

namespace Engine {
using Vulkan = GraphicsAPI::Vulkan;

GraphicsAPI::Vulkan::Vulkan() noexcept :
  instance_manager(std::make_unique<Vulkan::InstanceManager>()),
  surface_manager(std::make_unique<Vulkan::SurfaceManager>()),
  device_manager(std::make_unique<Vulkan::DeviceManager>()),
  descriptor_manager(std::make_unique<Vulkan::DescriptorManager>()) {
}

GraphicsAPI::Vulkan::~Vulkan() noexcept {
  if (device_manager->getDevice() != VK_NULL_HANDLE)
    vkDeviceWaitIdle(device_manager->getDevice());

  if (device_manager->getDevice() != VK_NULL_HANDLE) {
    for (auto &semaphore : image_available_semaphores)
      vkDestroySemaphore(device_manager->getDevice(), semaphore, VK_NULL_HANDLE);

    for (auto &semaphore : render_finished_semaphores)
      vkDestroySemaphore(device_manager->getDevice(), semaphore, VK_NULL_HANDLE);

    for (auto &fence : in_flight_fences)
      vkDestroyFence(device_manager->getDevice(), fence, VK_NULL_HANDLE);

    for (auto &framebuffer : frame_buffers)
      vkDestroyFramebuffer(device_manager->getDevice(), framebuffer, VK_NULL_HANDLE);
    frame_buffers.clear();

    for (auto& view : swapchain_image_views)
      vkDestroyImageView(device_manager->getDevice(), view, VK_NULL_HANDLE);
    swapchain_image_views.clear();

    if (render_pass != VK_NULL_HANDLE)
      vkDestroyRenderPass(device_manager->getDevice(), render_pass, VK_NULL_HANDLE);

    if (command_pool != VK_NULL_HANDLE)
      vkDestroyCommandPool(device_manager->getDevice(), command_pool, VK_NULL_HANDLE);

    if (swapchain != VK_NULL_HANDLE)
      vkDestroySwapchainKHR(device_manager->getDevice(), swapchain, VK_NULL_HANDLE);
  }
}

/* ========================================= */
/* ========== Swapchain & Rendering ========== */
/* ========================================= */

bool Vulkan::createSwapchain() {
  // Query surface capabilities
  VkSurfaceKHR surface = surface_manager->getSurface();
  VkSurfaceCapabilitiesKHR capabilities;

  if (vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device_manager->getPhysicalDevice(), surface,
                                             &capabilities) != VK_SUCCESS) {
    LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to get surface capabilities");
    return false;
  }

  // Get supported surface formats
  uint32_t format_count;
  if (vkGetPhysicalDeviceSurfaceFormatsKHR(device_manager->getPhysicalDevice(), surface, 
                                         &format_count, VK_NULL_HANDLE) != VK_SUCCESS) {
    LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to get surface format count");
    return false;
  }

  std::vector<VkSurfaceFormatKHR> formats(format_count);
  vkGetPhysicalDeviceSurfaceFormatsKHR(device_manager->getPhysicalDevice(), surface, 
                                     &format_count, formats.data());

  // Get supported present modes
  uint32_t present_mode_count;
  if (vkGetPhysicalDeviceSurfacePresentModesKHR(
    device_manager->getPhysicalDevice(),
    surface,
    &present_mode_count,
    VK_NULL_HANDLE) != VK_SUCCESS) {
      LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to get surface present mode count");  
      return false;
    }

  std::vector<VkPresentModeKHR> presentModes(present_mode_count);
  vkGetPhysicalDeviceSurfacePresentModesKHR(
    device_manager->getPhysicalDevice(),
    surface,
    &present_mode_count,
    presentModes.data()
  );

  // Choose optimal swapchain settings
  VkSurfaceFormatKHR surface_format = chooseSwapSurfaceFormat(formats);
  VkPresentModeKHR present_mode = choosePresentMode(presentModes);
  VkExtent2D extent = chooseSwapExtent(capabilities);

  // Determine image count
  uint32_t image_count = capabilities.minImageCount + 1;
  if (capabilities.maxImageCount > 0 && image_count > capabilities.maxImageCount)
    image_count = capabilities.maxImageCount;

  // Configure swapchain creation
  VkSwapchainCreateInfoKHR create_info {
    .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .surface = surface,
    .minImageCount = image_count,
    .imageFormat = surface_format.format,
    .imageColorSpace = surface_format.colorSpace,
    .imageExtent = extent,
    .imageArrayLayers = 1,
    .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
    .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 0,
    .pQueueFamilyIndices = VK_NULL_HANDLE,
    .preTransform = capabilities.currentTransform,
    .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
    .presentMode = present_mode,
    .clipped = VK_TRUE,
    .oldSwapchain = VK_NULL_HANDLE
  };

  if (vkCreateSwapchainKHR(device_manager->getDevice(), &create_info, VK_NULL_HANDLE, &swapchain) != VK_SUCCESS) {
    LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to create swapchain");
    return false;
  }

  // Retrieve swapchain images
  vkGetSwapchainImagesKHR(device_manager->getDevice(), swapchain, &image_count, VK_NULL_HANDLE);
  swapchain_images.resize(image_count);
  vkGetSwapchainImagesKHR(device_manager->getDevice(), swapchain, &image_count,
                        swapchain_images.data());

  swapchain_image_format = surface_format.format;
  swapchain_extent = extent;

  LOG_INFO("[GraphicsAPI::Vulkan]: Swapchain created successfully");
  return true;
}

bool Vulkan::createImageviews() {
  swapchain_image_views.resize(swapchain_images.size());

  for (size_t i = 0; i < swapchain_images.size(); i++) {
    VkImageViewCreateInfo view_info{
      .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
      .pNext = VK_NULL_HANDLE,
      .flags = 0,
      .image = swapchain_images[i],
      .viewType = VK_IMAGE_VIEW_TYPE_2D,
      .format = swapchain_image_format,
      .components = {
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
        VK_COMPONENT_SWIZZLE_IDENTITY,
      },
      .subresourceRange = {
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .baseMipLevel = 0,
        .levelCount = 1,
        .baseArrayLayer = 0,
        .layerCount = 1,
      },
    };

    if (vkCreateImageView(
          device_manager->getDevice(),
          &view_info,
          VK_NULL_HANDLE,
    &swapchain_image_views.at(i)) != VK_SUCCESS) {
      LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to create image view");
      return false;
    }
  }

  LOG_INFO("[GraphicsAPI::Vulkan]: Image views created successfully");
  return true;
}

bool Vulkan::createRenderpass() {
  // Color attachment configuration
  VkAttachmentDescription color_attachement{
    .flags = 0,
    .format = swapchain_image_format,
    .samples = VK_SAMPLE_COUNT_1_BIT,
    .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
    .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
    .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
    .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
    .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
    .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
  };

  // Color attachment reference
  VkAttachmentReference color_ref{
    .attachment = 0,
    .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
  };

  // Subpass configuration
  VkSubpassDescription subpass{
    .flags = 0,
    .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
    .inputAttachmentCount = 0,
    .pInputAttachments = VK_NULL_HANDLE,
    .colorAttachmentCount = 1,
    .pColorAttachments = &color_ref,
    .pResolveAttachments = VK_NULL_HANDLE,
    .pDepthStencilAttachment = VK_NULL_HANDLE,
    .preserveAttachmentCount = 0,
    .pPreserveAttachments = VK_NULL_HANDLE,
  };

  // Render pass creation
  VkRenderPassCreateInfo renderpass_info{
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .attachmentCount = 1,
    .pAttachments = &color_attachement,
    .subpassCount = 1,
    .pSubpasses = &subpass,
    .dependencyCount = 0,
    .pDependencies = VK_NULL_HANDLE,
  };

  if (vkCreateRenderPass(device_manager->getDevice(), &renderpass_info, VK_NULL_HANDLE, &render_pass) != VK_SUCCESS) {
    LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to create render pass");
    return false;
  }

  LOG_INFO("[GraphicsAPI::Vulkan]: Render pass created successfully");
  return true;
}

/* ======================================================= */
/* ========== Framebuffer, Command Pool, Buffers ========== */
/* ======================================================= */

bool Vulkan::createFramebuffers() {
  frame_buffers.resize(swapchain_image_views.size());

  for (size_t i = 0; i < frame_buffers.size(); i++) {
    VkImageView attachments[] = { swapchain_image_views[i] };

    VkFramebufferCreateInfo framebuffer_info{
      .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
      .pNext = VK_NULL_HANDLE,
      .flags = 0,
      .renderPass = render_pass,
      .attachmentCount = 1,
      .pAttachments = attachments,
      .width = swapchain_extent.width,
      .height = swapchain_extent.height,
      .layers = 1
    };

    if (vkCreateFramebuffer(
        device_manager->getDevice(),
        &framebuffer_info,
        VK_NULL_HANDLE,
        &frame_buffers[i]) != VK_SUCCESS) {
      LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to create framebuffer");
      return false;
    }
  }

  LOG_INFO("[GraphicsAPI::Vulkan]: Framebuffers created successfully");
  return true;
}

bool Vulkan::createCommandPool() {
  VkCommandPoolCreateInfo pool_info{
    .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
    .queueFamilyIndex = device_manager->getGraphicsQueueFamily(),
  };

  if (vkCreateCommandPool(device_manager->getDevice(), &pool_info, VK_NULL_HANDLE, &command_pool) != VK_SUCCESS) {
    LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to create command pool");
    return false;
  }

  LOG_INFO("[GraphicsAPI::Vulkan]: Command pool created successfully");
  return true;
}

bool Vulkan::createCommandBuffers() {
  command_buffers.resize(swapchain_images.size());
  
  VkCommandBufferAllocateInfo alloc_info{
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .commandPool = command_pool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = static_cast<uint32_t>(command_buffers.size())
  };

  if (vkAllocateCommandBuffers(device_manager->getDevice(), &alloc_info, command_buffers.data()) != VK_SUCCESS) {
    LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to allocate command buffers");
    return false;
  }

  LOG_INFO("[GraphicsAPI::Vulkan]: Command buffers allocated successfully");
  return true;
}

/* =========================================================== */
/* ========== Synchronization, Command Buffers, Presentation ========== */
/* =========================================================== */

bool Vulkan::createSyncObjects() {
  VkSemaphoreCreateInfo semaphore_info{
    .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
  };

  VkFenceCreateInfo fence_info{
    .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = VK_FENCE_CREATE_SIGNALED_BIT
  };
  
  in_flight_images.resize(swapchain_images.size(), VK_NULL_HANDLE);

  for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
    if (vkCreateSemaphore(
          device_manager->getDevice(),
          &semaphore_info,
          VK_NULL_HANDLE,
          &image_available_semaphores[i]) != VK_SUCCESS ||
        vkCreateSemaphore(
          device_manager->getDevice(),
          &semaphore_info,
          VK_NULL_HANDLE,
          &render_finished_semaphores[i]) != VK_SUCCESS ||
        vkCreateFence(device_manager->getDevice(), &fence_info, VK_NULL_HANDLE, &in_flight_fences[i]) != VK_SUCCESS) {
      LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to create sync objects");
      return false;
    }
  }

  LOG_INFO("[GraphicsAPI::Vulkan]: Sync objects created successfully");
  return true;
}

VkSurfaceFormatKHR &Vulkan::chooseSwapSurfaceFormat(std::span<VkSurfaceFormatKHR> formats) {
  for (VkSurfaceFormatKHR &format : formats)
    if (format.format == VK_FORMAT_B8G8R8A8_SRGB && format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
      return format;
  return formats.front();
}

VkPresentModeKHR Vulkan::choosePresentMode(std::span<VkPresentModeKHR> modes) {
  // vsync
  if (true) {
    for (VkPresentModeKHR &mode : modes) {
      if (mode == VK_PRESENT_MODE_MAILBOX_KHR)
        return mode;
    }

    LOG_WARN("[GraphicsAPI::Vulkan]: Vsync is enabled, but mailbox mode is not supported, falling back to FIFO mode");
    return VK_PRESENT_MODE_FIFO_KHR;
  }
  
  else {
    for (VkPresentModeKHR &mode : modes) {
      if (mode == VK_PRESENT_MODE_IMMEDIATE_KHR)
        return mode;
    }

    LOG_WARN("[GraphicsAPI::Vulkan]: Vsync is disabled, but immediate mode is not supported, falling back to FIFO mode");
    return VK_PRESENT_MODE_FIFO_KHR;
  }
}

VkExtent2D Vulkan::chooseSwapExtent(VkSurfaceCapabilitiesKHR &capabilities) {
  if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    return capabilities.currentExtent;
  return { 1280, 720 }; // TODO: Should be changed to current screen size
}

bool Vulkan::init(Window *window) {
  if (
    !instance_manager->init(window, window->getTitle(), true) ||
    !surface_manager->init(&getInstanceManager(), window) ||
    !device_manager->init(&getInstanceManager(), &getSurfaceManager()) ||
    !descriptor_manager->init(&getDeviceManager()) ||
    !createSwapchain() ||
    !createImageviews() ||
    !createRenderpass() ||
    !createFramebuffers() ||
    !createCommandPool() ||
    !createCommandBuffers() ||
    !createSyncObjects()
  ) {
      LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to initialize API");
      return false;
  }

  VkPhysicalDevice physical_device = device_manager->getPhysicalDevice();

  uint32_t loader_version;
  vkEnumerateInstanceVersion(&loader_version);

  VkPhysicalDeviceProperties properties{};
  vkGetPhysicalDeviceProperties(physical_device, &properties);

  VkPhysicalDeviceMemoryProperties mem_props{};
  vkGetPhysicalDeviceMemoryProperties(physical_device, &mem_props);

  uint32_t nextensions = 0;
  vkEnumerateDeviceExtensionProperties(physical_device, VK_NULL_HANDLE, &nextensions, VK_NULL_HANDLE);

  LOG_INFO(
    "[GraphicsAPI::Vulkan]: Initialized successfully:\n"
    "\tLoader version:               {:>35}.{}.{}\n"
    "\tAPI version:                  {:>35}.{}.{}\n"
    "\tDevice:                       {:>268}\n"
    "\tDevice Type:                  {:>41}\n"
    "\tDriver Version:               {:>32}.{}.{}\n"
    "\tVendor ID:                     {:>40}\n"
    "\tDevice ID:                     {:>40}\n"
    "\tNumber of Extensions:          {:>40}\n"
    "\tMax Image Dimension 2D:        {:>40}\n"
    "\tMax Uniform Buffers:           {:>40}\n"
    "\tMax Storage Buffers:           {:>40}\n"
    "\tMax Vertex Input Attributes:   {:>40}\n"
    "\tMax Color Attachments:         {:>40}\n"
    "\tMax Viewports:                 {:>40}\n",
    VK_VERSION_MAJOR(loader_version),
    VK_VERSION_MINOR(loader_version),
    VK_VERSION_PATCH(loader_version),
    VK_VERSION_MAJOR(properties.apiVersion),
    VK_VERSION_MINOR(properties.apiVersion),
    VK_VERSION_PATCH(properties.apiVersion),
    properties.deviceName,
    Vulkan::DeviceManager::deviceTypeToString(properties.deviceType),
    VK_VERSION_MAJOR(properties.driverVersion),
    VK_VERSION_MINOR(properties.driverVersion),
    VK_VERSION_PATCH(properties.driverVersion),
    properties.vendorID,
    properties.deviceID,
    nextensions,
    properties.limits.maxImageDimension2D,
    properties.limits.maxPerStageDescriptorUniformBuffers,
    properties.limits.maxPerStageDescriptorStorageBuffers,
    properties.limits.maxVertexInputAttributes,
    properties.limits.maxColorAttachments,
    properties.limits.maxViewports
  );

  return true;
}

void GraphicsAPI::Vulkan::enableVsync() {
};

bool GraphicsAPI::Vulkan::beginFrame() {
  VkFence fence = getFence(current_frame_index);
  GraphicsAPI::Vulkan::DeviceManager &device_manager = getDeviceManager();

  device_manager.waitForFences({ fence });
  vkAcquireNextImageKHR(
    device_manager.getDevice(),
    getSwapchain(),
    std::numeric_limits<uint64_t>::max(),
    getImageAvailableSemaphore(current_frame_index),
    VK_NULL_HANDLE,
    &current_image_index
  );
  device_manager.resetFences({ fence });

  VkCommandBuffer command_buffer = getCommandBuffer(current_image_index);
  vkResetCommandBuffer(command_buffer, 0);

  VkCommandBufferBeginInfo begin_info {
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .pInheritanceInfo = VK_NULL_HANDLE,
  };
  vkBeginCommandBuffer(command_buffer, &begin_info);

  // Begin render pass
  VkRenderPassBeginInfo renderpass_info{
    .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
    .pNext = VK_NULL_HANDLE,
    .renderPass = getRenderPass(),
    .framebuffer = getFramebufferForImage(current_image_index),
    .renderArea = { {0, 0}, getSwapchainExtent() },
    .clearValueCount = 1,
    .pClearValues = &clear_color,
  };
  vkCmdBeginRenderPass(command_buffer, &renderpass_info, VK_SUBPASS_CONTENTS_INLINE);

  return false;
};

bool GraphicsAPI::Vulkan::endFrame(Window *) {
  VkSwapchainKHR swapchain = getSwapchain();
  GraphicsAPI::Vulkan::DeviceManager &device_manager = getDeviceManager();
  VkCommandBuffer command_buffer = getCommandBuffer(current_image_index);
  VkSemaphore image_available_semaphore = getImageAvailableSemaphore(current_frame_index);
  VkSemaphore render_finished_semaphore = getRenderFinishedSemaphore(current_frame_index);
  const std::array wait_dst_stage_masks {
    static_cast<uint32_t>(VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT),
  };

  vkCmdEndRenderPass(command_buffer);
  vkEndCommandBuffer(command_buffer);

  VkSubmitInfo submit_info {
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext = VK_NULL_HANDLE,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &image_available_semaphore,
    .pWaitDstStageMask = wait_dst_stage_masks.data(),
    .commandBufferCount = 1,
    .pCommandBuffers = &command_buffer,
    .signalSemaphoreCount = 1,
    .pSignalSemaphores = &render_finished_semaphore,
  };

  device_manager.resetFences({ getFence(current_frame_index) });
  submitQueue(device_manager.getPresentQueue(), { submit_info }, current_frame_index);


  VkPresentInfoKHR present_info {
    .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
    .pNext = VK_NULL_HANDLE,
    .waitSemaphoreCount = 1,
    .pWaitSemaphores = &render_finished_semaphore,
    .swapchainCount = 1,
    .pSwapchains = &swapchain,
    .pImageIndices = &current_image_index,
    .pResults = VK_NULL_HANDLE,
  };

  vkQueuePresentKHR(device_manager.getPresentQueue(), &present_info);
  current_frame_index = (current_frame_index + 1) % GraphicsAPI::Vulkan::MAX_FRAMES_IN_FLIGHT;
  return false;
};

bool GraphicsAPI::Vulkan::drawIndexed(DrawInfo &mesh_data) {
  /* TODO: seperate
  GraphicsAPI::Vulkan::DescriptorManager &descriptor_manager = getDescriptorManager();
  VkDescriptorSet set = descriptor_manager.getSet(UniformBufferType::Camera, current_frame_index);
  vkCmdBindDescriptorSets(
      command_buffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      pipelines.at(0).getPipelineLayout(),
      0,
      1,
      &set,
      0,
      VK_NULL_HANDLE
  );
  */

  size_t offset = 0;
  auto &vk_draw_data = static_cast<DrawInfo::Vulkan &>(mesh_data);
  vkCmdBindVertexBuffers(vk_draw_data.command_buffer, 0, 1, &vk_draw_data.vertex_buffer, &offset);
  vkCmdBindIndexBuffer(vk_draw_data.command_buffer, vk_draw_data.index_buffer, 0, VK_INDEX_TYPE_UINT32);
  vkCmdDrawIndexed(vk_draw_data.command_buffer,vk_draw_data.index_count, 1, 0, 0, 0);

  return true;
}

bool GraphicsAPI::Vulkan::updateUBO(UniformBufferType, const void *, size_t, size_t) {
  return true;
}

VkResult Vulkan::submitQueue(VkQueue queue, const std::vector<VkSubmitInfo> &submits, uint32_t frame_index) {
  return vkQueueSubmit(queue, submits.size(), submits.data(), getFence(frame_index));
}

bool Vulkan::createBuffer(void *data, uint32_t size, VkBufferUsageFlags usage, 
                         VkBuffer &buffer, VkDeviceMemory &memory) {
  VkBuffer staging_buffer = VK_NULL_HANDLE;
  VkDeviceMemory staging_memory = VK_NULL_HANDLE;

  if (!createRawBuffer(
    size,
    VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
    VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
    staging_buffer,
    staging_memory)) {
      LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to create staging buffer");
      return false;
    }

  void *mapped = nullptr;
  vkMapMemory(device_manager->getDevice(), staging_memory, 0, size, 0, &mapped);

  if (data)
    std::memcpy(mapped, data, size);
  else
    std::memset(mapped, 0, size);

  vkUnmapMemory(device_manager->getDevice(), staging_memory);

  if (!createRawBuffer(
      size,
      VK_BUFFER_USAGE_TRANSFER_DST_BIT | usage,
      VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
      buffer,
      memory))
      return false;

  VkCommandBuffer command_buffer = beginSingleTimeCommands();
  if (!command_buffer) {
    LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to begin single time commands");
    return false;
  }

  VkBufferCopy copy_region {
    .srcOffset = 0,
    .dstOffset = 0,
    .size = size
  };
  vkCmdCopyBuffer(command_buffer, staging_buffer, buffer, 1, &copy_region);

  if(!endSingleTimeCommands(command_buffer)) {
    LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to end single time commands");
    return false;
  }

  vkDestroyBuffer(device_manager->getDevice(), staging_buffer, VK_NULL_HANDLE);
  vkFreeMemory(device_manager->getDevice(), staging_memory, VK_NULL_HANDLE);
  return true;
}

bool Vulkan::createRawBuffer(size_t size,
                           VkBufferUsageFlags usage,
                           VkMemoryPropertyFlags properties,
                           VkBuffer &buffer,
                           VkDeviceMemory &memory) {
  VkBufferCreateInfo buffer_info {
    .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = 0,
    .size = size,
    .usage = usage,
    .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    .queueFamilyIndexCount = 0,
    .pQueueFamilyIndices = VK_NULL_HANDLE,
  };

  if (vkCreateBuffer(device_manager->getDevice(), &buffer_info, VK_NULL_HANDLE, &buffer) != VK_SUCCESS) {
    LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to create buffer");
    return false;
  }

  VkMemoryRequirements mem_requirements {};
  vkGetBufferMemoryRequirements(device_manager->getDevice(), buffer, &mem_requirements);

  uint32_t memory_type = findMemoryType(mem_requirements.memoryTypeBits, properties);
  if (memory_type == std::numeric_limits<uint32_t>::max()) {
    LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to find suitable memory type");
    return false;
  }

  VkMemoryAllocateInfo alloc_info{
    .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .allocationSize = mem_requirements.size,
    .memoryTypeIndex = memory_type
  };

  if (vkAllocateMemory(device_manager->getDevice(), &alloc_info, VK_NULL_HANDLE, &memory) != VK_SUCCESS) {
    LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to allocate memory for buffer");
    return false;
  }

  if (vkBindBufferMemory(device_manager->getDevice(), buffer, memory, 0) != VK_SUCCESS) {
    LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to bind buffer memory");
    return false;
  }

  return true;
}

VkCommandBuffer Vulkan::beginSingleTimeCommands() const {
  VkCommandBufferAllocateInfo alloc_info{
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
    .pNext = VK_NULL_HANDLE,
    .commandPool = command_pool,
    .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
    .commandBufferCount = 1
  };

  VkCommandBuffer command_buffer = VK_NULL_HANDLE;
  if (vkAllocateCommandBuffers(device_manager->getDevice(), &alloc_info, &command_buffer) != VK_SUCCESS) {
    LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to allocate command buffer");
    return VK_NULL_HANDLE;
  }

  VkCommandBufferBeginInfo begin_info{
    .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
    .pNext = VK_NULL_HANDLE,
    .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    .pInheritanceInfo = VK_NULL_HANDLE,
  };

  if (vkBeginCommandBuffer(command_buffer, &begin_info) != VK_SUCCESS) {
    LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to begin command buffer");
    return VK_NULL_HANDLE;
  }

  return command_buffer;
}

bool Vulkan::endSingleTimeCommands(VkCommandBuffer command_buffer) const {
  VkResult result = vkEndCommandBuffer(command_buffer);
  if (result != VK_SUCCESS) {
    LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to end command buffer with result: {}", (uint32_t)result);
    return false;
  }

  VkSubmitInfo submit_info{
    .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
    .pNext = VK_NULL_HANDLE,
    .waitSemaphoreCount = 0,
    .pWaitSemaphores = VK_NULL_HANDLE,
    .pWaitDstStageMask = VK_NULL_HANDLE,
    .commandBufferCount = 1,
    .pCommandBuffers = &command_buffer,
    .signalSemaphoreCount = 0,
    .pSignalSemaphores = VK_NULL_HANDLE,
  };

  result = vkQueueSubmit(device_manager->getGraphicsQueue(), 1, &submit_info, VK_NULL_HANDLE);
  if (result != VK_SUCCESS) {
    LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to submit single time commands with result: {}", (uint32_t)result);
    return false;
  }

  result = vkQueueWaitIdle(device_manager->getGraphicsQueue());
  if (result != VK_SUCCESS) {
    LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to wait for queue idle with result: {}", (uint32_t)result);
    return false;
  }

  vkFreeCommandBuffers(device_manager->getDevice(), command_pool, 1, &command_buffer);
  return true;
}

uint32_t Vulkan::findMemoryType(uint32_t type_filter, VkMemoryPropertyFlags properties) const {
  VkPhysicalDeviceMemoryProperties mem_properties{};
  vkGetPhysicalDeviceMemoryProperties(device_manager->getPhysicalDevice(), &mem_properties);

  for (uint32_t i = 0; i < mem_properties.memoryTypeCount; ++i) {
    if ((type_filter & (1 << i)) && (mem_properties.memoryTypes[i].propertyFlags & properties) == properties) {
      return i;
    }
  }

  return std::numeric_limits<uint32_t>::max();
}

} /* namespace Engine */
#include <core/logging.hpp>
#include <core/graphics/vulkan/vkrenderer.hpp>

namespace Engine {

// Renderer::Vulkan::UniformBufferManager::~UniformBufferManager() noexcept {
//   GraphicsAPI::Vulkan::DeviceManager &device_manager = vulkan->getDeviceManager();
//   VkDevice device = device_manager.getDevice();
//   device_manager.waitIdle();

//   for (auto &[_, ubo] : uniform_buffers) {          
//     for (BufferFrame &bframe : ubo.frames) {
//       if (bframe.mapped) vkUnmapMemory(device, bframe.memory);
//       if (bframe.buffer) vkDestroyBuffer(device, bframe.buffer, nullptr);
//       if (bframe.memory) vkFreeMemory(device, bframe.memory, nullptr);
//     }
//     ubo.frames.clear();
//   }

//   uniform_buffers.clear();
// }

// bool Renderer::Vulkan::UniformBufferManager::create(UniformBufferType type, size_t size_in_bytes) {
//   if (uniform_buffers.find(type) != uniform_buffers.end()) {
//     LOG_WARN("[GraphicsAPI::Vulkan::UniformBufferManager]: Buffer of type `{}` already created", uniformBufferTypeToString(type));
//     return true;
//   }

//   UniformBuffer ubo{};
//   ubo.type = type;
//   ubo.size = size_in_bytes;
//   ubo.frames.resize(GraphicsAPI::Vulkan::MAX_FRAMES_IN_FLIGHT);

//   for (uint32_t i = 0; i < GraphicsAPI::Vulkan::MAX_FRAMES_IN_FLIGHT; ++i) {
//     if (!createBufferForFrame(ubo.frames[i], size_in_bytes)) {
//       LOG_ERROR("[GraphicsAPI::Vulkan::UniformBufferManager]: Failed to create uniform buffer for frame {}", i);
//       return false;
//     }
//   }

//   uniform_buffers[type] = std::move(ubo);
//   return true;
// }

// bool Renderer::Vulkan::UniformBufferManager::update(UniformBufferType type, uint32_t frame_index, const void *data, size_t size_in_bytes, size_t offset_in_bytes) const {
//   auto it = uniform_buffers.find(type);

//   if (it == uniform_buffers.end()) {
//     LOG_ERROR("[GraphicsAPI::Vulkan::UniformBufferManager]: Buffer of type `{}` not created", uniformBufferTypeToString(type));
//     return false;
//   }

//   const BufferFrame &bframe = it->second.frames.at(frame_index);
//   std::memcpy(
//     (std::byte *)bframe.mapped + offset_in_bytes,
//     data,
//     std::min(bframe.size, size_in_bytes));
//   return true;
// }

// VkDescriptorBufferInfo Renderer::Vulkan::UniformBufferManager::getDescriptorBufferInfo(UniformBufferType type, uint32_t frame_index) const {
//   VkDescriptorBufferInfo info{};
//   auto it = uniform_buffers.find(type);

//   if (it == uniform_buffers.end()) {
//     LOG_WARN("[UniformBufferManager::Vulkan]: Buffer of type `{}` not created", uniformBufferTypeToString(type));
//     return info;
//   }

//   const BufferFrame &bframe = it->second.frames.at(frame_index);
//   info.buffer = bframe.buffer;
//   info.offset = 0;
//   info.range = bframe.size;
//   return info;
// }

// bool Renderer::Vulkan::UniformBufferManager::createBufferForFrame(BufferFrame &bframe, uint32_t size) const {
//   GraphicsAPI::Vulkan::DeviceManager &device_manager = vulkan->getDeviceManager();
//   bframe.size = size;

//   if(!vulkan->createRawBuffer(bframe.size,
//       VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
//       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
//       bframe.buffer,
//       bframe.memory))
//     return false;

//   VkResult result = vkMapMemory(device_manager.getDevice(), bframe.memory, 0, size, 0, &bframe.mapped);
//   if (result != VK_SUCCESS) {
//     LOG_ERROR("[GraphicsAPI::Vulkan::UniformBufferManager]: Failed to map memory");
//     return false;
//   }

//   return true;
// }

}; /* namespace Engine */
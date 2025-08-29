#pragma once

#include <vector>
#include <unordered_map>
#include <array>
#include <span>

#include <core/graphics/vulkan/vulkan.hpp>
#include <core/graphics/vulkan/vkdevice.hpp>
#include <core/graphics/buffer.hpp>

#include <core/logging.hpp>

namespace Engine {

struct DescriptorSetLayoutInfo {
  uint32_t binding;
  VkDescriptorType type;
  VkShaderStageFlags stageFlags;
};

struct DescriptorData {
  VkDescriptorSetLayout layout = VK_NULL_HANDLE;
  VkDescriptorPool pool = VK_NULL_HANDLE;
  std::vector<VkDescriptorSet> sets;
};

struct GraphicsAPI::Vulkan::DescriptorManager {
  DescriptorManager() noexcept
    : device_manager(nullptr)
  {}

  ~DescriptorManager() noexcept {
    VkDevice device = device_manager->getDevice();

    for (auto &[type, data] : descriptor_data) {
      if (data.pool != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(device, data.pool, nullptr);

      if (data.layout != VK_NULL_HANDLE && device != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(device, data.layout, nullptr);
    }
    descriptor_data.clear();
    for (auto &l : layouts) l = VK_NULL_HANDLE;
  }

  bool init(DeviceManager *_device_manager){
    if(_device_manager == nullptr) {
      LOG_ERROR("[GraphicsAPI::Vulkan::DescriptorManager]: device manager is not initialized");
      return false;
    }

    device_manager = _device_manager;
    return true;
  }

  bool createLayout(UniformBufferType type, const DescriptorSetLayoutInfo& info) {
    DescriptorData data{};
    VkDevice device = device_manager->getDevice();

    VkDescriptorSetLayoutBinding layout_binding{};
    layout_binding.binding = info.binding;
    layout_binding.descriptorType = info.type;
    layout_binding.descriptorCount = 1;
    layout_binding.stageFlags = info.stageFlags;
    layout_binding.pImmutableSamplers = nullptr;

    VkDescriptorSetLayoutCreateInfo layoutInfo{};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &layout_binding;

    if (vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &data.layout) != VK_SUCCESS) {
      LOG_ERROR("[GraphicsAPI::Vulkan::DescriptorManager]: Failed to create descriptor set layout");
      return false;
    }

    descriptor_data[type] = data;

    const size_t idx = static_cast<size_t>(type);
    
    if (idx < layouts.size()) {
      layouts[idx] = data.layout;
    }

    return true;
  }

  bool allocateSets(UniformBufferType type) {
    VkDevice device = device_manager->getDevice();
    DescriptorData &data = descriptor_data.at(type);

    VkDescriptorPoolSize poolSize{};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = GraphicsAPI::Vulkan::MAX_FRAMES_IN_FLIGHT;

    VkDescriptorPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;
    poolInfo.maxSets = GraphicsAPI::Vulkan::MAX_FRAMES_IN_FLIGHT;

    if (vkCreateDescriptorPool(device, &poolInfo, nullptr, &data.pool) != VK_SUCCESS) {
      LOG_ERROR("[GraphicsAPI::Vulkan::DescriptorManager]: Failed to create descriptor pool");
      return false;
    }

    std::vector<VkDescriptorSetLayout> setLayouts(GraphicsAPI::Vulkan::MAX_FRAMES_IN_FLIGHT, data.layout);

    VkDescriptorSetAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = data.pool;
    allocInfo.descriptorSetCount = GraphicsAPI::Vulkan::MAX_FRAMES_IN_FLIGHT;
    allocInfo.pSetLayouts = setLayouts.data();

    data.sets.resize(GraphicsAPI::Vulkan::MAX_FRAMES_IN_FLIGHT);
    if (vkAllocateDescriptorSets(device, &allocInfo, data.sets.data()) != VK_SUCCESS) {
      LOG_ERROR("[GraphicsAPI::Vulkan::DescriptorManager]: Failed to allocate descriptor sets");
      return false;
    }

    return true;
  }

  bool updateSet(UniformBufferType type, const VkDescriptorBufferInfo &buffer_info, uint32_t frame_index) {
    DescriptorData &data = descriptor_data.at(type);

    if (frame_index >= data.sets.size()) {
      LOG_ERROR("[GraphicsAPI::Vulkan::DescriptorManager]: Invalid frame index {} >= {}", frame_index, data.sets.size());
      return false;
    }

    VkWriteDescriptorSet descriptor_write{};
    descriptor_write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_write.dstSet = data.sets.at(frame_index);
    descriptor_write.dstBinding = 0;
    descriptor_write.dstArrayElement = 0;
    descriptor_write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_write.descriptorCount = 1;
    descriptor_write.pBufferInfo = &buffer_info;

    vkUpdateDescriptorSets(device_manager->getDevice(), 1, &descriptor_write, 0, nullptr);
    return true;
  }

  VkDescriptorSet &getSet(UniformBufferType type, uint32_t frame_index) {
    return descriptor_data.at(type).sets.at(frame_index);
  }

  std::span<VkDescriptorSetLayout> getLayouts() {
    return std::span<VkDescriptorSetLayout> { layouts };
  }

private:
  DeviceManager *device_manager;
  std::unordered_map<UniformBufferType, DescriptorData> descriptor_data;
  std::array<VkDescriptorSetLayout, GraphicsAPI::Vulkan::MAX_FRAMES_IN_FLIGHT> layouts;
};

} /* namespace Engine */

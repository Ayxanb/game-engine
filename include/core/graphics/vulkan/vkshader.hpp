#pragma once

#include "vulkan/vulkan_core.h"
#include <vector>
#include <cassert>

#include <util/file_utils.hpp>
#include <core/graphics/shader.hpp>
#include <core/graphics/vulkan/vulkan.hpp>
#include <core/graphics/vulkan/vkdevice.hpp>

namespace Engine {

class Pipeline::Vulkan : public Pipeline {
  ShaderStages stages;
  VkPipeline pipeline = nullptr;
  VkPipelineLayout layout = nullptr;
  std::vector<VkShaderModule> modules;
  GraphicsAPI::Vulkan *vulkan = nullptr;

public:
  Vulkan(GraphicsAPI::Vulkan *_vulkan) noexcept : vulkan(_vulkan) {}
  ~Vulkan() noexcept override;
  
  bool create(ShaderStages) override;
  void bind(uint32_t frame_index) override {
    VkCommandBuffer command_buffer = vulkan->getCommandBuffer(frame_index);
    vkCmdBindPipeline(
      command_buffer,
      VK_PIPELINE_BIND_POINT_GRAPHICS,
      pipeline
    );
  }


private:
  VkShaderModule loadShader(File::Path);
};

} /* namespace Engine */
#include "vulkan/vulkan_core.h"
#include <core/logging.hpp>
#include <core/graphics/mesh.hpp>
#include <core/graphics/vulkan/vkshader.hpp>
#include <core/graphics/vulkan/vkdevice.hpp>
#include <core/graphics/vulkan/descriptor_manager.hpp>

#include <bit>
#include <vector>
#include <cstdint>

namespace Engine {

static std::array<VkVertexInputAttributeDescription, 4> getAttributeDescriptions() {
  return std::array<VkVertexInputAttributeDescription, 4> {
    VkVertexInputAttributeDescription {
      .location = 0,
      .binding  = 0,
      .format   = VK_FORMAT_R32G32B32_SFLOAT,
      .offset   = offsetof(Mesh::Vertex, position)
    },
    VkVertexInputAttributeDescription {
      .location = 1,
      .binding  = 0,
      .format   = VK_FORMAT_R32G32B32_SFLOAT,
      .offset   = offsetof(Mesh::Vertex, color)
    },
    VkVertexInputAttributeDescription {
      .location = 2,
      .binding  = 0,
      .format   = VK_FORMAT_R32G32_SFLOAT,
      .offset   = offsetof(Mesh::Vertex, uv)
    },
    VkVertexInputAttributeDescription {
      .location = 3,
      .binding  = 0,
      .format   = VK_FORMAT_R32G32B32_SFLOAT,
      .offset   = offsetof(Mesh::Vertex, normal)
    }
  };
}

Pipeline::Vulkan::~Vulkan() noexcept {
  GraphicsAPI::Vulkan::DeviceManager &device_manager = vulkan->getDeviceManager();
  VkDevice device = device_manager.getDevice();
  device_manager.waitIdle();
  
  if(pipeline != VK_NULL_HANDLE) {
    vkDestroyPipeline(device, pipeline, VK_NULL_HANDLE);
    LOG_INFO("[GraphicsAPI::Vulkan]: Graphics pipeline destroyed");
  }

  if(layout != VK_NULL_HANDLE) {
    vkDestroyPipelineLayout(device, layout, VK_NULL_HANDLE);
    LOG_INFO("[GraphicsAPI::Vulkan]: Graphics pipeline layout destroyed");
  }

  for (VkShaderModule module : modules) {
    vkDestroyShaderModule(device, module, VK_NULL_HANDLE);
    LOG_INFO("[GraphicsAPI::Vulkan]: Shader module {} destroyed", (void *)module);
  }
}

VkShaderModule Pipeline::Vulkan::loadShader(File::Path path) {
  VkDevice device = vulkan->getDeviceManager().getDevice();

  /* --- Load shader code --- */
  std::vector<char> code;
  if (!File::readContentBytes(path, code)) {
    LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to read shader file: {}", path.string());
    return VK_NULL_HANDLE;
  }

  /* --- Shader Module Create Info --- */
  VkShaderModuleCreateInfo create_info {
    .sType    = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
    .pNext    = VK_NULL_HANDLE,
    .flags    = 0,
    .codeSize = code.size(),
    .pCode    = std::bit_cast<uint32_t *>(code.data())
  };

  /* --- Create Shader Module --- */
  VkShaderModule shader_module = VK_NULL_HANDLE;
  if (vkCreateShaderModule(device, &create_info, nullptr, &shader_module) != VK_SUCCESS) {
    LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to create shader module: {}", path.relative_path().string());
    return VK_NULL_HANDLE;
  }

  /* Track module for cleanup */
  modules.push_back(shader_module);
  LOG_INFO("[GraphicsAPI::Vulkan]: Shader module {} created successfully", (void *)shader_module);
  return shader_module;
}

bool Pipeline::Vulkan::create(ShaderStages stages_in) {
  stages = stages_in;
  std::vector<VkPipelineShaderStageCreateInfo> stages_info;

  /* Load shader modules */
  VkShaderModule vert_module = VK_NULL_HANDLE;
  VkShaderModule frag_module = VK_NULL_HANDLE;

  if (!stages.vertex.empty()) {
    vert_module = loadShader(stages.vertex);
    if (vert_module == VK_NULL_HANDLE)
      return false;

    stages_info.push_back(
      VkPipelineShaderStageCreateInfo {
        .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext               = VK_NULL_HANDLE,
        .flags               = 0,
        .stage               = VK_SHADER_STAGE_VERTEX_BIT,
        .module              = vert_module,
        .pName               = "main",
        .pSpecializationInfo = VK_NULL_HANDLE
      }
    );
  }

  if (!stages.fragment.empty()) {
    frag_module = loadShader(stages.fragment);
    if (frag_module == VK_NULL_HANDLE)
      return false;

    stages_info.push_back(
      VkPipelineShaderStageCreateInfo {
        .sType               = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .pNext               = VK_NULL_HANDLE,
        .flags               = 0,
        .stage               = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module              = frag_module,
        .pName               = "main",
        .pSpecializationInfo = VK_NULL_HANDLE
      }
    );

  }

  /* --- Vertex Input --- */
  VkVertexInputBindingDescription binding_description {
    .binding   = 0,
    .stride    = sizeof(Mesh::Vertex),
    .inputRate = VK_VERTEX_INPUT_RATE_VERTEX
  };

  auto attribute_descriptions = getAttributeDescriptions();

  VkPipelineVertexInputStateCreateInfo vertex_input_info {
    .sType                           = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    .pNext                           = VK_NULL_HANDLE,
    .flags                           = 0,
    .vertexBindingDescriptionCount   = 1,
    .pVertexBindingDescriptions      = &binding_description,
    .vertexAttributeDescriptionCount = static_cast<uint32_t>(attribute_descriptions.size()),
    .pVertexAttributeDescriptions    = attribute_descriptions.data()
  };

  /* --- Input Assembly --- */
  VkPipelineInputAssemblyStateCreateInfo input_assembly {
    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
    .pNext                  = VK_NULL_HANDLE,
    .flags                  = 0,
    .topology               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
    .primitiveRestartEnable = VK_FALSE
  };

  /* --- Viewport & Scissor --- */
  VkExtent2D extent = vulkan->getSwapchainExtent();

  VkViewport viewport {
    .x        = 0.0f,
    .y        = 0.0f,
    .width    = static_cast<float>(extent.width),
    .height   = static_cast<float>(extent.height),
    .minDepth = 0.0f,
    .maxDepth = 1.0f
  };

  VkRect2D scissor {
    .offset { 0, 0 },
    .extent = extent,
  };

  VkPipelineViewportStateCreateInfo viewport_state {
    .sType         = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
    .pNext         = VK_NULL_HANDLE,
    .flags         = 0,
    .viewportCount = 1,
    .pViewports    = &viewport,
    .scissorCount  = 1,
    .pScissors     = &scissor
  };

  /* --- Rasterizer --- */
  VkPipelineRasterizationStateCreateInfo rasterizer {
    .sType                   = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
    .pNext                   = VK_NULL_HANDLE,
    .flags                   = 0,
    .depthClampEnable        = VK_FALSE,
    .rasterizerDiscardEnable = VK_FALSE,
    .polygonMode             = VK_POLYGON_MODE_FILL,
    .cullMode                = VK_CULL_MODE_NONE,
    .frontFace               = VK_FRONT_FACE_CLOCKWISE,
    .depthBiasEnable         = VK_FALSE,
    .depthBiasConstantFactor = 0.0f,
    .depthBiasClamp          = 0.0f,
    .depthBiasSlopeFactor    = 0.0f,
    .lineWidth               = 1.0f
  };

  /* --- Multisampling --- */
  VkPipelineMultisampleStateCreateInfo multisampling {
    .sType                 = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
    .pNext                 = VK_NULL_HANDLE,
    .flags                 = 0,
    .rasterizationSamples  = VK_SAMPLE_COUNT_1_BIT,
    .sampleShadingEnable   = VK_FALSE,
    .minSampleShading      = 1.0f,
    .pSampleMask           = VK_NULL_HANDLE,
    .alphaToCoverageEnable = VK_FALSE,
    .alphaToOneEnable      = VK_FALSE
  };

  /* --- Color Blend --- */
  VkPipelineColorBlendAttachmentState color_blend_attachment {
    .blendEnable         = VK_FALSE,
    .srcColorBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstColorBlendFactor = VK_BLEND_FACTOR_ZERO,
    .colorBlendOp        = VK_BLEND_OP_ADD,
    .srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE,
    .dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO,
    .alphaBlendOp        = VK_BLEND_OP_ADD,
    .colorWriteMask      = VK_COLOR_COMPONENT_R_BIT |
                           VK_COLOR_COMPONENT_G_BIT |
                           VK_COLOR_COMPONENT_B_BIT |
                           VK_COLOR_COMPONENT_A_BIT
  };

  VkPipelineColorBlendStateCreateInfo color_blending {
    .sType           = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
    .pNext           = VK_NULL_HANDLE,
    .flags           = 0,
    .logicOpEnable   = VK_FALSE,
    .logicOp         = VK_LOGIC_OP_COPY,
    .attachmentCount = 1,
    .pAttachments    = &color_blend_attachment,
    .blendConstants  { 0.0f, 0.0f, 0.0f, 0.0f }
  };

  /* --- Depth Stencil --- */
  VkPipelineDepthStencilStateCreateInfo depth_stencil {
    .sType                 = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
    .pNext                 = VK_NULL_HANDLE,
    .flags                 = 0,
    .depthTestEnable       = VK_TRUE,
    .depthWriteEnable      = VK_TRUE,
    .depthCompareOp        = VK_COMPARE_OP_LESS,
    .depthBoundsTestEnable = VK_FALSE,
    .stencilTestEnable     = VK_FALSE,
    .front                 {},
    .back                  {},
    .minDepthBounds        = 0.0f,
    .maxDepthBounds        = 1.0f
  };

  // GraphicsAPI::Vulkan::DescriptorManager &descriptor_manager = vulkan->getDescriptorManager();
  // std::span<VkDescriptorSetLayout> layouts = descriptor_manager.getLayouts();

  /* --- Pipeline Layout --- */
  VkPipelineLayoutCreateInfo layout_info {
    .sType                  = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
    .pNext                  = VK_NULL_HANDLE,
    .flags                  = 0,
    .setLayoutCount         = 0,
    .pSetLayouts            = VK_NULL_HANDLE,
    .pushConstantRangeCount = 0,
    .pPushConstantRanges    = VK_NULL_HANDLE
  };

  VkDevice device = vulkan->getDeviceManager().getDevice();

  if (vkCreatePipelineLayout(device, &layout_info, nullptr, &layout) != VK_SUCCESS) {
    LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to create pipeline layout");
    return false;
  }

  /* --- Graphics Pipeline --- */
  VkGraphicsPipelineCreateInfo pipeline_info {
    .sType               = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
    .pNext               = VK_NULL_HANDLE,
    .flags               = 0,
    .stageCount          = 2,
    .pStages             = stages_info.data(),
    .pVertexInputState   = &vertex_input_info,
    .pInputAssemblyState = &input_assembly,
    .pTessellationState  = VK_NULL_HANDLE,
    .pViewportState      = &viewport_state,
    .pRasterizationState = &rasterizer,
    .pMultisampleState   = &multisampling,
    .pDepthStencilState  = &depth_stencil,
    .pColorBlendState    = &color_blending,
    .pDynamicState       = VK_NULL_HANDLE,
    .layout              = layout,
    .renderPass          = vulkan->getRenderPass(),
    .subpass             = 0,
    .basePipelineHandle  = VK_NULL_HANDLE,
    .basePipelineIndex   = -1
  };

  if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipeline_info, nullptr, &pipeline) != VK_SUCCESS) {
    LOG_ERROR("[GraphicsAPI::Vulkan]: Failed to create graphics pipeline");
    return false;
  }

  LOG_INFO("[GraphicsAPI::Vulkan]: Graphics pipeline created successfully");
  return true;
}

} /* namespace Engine */
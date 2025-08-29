#include <core/logging.hpp>
#include <core/graphics/renderer.hpp>
#include <core/graphics/opengl/glmesh.hpp>
#include <core/graphics/vulkan/vkmesh.hpp>
#include <core/graphics/opengl/glshader.hpp>
#include <core/graphics/vulkan/vkshader.hpp>
#include <core/graphics/vulkan/vkbuffer.hpp>
#include <core/graphics/vulkan/descriptor_manager.hpp>

#include <cstdint>
#include <memory>

namespace Engine {

bool Renderer::init(Config::Renderer &config) {
  switch (config.backend) {
  case GraphicsAPI::Backend::OpenGL:
    graphics_api = std::make_unique<GraphicsAPI::OpenGL>();
    break;
  case GraphicsAPI::Backend::Vulkan:
    graphics_api = std::make_unique<GraphicsAPI::Vulkan>();
    break;
  }

  if (!graphics_api->init(window))
    return false;

  auto vulkan = static_cast<GraphicsAPI::Vulkan *>(graphics_api.get());

  mesh_manager = std::make_unique<MeshManager::Vulkan>(vulkan);
  ub_manager = std::make_unique<UniformBufferManager::Vulkan>(vulkan);

  // auto &descriptor_manager = vulkan->getDescriptorManager();

  /* TEST */
  if(!pipelines.emplace_back(std::make_unique<Pipeline::Vulkan>(vulkan))->create(config.shader_paths.at(0)))
    return false;

  return true;
}

bool Renderer::beginFrame() {
  graphics_api->beginFrame();
  return false;
};

bool Renderer::endFrame() {
  graphics_api->endFrame(window);
  return false;
};

bool Renderer::render(Mesh::Handle handle) {

  if (handle == Mesh::InvalidHandle) {
    LOG_ERROR("[Renderer] - Invalid mesh handle: {}", handle);
    return false;
  }
  
  // graphics_api->drawIndexed();
  return true;
}

bool Renderer::bindPipeline(uint32_t handle) {
  std::unique_ptr<Pipeline> &pipeline = pipelines.at(handle);
  uint32_t current_frame_index = graphics_api->getCurrentFrameIndex();

  pipeline->bind(current_frame_index);
  return true;
}

}; /* namespace Engine */
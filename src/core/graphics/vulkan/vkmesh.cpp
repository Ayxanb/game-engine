#include <core/graphics/vulkan/vkmesh.hpp>
#include <core/graphics/vulkan/vkdevice.hpp>
#include <memory>

namespace Engine {

MeshManager::Vulkan::~Vulkan() noexcept {
  auto &device_manager = vulkan->getDeviceManager();
  VkDevice device = device_manager.getDevice();
  device_manager.waitIdle();
  
  for (std::unique_ptr<MeshInfo> &mesh_data : meshes) {
    auto vk_mesh_data = static_cast<MeshInfo::Vulkan *>(mesh_data.get());

    if (vk_mesh_data->vertex_buffer)
      vkDestroyBuffer(device, vk_mesh_data->vertex_buffer, VK_NULL_HANDLE);
    if (vk_mesh_data->index_buffer)
      vkDestroyBuffer(device, vk_mesh_data->index_buffer, VK_NULL_HANDLE);

    if (vk_mesh_data->vertex_memory)
      vkFreeMemory(device, vk_mesh_data->vertex_memory, VK_NULL_HANDLE);
    if (vk_mesh_data->index_memory)
      vkFreeMemory(device, vk_mesh_data->index_memory, VK_NULL_HANDLE);
  }
}

Mesh::Handle MeshManager::Vulkan::addMesh(Mesh &mesh) {
  auto data = std::make_unique<MeshInfo::Vulkan>();
  auto vertices = mesh.getVerticesView();
  auto indices = mesh.getIndicesView();

  data->cpu_vertices.assign(vertices.begin(), vertices.end());
  data->cpu_indices.assign(indices.begin(), indices.end());

  data->gpu_uploaded = false;
  data->alive = true;

  Mesh::Handle handle = meshes.size();
  meshes.push_back(std::move(data));
  return handle;
}

void MeshManager::Vulkan::uploadPending() {
  for (std::unique_ptr<MeshInfo> &mesh_data : meshes) {
    auto vk_mesh_data = static_cast<MeshInfo::Vulkan *>(mesh_data.get());
    
    if (!mesh_data->alive || mesh_data->gpu_uploaded)
      continue;

    if (!mesh_data->cpu_vertices.empty())
      vulkan->createBuffer(mesh_data->cpu_vertices.data(),
                           std::span(mesh_data->cpu_vertices).size_bytes(),
                           VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                           vk_mesh_data->vertex_buffer,
                           vk_mesh_data->vertex_memory);

    if (!mesh_data->cpu_indices.empty())
      vulkan->createBuffer(mesh_data->cpu_indices.data(),
                           std::span(mesh_data->cpu_indices).size_bytes(),
                           VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
                           vk_mesh_data->index_buffer,
                           vk_mesh_data->index_memory);

    mesh_data->gpu_uploaded = true;
  }
}

} /* namespace Engine */
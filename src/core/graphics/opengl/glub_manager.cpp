#include <core/graphics/opengl/glrenderer.hpp>
#include <core/logging.hpp>

namespace Engine {

// bool Renderer::OpenGL::UniformBufferManager::create(UniformBufferType type, size_t size_in_bytes) {
//   if (uniform_buffers.find(type) != uniform_buffers.end()) {
//     LOG_WARN("[GraphicsAPI::OpenGL]: Uniform buffer type {} already exists (size: {} bytes)", 
//              static_cast<uint32_t>(type), size_in_bytes);
//     return true;
//   }

//   GLuint buffer = 0;
//   glGenBuffers(1, &buffer);
//   if (!buffer) {
//     LOG_ERROR("[GraphicsAPI::OpenGL]: Failed to generate uniform buffer for type {} (size: {} bytes)", 
//               static_cast<uint32_t>(type), size_in_bytes);
//     return false;
//   }

//   glBindBuffer(GL_UNIFORM_BUFFER, buffer);
//   glBufferData(GL_UNIFORM_BUFFER, size_in_bytes, nullptr, GL_DYNAMIC_DRAW);
//   glBindBuffer(GL_UNIFORM_BUFFER, 0);

//   uniform_buffers[type] = BufferData {
//     .buffer = buffer,
//     .size_in_bytes = size_in_bytes
//   };

//   LOG_INFO("[GraphicsAPI::OpenGL]: Created uniform buffer type {} (ID: {}, Size: {} bytes)", 
//            static_cast<uint32_t>(type), buffer, size_in_bytes);
//   return true;
// }

// bool Renderer::OpenGL::UniformBufferManager::update(UniformBufferType type, const void * data, 
//                                                    size_t size_in_bytes, size_t offset_in_bytes) const {
//   auto it = uniform_buffers.find(type);
//   if (it == uniform_buffers.end()) {
//     LOG_ERROR("[GraphicsAPI::OpenGL]: Uniform buffer type {} not found for update", static_cast<uint32_t>(type));
//     return false;
//   }
  
//   const size_t actual_size = std::min(it->second.size_in_bytes, size_in_bytes);
//   if (offset_in_bytes + actual_size > it->second.size_in_bytes) {
//     LOG_ERROR(
//       "[GraphicsAPI::OpenGL]: Update would exceed buffer bounds (Type: {}, Offset: {}, Size: {}, Capacity: {})",
//         static_cast<uint32_t>(type),
//         offset_in_bytes,
//         actual_size,
//         it->second.size_in_bytes
//     );
//     return false;
//   }

//   glBindBuffer(GL_UNIFORM_BUFFER, it->second.buffer);
//   glBufferSubData(GL_UNIFORM_BUFFER, offset_in_bytes, actual_size, data);
//   glBindBuffer(GL_UNIFORM_BUFFER, 0);
  
//   return true;
// }

// void Renderer::OpenGL::UniformBufferManager::bindUBO(UniformBufferType type, GLuint binding_point) const {
//   auto it = uniform_buffers.find(type);
//   if (it == uniform_buffers.end()) {
//     LOG_ERROR("[GraphicsAPI::OpenGL]: Uniform buffer type `{}` not found for binding", static_cast<uint32_t>(type));
//     return;
//   }

//   glBindBufferBase(GL_UNIFORM_BUFFER, binding_point, it->second.buffer);
// }

} // namespace Engine
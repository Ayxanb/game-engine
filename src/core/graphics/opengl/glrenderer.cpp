#include <core/logging.hpp>
#include <core/graphics/opengl/glrenderer.hpp>
#include <core/graphics/camera/camera.hpp>

namespace Engine {

// bool Renderer::OpenGL::render(Mesh::Handle handle) {

//   ub_manager.bindUBO(UniformBufferType::Camera, 0);
  
//   DrawInfo::OpenGL draw_info {};
//   draw_info.handle = handle;

//   if(draw_info.handle == std::numeric_limits<Mesh::Handle>::max()){
//     LOG_ERROR("[GraphicsAPI::OpenGL]: Invalid mesh handle {}", draw_info.handle);
//     return false;
//   }

//   auto &render_data = static_cast<DrawInfo::OpenGL &>(draw_info);
//   MeshData &mesh = mesh_manager->get(render_data.handle);

//   glBindVertexArray(mesh.vao);
//   glDrawElements(GL_TRIANGLES, mesh.index_count, GL_UNSIGNED_INT, nullptr);
//   glBindVertexArray(0);

//   return true;
// }

} /* namespace Engine */

#include <core/graphics/opengl/opengl.hpp>
#include <core/graphics/opengl/glrenderer.hpp>

namespace Engine {

// Mesh::Handle Renderer::OpenGL::MeshManager::upload(Mesh &mesh) {
//   MeshInfo data;
//   glGenVertexArrays(1, &data.vao);
//   glGenBuffers(1, &data.vbo);
//   glGenBuffers(1, &data.ibo);

//   std::span<Mesh::Vertex> vertices = mesh.getVerticesView();
//   std::span<Mesh::Index> indices = mesh.getIndicesView();
  
//   glBindVertexArray(data.vao);
//   glBindBuffer(GL_ARRAY_BUFFER, data.vbo);
//   glBufferData(GL_ARRAY_BUFFER, vertices.size_bytes(), vertices.data(), GL_STATIC_DRAW);

//   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, data.ibo);
//   glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size_bytes(), indices.data(), GL_STATIC_DRAW);

//   glEnableVertexAttribArray(0);           /* position */
//   glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex), (void *)0);

//   glEnableVertexAttribArray(1);           /* color */
//   glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex),
//                         (void *)offsetof(Mesh::Vertex, color));

//   glEnableVertexAttribArray(2);           /* normal */
//   glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex),
//                         (void *)offsetof(Mesh::Vertex, normal));

//   glEnableVertexAttribArray(3);           /* uv */
//   glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, sizeof(Mesh::Vertex),
//                         (void *)offsetof(Mesh::Vertex, uv));

//   glBindVertexArray(0);
//   data.index_count = indices.size();
//   Mesh::Handle handle = meshes.size();
//   meshes.push_back(data);
//   return handle;
// }

} /* namespace Engine */

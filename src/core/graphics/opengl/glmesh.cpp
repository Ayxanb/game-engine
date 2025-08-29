#include <core/graphics/opengl/glmesh.hpp>
#include <core/graphics/opengl/opengl.hpp>

#include <memory>

namespace Engine {

Mesh::Handle MeshManager::OpenGL::addMesh(Mesh &mesh) {
  auto mesh_data = std::make_unique<MeshInfo::OpenGL>();
  
  auto vertices = mesh.getVerticesView();
  auto indices = mesh.getIndicesView();

  mesh_data->cpu_vertices.assign(vertices.begin(), vertices.end());
  mesh_data->cpu_indices.assign(indices.begin(), indices.end());

  Mesh::Handle handle = meshes.size();
  meshes.push_back(std::move(mesh_data));
  return handle;
}

void MeshManager::OpenGL::uploadPending() {
  for (std::unique_ptr<MeshInfo> &mesh_data : meshes) {
    auto gl_mesh_data = static_cast<MeshInfo::OpenGL *>(mesh_data.get());
    
    glGenVertexArrays(1, &gl_mesh_data->vao);
    glGenBuffers(1, &gl_mesh_data->vbo);
    glGenBuffers(1, &gl_mesh_data->ibo);

    glBindVertexArray(gl_mesh_data->vao);
    glBindBuffer(GL_ARRAY_BUFFER, gl_mesh_data->vbo);
    glBufferData(
      GL_ARRAY_BUFFER,
      mesh_data->cpu_vertices.size() * sizeof(Mesh::Vertex),
      mesh_data->cpu_vertices.data(),
      GL_STATIC_DRAW
    );

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, gl_mesh_data->ibo);
    glBufferData(
      GL_ELEMENT_ARRAY_BUFFER,
      mesh_data->cpu_indices.size() * sizeof(Mesh::Index),
      mesh_data->cpu_indices.data(),
      GL_STATIC_DRAW
    );

    glEnableVertexAttribArray(0);           /* position */
    glVertexAttribPointer(
      0, 
      3, 
      GL_FLOAT, 
      GL_FALSE, 
      sizeof(Mesh::Vertex), 
      (void *)0
    );

    glEnableVertexAttribArray(1);           /* color */
    glVertexAttribPointer(
      1, 
      3, 
      GL_FLOAT, 
      GL_FALSE, 
      sizeof(Mesh::Vertex),
      (void *)offsetof(Mesh::Vertex, color));

    glEnableVertexAttribArray(2);           /* normal */
    glVertexAttribPointer(
      2,
      3, 
      GL_FLOAT, 
      GL_FALSE, 
      sizeof(Mesh::Vertex),
      (void *)offsetof(Mesh::Vertex, normal)
    );

    glEnableVertexAttribArray(3);           /* uv */
    glVertexAttribPointer(
      3, 
      2, 
      GL_FLOAT, 
      GL_FALSE, 
      sizeof(Mesh::Vertex),
      (void *)offsetof(Mesh::Vertex, uv)
    );
  }
}

} /* namespace Engine */

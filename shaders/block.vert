#version 460

/* Mesh::Vertex Attributes */
layout (location = 0) in vec3 in_position;
layout (location = 1) in vec3 in_color;
layout (location = 2) in vec2 in_texture_coord;
layout (location = 3) in vec3 in_normal;

/* Output to Fragment Shader */
layout(location = 0) out vec3 frag_color;
layout(location = 1) out vec2 frag_texture_coord;
layout(location = 2) out vec3 frag_normal;
layout(location = 3) out vec3 frag_world_pos;

/* Uniforms */
layout(std140, set = 0, binding = 0) uniform CameraUBO {
  mat4 proj_view;
};

void main() {
  frag_world_pos       = in_position;
  frag_color           = in_color;
  frag_texture_coord   = in_texture_coord;
  frag_normal          = in_normal;
  gl_Position          = proj_view * vec4(in_position, 1.0);

#ifdef VULKAN
  gl_Position.y *= -1;
#endif
}
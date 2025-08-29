#version 460

layout(location = 0) in vec3 fragColor;
layout(location = 0) out vec4 outColor;

layout(std140, binding = 0) uniform CameraUBO {
  mat4 proj_view;
};

void main() {
  outColor = vec4(fragColor, 1.0);
  // vec4(proj_view[0][0] / 10.0, 0.0, 0.0, 1.0);
}

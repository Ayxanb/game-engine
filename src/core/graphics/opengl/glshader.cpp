#include <core/graphics/opengl/glshader.hpp>
#include <core/logging.hpp>

namespace Engine {

Pipeline::OpenGL::~OpenGL() noexcept {
  if (program != 0) {
    glDeleteProgram(program);
    program = 0;
  }
  uniform_locations.clear();
}

/* Compile and link pipelines */
bool Pipeline::OpenGL::create(ShaderStages stages_in) {
  stages = stages_in;

  GLuint vertex_shader = 0;
  GLuint fragment_shader = 0;

  if (!stages.vertex.empty()) {
    vertex_shader = compileShader(GL_VERTEX_SHADER, stages.vertex);
    if (vertex_shader == 0) return false;
  }

  if (!stages.fragment.empty()) {
    fragment_shader = compileShader(GL_FRAGMENT_SHADER, stages.fragment);
    if (fragment_shader == 0) return false;
  }

  /* Create program and attach pipelines */
  program = glCreateProgram();
  if (vertex_shader != 0) glAttachShader(program, vertex_shader);
  if (fragment_shader != 0) glAttachShader(program, fragment_shader);

  glLinkProgram(program);

  /* Check linking status */
  GLint success = 0;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    char info_log[512];
    glGetProgramInfoLog(program, 512, nullptr, info_log);
    LOG_ERROR("[Pipeline::OpenGL] Link error: {}", info_log);
    return false;
  }

  /* Shaders can be deleted after linking */
  if (vertex_shader != 0) glDeleteShader(vertex_shader);
  if (fragment_shader != 0) glDeleteShader(fragment_shader);

  LOG_INFO("[Pipeline::OpenGL] Program created successfully");
  return true;
}

/* Bind program for use */
void Pipeline::OpenGL::bind(uint32_t slot) {
  (void)slot; // slot not used in OpenGL
  glUseProgram(program);
}

/* Compile individual shader */
GLuint Pipeline::OpenGL::compileShader(GLenum type, File::Path path) {
  GLuint shader = glCreateShader(type);

  std::string source;
  if (!File::readContent(path, source)) {
    LOG_ERROR("[Pipeline::OpenGL] Failed to read shader: {}", path.string());
    return 0;
  }

  const char *src = source.data();
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);

  GLint success = 0;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char info_log[512];
    glGetShaderInfoLog(shader, 512, nullptr, info_log);
    LOG_ERROR("[Pipeline::OpenGL] Compile error: {}", info_log);
    return 0;
  }

  return shader;
}

GLint Pipeline::OpenGL::getUniformLocation(std::string name) {
  auto it = uniform_locations.find(name);
  if (it != uniform_locations.end()) return it->second;

  GLint loc = glGetUniformLocation(program, name.c_str());
  uniform_locations[name] = loc;
  return loc;
}

} /* namespace Engine */

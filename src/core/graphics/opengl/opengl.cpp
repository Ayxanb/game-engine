#include <core/logging.hpp>
#include <core/graphics/opengl/opengl.hpp>
#include <core/graphics/opengl/glshader.hpp>

#include <glad/glad.h>

namespace Engine {

bool GraphicsAPI::OpenGL::init(Engine::Window *window) {
  window->makeContextCurrent();

  if (!gladLoadGL()) {
    LOG_ERROR("[GraphicsAPI::OpenGL] failed to initialize GLAD library");
    return false;
  }

  glEnable(GL_DEBUG_OUTPUT);                                 /* Enable debug output */
  glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);                     /* Immediate debug callbacks */
  glDebugMessageCallback(
    [](GLenum, GLenum type, GLuint, GLenum severity, GLsizei, const GLchar *message, const void *) {
      (void) type;
      (void) severity;
      (void) message;
      LOG_ERROR(
        "[GraphicsAPI::OpenGL-Debug]:\n"
        "\tType: {}\n"
        "\tseverity: {}\n"
        "\tMessage: {}\n", 
        type,
        severity,
        message
      );
    },
    nullptr
  );

  glEnable(GL_FRAMEBUFFER_SRGB);                             /* Enable sRGB frame_buffers */
  glEnable(GL_DEPTH_TEST);                                   /* Enable depth testing */

  // Ensure a valid OpenGL context is current before calling this
  GLint major = 0, minor = 0;
  glGetIntegerv(GL_MAJOR_VERSION, &major);
  glGetIntegerv(GL_MINOR_VERSION, &minor);

  GLint max_texture_units = 0;
  glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, &max_texture_units);

  GLint max_vertex_attribs = 0;
  glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &max_vertex_attribs);

  GLint max_uniform_blocks = 0;
  glGetIntegerv(GL_MAX_VERTEX_UNIFORM_BLOCKS, &max_uniform_blocks);

  GLint max_color_attachments = 0;
  glGetIntegerv(GL_MAX_COLOR_ATTACHMENTS, &max_color_attachments);

  GLint max_draw_buffers = 0;
  glGetIntegerv(GL_MAX_DRAW_BUFFERS, &max_draw_buffers);

  // Get extensions safely for core profile
  GLint nextensions = 0;
  glGetIntegerv(GL_NUM_EXTENSIONS, &nextensions);

  LOG_INFO(
    "[GraphicsAPI::OpenGL] initialized:\n"
    "\tVendor:                         {:>40}\n"
    "\tRenderer:                       {:>40}\n"
    "\tVersion:                        {:>31} (GL {}.{})\n"
    "\tGLSL Version:                   {:>40}\n"
    "\tNumber of Extensions:           {:>40}\n"
    "\tMax Texture Units:              {:>40}\n"
    "\tMax Vertex Attributes:          {:>40}\n"
    "\tMax Uniform Blocks (Vertex):    {:>40}\n"
    "\tMax Color Attachments:          {:>40}\n"
    "\tMax Draw Buffers:               {:>40}",
    (const char*)glGetString(GL_VENDOR),
    (const char*)glGetString(GL_RENDERER),
    (const char*)glGetString(GL_VERSION),
    major,
    minor,
    (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION),
    nextensions,
    max_texture_units,
    max_vertex_attribs,
    max_uniform_blocks,
    max_color_attachments,
    max_draw_buffers
  );

  return true;
}

void GraphicsAPI::OpenGL::enableVsync() {

};

bool GraphicsAPI::OpenGL::beginFrame() {
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  return true;
};

bool GraphicsAPI::OpenGL::endFrame(Window *window) {
  glfwSwapBuffers(window->getNativeHandle());
  return true;
};

bool GraphicsAPI::OpenGL::drawIndexed(DrawInfo &) {
  return true;
};

bool GraphicsAPI::OpenGL::updateUBO(UniformBufferType, const void *, size_t, size_t) {
  return true;
}

} /* namespace Engine */

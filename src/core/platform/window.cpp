#include <core/logging.hpp>
#include <core/platform/window.hpp>

#include <cstdint>
#include <glfw/glfw3.h>

using namespace std::chrono_literals;

namespace Engine {

bool Window::init(Config::Window &config) {
  /* Set window hints */
  glfwWindowHint(GLFW_DECORATED, !config.borderless);
  glfwWindowHint(GLFW_RESIZABLE, config.resizable);

  tick_interval = config.tick_interval;

  /* Determine window size */
  GLFWmonitor *primary = glfwGetPrimaryMonitor();
  const GLFWvidmode *mode = glfwGetVideoMode(primary);

  uint32_t width  = config.fullscreen ? mode->width  : config.width;
  uint32_t height = config.fullscreen ? mode->height : config.height;

  /* Create GLFW window */
  handle = glfwCreateWindow(width, height, config.title.data(),
                            config.fullscreen ? primary : nullptr, nullptr);

  if (!handle) {
    LOG_ERROR("Failed to create GLFW window");
    return false;
  }

  /* Center window if required */
  if (config.centered && !config.fullscreen) {
    glfwSetWindowPos(handle,
                     static_cast<int>(mode->width  / 2 - config.width  / 2),
                     static_cast<int>(mode->height / 2 - config.height / 2));
  }

  /* Set user pointer for input callbacks */
  glfwSetWindowUserPointer(handle, &input_manager);

  /* Framebuffer and window size callbacks (currently no-op) */
  glfwSetFramebufferSizeCallback(handle, [](GLFWwindow * /*handle*/, int /*w*/, int /*h*/) {});
  glfwSetWindowSizeCallback(handle, [](GLFWwindow * /*handle*/, int /*w*/, int /*h*/) {});

  /* Key input callback */
  glfwSetKeyCallback(handle, [](GLFWwindow *handle, int key, int /*scancode*/, int action, int /*mods*/) {
    auto *input = static_cast<InputManager *>(glfwGetWindowUserPointer(handle));
    if (input) input->onKey(key, action);
  });

  /* Mouse button callback */
  glfwSetMouseButtonCallback(handle, [](GLFWwindow *handle, int button, int action, int /*mods*/) {
    auto *input = static_cast<InputManager *>(glfwGetWindowUserPointer(handle));
    if (input) input->onMouseButton(button, action);
  });

  /* Cursor position callback */
  glfwSetCursorPosCallback(handle, [](GLFWwindow *handle, double x, double y) {
    auto *input = static_cast<InputManager *>(glfwGetWindowUserPointer(handle));
    if (input) input->onMouseMove(x, y);
  });

  /* Scroll callback */
  glfwSetScrollCallback(handle, [](GLFWwindow *handle, double x, double y) {
    auto *input = static_cast<InputManager *>(glfwGetWindowUserPointer(handle));
    if (input) input->onScroll(x, y);
  });

  /* Window focus and close callbacks (currently no-op) */
  glfwSetWindowFocusCallback(handle, [](GLFWwindow * /*handle*/, int /*focused*/) {});
  glfwSetWindowCloseCallback(handle, [](GLFWwindow * /*handle*/) {});

  /* Log window initialization details */
  LOG_INFO("[Window]: Initialized successfully:"
           "\n\ttitle:        `{}`"
           "\n\tsize:         ({} x {})"
           "\n\tmode:         {}"
           "\n\tborderless:   {}"
           "\n\tresizable:    {}",
           config.title, width, height,
           config.fullscreen ? "fullscreen" : "windowed",
           config.borderless ? "\33[1;32myes\33[0m" : "\33[1;31mno\33[0m",
           config.resizable ? "\33[1;32myes\33[0m" : "\33[1;31mno\33[0m");

  return true;
}

void Window::destroy() const noexcept {
  if (handle) {
    glfwDestroyWindow(handle);
  }
  LOG_INFO("[Window]: destroyed");
}

} /* namespace Engine */
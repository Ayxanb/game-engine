#pragma once

#include <cstdint>
#include <string_view>
#include <chrono>

#include <glfw/glfw3.h>
#include <glm/vec2.hpp>

#include <core/timer.hpp>
#include <core/config.hpp>
#include <core/platform/input_manager.hpp>

namespace Engine {

class Context;

/**
 * @class Window
 * @brief Wrapper around GLFWwindow providing convenience methods for window management and input handling.
 */
class Window {
public:
  using Title = std::string_view;    /**< Window title type */

  /** Default constructor */
  Window() noexcept = default;

  /** Destructor automatically destroys the GLFW window */
  ~Window() noexcept { destroy(); }

  /**
   * @brief Initialize window with configuration
   * @param config Window configuration
   * @return true if initialization succeeded
   */
  [[nodiscard]] bool init(Config::Window &config);

  /* --- GLFW convenience methods --- */

  /** Poll pending window events */
  inline void pollEvents() const noexcept { glfwPollEvents(); }

  /** Swap front and back buffers */
  inline void presentFrame() const noexcept { glfwSwapBuffers(handle); }

  /** Make this window's context current */
  inline void makeContextCurrent() const noexcept { glfwMakeContextCurrent(handle); }

  /** Check if the window should close */
  [[nodiscard]] inline bool shouldClose() const noexcept { 
    return handle && glfwWindowShouldClose(handle); 
  }

  /** Get current window size */
  [[nodiscard]] inline glm::uvec2 getSize() const noexcept {
    int w = 0, h = 0;
    if (handle) glfwGetWindowSize(handle, &w, &h);
    return glm::uvec2(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
  }

  /** Get window aspect ratio (width / height) */
  [[nodiscard]] inline float getAspectRatio() const noexcept {
    glm::uvec2 size = getSize();
    return size.y ? static_cast<float>(size.x) / static_cast<float>(size.y) : 0.0f;
  }

  /** Get framebuffer size */
  [[nodiscard]] inline glm::uvec2 getFrameBufferSize() const noexcept {
    int w = 0, h = 0;
    if (handle) glfwGetFramebufferSize(handle, &w, &h);
    return glm::uvec2(static_cast<uint32_t>(w), static_cast<uint32_t>(h));
  }

  /** Set the window title */
  inline void setTitle(Title title) const noexcept {
    if (handle) glfwSetWindowTitle(handle, title.data());
  }

  /** Get the window title */
  [[nodiscard]] inline Title getTitle() const noexcept {
    return handle ? Title{glfwGetWindowTitle(handle)} : "";
  }

  /** Set mouse cursor position */
  inline void setMousePosition(glm::uvec2 pos) const noexcept {
    if (handle) glfwSetCursorPos(handle, pos.x, pos.y);
  }

  /** Get mouse cursor position */
  [[nodiscard]] inline glm::uvec2 getCursorPos() const noexcept {
    double x = 0, y = 0;
    if (handle) glfwGetCursorPos(handle, &x, &y);
    return glm::uvec2(static_cast<uint32_t>(x), static_cast<uint32_t>(y));
  }

  /** Get required instance extensions for Vulkan/graphics API */
  [[nodiscard]] inline const char **getRequiredInstanceExtensions(uint32_t *count) const noexcept {
    return glfwGetRequiredInstanceExtensions(count);
  }

  /** Access native GLFW handle */
  [[nodiscard]] inline GLFWwindow *getNativeHandle() const noexcept { return handle; }

  /** Get tick interval (time between updates) */
  [[nodiscard]] inline std::chrono::milliseconds getTickInterval() const noexcept { return tick_interval; }

  /* --- Input convenience methods --- */

  __forceinline bool isKeyPressed(uint32_t key) const noexcept { return input_manager.isKeyPressed(key); }
  __forceinline bool isKeyHeld(uint32_t key)    const noexcept { return input_manager.isKeyHeld(key); }
  __forceinline bool isKeyReleased(uint32_t key)const noexcept { return input_manager.isKeyReleased(key); }

  __forceinline bool isMouseButtonPressed(uint32_t button) const noexcept { return input_manager.isMouseButtonPressed(button); }
  __forceinline bool isMouseButtonHeld(uint32_t button)    const noexcept { return input_manager.isMouseButtonHeld(button); }
  __forceinline bool isMouseButtonReleased(uint32_t button)const noexcept { return input_manager.isMouseButtonReleased(button); }

  __forceinline glm::uvec2 getScrollDelta() const noexcept { return input_manager.getScrollDelta(); }
  __forceinline glm::uvec2 getMousePosition() const noexcept { return input_manager.getMousePosition(); }
  __forceinline glm::uvec2 getMouseDelta() const noexcept { return input_manager.getMouseDelta(); }

  /** Update input manager states (keys, mouse, scroll) */
  void update() noexcept { input_manager.update(); }

private:
  /** Destroy GLFW window */
  void destroy() const noexcept;

  InputManager input_manager;       /**< Input manager for keyboard/mouse */
  GLFWwindow *handle = nullptr;     /**< Native GLFW window handle */
  std::chrono::milliseconds tick_interval; /**< Tick interval for updates */
};

} /* namespace Engine */

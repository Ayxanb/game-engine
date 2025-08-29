#pragma once
#include <array>
#include <cstdint>
#include <span>

#include <glfw/glfw3.h>
#include <glm/vec2.hpp>

namespace Engine {

class InputManager {
public:
  enum class KeyState {
      Up,
      Pressed,
      Held,
      Released,
  };

  static constexpr uint32_t MAX_KEYS = GLFW_KEY_LAST + 1;
  static constexpr uint32_t MAX_BUTTONS = GLFW_MOUSE_BUTTON_LAST + 1;

  InputManager() noexcept :
    keys(),
    mouse(),
    mouse_pos(0.0f, 0.0f),
    last_mouse_pos(0.0f, 0.0f),
    mouse_delta(0.0f, 0.0f),
    scroll_delta(0.0f, 0.0f) {}

  ~InputManager() noexcept = default;
  
  void update() {
    mouse_delta = glm::uvec2(0.0f, 0.0f);
    scroll_delta = glm::uvec2(0.0f, 0.0f);
    updateState(keys);
    updateState(mouse);
  }

  void onKey(uint32_t key, uint32_t action) {
    if (key >= MAX_KEYS) return;
    switch (action) {
      case GLFW_PRESS:   keys[key] = KeyState::Pressed; break;
      case GLFW_RELEASE: keys[key] = KeyState::Released; break;
      case GLFW_REPEAT:  break; // optional: could count as Held
    }
  }

  void onMouseButton(uint32_t button, uint32_t action) {
    if (button >= MAX_BUTTONS) return;
    switch (action) {
      case GLFW_PRESS:   mouse[button] = KeyState::Pressed;  break;
      case GLFW_RELEASE: mouse[button] = KeyState::Released; break;
    }
  }

  void onMouseMove(double x, double y) {
    last_mouse_pos = mouse_pos;
    mouse_pos = glm::uvec2(x, y);
    mouse_delta = glm::uvec2(x - last_mouse_pos.x, y - last_mouse_pos.y);
  }

  inline void onScroll(double dx, double dy) {
    scroll_delta = {dx, dy};
  }

  inline bool isKeyPressed(uint32_t key) const {
    return key < MAX_KEYS && keys[key] == KeyState::Pressed;
  }
  inline bool isKeyHeld(uint32_t key) const {
    return key < MAX_KEYS && (keys[key] == KeyState::Held || keys[key] == KeyState::Pressed);
  }
  inline bool isKeyReleased(uint32_t key) const {
    return key < MAX_KEYS && keys[key] == KeyState::Released;
  }

  // Mouse queries
  inline bool isMouseButtonPressed(uint32_t button) const {
    return button < MAX_BUTTONS && mouse[button] == KeyState::Pressed;
  }
  inline bool isMouseButtonHeld(uint32_t button) const {
    return button < MAX_BUTTONS && (mouse[button] == KeyState::Held || mouse[button] == KeyState::Pressed);
  }
  inline bool isMouseButtonReleased(uint32_t button) const {
    return button < MAX_BUTTONS && mouse[button] == KeyState::Released;
  }

  inline glm::uvec2 getMousePosition() const { return mouse_pos; }
  inline glm::uvec2 getMouseDelta()    const { return mouse_delta; }
  inline glm::uvec2 getScrollDelta()   const { return scroll_delta; }

private:
  std::array<KeyState, MAX_KEYS> keys;
  std::array<KeyState, MAX_BUTTONS> mouse;

  glm::uvec2 mouse_pos;
  glm::uvec2 last_mouse_pos;
  glm::uvec2 mouse_delta;
  glm::uvec2 scroll_delta;

  void updateState(std::span<KeyState> states) {
    for (KeyState &state : states)
      if (state == KeyState::Pressed)  state = KeyState::Held;
      else if (state == KeyState::Released) state = KeyState::Up;
  }
};

} /* namespace Engine */
#pragma once
#include "camera.hpp"
#include <core/graphics/camera/camera.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Engine {

class Orthographic : public Camera {
  float left = 0.0f;
  float right = 0.0f;
  float bottom = 0.0f;
  float top = 0.0f;

public:
Orthographic(
    float _left,
    float _right,
    float _bottom,
    float _top,
    float _near,
    float _far) noexcept :
    Camera(
      glm::vec3(0.0f, 0.0f, 5.0f),   // position in front of scene
      glm::vec3(0.0f, 0.0f, -1.0f),  // forward orientation
      _near,
      _far
    ),
    left(_left),
    right(_right),
    bottom(_bottom),
    top(_top) {}
  ~Orthographic() noexcept = default;

  inline glm::mat4 getProjectionMatrix() const override {
    return glm::ortho(left, right, bottom, top, near, far);
  }

  inline glm::mat4 getViewMatrix() const override {
    return glm::lookAt(position, position + orientation, Camera::UP);
  }
};

}; /* namespace Engine */
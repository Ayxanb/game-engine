#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

namespace Engine{

/* A base camera class that can be extended as needed */
class Camera {
protected:
  glm::vec3 position;
  glm::vec3 orientation;

  float near;
  float far;

public:
  static constexpr inline auto UP = glm::vec3(0.0f, 1.0f, 0.0f);

  Camera(
    glm::vec3 _position,
    glm::vec3 _orientation,
    float _near,
    float _far) noexcept :
    position(_position),
    orientation(_orientation),
    near(_near),
    far(_far) {}

  /* this is what sent to GPU, if changed pipelines must be updated too. */
  struct UBOLayout {
    glm::mat4 proj_view;
  };

  Camera() noexcept = default;
  virtual ~Camera() noexcept = default;

  void update() {
    
  }

  inline void setPosition(glm::vec3 _position) { position = _position; }
  inline void setOrientation(glm::vec3 _orientation) { orientation = _orientation; }
  inline void setForward(glm::vec3 _forward) { orientation = _forward; }

  inline glm::vec3 getPosition() const { return position; }
  inline glm::vec3 getOrientation() const { return orientation; }

  virtual glm::mat4 getProjectionMatrix() const = 0;
  virtual glm::mat4 getViewMatrix() const = 0;

};

};
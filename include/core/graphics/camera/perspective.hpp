#include <core/graphics/camera/camera.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace Engine {

class Perspective : public Camera {
  float fov = 0.0f;
  float aspect = 0.0f;

public:
  Perspective(float _fov, float _aspect, float _near, float _far) noexcept :
      Camera(
        glm::vec3(0.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, -1.0f),
        _near,
        _far),
      fov(_fov),
      aspect(_aspect) {};
  ~Perspective() noexcept = default;


  glm::mat4 getProjectionMatrix() const override {
    return glm::perspective(glm::radians(fov), aspect, near, far);
  }

  glm::mat4 getViewMatrix() const override {
    return glm::lookAt(position, position + orientation, Camera::UP);
  }
};

}; /* namespace Engine */
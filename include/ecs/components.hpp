#pragma once

#include <glm/glm.hpp>

namespace Engine::ECS::Component {

using Position = glm::vec3;
using Velocity = glm::vec3;
using Acceletaion = glm::vec3;
using Rotation = glm::vec3;

struct Material  {
  uint32_t handle;  /* shader index */
};

struct Mesh {
  uint32_t handle;  /* mesh index */
};

}; /* namespace Engine::ECS */
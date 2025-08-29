#pragma once

#include <ecs/ecs.hpp>

namespace Engine::ECS {

class System {
protected:
  Registry &manager;

public:
  explicit System(Registry & _manager) noexcept : manager(_manager) {};
  virtual ~System() = default;
  virtual void tick(float) = 0;

};




}; /* namespace Engine::ECS::System */
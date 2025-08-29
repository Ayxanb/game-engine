#pragma once

namespace Engine {
class Instance;
class Window;

struct Application {
  
  Application(Instance &_instance) noexcept : instance(_instance) {};
  virtual ~Application() noexcept = default;
  
  virtual bool onInit() = 0;
  virtual bool onTick(float) = 0;
  virtual bool onUpdate() = 0;
  virtual bool onRender() = 0;

protected:
  Engine::Instance &instance;

};

};

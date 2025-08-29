#include <engine.hpp>
#include <core/logging.hpp>
#include <core/application.hpp>
#include <core/graphics/camera/perspective.hpp>
#include <core/graphics/camera/orthographic.hpp>
#include <memory>

namespace Engine {

bool Engine::Instance::init(Config &config) {
  Engine::Logger::init(config.logger);

  /* Initialize GLFW */
  if (!glfwInit()) {
    LOG_ERROR("[Engine]: Failed to initialize GLFW library");
    return false;
  }

  /* Apply graphics backend-specific window hints */
  GraphicsAPI::applyWindowHints(config.renderer.backend);

  /* Create and initialize main window */
  window = std::make_unique<Window>();
  if (!window->init(config.window)) {
    return false;
  }

  /* Create and initialize renderer */
  /* Renderer::create(window.get(), config.renderer.backend) */
  renderer = std::make_unique<Renderer>(window.get());
  if (!renderer->init(config.renderer)) {
    return false;
  }

  /* Set up default perspective camera */
  camera = std::make_unique<Perspective>(
    config.camera.fov,
    window->getAspectRatio(),
    config.camera.near,
    config.camera.far
  );

  return true;
}

void Engine::Instance::run(Application &app) {
  /* Application-specific initialization */
  if (!app.onInit()) {
    return;
  }

  Timer timer;
  auto tick_interval = window->getTickInterval();

  LOG_INFO("[Engine]: entering loop...");

  /* Main loop */
  while (!window->shouldClose()) {
    window->pollEvents();

    /* Application update */
    if (!app.onUpdate()) {
      return;
    }

    /* Call tick if interval passed */
    if (timer.shouldTick(tick_interval)) {
      app.onTick(timer.deltaTime());
    }

    /* Begin rendering */
    renderer->beginFrame();

    /* Update camera UBO (projection * view) */
    if (!renderer->updateUniformBuffer(
      UniformBufferType::Camera,
      camera->getProjectionMatrix() * camera->getViewMatrix()
    )) {
      LOG_ERROR("[Engine]: Failed to update camera UBO");
      return;
    }

    /* Application rendering */
    if (!app.onRender()) {
      return;
    }
    
    /* Finalize frame */
    renderer->endFrame();
    
    window->update();
  }

  LOG_INFO("[Engine]: exiting loop...");
}

} /* namespace Engine */
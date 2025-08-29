#pragma once

#include <memory>

#include <core/timer.hpp>
#include <core/config.hpp>
#include <core/logging.hpp>
#include <core/platform/window.hpp>
#include <core/graphics/renderer.hpp>
#include <core/graphics/camera/camera.hpp>

namespace Engine {

struct Application;

/**
 * @class Instance
 * @brief Core engine instance that manages the window, renderer, and camera.
 *
 * This class provides initialization and lifecycle management
 * of the engine runtime. It is responsible for:
 * - Creating and owning the main application window
 * - Managing the rendering backend
 * - Managing the active camera
 * - Running the main loop
 */
class Instance {
  std::unique_ptr<Window> window;   /**< Main application window */
  std::unique_ptr<Renderer> renderer; /**< Rendering backend */
  std::unique_ptr<Camera> camera;   /**< Active camera */

public:
  Instance() = default;
  ~Instance() = default;

  Instance(const Instance &) = delete;
  Instance &operator=(const Instance &) = delete;

  /**
   * @brief Access the renderer
   * @return Reference to the renderer
   */
  inline Renderer &getRenderer() { return *renderer; }

  /**
   * @brief Access the window
   * @return Reference to the window
   */
  inline Window &getWindow() { return *window; }

  /**
   * @brief Access the camera
   * @return Reference to the camera
   */
  inline Camera &getCamera() { return *camera; }

  /**
   * @brief Initialize the engine with configuration
   * @param config Engine configuration
   * @return true if initialization was successful, false otherwise
   */
  [[nodiscard]] bool init(Config &);

  /**
   * @brief Run the main loop with the given application
   * @param app Application instance implementing game logic
   */
  void run(Application &);
};

} /* namespace Engine */
#pragma once

#include "graphics/shader.hpp"
#include <chrono>
#include <string>
#include <vector>
#include <cstdint>

#include <core/timer.hpp>
#include <util/file_utils.hpp>
#include <core/graphics/shader.hpp>
#include <core/graphics/graphics_api.hpp>

using namespace std::chrono_literals;

namespace Engine {

struct Config {
  struct Logger;
  struct Window;
  struct Renderer;
  struct Camera;

  /* essential configs for engine initialization */
  Config::Logger &logger;
  Config::Window &window;
  Config::Renderer &renderer;
  Config::Camera &camera;
};

struct Config::Logger {
  /* leaving this empty will make Logger use std::clog */
  File::Path file_path {};
};

struct Config::Window {
  std::string title;
  uint32_t width;
  uint32_t height;

  bool fullscreen = false;
  bool borderless = false;
  bool resizable = true;
  bool centered = false;

  /* determines how fast should game logic update */
  std::chrono::milliseconds tick_interval = 16ms;
};

struct Config::Renderer {
  GraphicsAPI::Backend backend;
  std::vector<Pipeline::ShaderStages> shader_paths;

  bool vsync = false;
  uint32_t frames_in_flight = 3;
};

struct Config::Camera {
  float near;
  float far;

  /* for perspective projection */
  float fov = 0.0f;

  /* for orthographic projection */
  float left = 0.0f;
  float right = 0.0f;
  float bottom = 0.0f;
  float top = 0.0f;

};

} /* namespace Engine */
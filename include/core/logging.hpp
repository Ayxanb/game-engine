#pragma once

#include <memory>
#include <string_view>
#include <iostream>
#include <fstream>
#include <mutex>
#include <format>
#include <chrono>
#include <cassert>

#include <core/config.hpp>

#ifdef DEBUG
  #define LOG_INFO(fmt, ...)   Engine::Logger::instance().info(fmt __VA_OPT__(,) __VA_ARGS__)
  #define LOG_WARN(fmt, ...)   Engine::Logger::instance().warn(fmt __VA_OPT__(,) __VA_ARGS__)
  #define LOG_ERROR(fmt, ...)  Engine::Logger::instance().error(fmt __VA_OPT__(,) __VA_ARGS__)
  #define LOG_DEBUG(fmt, ...)  Engine::Logger::instance().debug(fmt __VA_OPT__(,) __VA_ARGS__)
#else
  #define LOG_INFO(fmt, ...) 
  #define LOG_WARN(fmt, ...) 
  #define LOG_ERROR(fmt, ...)
  #define LOG_DEBUG(fmt, ...)
#endif

namespace Engine {

class Logger {
public:
  enum class Level { Debug, Info, Warning, Error };

  Logger() noexcept = default;
  ~Logger() noexcept = default;

  static void init(Config::Logger &config) {
    logger = std::make_unique<Logger>();
    logger->initialize(config);
  }

  [[nodiscard]] static Logger &instance() {
    assert(logger && "Logger accessed before initialization");
    return *logger;
  }

  template <typename... Args>
  void log(Level level, std::string_view fmt, Args&&... args) {
    assert(initialized && "Logger used before initialization");

    std::scoped_lock lock(log_mutex);
    std::string message = std::vformat(fmt, std::make_format_args(args...));

    auto now = std::chrono::system_clock::now();

    std::string output = std::format(
      "[{:%Y-%m-%d}] [{}{}{}] - {}",
      now,
      (use_colors && !logging_to_file) ? levelToColor(level) : "",
      levelToString(level),
      (use_colors && !logging_to_file) ? "\033[0m" : "",
      message
    );

    /* why? */
    (level == Level::Error ? std::cerr : (*log_stream)) << output << std::endl;
  }

  template <typename... Args> inline void info (std::string_view f, Args&&... a) { log(Level::Info,    f, std::forward<Args>(a)...); }
  template <typename... Args> inline void warn (std::string_view f, Args&&... a) { log(Level::Warning, f, std::forward<Args>(a)...); }
  template <typename... Args> inline void error(std::string_view f, Args&&... a) { log(Level::Error,   f, std::forward<Args>(a)...); }
  template <typename... Args> inline void debug(std::string_view f, Args&&... a) { log(Level::Debug,   f, std::forward<Args>(a)...); }

private:
  static std::unique_ptr<Logger> logger;

  std::mutex log_mutex;
  bool initialized       = false;
  bool logging_to_file   = false;
  bool use_colors        = true;

  std::ofstream log_file;
  std::ostream *log_stream = &std::clog;

  void initialize(Config::Logger &config) {
    log_file.open(config.file_path);
      
    if ((logging_to_file = log_file.is_open()))
      log_stream = &log_file;

    use_colors = !logging_to_file;
    initialized = true;
  }

  [[nodiscard]] constexpr std::string_view levelToString(Level level) const noexcept {
    switch (level) {
    case Level::Debug:   return "DEBUG";
    case Level::Info:    return "INFO";
    case Level::Warning: return "WARN";
    case Level::Error:   return "ERROR";
    }
    return "UNKNOWN";
  }

  [[nodiscard]] std::string levelToColor(Level level) const {
    switch (level) {
    case Level::Debug:   return "\033[36m"; /* Cyan */
    case Level::Info:    return "\033[32m"; /* Green */
    case Level::Warning: return "\033[33m"; /* Yellow */
    case Level::Error:   return "\033[31m"; /* Red */
    }
    return "";
  }
};

inline std::unique_ptr<Logger> Logger::logger = nullptr;

} /* namespace Engine */

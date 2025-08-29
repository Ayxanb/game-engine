#pragma once
#include <chrono>

namespace Engine {

class Timer {
  using clock       = std::chrono::high_resolution_clock;
  using time_point  = std::chrono::time_point<clock>;

public:
  Timer() noexcept { reset(); }
  ~Timer() noexcept = default;

  /* Resets both tick and frame times */
  void reset() noexcept {
    auto now = clock::now();
    last_tick_time  = now;
    last_frame_time = now;
  }

  /* Returns delta time in seconds as float */
  float deltaTime() noexcept {
    time_point now   = clock::now();
    float delta = std::chrono::duration<float>(now - std::exchange(last_frame_time, now)).count();
    return delta;
  }

  /* Checks if a fixed interval has passed (e.g., for 60 Hz logic updates) */
  bool shouldTick(std::chrono::milliseconds interval) noexcept {
    if (clock::now() - last_tick_time >= interval) {
      last_tick_time += interval;
      return true;
    }
    return false;
  }

  /* Get current time point */
  static inline time_point now() noexcept {
    return clock::now();
  }

private:
  time_point last_tick_time;
  time_point last_frame_time;
};

} /* namespace Engine */

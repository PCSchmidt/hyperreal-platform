// Fixed-timestep simulation clock. The simulation never reads wall-clock time;
// time compression changes only how many ticks are executed per wall second.
#pragma once
#include <cstdint>

namespace hr {

class FixedClock {
 public:
  // start_utc_ms: scenario start (Unix epoch ms). tick_ms: simulated milliseconds per tick (> 0).
  constexpr FixedClock(std::int64_t start_utc_ms, std::int64_t tick_ms) noexcept
      : start_(start_utc_ms), tick_ms_(tick_ms) {}

  constexpr std::uint64_t tick() const noexcept { return tick_; }
  constexpr void advance(std::uint64_t n = 1) noexcept { tick_ += n; }
  constexpr std::int64_t tick_ms() const noexcept { return tick_ms_; }
  constexpr double dt_seconds() const noexcept { return static_cast<double>(tick_ms_) / 1000.0; }
  constexpr std::int64_t utc_ms() const noexcept { return utc_ms_at(tick_); }
  constexpr std::int64_t utc_ms_at(std::uint64_t t) const noexcept {
    return start_ + static_cast<std::int64_t>(t) * tick_ms_;
  }

 private:
  std::int64_t start_;
  std::int64_t tick_ms_;
  std::uint64_t tick_ = 0;
};

}  // namespace hr

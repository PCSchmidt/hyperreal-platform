// Counter-based deterministic RNG with explicit streams.
//
// value(n) depends only on (seed, stream, n), never on the call history of other
// streams, so adding a feature that draws from a new stream cannot shift the
// numbers seen by existing features (platform spec section 4.2).
#pragma once
#include <cstdint>

namespace hr {

// SplitMix64 finalizer (Steele, Lea, Flood).
constexpr std::uint64_t mix64(std::uint64_t z) noexcept {
  z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9ULL;
  z = (z ^ (z >> 27)) * 0x94d049bb133111ebULL;
  return z ^ (z >> 31);
}

constexpr std::uint64_t kGoldenGamma = 0x9e3779b97f4a7c15ULL;

class Rng {
 public:
  constexpr Rng(std::uint64_t seed, std::uint64_t stream) noexcept
      : base_(mix64(seed ^ mix64(stream + kGoldenGamma))) {}

  // Value at an absolute counter position (pure function).
  constexpr std::uint64_t at(std::uint64_t counter) const noexcept {
    return mix64(base_ + (counter + 1) * kGoldenGamma);
  }

  constexpr std::uint64_t next_u64() noexcept { return at(counter_++); }

  // Uniform double in [0, 1) with 53 bits of precision.
  constexpr double uniform01() noexcept {
    return static_cast<double>(next_u64() >> 11) * 0x1.0p-53;
  }

  // Uniform integer in [0, n); n must be > 0. Rejection sampling avoids modulo bias.
  constexpr std::uint64_t below(std::uint64_t n) noexcept {
    const std::uint64_t max = ~std::uint64_t{0};
    const std::uint64_t limit = max - (max % n) - 1;
    std::uint64_t v = next_u64();
    while (v > limit) v = next_u64();
    return v % n;
  }

  constexpr std::uint64_t counter() const noexcept { return counter_; }
  constexpr void seek(std::uint64_t counter) noexcept { counter_ = counter; }

 private:
  std::uint64_t base_;
  std::uint64_t counter_ = 0;
};

}  // namespace hr

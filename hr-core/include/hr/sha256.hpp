// Self-contained SHA-256 (FIPS 180-4) used for content hashes and state hashes.
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <string_view>

namespace hr {

class Sha256 {
 public:
  using Digest = std::array<std::uint8_t, 32>;

  Sha256() noexcept { reset(); }
  void reset() noexcept;
  void update(const void* data, std::size_t len) noexcept;
  void update(std::string_view s) noexcept { update(s.data(), s.size()); }
  Digest finish() noexcept;  // call reset() before reusing the object

  static Digest hash(std::string_view s) noexcept;
  static std::string hex(const Digest& d);
  static std::string hash_hex(std::string_view s) { return hex(hash(s)); }

 private:
  void process_block(const std::uint8_t* block) noexcept;
  std::array<std::uint32_t, 8> h_{};
  std::array<std::uint8_t, 64> buf_{};
  std::size_t buf_len_ = 0;
  std::uint64_t total_len_ = 0;
};

}  // namespace hr

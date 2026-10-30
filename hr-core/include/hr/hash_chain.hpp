// Rolling state-hash chain recorded in replays (DATA_CONTRACTS section 2.2).
//
//   link_n = SHA256( link_{n-1}_hex || tick_le64 || canonical_state_bytes )
//
// A re-simulation must reproduce every recorded link; the first mismatch is
// reported as a desync with its tick.
#pragma once
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

#include "hr/error.hpp"

namespace hr {

struct HashLink {
  std::uint64_t tick;
  std::string hex;  // 64 lowercase hex chars
};

class HashChain {
 public:
  // Appends a link for `tick` over canonical state bytes; returns the new link's hex.
  const std::string& append(std::uint64_t tick, std::string_view canonical_state);
  const std::vector<HashLink>& links() const noexcept { return links_; }
  std::string head() const;  // hex of the latest link, or 64 zeros if empty

  // Compares against a reference chain. Returns Ok, or Desync with the first
  // mismatching tick written to `first_bad_tick` (if non-null). Length differences are Desync.
  static ErrorCode compare(const std::vector<HashLink>& expected, const std::vector<HashLink>& actual,
                           std::uint64_t* first_bad_tick = nullptr);

 private:
  std::vector<HashLink> links_;
};

}  // namespace hr

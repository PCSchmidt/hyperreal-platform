#include "hr/hash_chain.hpp"

#include "hr/sha256.hpp"

namespace hr {

std::string HashChain::head() const {
  return links_.empty() ? std::string(64, '0') : links_.back().hex;
}

const std::string& HashChain::append(std::uint64_t tick, std::string_view canonical_state) {
  Sha256 h;
  h.update(head());
  std::uint8_t le[8];
  for (int i = 0; i < 8; ++i) le[i] = static_cast<std::uint8_t>(tick >> (8 * i));
  h.update(le, 8);
  h.update(canonical_state);
  links_.push_back({tick, Sha256::hex(h.finish())});
  return links_.back().hex;
}

ErrorCode HashChain::compare(const std::vector<HashLink>& expected, const std::vector<HashLink>& actual,
                             std::uint64_t* first_bad_tick) {
  const std::size_t n = expected.size() < actual.size() ? expected.size() : actual.size();
  for (std::size_t i = 0; i < n; ++i) {
    if (expected[i].tick != actual[i].tick || expected[i].hex != actual[i].hex) {
      if (first_bad_tick) *first_bad_tick = expected[i].tick;
      return ErrorCode::Desync;
    }
  }
  if (expected.size() != actual.size()) {
    if (first_bad_tick) *first_bad_tick = expected.size() > n ? expected[n].tick : actual[n].tick;
    return ErrorCode::Desync;
  }
  return ErrorCode::Ok;
}

}  // namespace hr

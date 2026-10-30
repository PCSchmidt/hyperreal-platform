#include <doctest/doctest.h>

#include <string>

#include "hr/hash_chain.hpp"

TEST_CASE("Identical state sequences produce identical chains") {
  hr::HashChain a, b;
  for (std::uint64_t t = 0; t < 100; ++t) {
    a.append(t, "state-" + std::to_string(t));
    b.append(t, "state-" + std::to_string(t));
  }
  CHECK(hr::HashChain::compare(a.links(), b.links()) == hr::ErrorCode::Ok);
  CHECK(a.head() == b.head());
}

TEST_CASE("A single changed state is reported as a desync at that tick") {
  hr::HashChain a, b;
  for (std::uint64_t t = 0; t < 50; ++t) {
    a.append(t, "s" + std::to_string(t));
    b.append(t, t == 31 ? std::string("TAMPERED") : "s" + std::to_string(t));
  }
  std::uint64_t bad = 0;
  CHECK(hr::HashChain::compare(a.links(), b.links(), &bad) == hr::ErrorCode::Desync);
  CHECK(bad == 31);
}

TEST_CASE("Chain is order-dependent: tampering earlier state changes all later links") {
  hr::HashChain a, b;
  a.append(0, "x");
  a.append(1, "y");
  b.append(0, "z");
  b.append(1, "y");
  CHECK(a.links()[1].hex != b.links()[1].hex);
}

TEST_CASE("Length mismatch is a desync; empty chains compare equal") {
  hr::HashChain a, b;
  CHECK(hr::HashChain::compare(a.links(), b.links()) == hr::ErrorCode::Ok);
  CHECK(a.head() == std::string(64, '0'));
  a.append(0, "x");
  CHECK(hr::HashChain::compare(a.links(), b.links()) == hr::ErrorCode::Desync);
}

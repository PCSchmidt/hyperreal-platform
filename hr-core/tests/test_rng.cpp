#include <doctest/doctest.h>

#include <set>

#include "hr/rng.hpp"

TEST_CASE("mix64 matches the published SplitMix64 reference sequence for seed 0") {
  // SplitMix64 seeded with 0: state += gamma; output = mix(state).
  CHECK(hr::mix64(1 * hr::kGoldenGamma) == 0xe220a8397b1dcdafULL);
  CHECK(hr::mix64(2 * hr::kGoldenGamma) == 0x6e789e6aa1b965f4ULL);
  CHECK(hr::mix64(3 * hr::kGoldenGamma) == 0x06c45d188009454fULL);
}

TEST_CASE("Rng is a pure function of seed, stream and counter") {
  hr::Rng a(42, 7), b(42, 7);
  for (int i = 0; i < 1000; ++i) CHECK(a.next_u64() == b.next_u64());
  CHECK(hr::Rng(42, 7).at(500) == hr::Rng(42, 7).at(500));
}

TEST_CASE("Streams are independent: drawing from one does not shift another") {
  hr::Rng traffic(1, 1), weather(1, 2);
  const auto w0 = hr::Rng(1, 2).at(0);
  for (int i = 0; i < 100; ++i) (void)traffic.next_u64();
  CHECK(weather.next_u64() == w0);
  CHECK(hr::Rng(1, 1).at(0) != hr::Rng(1, 2).at(0));
  CHECK(hr::Rng(1, 1).at(0) != hr::Rng(2, 1).at(0));
}

TEST_CASE("seek reproduces a position") {
  hr::Rng r(9, 9);
  for (int i = 0; i < 10; ++i) (void)r.next_u64();
  const auto v = r.next_u64();
  r.seek(10);
  CHECK(r.next_u64() == v);
}

TEST_CASE("uniform01 stays in [0,1) and below() stays in range") {
  hr::Rng r(3, 3);
  for (int i = 0; i < 10000; ++i) {
    const double u = r.uniform01();
    CHECK(u >= 0.0);
    CHECK(u < 1.0);
    CHECK(r.below(7) < 7);
  }
}

TEST_CASE("below() covers all residues") {
  hr::Rng r(5, 5);
  std::set<std::uint64_t> seen;
  for (int i = 0; i < 1000; ++i) seen.insert(r.below(10));
  CHECK(seen.size() == 10);
}

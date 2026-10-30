#include <doctest/doctest.h>

#include "hr/clock.hpp"

TEST_CASE("FixedClock advances in whole ticks and maps to UTC milliseconds") {
  hr::FixedClock c(1'800'000'000'000, 60'000);
  CHECK(c.tick() == 0);
  CHECK(c.utc_ms() == 1'800'000'000'000);
  c.advance(3);
  CHECK(c.tick() == 3);
  CHECK(c.utc_ms() == 1'800'000'000'000 + 180'000);
  CHECK(c.utc_ms_at(10) == 1'800'000'000'000 + 600'000);
  CHECK(c.dt_seconds() == doctest::Approx(60.0));
}

#include <doctest/doctest.h>

#include <ostream>
#include <string>

#include "hr/error.hpp"

TEST_CASE("Error codes round-trip through their wire names") {
  using hr::ErrorCode;
  for (auto c : {ErrorCode::Ok, ErrorCode::Schema, ErrorCode::Version, ErrorCode::Hash,
                 ErrorCode::RuleViolation, ErrorCode::RoleForbidden, ErrorCode::Desync, ErrorCode::Limit}) {
    ErrorCode back = ErrorCode::Schema;
    CHECK(hr::from_string(hr::to_string(c), back));
    CHECK(back == c);
  }
  ErrorCode out = ErrorCode::Ok;
  CHECK_FALSE(hr::from_string("E_NOPE", out));
  CHECK(hr::to_string(ErrorCode::RoleForbidden) == "E_ROLE_FORBIDDEN");
}

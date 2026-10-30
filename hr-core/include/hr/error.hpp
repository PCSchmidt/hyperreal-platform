// Shared typed error codes (see docs/DATA_CONTRACTS.md section 4).
#pragma once
#include <string_view>

namespace hr {

enum class ErrorCode {
  Ok = 0,
  Schema,         // E_SCHEMA
  Version,        // E_VERSION
  Hash,           // E_HASH
  RuleViolation,  // E_RULE_VIOLATION
  RoleForbidden,  // E_ROLE_FORBIDDEN
  Desync,         // E_DESYNC
  Limit,          // E_LIMIT
};

// Stable wire name, e.g. "E_ROLE_FORBIDDEN". "OK" for ErrorCode::Ok.
std::string_view to_string(ErrorCode code) noexcept;

// Parses a wire name; returns false if unknown.
bool from_string(std::string_view name, ErrorCode& out) noexcept;

}  // namespace hr

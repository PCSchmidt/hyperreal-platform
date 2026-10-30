#include "hr/error.hpp"

namespace hr {

namespace {
struct Entry {
  ErrorCode code;
  std::string_view name;
};
constexpr Entry kEntries[] = {
    {ErrorCode::Ok, "OK"},
    {ErrorCode::Schema, "E_SCHEMA"},
    {ErrorCode::Version, "E_VERSION"},
    {ErrorCode::Hash, "E_HASH"},
    {ErrorCode::RuleViolation, "E_RULE_VIOLATION"},
    {ErrorCode::RoleForbidden, "E_ROLE_FORBIDDEN"},
    {ErrorCode::Desync, "E_DESYNC"},
    {ErrorCode::Limit, "E_LIMIT"},
};
}  // namespace

std::string_view to_string(ErrorCode code) noexcept {
  for (const auto& e : kEntries) {
    if (e.code == code) return e.name;
  }
  return "E_UNKNOWN";
}

bool from_string(std::string_view name, ErrorCode& out) noexcept {
  for (const auto& e : kEntries) {
    if (e.name == name) {
      out = e.code;
      return true;
    }
  }
  return false;
}

}  // namespace hr

/*
  Copyright 2026 Lukáš Růžička

  This file is part of build_cxx.

  build_cxx is free software: you can redistribute it and/or modify it under the
  terms of the GNU Lesser General Public License as published by the Free
  Software Foundation, either version 3 of the License, or (at your option) any
  later version.

  build_cxx is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR
  A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more
  details.

  You should have received a copy of the GNU Lesser General Public License along
  with build_cxx. If not, see <https://www.gnu.org/licenses/>.
*/

#include "build_cxx/common/target_status.hxx"

#include <stdexcept>
#include <variant>

namespace build_cxx::common {

namespace {

struct merge_visitor {
  explicit merge_visitor(target_status::status_t &aDest) : dest{aDest} {}

  void operator()(std::monostate const) {
    throw std::runtime_error{
        "Internal error: merging with uninitialized target status"};
  }

  void operator()(target_status::needs_update_t const) {
    dest = target_status::needs_update;
  }

  void operator()(target_status::file_mod_time_t const value) {
    if (auto *const dest_mod_time_ptr =
            std::get_if<target_status::file_mod_time_t>(&dest);
        dest_mod_time_ptr != nullptr) {
      *dest_mod_time_ptr = std::max(*dest_mod_time_ptr, value);
    } else if (!std::holds_alternative<target_status::needs_update_t>(dest)) {
      dest = value;
    }
  }

private:
  target_status::status_t &dest;
};

} // namespace

void target_status::merge_with(target_status const rhs) {
  std::visit(merge_visitor{status}, rhs.status);
}

bool target_status::certainly_needs_update() const {
  require_initialized();
  return std::holds_alternative<needs_update_t>(status);
}

namespace {

struct needs_update_visitor {
  explicit needs_update_visitor(
      target_status::file_mod_time_t const aMy_mod_time)
      : my_mod_time{aMy_mod_time} {}

  // other is empty
  [[nodiscard]] bool operator()(std::monostate const) const { return false; }

  // other needs update
  [[nodiscard]] bool operator()(target_status::needs_update_t const) const {
    return true;
  }

  [[nodiscard]] bool
  operator()(target_status::file_mod_time_t const other_mod_time) const {
    return my_mod_time < other_mod_time;
  }

private:
  target_status::file_mod_time_t my_mod_time;
};

} // namespace

bool target_status::needs_update_compared_to(target_status const other) const {
  require_initialized();
  if (std::holds_alternative<needs_update_t>(status)) {
    return true;
  } else {
    auto *const self_mod_time_ptr =
        std::get_if<target_status::file_mod_time_t>(&status);
    return std::visit(needs_update_visitor{*self_mod_time_ptr}, other.status);
  }
}

void target_status::require_initialized() const {
  if (std::holds_alternative<std::monostate>(status)) {
    throw std::runtime_error{
        "Internal error: querying uninitialized target status"};
  }
}

} // namespace build_cxx::common

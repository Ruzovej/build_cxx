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

#include <algorithm>
#include <stdexcept>

namespace build_cxx::common {

namespace {

// intentional UB ... so the optimizer can slice & dice around it
[[noreturn]] void my_unreachable() {
#if __has_builtin(__builtin_unreachable)
  __builtin_unreachable();
#elif __has_builtin(__assume)
  __assume(false);
#else
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wpragmas"
#pragma GCC diagnostic ignored "-Winvalid-noreturn"
  return;
#pragma GCC diagnostic pop
#endif
}

} // namespace

void target_status::merge_with(target_status const rhs) {
  rhs.require_initialized();

  switch (rhs.kind) {
  case kind_t::explicitly_needs_update: {
    kind = kind_t::explicitly_needs_update;
    return;
  }
  case kind_t::file_mod_time: {
    if (kind == kind_t::file_mod_time) {
      mod_time = std::max(mod_time, rhs.mod_time);
    } else if (kind != kind_t::explicitly_needs_update) {
      // overwrites uninitialized and transitively_needs_update
      *this = rhs;
    }
    return;
  }
  case kind_t::transitively_needs_update: {
    if (kind == kind_t::uninitialized) {
      kind = kind_t::transitively_needs_update;
    }
    return;
  }
  default: {
    my_unreachable();
    break;
  }
  }
}

bool target_status::certainly_needs_update() const {
  require_initialized();

  return kind == kind_t::explicitly_needs_update;
}

bool target_status::needs_update_compared_to(target_status const other) const {
  require_initialized();
  other.require_initialized();

  if (kind != kind_t::file_mod_time) {
    // ==> this: explicitly or transitively needs update

    if (other.kind == kind_t::transitively_needs_update) {
      throw std::runtime_error{"Internal error: comparing with "
                               "'transitively_needs_update' target status"};
    }

    // ==> other: file mod time or explicitly needs update

    return true; // whatever the combination, this is the result
  }

  // ==> this: file mod time
  switch (other.kind) {
  case kind_t::explicitly_needs_update: {
    return true;
  }
  case kind_t::file_mod_time: {
    return mod_time < other.mod_time;
  }
  case kind_t::transitively_needs_update: {
    throw std::runtime_error{"Internal error: comparing with "
                             "'transitively_needs_update' target status"};
  }
  default: {
    my_unreachable();
    return true; // to silence compiler warning
  }
  }
}

void target_status::require_initialized() const {
  if (kind == kind_t::uninitialized) {
    throw std::runtime_error{
        "Internal error: querying uninitialized target status"};
  }
}

} // namespace build_cxx::common

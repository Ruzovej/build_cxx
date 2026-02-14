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

#pragma once

#include "build_cxx/common/macros.h"

namespace build_cxx::common {

struct BUILD_CXX_DLL_EXPORT target_status {
  using file_mod_time_t = long long;

  enum class kind_t : char {
    uninitialized,
    // lowest priority; serves only as a placeholder and must be overwritten by
    // one of those below (or comparison, etc. will throw):
    transitively_needs_update,
    // when comparing with other, it may indicate to build/skip this:
    file_mod_time,
    // highest priority; indicates this should be built:
    explicitly_needs_update,
  };

  struct transitively_needs_update_t {};
  static transitively_needs_update_t constexpr transitively_needs_update{};

  struct explicitly_needs_update_t {};
  static explicitly_needs_update_t constexpr explicitly_needs_update{};

  constexpr target_status() = default;
  constexpr target_status(transitively_needs_update_t const) noexcept
      : kind{kind_t::transitively_needs_update} {
    // force 2 lines
  }
  constexpr explicit target_status(file_mod_time_t const mod_time) noexcept
      : kind{kind_t::file_mod_time}, mod_time{mod_time} {
    // force 2 lines
  }
  constexpr target_status(explicitly_needs_update_t const) noexcept
      : kind{kind_t::explicitly_needs_update} {
    // force 2 lines
  }

  void merge_with(target_status const rhs);

  [[nodiscard]] bool certainly_needs_update() const;

  [[nodiscard]] bool needs_update_compared_to(target_status const other) const;

private:
  void require_initialized() const;

  kind_t kind{kind_t::uninitialized};
  file_mod_time_t mod_time{};
};

} // namespace build_cxx::common

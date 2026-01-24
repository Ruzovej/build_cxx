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

#include <variant>

#include "build_cxx/common/macros.h"

namespace build_cxx::common {

struct BUILD_CXX_DLL_EXPORT target_status {
  struct needs_update_t {};
  static needs_update_t constexpr needs_update{};
  using file_modification_time_t = long long;

  using status_t =
      std::variant<std::monostate, needs_update_t, file_modification_time_t>;

  constexpr target_status() = default;
  constexpr target_status(needs_update_t) : status{needs_update_t{}} {}
  constexpr target_status(file_modification_time_t const mod_time)
      : status{mod_time} {}

  void merge_with(target_status const &rhs);

  [[nodiscard]] bool certainly_needs_update() const;

  [[nodiscard]] bool needs_update_compared_to(target_status const &other) const;

private:
  void require_nondefault_state() const;

  status_t status;
};

} // namespace build_cxx::common

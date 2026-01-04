/*
  Copyright 2025 Lukáš Růžička

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

#include <string_view>

#include "build_cxx/common/abstract_target.hxx"

namespace build_cxx::common {

struct proxy {
  proxy(std::string_view const aName) : name{aName} {}

  explicit proxy(std::string_view const target_namespace,
                 std::string_view const aName)
      : name{aName}, target{resolve_target(target_namespace)} {}

  abstract_target *resolve_target(std::string_view const target_namespace) {
    return nullptr; // TODO implement later...
  }

  [[nodiscard]] std::string_view get_name() const { return name; }

  [[nodiscard]] abstract_target const *get_target() const { return target; }

  //[[nodiscard]] operator abstract_target *() const { return target; }

private:
  std::string_view name;

  abstract_target *target{nullptr};
};

} // namespace build_cxx::common

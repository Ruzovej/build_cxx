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

#include "build_cxx/common/target_builder.hxx"

namespace build_cxx::common {

target_builder::target_builder(std::string &&aName,
                               std::string_view const aFilename,
                               int const aLine, int const aIndex,
                               adjust_target_fn *const aFn)
    : name{std::move(aName)}, filename{aFilename}, line{aLine}, index{aIndex},
      fn{aFn} {}

void target_builder::update_target() { fn(*this); }

std::vector<target_builder> &get_target_builders_vector() {
  static std::vector<target_builder> registered_targets;
  return registered_targets;
}

} // namespace build_cxx::common

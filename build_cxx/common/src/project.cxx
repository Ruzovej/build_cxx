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

#include "build_cxx/common/project.hxx"

namespace build_cxx::common {

project::project(std::string_view const aName, std::string_view const aVersion,
                 std::string_view const aRoot_file) noexcept
    : name{aName}, version{aVersion}, root_file{aRoot_file} {}

void project::add_target(abstract_target *const target) noexcept {
  if (first == nullptr) {
    first = last = target;
  } else {
    last->next = target;
    last = target;
  }
  target->parent_project = this;
}

} // namespace build_cxx::common

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

#include "build_cxx/common/target_alias.hxx"

#include <stdexcept>

namespace build_cxx::common {

void target_alias::initialize_status() {
  status = target_status::transitively_needs_update;
}

void target_alias::update_status(target_status const newest_dep_status) {
  status = newest_dep_status;
}

void target_alias::recipe(
    std::vector<abstract_target const *> const &resolved_deps) const {
  if (resolved_deps.empty()) {
    throw std::runtime_error{
        "target_alias should have at least one dependency"};
  }
  // this kind of target doesn't have a recipe -> due to proper order, all that
  // is aliased is built before
}

} // namespace build_cxx::common

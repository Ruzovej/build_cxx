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

#include "build_cxx/common/phony_target.hxx"

#include <limits>

#include "build_cxx/common/project.hxx"

namespace build_cxx::common {

std::optional<abstract_target::modification_time_t>
phony_target::last_modification_time() const {
  return std::nullopt;
}

std::string phony_target::resolve_name(std::string_view const project_name,
                                       std::string_view const target_name) {
  return std::string{project_name} + "::" + std::string{target_name};
}

void phony_target::resolve_own_traits() {
  resolved_kind = kind;
  resolved_name = resolve_name(parent_project->name, name);
}

void phony_target::build(
    std::vector<abstract_target const *> const &resolved_deps) {
  // nothing to manage ...
  recipe(resolved_deps);
}

} // namespace build_cxx::common

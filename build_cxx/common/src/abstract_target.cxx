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

#include "build_cxx/common/abstract_target.hxx"

namespace build_cxx::common {

abstract_target::abstract_target(location const *const aLoc,
                                 bool const aInclude_in_all,
                                 std::string_view const aName,
                                 std::string_view const *const aRaw_deps,
                                 std::size_t const aNum_deps) noexcept
    : loc{aLoc}, include_in_all{aInclude_in_all}, name{aName},
      raw_deps{aRaw_deps}, num_deps{aNum_deps} {}

void abstract_target::build(
    std::vector<abstract_target const *> const &resolved_deps) {
  initialize_status();

  auto worst_status{status};

  if (!worst_status.certainly_needs_update()) {
    for (auto const dep : resolved_deps) {
      worst_status.merge_with(dep->get_status());

      if (worst_status.certainly_needs_update()) {
        break;
      }
    }
  }

  if (status.needs_update_compared_to(worst_status)) {
    recipe(resolved_deps);
    update_status(worst_status);
  }
}

} // namespace build_cxx::common

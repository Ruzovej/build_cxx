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

#include <vector>

#include "build_cxx/common/abstract_target.hxx"
#include "build_cxx/common/macros.h"
#include "build_cxx/common/phony_target.hxx"

namespace build_cxx::common {

// Recommendation: don't alias any phony target, because then it behaves exactly
// the same as if this was phony target itself
struct BUILD_CXX_DLL_EXPORT target_alias : phony_target {
  using phony_target::phony_target;

  void initialize_status() override;
  void update_status(target_status const newest_dep_status) override;

  void recipe(
      std::vector<abstract_target const *> const &resolved_deps) const override;
};

} // namespace build_cxx::common

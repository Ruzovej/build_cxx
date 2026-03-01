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

#include <build_cxx/common/abstract_target.hxx>
#include <build_cxx/common/macros.h>
#include <build_cxx/common/target_status.hxx>

namespace build_cxx::driver {

// TODO ... EXPORT or HIDE?!
struct BUILD_CXX_DLL_EXPORT build_request {
  common::abstract_target *tgt{nullptr};
  std::vector<common::abstract_target const *> const *deps{nullptr};
  common::target_status newest_dep_status{};
};

} // namespace build_cxx::driver

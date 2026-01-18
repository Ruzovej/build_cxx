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

#include <string>
#include <vector>

#include <build_cxx/common/abstract_target.hxx>
#include <build_cxx/common/macros.h>

namespace build_cxx::client {

[[nodiscard]] BUILD_CXX_DLL_EXPORT std::string
abstract_target_basic_info(common::abstract_target const *const at,
                           bool const brief = false);

[[nodiscard]] BUILD_CXX_DLL_EXPORT std::string abstract_target_build_info(
    common::abstract_target const *const at,
    std::vector<common::abstract_target const *> const &deps);

} // namespace build_cxx::client

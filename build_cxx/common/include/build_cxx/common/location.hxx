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

#include <string_view>

#include "build_cxx/common/macros.h"

#define BUILD_CXX_CURRENT_LOCATION                                             \
  build_cxx::common::location { __FILE__, __LINE__ }

namespace build_cxx::common {

struct BUILD_CXX_DLL_EXPORT location {
  // this should indicate that it's not tied to "baked-in" target (but to some
  // dynamically determined one)
  static int constexpr no_index{-1};

  std::string_view filename;
  int line;
  int index{no_index};
};

} // namespace build_cxx::common

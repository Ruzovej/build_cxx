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

#include "build_cxx/impl/implicit_name.h"
#include "build_cxx/impl/main.hxx"
#include "build_cxx/impl/registrator.hxx"
#include "build_cxx/impl/target_builder.hxx"

// https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
// I hope others support `__COUNTER__` too
#define BUILD_CXX_GENERIC_TARGET_IMPL(name, index)                             \
  static BUILD_CXX_ADJUST_TARGET_FN(                                           \
      BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_ADJUST_TARGET_FN_, index));       \
  static ::build_cxx::impl::registrator const BUILD_CXX_IMPL_IMPLICIT_NAME(    \
      BUILD_CXX_REGISTRATOR_, index){                                          \
      name, __FILE__, __LINE__, index,                                         \
      BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_ADJUST_TARGET_FN_, index)};       \
  BUILD_CXX_ADJUST_TARGET_FN(                                                  \
      BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_ADJUST_TARGET_FN_, index))

#define BUILD_CXX_GENERIC_TARGET(name)                                         \
  BUILD_CXX_GENERIC_TARGET_IMPL(name, __COUNTER__)

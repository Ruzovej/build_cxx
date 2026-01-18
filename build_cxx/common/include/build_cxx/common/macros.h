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

#define BUILD_CXX_IMPL_IDENTITY(x) x
#define BUILD_CXX_IMPL_STRINGIFY(x) #x
#define BUILD_CXX_IMPL_GET_PROJECT_SYMBOL_NAME_STR(x)                          \
  BUILD_CXX_IMPL_STRINGIFY(x)

#define BUILD_CXX_GET_PROJECT_SYMBOL_NAME build_cxx_get_project_v_0_1_0_beta
#define BUILD_CXX_GET_PROJECT_SYMBOL_NAME_STR                                  \
  BUILD_CXX_IMPL_GET_PROJECT_SYMBOL_NAME_STR(                                  \
      BUILD_CXX_IMPL_IDENTITY(BUILD_CXX_GET_PROJECT_SYMBOL_NAME))

#define BUILD_CXX_DLL_HIDE __attribute__((visibility("hidden")))
#define BUILD_CXX_DLL_EXPORT __attribute__((visibility("default")))
#define BUILD_CXX_DLL_EXPORT_C_SYMBOL extern "C" BUILD_CXX_DLL_EXPORT

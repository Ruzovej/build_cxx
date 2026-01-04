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

#include <build_cxx/common/phony_target.hxx>
#include <build_cxx/common/project.hxx>
#include <build_cxx/common/target_builder.hxx> // TODO remove ...

#include "build_cxx/client/macros.h"
#include "build_cxx/client/registrator.hxx"

namespace build_cxx::common {
struct BUILD_CXX_DLL_HIDE register_target {
  explicit register_target(abstract_target *const at);
};

} // namespace build_cxx::common

#define BUILD_CXX_PROJECT(name, version)                                       \
  static ::build_cxx::common::project &this_project() {                        \
    static ::build_cxx::common::project p{name, version};                      \
    return p;                                                                  \
  }                                                                            \
                                                                               \
  ::build_cxx::common::register_target::register_target(                       \
      abstract_target *const at) {                                             \
    this_project().add_target(at);                                             \
  }                                                                            \
                                                                               \
  BUILD_CXX_DLL_EXPORT_C_SYMBOL ::build_cxx::common::project *                 \
  build_cxx_get_project() {                                                    \
    return &this_project();                                                    \
  }

// https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
// I hope others support `__COUNTER__` too

#define BUILD_CXX_PHONY_TARGET_IMPL(index, name, ...)                          \
  static ::build_cxx::common::location constexpr BUILD_CXX_IMPL_IMPLICIT_NAME( \
      BUILD_CXX_LOCATION_, index){__FILE__, __LINE__, index};                  \
                                                                               \
  static std::string_view constexpr BUILD_CXX_IMPL_IMPLICIT_NAME(              \
      BUILD_CXX_DEPS_, index)[]{__VA_ARGS__};                                  \
                                                                               \
  static BUILD_CXX_PHONY_TARGET_FN(                                            \
      BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_PHONY_TARGET_FN_, index));        \
                                                                               \
  static ::build_cxx::common::phony_target BUILD_CXX_IMPL_IMPLICIT_NAME(       \
      BUILD_CXX_PHONY_TARGET_,                                                 \
      index){&BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_LOCATION_, index), name,  \
             BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_DEPS_, index),             \
             sizeof(BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_DEPS_, index)) /    \
                 sizeof(std::string_view),                                     \
             BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_PHONY_TARGET_FN_, index)}; \
                                                                               \
  static ::build_cxx::common::register_target BUILD_CXX_IMPL_IMPLICIT_NAME(    \
      BUILD_CXX_REGISTER_PHONY_TARGET_,                                        \
      index){&BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_PHONY_TARGET_, index)};   \
                                                                               \
  BUILD_CXX_PHONY_TARGET_FN(                                                   \
      BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_PHONY_TARGET_FN_, index))

#define BUILD_CXX_PHONY_TARGET(name, ...)                                      \
  BUILD_CXX_PHONY_TARGET_IMPL(__COUNTER__, name, __VA_ARGS__)

#define BUILD_CXX_GENERIC_TARGET_IMPL(index, name, ...)                        \
  static std::string_view constexpr BUILD_CXX_IMPL_IMPLICIT_NAME(              \
      BUILD_CXX_TARGET_DEPS_, index)[]{__VA_ARGS__};                           \
  static BUILD_CXX_ADJUST_TARGET_FN(                                           \
      BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_ADJUST_TARGET_FN_, index));       \
  static ::build_cxx::client::registrator const BUILD_CXX_IMPL_IMPLICIT_NAME(  \
      BUILD_CXX_REGISTRATOR_, index){                                          \
      __FILE__,                                                                \
      __LINE__,                                                                \
      index,                                                                   \
      name,                                                                    \
      BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_TARGET_DEPS_, index),             \
      sizeof(BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_TARGET_DEPS_, index)) /    \
          sizeof(std::string_view),                                            \
      BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_ADJUST_TARGET_FN_, index)};       \
  BUILD_CXX_ADJUST_TARGET_FN(                                                  \
      BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_ADJUST_TARGET_FN_, index))

#define BUILD_CXX_GENERIC_TARGET(name, ...)                                    \
  BUILD_CXX_GENERIC_TARGET_IMPL(__COUNTER__, name, __VA_ARGS__)

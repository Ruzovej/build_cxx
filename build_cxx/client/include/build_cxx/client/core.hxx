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

#include <build_cxx/common/abstract_target.hxx>
#include <build_cxx/common/file_target.hxx>
#include <build_cxx/common/macros.h>
#include <build_cxx/common/phony_target.hxx>
#include <build_cxx/common/project.hxx>

#include "build_cxx/client/name_macros.h"

namespace build_cxx::client {

struct BUILD_CXX_DLL_HIDE register_target {
  explicit register_target(common::abstract_target *const at);

private:
  register_target(register_target const &) = delete;
  register_target &operator=(register_target const &) = delete;
  register_target(register_target &&) = delete;
  register_target &operator=(register_target &&) = delete;

  void *operator new(std::size_t) = delete;
  void operator delete(void *) = delete;
  void *operator new[](std::size_t) = delete;
  void operator delete[](void *) = delete;
};

} // namespace build_cxx::client

#define BUILD_CXX_PROJECT(name, version)                                       \
  static build_cxx::common::project &this_project() {                          \
    static build_cxx::common::project p{name, version, __FILE__};              \
    return p;                                                                  \
  }                                                                            \
                                                                               \
  build_cxx::client::register_target::register_target(                         \
      build_cxx::common::abstract_target *const at) {                          \
    this_project().add_target(at);                                             \
  }                                                                            \
                                                                               \
  BUILD_CXX_DLL_EXPORT_C_SYMBOL build_cxx::common::project *                   \
  BUILD_CXX_GET_PROJECT_SYMBOL_NAME() {                                        \
    return &this_project();                                                    \
  }

// 01, helpers & implementation details

#define BUILD_CXX_DEFINE_LOCATION(aLocation_name, aIndex)                      \
  static build_cxx::common::location constexpr aLocation_name {                \
    __FILE__, __LINE__, aIndex                                                 \
  }

// This will fail with zero deps & when compiled with `-Wpedantic`, etc.:
#define BUILD_CXX_DEFINE_DEPS_ARRAY(aDeps_name, aNum_deps, ...)                \
  static std::string_view constexpr aDeps_name[]{__VA_ARGS__};                 \
  static std::size_t constexpr aNum_deps {                                     \
    sizeof(aDeps_name) / sizeof(aDeps_name[0])                                 \
  }

#define BUILD_CXX_DECLARE_ABSTRACT_TARGET_CHILD(aDerived_name, aBase_name)     \
  struct BUILD_CXX_DLL_HIDE aDerived_name : build_cxx::common::aBase_name {    \
    using aBase_name::aBase_name;                                              \
                                                                               \
    void recipe(std::vector<build_cxx::common::abstract_target const *> const  \
                    &resolved_deps) const override;                            \
  }

#define BUILD_CXX_DEFINE_TARGET(aTarget_type_t, aTarget_var_name,              \
                                aLocation_name, aInclude_in_all, aName,        \
                                aDeps_name, aNum_deps)                         \
  static aTarget_type_t aTarget_var_name {                                     \
    &aLocation_name, aInclude_in_all, aName, aDeps_name, aNum_deps             \
  }

#define BUILD_CXX_DEFINE_REGISTRATOR(aRegistrator_name, aTarget_var_name)      \
  static build_cxx::client::register_target aRegistrator_name {                \
    &aTarget_var_name                                                          \
  }

#define BUILD_CXX_DEFINE_TARGET_RECIPE(aDerived_name)                          \
  void aDerived_name::recipe(                                                  \
      std::vector<build_cxx::common::abstract_target const *> const            \
          &resolved_deps) const

// 02

#define BUILD_CXX_INDEXED_FINAL_TARGET_WITH_NAMES(                             \
    aIndex, aLocation_name, aDeps_name, aNum_deps, aTarget_type_t,             \
    aInclude_in_all, aTarget_var_name, aName, aRegistrator_name, ...)          \
  BUILD_CXX_DEFINE_LOCATION(aLocation_name, aIndex);                           \
                                                                               \
  BUILD_CXX_DEFINE_DEPS_ARRAY(aDeps_name, aNum_deps, __VA_ARGS__);             \
                                                                               \
  BUILD_CXX_DEFINE_TARGET(aTarget_type_t, aTarget_var_name, aLocation_name,    \
                          aInclude_in_all, aName, aDeps_name, aNum_deps);      \
                                                                               \
  BUILD_CXX_DEFINE_REGISTRATOR(aRegistrator_name, aTarget_var_name)

// 02.5

#define BUILD_CXX_INDEXED_CUSTOM_TARGET_WITH_NAMES(                            \
    aIndex, aLocation_name, aDeps_name, aNum_deps, aTarget_type_t,             \
    aInclude_in_all, aDerived_name, aTarget_var_name, aName,                   \
    aRegistrator_name, ...)                                                    \
  BUILD_CXX_DECLARE_ABSTRACT_TARGET_CHILD(aDerived_name, aTarget_type_t);      \
                                                                               \
  BUILD_CXX_INDEXED_FINAL_TARGET_WITH_NAMES(                                   \
      aIndex, aLocation_name, aDeps_name, aNum_deps, aDerived_name,            \
      aInclude_in_all, aTarget_var_name, aName, aRegistrator_name,             \
      __VA_ARGS__);                                                            \
                                                                               \
  BUILD_CXX_DEFINE_TARGET_RECIPE(aDerived_name)

// 03

#define BUILD_CXX_INDEXED_CUSTOM_TARGET(index, include_in_all, given_target_t, \
                                        name, ...)                             \
  BUILD_CXX_INDEXED_CUSTOM_TARGET_WITH_NAMES(                                  \
      index, BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_LOCATION_, index),         \
      BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_DEPS_, index),                    \
      BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_NUM_DEPS_, index),                \
      given_target_t, include_in_all,                                          \
      BUILD_CXX_IMPL_IMPLICIT_NAME(given_target_t, index),                     \
      BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_CUSTOM_TARGET_, index), name,     \
      BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_REGISTER_CUSTOM_TARGET_, index),  \
      __VA_ARGS__)

#define BUILD_CXX_INDEXED_FINAL_TARGET(index, include_in_all, given_target_t,  \
                                       name, ...)                              \
  BUILD_CXX_INDEXED_FINAL_TARGET_WITH_NAMES(                                   \
      index, BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_LOCATION_, index),         \
      BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_DEPS_, index),                    \
      BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_NUM_DEPS_, index),                \
      build_cxx::common::given_target_t, include_in_all,                       \
      BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_FINAL_TARGET_, index), name,      \
      BUILD_CXX_IMPL_IMPLICIT_NAME(BUILD_CXX_REGISTER_FINAL_TARGET_, index),   \
      __VA_ARGS__)

// 04

// https://gcc.gnu.org/onlinedocs/cpp/Common-Predefined-Macros.html
// I hope others support `__COUNTER__` too
#define BUILD_CXX_CUSTOM_TARGET(given_target_t, include_in_all, name, ...)     \
  BUILD_CXX_INDEXED_CUSTOM_TARGET(__COUNTER__, include_in_all, given_target_t, \
                                  name, __VA_ARGS__)

#define BUILD_CXX_FINAL_TARGET(given_target_t, include_in_all, name, ...)      \
  BUILD_CXX_INDEXED_FINAL_TARGET(__COUNTER__, include_in_all, given_target_t,  \
                                 name, __VA_ARGS__)

// 05, macros for users

// 05.1

#define BUILD_CXX_FILE_TARGET(name, ...)                                       \
  BUILD_CXX_CUSTOM_TARGET(file_target, true, name, __VA_ARGS__)

#define BUILD_CXX_HIDDEN_FILE_TARGET(name, ...)                                \
  BUILD_CXX_CUSTOM_TARGET(file_target, false, name, __VA_ARGS__)

// 05.2

#define BUILD_CXX_READ_ONLY_FILE_TARGET(name, ...)                             \
  BUILD_CXX_FINAL_TARGET(read_only_file_target, true, name, __VA_ARGS__)

#define BUILD_CXX_HIDDEN_READ_ONLY_FILE_TARGET(name, ...)                      \
  BUILD_CXX_FINAL_TARGET(read_only_file_target, false, name, __VA_ARGS__)

// 05.3

#define BUILD_CXX_PHONY_TARGET(name, ...)                                      \
  BUILD_CXX_CUSTOM_TARGET(phony_target, true, name, __VA_ARGS__)

#define BUILD_CXX_HIDDEN_PHONY_TARGET(name, ...)                               \
  BUILD_CXX_CUSTOM_TARGET(phony_target, false, name, __VA_ARGS__)

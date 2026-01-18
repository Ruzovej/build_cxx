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

#include "build_cxx/common/abstract_target.hxx"
#include "build_cxx/common/location.hxx"
#include "build_cxx/common/macros.h"

namespace build_cxx::common {

struct BUILD_CXX_DLL_EXPORT project {
  explicit project(std::string_view const aName,
                   std::string_view const aVersion,
                   std::string_view const aRoot_file) noexcept;

  ~project() = default;

  void add_target(abstract_target *const target) noexcept;

  // TODO private & getters, (setters?!), etc.:
  std::string_view name;
  std::string_view version;
  // non owned:
  abstract_target *first{nullptr};
  abstract_target *last{nullptr};
  std::string_view root_file;

private:
  project(project const &) = delete;
  project &operator=(project const &) = delete;
  project(project &&) = delete;
  project &operator=(project &&) = delete;

  void *operator new(std::size_t) = delete;
  void operator delete(void *) = delete;
  void *operator new[](std::size_t) = delete;
  void operator delete[](void *) = delete;
};

} // namespace build_cxx::common

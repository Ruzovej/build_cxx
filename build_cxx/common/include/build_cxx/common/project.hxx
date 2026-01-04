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

#include <string_view>

#include "build_cxx/common/abstract_target.hxx"

namespace build_cxx::common {

struct project {
  explicit project(std::string_view const aName,
                   std::string_view const aVersion)
      : name{aName}, version{aVersion} {}

  ~project() = default;

  void add_target(abstract_target *const target) {
    if (first == nullptr) {
      first = last = target;
    } else {
      last->next = target;
      last = target;
    }
  }

  // TODO private & getters, (setters?!), etc.:
  std::string_view name;
  std::string_view version;
  // non owned:
  abstract_target *first{nullptr};
  abstract_target *last{nullptr};

private:
  project(project const &) = delete;
  project &operator=(project const &) = delete;
  project(project &&) = delete;
  project &operator=(project &&) = delete;
};

} // namespace build_cxx::common

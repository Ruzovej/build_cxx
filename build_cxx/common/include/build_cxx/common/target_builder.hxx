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

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace build_cxx::impl {

struct target_builder;

using adjust_target_fn = void(target_builder & /*current_target*/);

#define BUILD_CXX_ADJUST_TARGET_FN(fn_name)                                    \
  void fn_name(::build_cxx::impl::target_builder &current_target)

struct target_builder {
  explicit target_builder(std::string &&aName, std::string_view const aFilename,
                          int const aLine, int const aIndex,
                          adjust_target_fn *const aFn)
      : name(std::move(aName)), filename(aFilename), line(aLine), index(aIndex),
        fn(aFn) {}

  target_builder(target_builder const &) = default;
  target_builder &operator=(target_builder const &) = default;
  target_builder(target_builder &&) = default;
  target_builder &operator=(target_builder &&) = default;

  void adjust_target() { fn(*this); }

  std::string name;
  std::string_view filename;
  int line;
  int index;

private:
  adjust_target_fn *fn;
};

inline std::vector<target_builder> &get_target_builders_vector() {
  static std::vector<target_builder> registered_targets;
  return registered_targets;
}

} // namespace build_cxx::impl

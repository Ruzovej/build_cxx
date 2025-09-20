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

#include "build_cxx/impl/target_builder.hxx"

namespace build_cxx::impl {

struct registrator {
  explicit registrator(std::string &&name, std::string_view const filename,
                       int const line, int const idx,
                       adjust_target_fn *const fn) {
    get_target_builders_vector().emplace_back(std::move(name), filename, line,
                                              idx, fn);
  }

private:
  registrator(registrator const &) = delete;
  registrator &operator=(registrator const &) = delete;
  registrator(registrator &&) = delete;
  registrator &operator=(registrator &&) = delete;
};

} // namespace build_cxx::impl

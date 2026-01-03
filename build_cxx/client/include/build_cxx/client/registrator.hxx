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

#include <build_cxx/common/target_builder.hxx>

namespace build_cxx::client {

struct registrator {
  explicit registrator(std::string &&name, std::string_view const filename,
                       int const line, int const idx,
                       common::adjust_target_fn *const fn);

private:
  registrator(registrator const &) = delete;
  registrator &operator=(registrator const &) = delete;
  registrator(registrator &&) = delete;
  registrator &operator=(registrator &&) = delete;

  void *operator new(std::size_t) = delete;
  void operator delete(void *) = delete;
  void *operator new[](std::size_t) = delete;
  void operator delete[](void *) = delete;
};

} // namespace build_cxx::client

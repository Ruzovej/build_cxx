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

#include "build_cxx/driver/dlopen_scoped.hxx"

#include <dlfcn.h>

#include <exception>
#include <iostream>

#include <build_cxx/common/macros.h>

namespace build_cxx::driver {

dlopen_scoped::dlopen_scoped(char const *const filename)
    : handle{dlopen(filename, RTLD_LAZY /*| RTLD_GLOBAL*/)} {
  if (handle == nullptr) {
    throw std::runtime_error{std::string{"dlopen failed - error: "} +
                             dlerror()};
  }
}

build_cxx::common::project *dlopen_scoped::get_project() const {
  static auto constexpr symbol_name{
      BUILD_CXX_GET_PROJECT_SYMBOL_NAME_STR};

  auto symbol{dlsym(handle, symbol_name)};
  if (symbol == nullptr) { // not mandatory ... TODO later make it so!
    return nullptr;
  }

  using get_proj_fn = build_cxx::common::project *();

  auto const fn{reinterpret_cast<get_proj_fn *>(symbol)};

  return fn();
}

dlopen_scoped::~dlopen_scoped() noexcept {
  if (handle != nullptr) {
    if (dlclose(handle) < 0) {
      std::cerr << "Warning: dlclose failed - error: " << dlerror() << '\n';
    }
  }
}

} // namespace build_cxx::driver

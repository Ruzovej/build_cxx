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

namespace build_cxx::os_wrapper {

int current_errno() noexcept;

// returns: `syscall_ret` if successful (e.g. `0 <= syscall_ret`);
// throws: std::runtime_error on failure with detailed message
// NOTE: use the syscall directly, without this wrapper, if failure is allowed
int syscall_helper(std::string_view const file, int const line,
                   int const syscall_ret);

} // namespace build_cxx::os_wrapper

#define BUILD_CXX_SYSCALL_HELPER(fn)                                           \
  ::build_cxx::os_wrapper::syscall_helper(__FILE__, __LINE__, (fn))

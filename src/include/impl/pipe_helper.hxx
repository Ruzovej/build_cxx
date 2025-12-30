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

#include "impl/native_fd_t.hxx"

namespace build_cxx::os_wrapper {

struct pipe_helper {
  friend void swap(pipe_helper &lhs, pipe_helper &rhs) noexcept;

  pipe_helper() noexcept = default;

  void init();

  ~pipe_helper() noexcept;

  pipe_helper(pipe_helper &&rhs) noexcept;
  pipe_helper &operator=(pipe_helper &&rhs) noexcept;

  [[nodiscard]] native_fd_t get_out() const noexcept { return fds[0]; }
  [[nodiscard]] native_fd_t get_in() const noexcept { return fds[1]; }

  void close_out() noexcept;
  void close_in() noexcept;

private:
  pipe_helper(pipe_helper const &) = delete;
  pipe_helper &operator=(pipe_helper const &) = delete;

  native_fd_t fds[2]{invalid_fd, invalid_fd};
};

} // namespace build_cxx::os_wrapper

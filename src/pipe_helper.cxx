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

#include "impl/pipe_helper.hxx"

#include <unistd.h>

#include <iostream>
#include <utility>

#include "impl/syscall_helper.hxx"

namespace build_cxx::os_wrapper {

namespace {

void close_fd(int &fd) noexcept {
  if (fd != invalid_fd) {
    // https://linux.die.net/man/2/close
    if (close(fd) == -1) {
      auto const errno_val{current_errno()};
      switch (errno_val) {
      case EBADF:
        std::cerr << "`close` failed - invalid file descriptor!\n";
        break;
      case EINTR:
        std::cerr << "`close` failed - was interrupted by a signal!\n";
        break;
      case EIO:
        std::cerr << "`close` failed - I/O error occurred!\n";
        break;
      default:
        std::cerr << "`close` failed - by https://linux.die.net/man/2/close "
                     "unspecified `errno` "
                  << errno_val << "!\n";
        break;
      }
    }
    fd = invalid_fd;
  }
}

} // namespace

void swap(pipe_helper &lhs, pipe_helper &rhs) noexcept {
  std::swap(lhs.fds[0], rhs.fds[0]);
  std::swap(lhs.fds[1], rhs.fds[1]);
}

void pipe_helper::init() { BUILD_CXX_SYSCALL_HELPER(pipe(fds)); }

pipe_helper::~pipe_helper() noexcept {
  close_out();
  close_in();
}

pipe_helper::pipe_helper(pipe_helper &&rhs) noexcept
    : fds{std::exchange(rhs.fds[0], invalid_fd),
          std::exchange(rhs.fds[1], invalid_fd)} {}

pipe_helper &pipe_helper::operator=(pipe_helper &&rhs) noexcept {
  if (this != &rhs) {
    pipe_helper tmp{std::move(rhs)};
    swap(*this, tmp);
  }
  return *this;
}

void pipe_helper::close_out() noexcept { close_fd(fds[0]); }

void pipe_helper::close_in() noexcept { close_fd(fds[1]); }

} // namespace build_cxx::os_wrapper

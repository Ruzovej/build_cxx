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
#include <vector>

#include "impl/pipe_helper.hxx"
#include "impl/process_handle_t.hxx"

namespace build_cxx::os_wrapper {

struct exec_path_args {
  enum class state : char { uninitialzied, ready, running, finished };

  explicit exec_path_args(std::string &&aPath,
                          std::vector<std::string> &&aArgs) noexcept
      : path{std::move(aPath)}, args{std::move(aArgs)}, current_state{
                                                            state::ready} {}

  exec_path_args(exec_path_args &&rhs) noexcept;

  ~exec_path_args() noexcept;

  struct states {
    state previous;
    state current;
  };

  [[nodiscard]] states update_and_get_state();

  void send_to_stdin(std::string_view const data);
  void close_stdin();

  [[nodiscard]] std::string_view get_stdout(bool const whole = false) noexcept {
    update_buffer(true);
    return get_buffer(stdout_buffer, stdout_consumed_bytes, whole);
  }
  [[nodiscard]] std::string_view get_stderr(bool const whole = false) noexcept {
    update_buffer(false);
    return get_buffer(stderr_buffer, stderr_consumed_bytes, whole);
  }

  void do_kill();

  [[nodiscard]] long long time_running_ms() const;

  [[nodiscard]] int get_return_code() const;

  // `pid` of the child process
  [[nodiscard]] process_handle_t get_process_handle() const { return handle; }

private:
  exec_path_args(exec_path_args const &rhs) noexcept = delete;
  exec_path_args &operator=(exec_path_args const &rhs) noexcept = delete;
  exec_path_args &operator=(exec_path_args &&rhs) noexcept = delete;

  std::string path;
  std::vector<std::string> args;

  long long time_spawned_ms{-1};
  long long time_finished_ms{-1};

  process_handle_t handle{invalid_process_handle};
  pipe_helper stdin_pipe;
  pipe_helper stdout_pipe;
  pipe_helper stderr_pipe;

  state current_state{state::uninitialzied};

  int return_code{};

  std::string stdout_buffer;
  std::size_t stdout_consumed_bytes{0};
  std::string stderr_buffer;
  std::size_t stderr_consumed_bytes{0};

  void query_status();

  void update_buffer(bool const for_stdout);

  static std::string_view get_buffer(std::string const &buffer,
                                     std::size_t &consumed_bytes,
                                     bool const whole) noexcept {
    if (whole) {
      consumed_bytes = buffer.size();
      return buffer;
    } else {
      std::string_view const ret{buffer.data() + consumed_bytes,
                                 buffer.size() - consumed_bytes};
      consumed_bytes = buffer.size();
      return ret;
    }
  }
};

} // namespace build_cxx::os_wrapper

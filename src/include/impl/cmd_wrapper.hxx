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

namespace exec_cmd
{
/**
 * @short
 * very simple bash command(s) runner - limitations:
 *	- input provided is static
 *	- it is executed synchronously and parent process waits for its child termination or for timeout
 *	- stdout and stderr are collected only after command finishes/timeouts
 *  - pipes are opened even when given in/out-put parameters are empty/unset
 * 
 * @param timeout_ms Wait for the child process at most this long; pass `-1` for blocking indefinitely
 * @param command The command to execute, as a string, i.e. `ls -lAh`
 * @param input The input to pass to the command, as a string
 * @param output A pointer to a string to capture the standard output of the
 *     command. The string is resized to hold the
 *     complete output. Pass `nullptr` to ignore it
 * @param error A pointer to a string to capture the standard error of the
 *     command. The string is resized to hold the
 *     complete error. Pass `nullptr` to ignore it; may be same as `output` to redirect it to single stream
 * @param supress_logging disables logging on current (parent) process about state/progress
 * @return The exit status of the command.
 * @throws `std::runtime_error` if either any syscall fails or if the child process doesn't terminate in given time window
*/
int bash(
  int const              timeout_ms,
  std::string_view const command,
  std::string_view const input           = "",
  std::string *const     output          = nullptr,
  std::string *const     error           = nullptr,
  bool const             supress_logging = true);

}  // namespace exec_cmd

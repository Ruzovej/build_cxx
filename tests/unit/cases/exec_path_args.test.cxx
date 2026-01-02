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

#include <csignal>

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <new>
#include <optional>

#include <doctest/doctest.h>
#include <exec_path_args/exec_path_args.hxx>

namespace build_cxx::os_wrapper {
namespace {

std::string space_to_newline(std::string str) {
  std::transform(str.begin(), str.end(), str.begin(),
                 [](char c) { return c == ' ' ? '\n' : c; });
  return str;
}

// TODO delete this later ...
TEST_CASE("exec_path_args") {
  using exec_path_args::os_wrapper::exec_path_args;
  
  SUBCASE("simple shell command") {
    static auto constexpr shell_cmd = [](std::string &&cmd_str) {
      exec_path_args cmd{"/usr/bin/env", {"sh", "-c", std::move(cmd_str)}};

      REQUIRE_FALSE(cmd.manages_process());
      REQUIRE_FALSE(cmd.is_finished());
      return cmd;
    };

    SUBCASE("happy path - nonblocking") {
      exec_path_args cmd{
          shell_cmd("printf \"Hello stdout!\"; printf \"Hello stderr!\" 1>&2")};

      {
        exec_path_args::states state;
        REQUIRE_NOTHROW(state = cmd.update_and_get_state());
        REQUIRE_EQ(state.previous, exec_path_args::state::ready);
        REQUIRE_EQ(state.current, exec_path_args::state::running);
        REQUIRE(cmd.manages_process());
      }

      {
        exec_path_args::states state;
        REQUIRE_NOTHROW(state = cmd.update_and_get_state());
        REQUIRE_EQ(state.previous, exec_path_args::state::running);
        WARN_EQ(state.current, exec_path_args::state::running);
      }
      // potential for flakiness -> hence there are `WARN`s on this "boundary"
      {
        exec_path_args::state prev_state;
        REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
        WARN_EQ(prev_state, exec_path_args::state::running);
      }

      // querying again won't change anything
      for (int i{0}; i < 2; ++i) {
        {
          exec_path_args::states state;
          REQUIRE_NOTHROW(state = cmd.update_and_get_state());
          REQUIRE_EQ(state.previous, exec_path_args::state::finished);
          REQUIRE_EQ(state.current, exec_path_args::state::finished);
        }

        REQUIRE_EQ(cmd.read_stdout(true), "Hello stdout!");
        REQUIRE_EQ(cmd.read_stdout(false), "");

        REQUIRE_EQ(cmd.read_stderr(true), "Hello stderr!");
        REQUIRE_EQ(cmd.read_stderr(false), "");

        REQUIRE_EQ(cmd.get_return_code(), EXIT_SUCCESS);
        REQUIRE_LT(0.0, cmd.time_running_ms());
      }

      // consume outputs ...
      REQUIRE_EQ(cmd.get_stdout(), "Hello stdout!");
      REQUIRE_EQ(cmd.get_stdout(), "");
      REQUIRE_EQ(cmd.read_stdout(true), "");

      REQUIRE_EQ(cmd.get_stderr(), "Hello stderr!");
      REQUIRE_EQ(cmd.get_stderr(), "");
      REQUIRE_EQ(cmd.read_stderr(true), "");
    }

    SUBCASE("happy path - blocking") {
      exec_path_args cmd{
          shell_cmd("printf \"Hello stdout!\"; printf \"Hello stderr!\" 1>&2")};

      {
        exec_path_args::state prev_state;
        REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
        REQUIRE_EQ(prev_state, exec_path_args::state::ready);
        REQUIRE(cmd.manages_process());
      }

      REQUIRE_EQ(cmd.read_stdout(true), "Hello stdout!");
      REQUIRE_EQ(cmd.read_stderr(true), "Hello stderr!");
      REQUIRE_EQ(cmd.get_return_code(), EXIT_SUCCESS);
      REQUIRE_LT(0.0, cmd.time_running_ms());
    }

    SUBCASE("non-zero return code") {
      static auto constexpr expected_val{42};
      exec_path_args cmd{shell_cmd("exit " + std::to_string(expected_val))};

      {
        exec_path_args::state prev_state;
        REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
        REQUIRE_EQ(prev_state, exec_path_args::state::ready);
        REQUIRE(cmd.manages_process());
      }

      REQUIRE_EQ(cmd.read_stdout(true), "");
      REQUIRE_EQ(cmd.read_stderr(true), "");
      REQUIRE_EQ(cmd.get_return_code(), expected_val);
      REQUIRE_LT(0.0, cmd.time_running_ms());
    }

    SUBCASE("not waiting for it") {
      // in `shell`, `sleep` accepts seconds
      exec_path_args cmd{shell_cmd("sleep 1; printf \"Done!\"")};

      {
        exec_path_args::states state;
        REQUIRE_NOTHROW(state = cmd.update_and_get_state());
        REQUIRE_EQ(state.previous, exec_path_args::state::ready);
        REQUIRE_EQ(state.current, exec_path_args::state::running);
        REQUIRE(cmd.manages_process());
      }

      REQUIRE_NOTHROW(cmd.do_kill()); // it should get there safely, way sooner
                                      // than 1 second after forking ...
      REQUIRE(cmd.is_finished());

      SUBCASE("explicitly updating the status after kill") {
        exec_path_args::state prev_state;
        REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
        REQUIRE_EQ(prev_state, exec_path_args::state::finished);
      }

      SUBCASE("not updating the status after kill") {
        // is expetected to be equivalent ...
      }

      REQUIRE(cmd.is_finished());
      REQUIRE_EQ(cmd.read_stdout(true), "");
      REQUIRE_EQ(cmd.read_stderr(true), "");
      REQUIRE_NE(cmd.get_return_code(), EXIT_SUCCESS);
      REQUIRE_LT(0.0, cmd.time_running_ms());
    }

    SUBCASE("various operations") {
      SUBCASE("move opearations") {
        std::optional<exec_path_args> cmd{shell_cmd("echo Hello")};
        REQUIRE_FALSE(cmd->manages_process());

        SUBCASE("not spawned yet") {
          SUBCASE("ctor") {
            exec_path_args cmd2{std::move(*cmd)};
            REQUIRE_FALSE(cmd2.manages_process());
            REQUIRE_FALSE(cmd->manages_process());
          }

          SUBCASE("assignment") {
            exec_path_args cmd2;
            auto const ptr{std::launder(&cmd2)}; // prevent optimizations ...
            *ptr = std::move(*cmd);
            REQUIRE_FALSE(ptr->manages_process());
            REQUIRE_FALSE(cmd->manages_process());
          }
        }

        SUBCASE("just spawned") {
          {
            exec_path_args::states state;
            REQUIRE_NOTHROW(state = cmd->update_and_get_state());
            REQUIRE_EQ(state.previous, exec_path_args::state::ready);
            REQUIRE_EQ(state.current, exec_path_args::state::running);
            REQUIRE(cmd->manages_process());
          }

          exec_path_args cmd2{std::move(*cmd)};

          SUBCASE("without imediate reset") {}

          SUBCASE("with imediate reset") { cmd.reset(); }

          REQUIRE(cmd2.manages_process());
          REQUIRE_FALSE(cmd->manages_process());

          {
            exec_path_args::state prev_state;
            REQUIRE_THROWS(prev_state = cmd->finish_and_get_prev_state());
            REQUIRE_NOTHROW(prev_state = cmd2.finish_and_get_prev_state());
            REQUIRE_EQ(prev_state, exec_path_args::state::running);
          }

          REQUIRE_EQ(cmd2.read_stdout(true), "Hello\n");
          REQUIRE_EQ(cmd2.read_stderr(true), "");
          REQUIRE_EQ(cmd2.get_return_code(), EXIT_SUCCESS);
          REQUIRE_LT(0.0, cmd2.time_running_ms());
        }

        SUBCASE("after finishing") {
          {
            exec_path_args::state prev_state;
            REQUIRE_NOTHROW(prev_state = cmd->finish_and_get_prev_state());
            REQUIRE_EQ(prev_state, exec_path_args::state::ready);
          }

          exec_path_args cmd2{std::move(*cmd)};

          SUBCASE("without imediate reset") {}

          SUBCASE("with imediate reset") { cmd.reset(); }

          REQUIRE(cmd2.manages_process());
          REQUIRE_FALSE(cmd->manages_process());

          REQUIRE_EQ(cmd2.read_stdout(true), "Hello\n");
          REQUIRE_EQ(cmd2.read_stderr(true), "");
          REQUIRE_EQ(cmd2.get_return_code(), EXIT_SUCCESS);
          REQUIRE_LT(0.0, cmd2.time_running_ms());
        }

        cmd.reset();
      }

      SUBCASE("operations on un-started process") {
        exec_path_args cmd_default_constructed{};

        exec_path_args cmd_moved_from{
            shell_cmd("whatever ... won't be started for the purpose of the "
                      "test case ...")};

        exec_path_args cmd_move_constructed{std::move(cmd_default_constructed)};

        SUBCASE("state checks") {
          REQUIRE_FALSE(cmd_default_constructed.manages_process());
          REQUIRE_FALSE(cmd_default_constructed.is_finished());

          REQUIRE_FALSE(cmd_moved_from.manages_process());
          REQUIRE_FALSE(cmd_moved_from.is_finished());

          REQUIRE_FALSE(cmd_move_constructed.manages_process());
          REQUIRE_FALSE(cmd_move_constructed.is_finished());
        }

        SUBCASE("stdin operations") {
          REQUIRE_THROWS(cmd_default_constructed.send_to_stdin("data"));
          REQUIRE_THROWS(cmd_default_constructed.close_stdin());

          REQUIRE_THROWS(cmd_moved_from.send_to_stdin("data"));
          REQUIRE_THROWS(cmd_moved_from.close_stdin());

          REQUIRE_THROWS(cmd_move_constructed.send_to_stdin("data"));
          REQUIRE_THROWS(cmd_move_constructed.close_stdin());
        }

        SUBCASE("stdout & stderr operations") {
          [[maybe_unused]] std::string_view str;

          REQUIRE_THROWS(str = cmd_default_constructed.read_stdout());
          REQUIRE_THROWS(str = cmd_default_constructed.read_stderr());

          REQUIRE_THROWS(str = cmd_moved_from.read_stdout());
          REQUIRE_THROWS(str = cmd_moved_from.read_stderr());

          REQUIRE_THROWS(str = cmd_move_constructed.read_stdout());
          REQUIRE_THROWS(str = cmd_move_constructed.read_stderr());
        }

        SUBCASE("termination related") {
          [[maybe_unused]] int ret_code;
          [[maybe_unused]] double time_ms;

          REQUIRE_THROWS(ret_code = cmd_default_constructed.get_return_code());
          REQUIRE_THROWS(time_ms = cmd_default_constructed.time_running_ms());

          REQUIRE_THROWS(ret_code = cmd_moved_from.get_return_code());
          REQUIRE_THROWS(time_ms = cmd_moved_from.time_running_ms());

          REQUIRE_THROWS(ret_code = cmd_move_constructed.get_return_code());
          REQUIRE_THROWS(time_ms = cmd_move_constructed.time_running_ms());

          // this one has checks inside, because it's used in d-tor ...:
          REQUIRE_NOTHROW(cmd_default_constructed.do_kill());

          REQUIRE_NOTHROW(cmd_moved_from.do_kill());

          REQUIRE_NOTHROW(cmd_move_constructed.do_kill());
        }
      }

      SUBCASE("swap") {
        exec_path_args cmd1{shell_cmd("echo cmd1; exit 1")};
        exec_path_args cmd2{shell_cmd("echo cmd2; exit 2")};

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd1.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::ready);
        }

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd2.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::ready);
        }

        swap(cmd1, cmd2);

        REQUIRE_EQ(cmd1.read_stdout(true), "cmd2\n");
        REQUIRE_EQ(cmd1.get_return_code(), 2);
        REQUIRE_LT(0.0, cmd1.time_running_ms());

        REQUIRE_EQ(cmd2.read_stdout(true), "cmd1\n");
        REQUIRE_EQ(cmd2.get_return_code(), 1);
        REQUIRE_LT(0.0, cmd2.time_running_ms());
      }
    }
  }
}

} // namespace
} // namespace build_cxx::os_wrapper

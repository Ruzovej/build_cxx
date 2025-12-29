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

#include "impl/exec_path_args.hxx"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <optional>

#include <doctest/doctest.h>

#include <ips/ips.hxx>

namespace build_cxx::os_wrapper {
namespace {

std::string space_to_newline(std::string str) {
  std::transform(str.begin(), str.end(), str.begin(),
                 [](char c) { return c == ' ' ? '\n' : c; });
  return str;
}

TEST_CASE("exec_path_args") {
  SUBCASE("simple bash command") {
    static auto constexpr bash_cmd = [](std::string &&cmd_str) {
      return exec_path_args{"/usr/bin/env",
                            {"bash", "bash", "-c", std::move(cmd_str)}};
    };

    SUBCASE("happy path - nonblocking") {
      exec_path_args cmd{
          bash_cmd("printf \"Hello stdout!\"; printf \"Hello stderr!\" 1>&2")};

      {
        exec_path_args::states state;
        REQUIRE_NOTHROW(state = cmd.update_and_get_state());
        REQUIRE_EQ(state.previous, exec_path_args::state::ready);
        REQUIRE_EQ(state.current, exec_path_args::state::running);
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

        REQUIRE_EQ(cmd.get_stdout(true), "Hello stdout!");
        REQUIRE_EQ(cmd.get_stdout(false), "");
        REQUIRE_EQ(cmd.get_stderr(true), "Hello stderr!");
        REQUIRE_EQ(cmd.get_stderr(false), "");
        REQUIRE_EQ(cmd.get_return_code(), EXIT_SUCCESS);
      }
    }

    SUBCASE("happy path - blocking") {
      exec_path_args cmd{
          bash_cmd("printf \"Hello stdout!\"; printf \"Hello stderr!\" 1>&2")};

      {
        exec_path_args::state prev_state;
        REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
        REQUIRE_EQ(prev_state, exec_path_args::state::ready);
      }

      REQUIRE_EQ(cmd.get_stdout(true), "Hello stdout!");
      REQUIRE_EQ(cmd.get_stderr(true), "Hello stderr!");
      REQUIRE_EQ(cmd.get_return_code(), EXIT_SUCCESS);
    }

    SUBCASE("non-zero return code") {
      static auto constexpr expected_val{42};

      exec_path_args cmd{bash_cmd("exit " + std::to_string(expected_val))};

      {
        exec_path_args::state prev_state;
        REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
        REQUIRE_EQ(prev_state, exec_path_args::state::ready);
      }

      REQUIRE_EQ(cmd.get_stdout(true), "");
      REQUIRE_EQ(cmd.get_stderr(true), "");
      REQUIRE_EQ(cmd.get_return_code(), expected_val);
    }

    SUBCASE("not waiting for it") {
      // in `bash`, `sleep` accepts seconds
      exec_path_args cmd{bash_cmd("sleep 1; printf \"Done!\"")};

      {
        exec_path_args::states state;
        REQUIRE_NOTHROW(state = cmd.update_and_get_state());
        REQUIRE_EQ(state.previous, exec_path_args::state::ready);
        REQUIRE_EQ(state.current, exec_path_args::state::running);
      }

      cmd.do_kill();

      SUBCASE("re-checking the status after kill") {
        exec_path_args::states state;
        REQUIRE_NOTHROW(state = cmd.update_and_get_state());
        REQUIRE_EQ(state.previous, exec_path_args::state::finished);
        REQUIRE_EQ(state.current, exec_path_args::state::finished);
      }

      SUBCASE("not checking the status after kill") {
        // should be equivalent ...
      }

      REQUIRE_EQ(cmd.get_stdout(true), "");
      REQUIRE_EQ(cmd.get_stderr(true), "");
      REQUIRE_NE(cmd.get_return_code(), EXIT_SUCCESS);
    }
  }

  SUBCASE("some_cli_app") {
    auto const some_cli_app_path{std::filesystem::current_path() /
                                 "build/tests/unit/some_cli_app"};

    REQUIRE(std::filesystem::exists(some_cli_app_path));
    REQUIRE(std::filesystem::is_regular_file(some_cli_app_path));

    auto const some_cli_app = [&](auto &&...args) {
      // TODO fix this first version ...:
      // return exec_path_args{some_cli_app_path.string(),
      //                      {std::forward<decltype(args)>(args)...}};
      // and remove this one:
      return exec_path_args{"/usr/bin/env",
                            {"bash", some_cli_app_path.string(),
                             std::forward<decltype(args)>(args)...}};
    };

    SUBCASE("basic functionality, without synchronization") {
      SUBCASE("exit") {
        auto const exit_code{"15"};
        exec_path_args cmd{some_cli_app("--exit", exit_code)};

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::ready);
        }

        REQUIRE_EQ(cmd.get_stderr(true), "");
        REQUIRE_EQ(cmd.get_stdout(true), "");
        REQUIRE_EQ(cmd.get_return_code(), std::stoi(exit_code));
      }

      SUBCASE("exit is always last action") {
        auto const exit_code{"13"};
        exec_path_args cmd{some_cli_app("--exit", exit_code,            // ...
                                        "--stdout", "won't be printed", // ...
                                        "--sync"                        // ...
                                        )};

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::ready);
        }

        REQUIRE_EQ(cmd.get_stderr(true), "");
        REQUIRE_EQ(cmd.get_stdout(true), "");
        REQUIRE_EQ(cmd.get_return_code(), std::stoi(exit_code));
      }

      SUBCASE("sleep") {
        exec_path_args cmd{some_cli_app("--sleep", "1")};

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::ready);
        }

        REQUIRE_EQ(cmd.get_stderr(true), "");
        REQUIRE_EQ(cmd.get_stdout(true), "");
        REQUIRE_EQ(cmd.get_return_code(), EXIT_SUCCESS);
      }

      SUBCASE("stdout") {
        auto const text{"Hello!"};
        exec_path_args cmd{some_cli_app("--stdout", text)};

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::ready);
        }

        REQUIRE_EQ(cmd.get_stderr(true), "");
        REQUIRE_EQ(cmd.get_stdout(true), std::string{text} + "\n");
        REQUIRE_EQ(cmd.get_stdout(false), ""); // consumed ...
        REQUIRE_EQ(cmd.get_stdout(true),
                   std::string{text} + "\n");  // still reachable
        REQUIRE_EQ(cmd.get_stdout(false), ""); // still consumed ...
        REQUIRE_EQ(cmd.get_return_code(), EXIT_SUCCESS);
      }

      SUBCASE("stderr") {
        auto const text{"Hello!"};
        exec_path_args cmd{some_cli_app("--stderr", text)};

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::ready);
        }

        REQUIRE_EQ(cmd.get_stderr(true), std::string{text} + "\n");
        REQUIRE_EQ(cmd.get_stderr(false), "");                      // ditto ...
        REQUIRE_EQ(cmd.get_stderr(true), std::string{text} + "\n"); // ...
        REQUIRE_EQ(cmd.get_stderr(false), "");                      // ...
        REQUIRE_EQ(cmd.get_stdout(true), "");
        REQUIRE_EQ(cmd.get_return_code(), EXIT_SUCCESS);
      }

      SUBCASE("stdout & stderr") {
        auto const text{"Hello!"};
        exec_path_args cmd{some_cli_app("--stdout", text, "--stderr", text)};

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::ready);
        }

        REQUIRE_EQ(cmd.get_stdout(true), std::string{text} + "\n");
        REQUIRE_EQ(cmd.get_stdout(false), "");
        REQUIRE_EQ(cmd.get_stdout(true), std::string{text} + "\n");
        REQUIRE_EQ(cmd.get_stdout(false), "");
        REQUIRE_EQ(cmd.get_stderr(true), std::string{text} + "\n");
        REQUIRE_EQ(cmd.get_stderr(false), "");
        REQUIRE_EQ(cmd.get_stderr(true), std::string{text} + "\n");
        REQUIRE_EQ(cmd.get_stderr(false), "");
        REQUIRE_EQ(cmd.get_return_code(), EXIT_SUCCESS);
      }

      SUBCASE("echo") {
        exec_path_args cmd{some_cli_app("--echo", "1")};

        {
          exec_path_args::states state;
          REQUIRE_NOTHROW(state = cmd.update_and_get_state());
          REQUIRE_EQ(state.previous, exec_path_args::state::ready);
          REQUIRE_EQ(state.current, exec_path_args::state::running);
        }

        auto const text{"Hello! "}; // see the ' ' at the end
        REQUIRE_NOTHROW(cmd.send_to_stdin(text));

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::running);
        }

        REQUIRE_EQ(cmd.get_stderr(true), "");
        REQUIRE_EQ(cmd.get_stdout(true), space_to_newline(text));
        REQUIRE_EQ(cmd.get_return_code(), EXIT_SUCCESS);
      }
    }

    SUBCASE("synchronized") {
      auto const sem_name{"/some_cli_app_shared_sem"};

      auto const some_cli_app_synced = [&](auto &&...args) {
        return some_cli_app("--sem-name", sem_name,
                            std::forward<decltype(args)>(args)...);
      };

      std::optional<ips> my_sem;
      REQUIRE_NOTHROW(my_sem.emplace(sem_name, true));

      SUBCASE("basic functionality: sync") {
        auto const exit_code{"10"};
        exec_path_args cmd{some_cli_app_synced("--sync", "--exit", exit_code)};

        {
          exec_path_args::states state;
          REQUIRE_NOTHROW(state = cmd.update_and_get_state());
          REQUIRE_EQ(state.previous, exec_path_args::state::ready);
          REQUIRE_EQ(state.current, exec_path_args::state::running);
        }

        REQUIRE(my_sem->wait_and_notify(40));

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::running);
        }

        REQUIRE_EQ(cmd.get_stderr(true), "");
        REQUIRE_EQ(cmd.get_stdout(true), "");
        REQUIRE_EQ(cmd.get_return_code(), std::stoi(exit_code));
      }

      SUBCASE("happy path") {
        auto const to_stderr{"How is it going?"};
        auto const to_stdout{"Fine, thank You!"};
        auto const exit_code{"42"};

        exec_path_args cmd{some_cli_app_synced("--stderr", to_stderr, // 1
                                               "--stdout", to_stdout, // 2
                                               "--sleep", "5",        // 3
                                               "--sync",              // ...
                                               "--echo", "3",         // 4
                                               "--sync",              // ...
                                               "--exit", exit_code    // 5
                                               )};

        {
          exec_path_args::states state;
          REQUIRE_NOTHROW(state = cmd.update_and_get_state());
          REQUIRE_EQ(state.previous, exec_path_args::state::ready);
          REQUIRE_EQ(state.current, exec_path_args::state::running);
        }

        REQUIRE(my_sem->wait_and_notify(40));

        REQUIRE_EQ(cmd.get_stdout(true), std::string{to_stdout} + '\n');
        REQUIRE_EQ(cmd.get_stderr(true), std::string{to_stderr} + '\n');

        {
          exec_path_args::states state;
          REQUIRE_NOTHROW(state = cmd.update_and_get_state());
          REQUIRE_EQ(state.previous, exec_path_args::state::running);
          REQUIRE_EQ(state.current, exec_path_args::state::running);
        }

        // pay attention to the space at the end ...:
        auto const expected_echo_input{"const std::string_view data "};
        REQUIRE_NOTHROW(cmd.send_to_stdin(expected_echo_input));

        REQUIRE(my_sem->wait_and_notify(40));

        REQUIRE_EQ(cmd.get_stdout(false),
                   space_to_newline(expected_echo_input));
        REQUIRE_EQ(cmd.get_stdout(false), ""); // consumed in previous line
        REQUIRE_EQ(cmd.get_stderr(false), "");

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::running);
        }

        REQUIRE_EQ(cmd.get_return_code(), std::stoi(exit_code));
      }
    }
  }
}

} // namespace
} // namespace build_cxx::os_wrapper

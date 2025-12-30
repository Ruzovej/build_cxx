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

#include <csignal>

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
      exec_path_args cmd{"/usr/bin/env",
                         {"bash", "bash", "-c", std::move(cmd_str)}};
      REQUIRE_FALSE(cmd.manages_process());
      REQUIRE_FALSE(cmd.is_finished());
      return cmd;
    };

    SUBCASE("happy path - nonblocking") {
      exec_path_args cmd{
          bash_cmd("printf \"Hello stdout!\"; printf \"Hello stderr!\" 1>&2")};

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
      }
    }

    SUBCASE("happy path - blocking") {
      exec_path_args cmd{
          bash_cmd("printf \"Hello stdout!\"; printf \"Hello stderr!\" 1>&2")};

      {
        exec_path_args::state prev_state;
        REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
        REQUIRE_EQ(prev_state, exec_path_args::state::ready);
        REQUIRE(cmd.manages_process());
      }

      REQUIRE_EQ(cmd.read_stdout(true), "Hello stdout!");
      REQUIRE_EQ(cmd.read_stderr(true), "Hello stderr!");
      REQUIRE_EQ(cmd.get_return_code(), EXIT_SUCCESS);
    }

    SUBCASE("non-zero return code") {
      static auto constexpr expected_val{42};
      exec_path_args cmd{bash_cmd("exit " + std::to_string(expected_val))};

      {
        exec_path_args::state prev_state;
        REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
        REQUIRE_EQ(prev_state, exec_path_args::state::ready);
        REQUIRE(cmd.manages_process());
      }

      REQUIRE_EQ(cmd.read_stdout(true), "");
      REQUIRE_EQ(cmd.read_stderr(true), "");
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
        REQUIRE(cmd.manages_process());
      }

      REQUIRE_NOTHROW(cmd.do_kill());
      REQUIRE(cmd.is_finished());

      SUBCASE("re-checking the status after kill") {
        exec_path_args::state prev_state;
        REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
        REQUIRE_EQ(prev_state, exec_path_args::state::finished);
      }

      SUBCASE("not checking the status after kill") {
        // should be equivalent ...
      }

      REQUIRE_EQ(cmd.read_stdout(true), "");
      REQUIRE_EQ(cmd.read_stderr(true), "");
      REQUIRE_NE(cmd.get_return_code(), EXIT_SUCCESS);
    }

    SUBCASE("various operations") {
      SUBCASE("move ctor") {
        std::optional<exec_path_args> cmd{bash_cmd("echo Hello")};
        REQUIRE_FALSE(cmd->manages_process());

        SUBCASE("not spawned yet") {
          exec_path_args cmd2{std::move(*cmd)};
          REQUIRE_FALSE(cmd2.manages_process());
          REQUIRE_FALSE(cmd->manages_process());
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

          SUBCASE("with imediate reset") { REQUIRE_NOTHROW(cmd.reset()); }

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
        }

        SUBCASE("after finishing") {
          {
            exec_path_args::state prev_state;
            REQUIRE_NOTHROW(prev_state = cmd->finish_and_get_prev_state());
            REQUIRE_EQ(prev_state, exec_path_args::state::ready);
          }

          exec_path_args cmd2{std::move(*cmd)};

          SUBCASE("without imediate reset") {}

          SUBCASE("with imediate reset") { REQUIRE_NOTHROW(cmd.reset()); }

          REQUIRE(cmd2.manages_process());
          REQUIRE_FALSE(cmd->manages_process());

          REQUIRE_EQ(cmd2.read_stdout(true), "Hello\n");
          REQUIRE_EQ(cmd2.read_stderr(true), "");
          REQUIRE_EQ(cmd2.get_return_code(), EXIT_SUCCESS);
        }

        REQUIRE_NOTHROW(cmd.reset());
      }

      SUBCASE("operations on un-started process") {
        exec_path_args cmd1{bash_cmd("whatever ... won't be started for the "
                                     "purpose of the test case ...")};
        exec_path_args cmd2{std::move(cmd1)};

        SUBCASE("state checks") {
          REQUIRE_FALSE(cmd1.manages_process());
          REQUIRE_FALSE(cmd1.is_finished());

          REQUIRE_FALSE(cmd2.manages_process());
          REQUIRE_FALSE(cmd2.is_finished());
        }

        SUBCASE("stdin operations") {
          REQUIRE_THROWS(cmd1.send_to_stdin("data"));
          REQUIRE_THROWS(cmd1.close_stdin());

          REQUIRE_THROWS(cmd1.send_to_stdin("data"));
          REQUIRE_THROWS(cmd1.close_stdin());

          REQUIRE_THROWS(cmd2.send_to_stdin("data"));
          REQUIRE_THROWS(cmd2.close_stdin());
        }

        SUBCASE("stdout & stderr operations") {
          [[maybe_unused]] std::string_view str;

          REQUIRE_THROWS(str = cmd1.read_stdout());
          REQUIRE_THROWS(str = cmd1.read_stderr());

          REQUIRE_THROWS(str = cmd2.read_stdout());
          REQUIRE_THROWS(str = cmd2.read_stderr());
        }

        SUBCASE("termination related") {
          [[maybe_unused]] int ret_code;
          [[maybe_unused]] long long time_ms;

          REQUIRE_THROWS(ret_code = cmd1.get_return_code());
          REQUIRE_THROWS(time_ms = cmd1.time_running_ms());

          REQUIRE_THROWS(ret_code = cmd2.get_return_code());
          REQUIRE_THROWS(time_ms = cmd2.time_running_ms());

          // this one has checks inside, because it's used in d-tor ...:
          REQUIRE_NOTHROW(cmd1.do_kill());

          REQUIRE_NOTHROW(cmd2.do_kill());
        }
      }
    }
  }

  SUBCASE("some_cli_app") {
    auto const some_cli_app_path{std::filesystem::current_path() /
                                 "build/tests/unit/some_cli_app"};

    REQUIRE(std::filesystem::exists(some_cli_app_path));
    REQUIRE(std::filesystem::is_regular_file(some_cli_app_path));

    auto const some_cli_app = [&](auto &&...args) {
      // TODO fix this first version ...:
      // exec_path_args cmd{some_cli_app_path.string(),
      //                    {std::forward<decltype(args)>(args)...}};
      // and remove this one:
      exec_path_args cmd{"/usr/bin/env",
                         {"bash", some_cli_app_path.string(),
                          std::forward<decltype(args)>(args)...}};
      REQUIRE_FALSE(cmd.manages_process());
      REQUIRE_FALSE(cmd.is_finished());
      return cmd;
    };

    SUBCASE("basic functionality, without synchronization") {
      SUBCASE("exit") {
        auto const exit_code{"11"};
        exec_path_args cmd{some_cli_app("--exit", exit_code)};

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::ready);
          REQUIRE(cmd.manages_process());
        }

        REQUIRE_EQ(cmd.read_stderr(true), "");
        REQUIRE_EQ(cmd.read_stdout(true), "");
        REQUIRE_EQ(cmd.get_return_code(), std::stoi(exit_code));
      }

      SUBCASE("exit is always last action") {
        auto const exit_code{"12"};
        exec_path_args cmd{some_cli_app("--exit", exit_code,            // ...
                                        "--stdout", "won't be printed", // ...
                                        "--sync"                        // ...
                                        )};

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::ready);
          REQUIRE(cmd.manages_process());
        }

        REQUIRE_EQ(cmd.read_stderr(true), "");
        REQUIRE_EQ(cmd.read_stdout(true), "");
        REQUIRE_EQ(cmd.get_return_code(), std::stoi(exit_code));
      }

      SUBCASE("sleep") {
        exec_path_args cmd{some_cli_app("--sleep", "1")};

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::ready);
          REQUIRE(cmd.manages_process());
        }

        REQUIRE_EQ(cmd.read_stderr(true), "");
        REQUIRE_EQ(cmd.read_stdout(true), "");
        REQUIRE_EQ(cmd.get_return_code(), EXIT_SUCCESS);
      }

      SUBCASE("stdout") {
        auto const text{"Hello!"};
        exec_path_args cmd{some_cli_app("--stdout", text)};

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::ready);
          REQUIRE(cmd.manages_process());
        }

        REQUIRE_EQ(cmd.read_stderr(true), "");
        REQUIRE_EQ(cmd.read_stdout(true), std::string{text} + "\n");
        REQUIRE_EQ(cmd.read_stdout(false), ""); // consumed ...
        REQUIRE_EQ(cmd.read_stdout(true),
                   std::string{text} + "\n");   // still reachable
        REQUIRE_EQ(cmd.read_stdout(false), ""); // still consumed ...
        REQUIRE_EQ(cmd.get_return_code(), EXIT_SUCCESS);
      }

      SUBCASE("sleep interupted") {
        exec_path_args cmd{some_cli_app("--sleep", "1000",             // ...
                                        "--stdout", "won't be printed" // ...
                                        )};

        {
          exec_path_args::states state;
          REQUIRE_NOTHROW(state = cmd.update_and_get_state());
          REQUIRE_EQ(state.previous, exec_path_args::state::ready);
          REQUIRE_EQ(state.current, exec_path_args::state::running);
          REQUIRE(cmd.manages_process());
        }

        REQUIRE_NOTHROW(cmd.do_kill());
        REQUIRE(cmd.is_finished());

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::finished);
        }

        REQUIRE_EQ(cmd.read_stderr(true), "");
        REQUIRE_EQ(cmd.read_stdout(true), "");
        REQUIRE_EQ(cmd.get_return_code(), SIGKILL);
      }

      SUBCASE("stderr") {
        auto const text{"Hello!"};
        exec_path_args cmd{some_cli_app("--stderr", text)};

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::ready);
          REQUIRE(cmd.manages_process());
        }

        REQUIRE_EQ(cmd.read_stderr(true), std::string{text} + "\n");
        REQUIRE_EQ(cmd.read_stderr(false), ""); // ditto ...
        REQUIRE_EQ(cmd.read_stderr(true), std::string{text} + "\n"); // ...
        REQUIRE_EQ(cmd.read_stderr(false), "");                      // ...
        REQUIRE_EQ(cmd.read_stdout(true), "");
        REQUIRE_EQ(cmd.get_return_code(), EXIT_SUCCESS);
      }

      SUBCASE("invalid argument") {
        auto const exit_code{"13"};
        exec_path_args cmd{some_cli_app("--exit", exit_code, "--invalid")};

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::ready);
          REQUIRE(cmd.manages_process());
        }

        REQUIRE_EQ(cmd.read_stderr(true),
                   "some_cli_app caught `input_exception`: Unknown argument: "
                   "--invalid\n");
        REQUIRE_EQ(cmd.read_stdout(true), "");
        REQUIRE_NE(cmd.get_return_code(), std::stoi(exit_code));
        REQUIRE_EQ(cmd.get_return_code(), EXIT_FAILURE);
      }

      SUBCASE("stdout & stderr") {
        auto const text{"Hello!"};
        exec_path_args cmd{some_cli_app("--stdout", text, "--stderr", text)};

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::ready);
          REQUIRE(cmd.manages_process());
        }

        REQUIRE_EQ(cmd.read_stdout(true), std::string{text} + "\n");
        REQUIRE_EQ(cmd.read_stdout(false), "");
        REQUIRE_EQ(cmd.read_stdout(true), std::string{text} + "\n");
        REQUIRE_EQ(cmd.read_stdout(false), "");
        REQUIRE_EQ(cmd.read_stderr(true), std::string{text} + "\n");
        REQUIRE_EQ(cmd.read_stderr(false), "");
        REQUIRE_EQ(cmd.read_stderr(true), std::string{text} + "\n");
        REQUIRE_EQ(cmd.read_stderr(false), "");
        REQUIRE_EQ(cmd.get_return_code(), EXIT_SUCCESS);
      }

      SUBCASE("echo 1") {
        exec_path_args cmd{some_cli_app("--echo", "1")};

        {
          exec_path_args::states state;
          REQUIRE_NOTHROW(state = cmd.update_and_get_state());
          REQUIRE_EQ(state.previous, exec_path_args::state::ready);
          REQUIRE_EQ(state.current, exec_path_args::state::running);
          REQUIRE(cmd.manages_process());
        }

        auto const text{"Hello! "}; // see the ' ' at the end
        REQUIRE_NOTHROW(cmd.send_to_stdin(text));

        // close it
        REQUIRE_NOTHROW(cmd.close_stdin());
        // exceptions will indicate that nothing more can be passed to it
        REQUIRE_THROWS(cmd.close_stdin());
        REQUIRE_THROWS(cmd.send_to_stdin(text));

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::running);
        }

        REQUIRE_EQ(cmd.read_stderr(true), "");
        REQUIRE_EQ(cmd.read_stdout(true), space_to_newline(text));
        REQUIRE_EQ(cmd.get_return_code(), EXIT_SUCCESS);
      }

      SUBCASE("echo 2") {
        exec_path_args cmd{some_cli_app("--echo", "1")};

        {
          exec_path_args::states state;
          REQUIRE_NOTHROW(state = cmd.update_and_get_state());
          REQUIRE_EQ(state.previous, exec_path_args::state::ready);
          REQUIRE_EQ(state.current, exec_path_args::state::running);
          REQUIRE(cmd.manages_process());
        }

        auto const text{"Hello!"}; // NO ' ' at the end
        REQUIRE_NOTHROW(cmd.send_to_stdin(text));
        REQUIRE_NOTHROW(cmd.close_stdin());

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::running);
        }

        REQUIRE_EQ(cmd.read_stderr(true), "");
        REQUIRE_EQ(cmd.read_stdout(true), space_to_newline(text) + '\n');
        REQUIRE_EQ(cmd.get_return_code(), EXIT_SUCCESS);
      }

      SUBCASE("handled exception") {
        auto const exit_code{"14"};
        auto const exception_text{"handled"};
        exec_path_args cmd{some_cli_app("--handled-exception",
                                        exception_text, // ...
                                                        // won't be reached:
                                        "--exit", exit_code,           // ...
                                        "--stdout", "won't be printed" // ...
                                        )};

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::ready);
          REQUIRE(cmd.manages_process());
        }

        REQUIRE_EQ(cmd.read_stderr(true),
                   "some_cli_app caught `std::exception`: handled\n");
        REQUIRE_EQ(cmd.read_stdout(true), "");
        REQUIRE_NE(cmd.get_return_code(), std::stoi(exit_code));
        REQUIRE_EQ(cmd.get_return_code(), EXIT_FAILURE);
      }

      SUBCASE("unhandled exception") {
        auto const exit_code{"15"};
        auto const exception_text{"unhandled"};
        exec_path_args cmd{some_cli_app("--unhandled-exception",
                                        exception_text, // ...
                                                        // won't be reached:
                                        "--handled-exception",
                                        exception_text,                // ...
                                        "--exit", exit_code,           // ...
                                        "--stdout", "won't be printed" // ...
                                        )};

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::ready);
          REQUIRE(cmd.manages_process());
        }

        WARN_EQ(cmd.read_stderr(true),
                "terminate called after throwing an instance of 'char "
                "const*'\n"); // IMHO OS dependent ... -> only `WARN`
        REQUIRE_EQ(cmd.read_stdout(true), "");
        REQUIRE_NE(cmd.get_return_code(), std::stoi(exit_code));
        REQUIRE_EQ(cmd.get_return_code(), SIGABRT);
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
        auto const exit_code{"16"};
        exec_path_args cmd{some_cli_app_synced("--sync", "--exit", exit_code)};

        {
          exec_path_args::states state;
          REQUIRE_NOTHROW(state = cmd.update_and_get_state());
          REQUIRE_EQ(state.previous, exec_path_args::state::ready);
          REQUIRE_EQ(state.current, exec_path_args::state::running);
          REQUIRE(cmd.manages_process());
        }

        REQUIRE(my_sem->wait_and_notify(40));

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::running);
        }

        REQUIRE_EQ(cmd.read_stderr(true), "");
        REQUIRE_EQ(cmd.read_stdout(true), "");
        REQUIRE_EQ(cmd.get_return_code(), std::stoi(exit_code));
      }

      SUBCASE("uninitialized semaphore in child process") {
        exec_path_args cmd{some_cli_app("--sync", "--exit", "0")};

        {
          exec_path_args::states state;
          REQUIRE_NOTHROW(state = cmd.update_and_get_state());
          REQUIRE_EQ(state.previous, exec_path_args::state::ready);
          REQUIRE_EQ(state.current, exec_path_args::state::running);
          REQUIRE(cmd.manages_process());
        }

        REQUIRE_FALSE(my_sem->wait_and_notify(
            1)); // so it doesn't waste too much time ...

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::running);
        }

        REQUIRE_EQ(cmd.read_stderr(true),
                   "some_cli_app caught `std::exception`: Semaphore name not "
                   "specified for sync operation\n");
        REQUIRE_EQ(cmd.read_stdout(true), "");
        REQUIRE_EQ(cmd.get_return_code(), EXIT_FAILURE);
      }

      SUBCASE("missed semaphore in child process") {
        exec_path_args cmd{some_cli_app_synced("--sleep", "1000", // ...
                                               "--sync",          // ...
                                               "--exit", "0"      // ...
                                               )};

        {
          exec_path_args::states state;
          REQUIRE_NOTHROW(state = cmd.update_and_get_state());
          REQUIRE_EQ(state.previous, exec_path_args::state::ready);
          REQUIRE_EQ(state.current, exec_path_args::state::running);
          REQUIRE(cmd.manages_process());
        }

        REQUIRE_FALSE(my_sem->wait_and_notify(
            1)); // so it doesn't waste too much time ...

        {
          exec_path_args::states state;
          REQUIRE_NOTHROW(state = cmd.update_and_get_state());
          REQUIRE_EQ(state.previous, exec_path_args::state::running);
          REQUIRE_EQ(state.current, exec_path_args::state::running);
        }

        REQUIRE_NOTHROW(cmd.do_kill());
        REQUIRE(cmd.is_finished());

        {
          exec_path_args::state prev_state;
          REQUIRE_NOTHROW(prev_state = cmd.finish_and_get_prev_state());
          REQUIRE_EQ(prev_state, exec_path_args::state::finished);
        }

        REQUIRE_EQ(cmd.read_stderr(true), "");
        REQUIRE_EQ(cmd.read_stdout(true), "");
        REQUIRE_EQ(cmd.get_return_code(), SIGKILL);
      }

      SUBCASE("complex happy path") {
        auto const to_stderr{"How is it going?"};
        auto const to_stdout{"Fine, thank You!"};
        auto const exit_code{"17"};
        exec_path_args cmd{some_cli_app_synced("--stderr", to_stderr, // 1
                                               "--stdout", to_stdout, // 2
                                               "--sleep", "1",        // 3
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
          REQUIRE(cmd.manages_process());
        }

        REQUIRE(my_sem->wait_and_notify(40));

        REQUIRE_EQ(cmd.read_stdout(true), std::string{to_stdout} + '\n');
        REQUIRE_EQ(cmd.read_stderr(true), std::string{to_stderr} + '\n');

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

        REQUIRE_EQ(cmd.read_stdout(false),
                   space_to_newline(expected_echo_input));
        REQUIRE_EQ(cmd.read_stdout(false), ""); // consumed in previous line
        REQUIRE_EQ(cmd.read_stderr(false), "");

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

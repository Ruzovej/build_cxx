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
#include <chrono>
#include <filesystem>
#include <iostream>
#include <optional>
#include <thread>

#include <doctest/doctest.h>

#include <ips/ips.hxx>

namespace build_cxx::os_wrapper {
namespace {

TEST_CASE("exec_path_args") {
  SUBCASE("simple bash command") {
    static auto constexpr bash_cmd = [](std::string &&cmd_str) {
      return exec_path_args{"/usr/bin/env",
                            {"bash", "bash", "-c", std::move(cmd_str)}};
    };

    SUBCASE("happy path") {
      exec_path_args cmd{
          bash_cmd("printf \"Hello stdout!\"; printf \"Hello stderr!\" 1>&2")};

      {
        exec_path_args::states state;
        REQUIRE_NOTHROW(state = cmd.update_and_get_state());
        REQUIRE_EQ(state.previous, exec_path_args::state::ready);
        REQUIRE_EQ(state.current, exec_path_args::state::running);
      }

      { // potential for flakiness:
        {
          exec_path_args::states state;
          REQUIRE_NOTHROW(state = cmd.update_and_get_state());
          REQUIRE_EQ(state.previous, exec_path_args::state::running);
          WARN_EQ(state.current, exec_path_args::state::running);
        }

        // how dirty ... TODO some better way?
        std::this_thread::sleep_for(std::chrono::milliseconds{5});

        {
          exec_path_args::states state;
          REQUIRE_NOTHROW(state = cmd.update_and_get_state());
          WARN_EQ(state.previous, exec_path_args::state::running);
          REQUIRE_EQ(state.current, exec_path_args::state::finished);
        }

        {
          exec_path_args::states state;
          REQUIRE_NOTHROW(state = cmd.update_and_get_state());
          REQUIRE_EQ(state.previous, exec_path_args::state::finished);
          REQUIRE_EQ(state.current, exec_path_args::state::finished);
        }
      }

      REQUIRE_EQ(cmd.get_stdout(true), "Hello stdout!");
      REQUIRE_EQ(cmd.get_stderr(true), "Hello stderr!");
      REQUIRE_EQ(cmd.get_return_code(), EXIT_SUCCESS);
    }

    SUBCASE("non-zero return code") {
      static auto constexpr expected_val{42};

      exec_path_args cmd{bash_cmd("exit " + std::to_string(expected_val))};

      {
        exec_path_args::states state;
        REQUIRE_NOTHROW(state = cmd.update_and_get_state());
        REQUIRE_EQ(state.previous, exec_path_args::state::ready);
        REQUIRE_EQ(state.current, exec_path_args::state::running);
      }

      // how dirty ... TODO some better way?
      std::this_thread::sleep_for(std::chrono::milliseconds{5});

      {
        exec_path_args::states state;
        REQUIRE_NOTHROW(state = cmd.update_and_get_state());
        REQUIRE_EQ(state.previous, exec_path_args::state::running);
        REQUIRE_EQ(state.current, exec_path_args::state::finished);
      }

      {
        exec_path_args::states state;
        REQUIRE_NOTHROW(state = cmd.update_and_get_state());
        REQUIRE_EQ(state.previous, exec_path_args::state::finished);
        REQUIRE_EQ(state.current, exec_path_args::state::finished);
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

      {
        exec_path_args::states state;
        REQUIRE_NOTHROW(state = cmd.update_and_get_state());
        REQUIRE_EQ(state.previous, exec_path_args::state::finished);
        REQUIRE_EQ(state.current, exec_path_args::state::finished);
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

    auto const sem_name{"/some_cli_app_shared_sem"};

    auto const some_cli_app = [&](auto &&...args) {
      // TODO fix this first version ...:
      // return exec_path_args{
      //    some_cli_app_path.string(),
      //    {"--sem-name", sem_name, std::forward<decltype(args)>(args)...}};
      // and remove this one:
      return exec_path_args{"/usr/bin/env",
                            {"bash", some_cli_app_path.string(), "--sem-name",
                             sem_name, std::forward<decltype(args)>(args)...}};
    };

    std::optional<ips> my_sem;
    REQUIRE_NOTHROW(my_sem.emplace(sem_name, true));

    SUBCASE("basic functionality: sync") {
      auto const exit_code{"10"};
      exec_path_args cmd{some_cli_app("--sync", "--exit", exit_code)};

      {
        exec_path_args::states state;
        REQUIRE_NOTHROW(state = cmd.update_and_get_state());
        REQUIRE_EQ(state.previous, exec_path_args::state::ready);
        REQUIRE_EQ(state.current, exec_path_args::state::running);
      }

      REQUIRE(my_sem->wait_and_notify(40));

      std::this_thread::sleep_for(std::chrono::milliseconds{10});

      {
        exec_path_args::states state;
        REQUIRE_NOTHROW(state = cmd.update_and_get_state());
        REQUIRE_EQ(state.previous, exec_path_args::state::running);
        REQUIRE_EQ(state.current, exec_path_args::state::finished);
      }

      REQUIRE_EQ(cmd.get_return_code(), std::stoi(exit_code));
      REQUIRE_EQ(cmd.get_stderr(true), "");
      REQUIRE_EQ(cmd.get_stdout(true), "");
    }

    SUBCASE("basic functionality & happy path") {
      auto const to_stderr{"How is it going?"};
      auto const to_stdout{"Fine, thank You!"};
      auto const exit_code{"42"};

      exec_path_args cmd{some_cli_app("--stderr", to_stderr, // 1
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

      // std::this_thread::sleep_for(std::chrono::milliseconds{100});
      // std::this_thread::sleep_for(std::chrono::milliseconds{10});

      // TODO why does this fail? The output below is really written out (as can
      // be seen if the checks are switched ...)
      REQUIRE(my_sem->wait_and_notify(40));
      // REQUIRE_NOTHROW((void)my_sem->wait_and_notify(40));

      REQUIRE_EQ(cmd.get_stdout(true), std::string{to_stdout} + '\n');
      REQUIRE_EQ(cmd.get_stderr(true), std::string{to_stderr} + '\n');

      {
        exec_path_args::states state;
        REQUIRE_NOTHROW(state = cmd.update_and_get_state());
        REQUIRE_EQ(state.previous, exec_path_args::state::running);
        REQUIRE_EQ(state.current, exec_path_args::state::running);
      }

      // pay attention to the space at the end ...:
      std::string_view const expected_echo_input{
          "const std::string_view data "};
      REQUIRE_NOTHROW(cmd.send_to_stdin(expected_echo_input));

      // std::this_thread::sleep_for(std::chrono::milliseconds{10});
      REQUIRE(my_sem->wait_and_notify(40));
      // REQUIRE_NOTHROW((void)my_sem->wait_and_notify(40));

      std::string str{expected_echo_input};
      std::transform(str.begin(), str.end(), str.begin(),
                     [](char c) { return c == ' ' ? '\n' : c; });
      REQUIRE_EQ(cmd.get_stdout(false), str);
      REQUIRE_EQ(cmd.get_stderr(false), "");

      {
        exec_path_args::states state;
        REQUIRE_NOTHROW(state = cmd.update_and_get_state(-1));
        REQUIRE_EQ(state.previous, exec_path_args::state::running);
        REQUIRE_EQ(state.current, exec_path_args::state::finished);
      }

      REQUIRE_EQ(cmd.get_return_code(), std::stoi(exit_code));
    }
  }
}

} // namespace
} // namespace build_cxx::os_wrapper

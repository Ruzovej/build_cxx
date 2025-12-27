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

#include <chrono>
#include <thread>

#include <doctest/doctest.h>

namespace build_cxx::os_wrapper {
namespace {

exec_path_args bash_cmd(std::string &&cmd_str) {
  return exec_path_args{"/usr/bin/env",
                        {"bash", "bash", "-c", std::move(cmd_str)}};
}

TEST_CASE("exec_path_args") {
  SUBCASE("simple bash command - happy path") {
    exec_path_args cmd{
        bash_cmd("printf \"Hello stdout!\"; printf \"Hello stderr!\" 1>&2")};

    {
      auto const state{cmd.update_and_get_state()};
      REQUIRE_EQ(state.previous, exec_path_args::state::ready);
      REQUIRE_EQ(state.current, exec_path_args::state::running);
    }

    { // potential for flakiness:
      {
        auto const state{cmd.update_and_get_state()};
        REQUIRE_EQ(state.previous, exec_path_args::state::running);
        WARN_EQ(state.current, exec_path_args::state::running);
      }

      // how dirty ... TODO some better way?
      std::this_thread::sleep_for(std::chrono::milliseconds{5});

      {
        auto const state{cmd.update_and_get_state()};
        WARN_EQ(state.previous, exec_path_args::state::running);
        REQUIRE_EQ(state.current, exec_path_args::state::finished);
      }

      {
        auto const state{cmd.update_and_get_state()};
        REQUIRE_EQ(state.previous, exec_path_args::state::finished);
        REQUIRE_EQ(state.current, exec_path_args::state::finished);
      }
    }

    REQUIRE_EQ(cmd.get_stdout(true), "Hello stdout!");
    REQUIRE_EQ(cmd.get_stderr(true), "Hello stderr!");
    REQUIRE_EQ(cmd.get_return_code(), EXIT_SUCCESS);
  }

  SUBCASE("simple bash command - non-zero return code") {
    static auto constexpr expected_val{42};

    exec_path_args cmd{bash_cmd("exit " + std::to_string(expected_val))};

    {
      auto const state{cmd.update_and_get_state()};
      REQUIRE_EQ(state.previous, exec_path_args::state::ready);
      REQUIRE_EQ(state.current, exec_path_args::state::running);
    }

    // how dirty ... TODO some better way?
    std::this_thread::sleep_for(std::chrono::milliseconds{5});

    {
      auto const state{cmd.update_and_get_state()};
      REQUIRE_EQ(state.previous, exec_path_args::state::running);
      REQUIRE_EQ(state.current, exec_path_args::state::finished);
    }

    {
      auto const state{cmd.update_and_get_state()};
      REQUIRE_EQ(state.previous, exec_path_args::state::finished);
      REQUIRE_EQ(state.current, exec_path_args::state::finished);
    }

    REQUIRE_EQ(cmd.get_stdout(true), "");
    REQUIRE_EQ(cmd.get_stderr(true), "");
    REQUIRE_EQ(cmd.get_return_code(), expected_val);
  }

  SUBCASE("simple bash command - not waiting for it") {
    exec_path_args cmd{bash_cmd("sleep 1; printf \"Done!\"")};

    {
      auto const state{cmd.update_and_get_state()};
      REQUIRE_EQ(state.previous, exec_path_args::state::ready);
      REQUIRE_EQ(state.current, exec_path_args::state::running);
    }

    // how dirty ... TODO some better way?
    std::this_thread::sleep_for(std::chrono::milliseconds{5});

    cmd.do_kill();

    {
      auto const state{cmd.update_and_get_state()};
      REQUIRE_EQ(state.previous, exec_path_args::state::finished);
      REQUIRE_EQ(state.current, exec_path_args::state::finished);
    }

    REQUIRE_EQ(cmd.get_stdout(true), "");
    REQUIRE_EQ(cmd.get_stderr(true), "");
    REQUIRE_NE(cmd.get_return_code(), EXIT_SUCCESS);
  }
}

} // namespace
} // namespace build_cxx::os_wrapper

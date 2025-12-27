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

#include <cstdlib>

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <type_traits>
#include <variant>
#include <vector>

namespace {

struct exit_with {
  int code{EXIT_SUCCESS};
};
struct sleep_for_ms {
  int ms;
};
struct echo_stdin_to_stdout {
  int count;
};
struct to_stdout {
  std::string msg;
};
struct to_stderr {
  std::string msg;
};

using action_variant = std::variant<exit_with, sleep_for_ms,
                                    echo_stdin_to_stdout, to_stdout, to_stderr>;

} // namespace

int main(int const argc, char const **argv) {
  auto consume_arg = [argc2 = argc - 1, argv2 = argv + 1](
                         bool const require, std::string_view const err_msg =
                                                 "") mutable -> char const * {
    if (0 < argc2) {
      --argc2;
      return *(argv2++);
    } else {
      if (require) {
        throw std::runtime_error{std::string{"Not enough arguments: "} +
                                 std::string{err_msg}};
      } else {
        return nullptr;
      }
    }
  };

  std::vector<action_variant> actions;

  char const *arg;
  while ((arg = consume_arg(false)) != nullptr) {
    std::string_view const arg_sv{arg};
    if (arg_sv == "--exit") {
      actions.emplace_back(
          exit_with{std::stoi(consume_arg(true, "missing exit code"))});
    } else if (arg_sv == "--sleep") {
      actions.emplace_back(
          sleep_for_ms{std::stoi(consume_arg(true, "missing sleep duration"))});
    } else if (arg_sv == "--echo") {
      actions.emplace_back(echo_stdin_to_stdout{
          std::stoi(consume_arg(true, "missing echo count"))});
    } else if (arg_sv == "--stdout") {
      actions.emplace_back(
          to_stdout{consume_arg(true, "missing stdout message")});
    } else if (arg_sv == "--stderr") {
      actions.emplace_back(
          to_stderr{consume_arg(true, "missing stderr message")});
    } else {
      throw std::runtime_error{std::string{"Unknown argument: "} +
                               std::string{arg_sv}};
    }
  }

  for (auto const &action : actions) {
    std::visit(
        [](auto &&arg) {
          using T = std::decay_t<decltype(arg)>;
          if constexpr (std::is_same_v<T, exit_with>) {
            std::exit(arg.code);
          } else if constexpr (std::is_same_v<T, sleep_for_ms>) {
            std::this_thread::sleep_for(std::chrono::milliseconds{arg.ms});
          } else if constexpr (std::is_same_v<T, echo_stdin_to_stdout>) {
            std::string str;
            for (int i{0}; i < arg.count; ++i) {
              std::cin >> str;
              std::cout << str << '\n';
            }
          } else if constexpr (std::is_same_v<T, to_stdout>) {
            std::cout << arg.msg << std::endl;
          } else if constexpr (std::is_same_v<T, to_stderr>) {
            std::cerr << arg.msg << '\n';
          } else {
            static_assert(!std::is_same_v<T, T>, "non-exhaustive visitor!");
          }
        },
        action);
  }

  return EXIT_SUCCESS;
}

/*
  Copyright 2026 Lukáš Růžička

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

#include "env.hxx"

#include <cctype>
#include <cstdlib>

#include <stdexcept>
#include <string>
#include <string_view>

#include <cli11_wrapper/argv_parser.hxx>

namespace build_cxx::system_tests {

namespace {

std::string to_env_name(std::string_view const name) {
  static std::string_view constexpr prefix{"BUILDCXXSYSTEMTEST_"};

  std::string result;
  result.reserve(prefix.size() + name.size());

  result += prefix;
  for (char const c : name) {
    result += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  }

  return result;
}

[[nodiscard]] cli11_wrapper::env_var_name
env_var_name(std::string_view const name) {
  return cli11_wrapper::env_var_name{to_env_name(name)};
}

} // namespace

env const &env::inst() noexcept {
  // force 2 lines
  return inst_priv();
}

int env::setup(cli11_wrapper::args &args) {
  // force 2 lines
  return inst_priv().do_setup(args);
}

env::env() noexcept = default;

env::~env() noexcept = default;

env &env::inst_priv() noexcept {
  static env e;
  return e;
}

int env::do_setup(cli11_wrapper::args &args) {
  if (initialized) {
    throw std::runtime_error{"env::setup() called more than once"};
  }

  cli11_wrapper::argv_parser parser{"TODO app desc.",
                                    args.argv()[0],
                                    {
                                        // TODO config files?!
                                    },
                                    args.argc(),
                                    args.argv()};

  parser.set_allow_extras(true);

  parser.add_option(
      env_var_name("driver_exec"), "--driver_exec", build_cxx_driver_path,
      "path (relative to the repo root) of the executable to be tested", true);

  parser.add_option(env_var_name("repo_root"), "--repo_root",
                    build_cxx_repo_root,
                    "absolute path to the root of the repository", true);

  parser.add_option(env_var_name("system_tests_root"), "--system_tests_root",
                    build_cxx_system_test_cases_root,
                    "path (relative to the repo root) of the system test cases",
                    true);

  CLI11_WRAPPER_PARSE(parser);

  args = std::move(parser.get_parsed_extras_c_like());

  initialized = true;

  return EXIT_SUCCESS;
}

} // namespace build_cxx::system_tests

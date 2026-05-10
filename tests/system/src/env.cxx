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

#include <string>
#include <string_view>

#include <cli11_wrapper/argv_parser.hxx>

namespace build_cxx::system_tests {

namespace {

std::string to_env_name(std::string_view const name) {
  static std::string_view constexpr prefix{"BUILDCXX_"};

  std::string result;
  result.reserve(prefix.size() + name.size());

  result += prefix;
  for (char const c : name) {
    result += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
  }

  return result;
}

} // namespace

env &env::instance() noexcept {
  static env e;
  return e;
}

int env::setup(int &argc, char **&argv) {
  cli11_wrapper::argv_parser parser{"TODO app desc.",
                                    "TODO app name",
                                    {
                                        // TODO config files
                                    },
                                    argc,
                                    argv};

  parser.set_allow_extras(true);

  CLI11_WRAPPER_PARSE(parser);

  auto const extras{parser.get_parsed_extras_c_like()};

  return EXIT_SUCCESS;
}

env::env() noexcept = default;

env::~env() noexcept = default;

} // namespace build_cxx::system_tests

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

#define DOCTEST_CONFIG_IMPLEMENT

#include <cctype>
#include <cstdlib>

#include <stdexcept>
#include <string>
#include <string_view>

#include <cli11_wrapper/args.hxx>
#include <cli11_wrapper/argv_parser.hxx>
#include <doctest/doctest.h>

#include "build_cxx/system_tests/impl/env.hxx"

namespace build_cxx::system_tests::impl {

namespace {

[[nodiscard]] std::string to_env_name(std::string_view const name) {
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

[[nodiscard]] std::string opt_name(std::string_view const name) {
  std::string res;
  res.reserve(name.size() + 2);
  res += "--";
  res += name;
  return res;
}

} // namespace

env &env::inst() noexcept {
  static env e;
  return e;
}

env::env() noexcept = default;

env::~env() noexcept = default;

int env::setup(cli11_wrapper::args &args) {
  if (initialized) {
    // called more than once:
    return EXIT_FAILURE;
  }

  cli11_wrapper::argv_parser parser{"TODO app desc.",
                                    args.argv()[0],
                                    {
                                        // TODO config files?!
                                    },
                                    args.argc(),
                                    args.argv()};

  auto const add_option = [&parser](std::string_view const name,
                                    std::string &target, std::string &&desc,
                                    bool const required = false) {
    parser.add_option(env_var_name(name), opt_name(name), target,
                      std::move(desc), required);
  };

  parser.set_allow_extras(true);

  add_option("driver_exec", te.build_cxx_driver_path,
             "path (relative to the repo root) of the executable to be tested",
             true);

  add_option("repo_root", te.build_cxx_repo_root,
             "absolute path to the root of the repository", true);

  add_option("system_tests_root", te.build_cxx_system_test_cases_root,
             "path (relative to the repo root) of the system test cases", true);

  add_option("cc", te.cc, "C compiler");

  add_option("c_flags", te.c_flags, "C compiler flags");

  add_option("cxx", te.cxx, "C++ compiler");

  add_option("cxx_flags", te.cxx_flags, "C++ compiler flags");

  add_option("ld", te.ld, "linker");

  add_option("ld_flags", te.ld_flags, "linker flags");

  add_option("ar", te.ar, "archiver");

  add_option("ar_flags", te.ar_flags, "archiver flags");

  add_option("ranlib", te.ranlib, "ranlib");

  add_option("ranlib_flags", te.ranlib_flags, "ranlib flags");

  add_option("strip", te.strip, "strip");

  add_option("strip_flags", te.strip_flags, "strip flags");

  CLI11_WRAPPER_PARSE(parser);

  // --help for whatever reason causes it to improperly continue ...
  args = std::move(parser.get_parsed_extras_c_like());

  initialized = true;

  return EXIT_SUCCESS;
}

} // namespace build_cxx::system_tests::impl

int main(int argc, char **argv) {
  cli11_wrapper::args args{argc, argv};

  auto const res{build_cxx::system_tests::impl::env::inst().setup(args)};

  if (res == EXIT_SUCCESS)
    return doctest::Context{args.argc(), args.argv()}.run();

  return res;
}

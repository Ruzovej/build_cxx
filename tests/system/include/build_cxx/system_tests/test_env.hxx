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

#pragma once

#include <string>

#define TEST_ENV() ::build_cxx::system_tests::test_env::inst()

namespace build_cxx::system_tests {

namespace impl {

// Not to be used in test cases, it's only purpose is initialization of test_env
struct env;

} // namespace impl
struct test_env {
  friend struct impl::env;

  [[nodiscard]] static test_env const &inst() noexcept;

  // values:
  std::string build_cxx_driver_path;
  std::string build_cxx_repo_root;
  std::string build_cxx_system_test_cases_root;
  std::string cc{"gcc"};
  std::string c_flags;
  std::string cxx{"g++"};
  std::string cxx_flags;
  std::string ld{"ld"};
  std::string ld_flags;
  std::string ar{"ar"};
  std::string ar_flags;
  std::string ranlib{"ranlib"};
  std::string ranlib_flags;
  std::string strip{"strip"};
  std::string strip_flags;

  // TODO

private:
  test_env() noexcept;
  ~test_env() noexcept;

  test_env(const test_env &) = delete;
  test_env &operator=(const test_env &) = delete;
  test_env(test_env &&) = delete;
  test_env &operator=(test_env &&) = delete;
};

} // namespace build_cxx::system_tests

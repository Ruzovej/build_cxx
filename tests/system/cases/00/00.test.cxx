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

#include <filesystem>

#include <doctest/doctest.h>

#include "build_cxx/system_tests/env.hxx"

namespace {

TEST_CASE("correct folder is set, build_cxx_driver exists & is executable by "
          "current user") {
  REQUIRE_EQ(std::filesystem::current_path(), TEST_ENV().build_cxx_repo_root);

  REQUIRE(std::filesystem::exists(TEST_ENV().build_cxx_driver_path));

  REQUIRE(std::filesystem::is_regular_file(TEST_ENV().build_cxx_driver_path));

  REQUIRE_EQ(
      std::filesystem::status(TEST_ENV().build_cxx_driver_path).permissions() &
          std::filesystem::perms::owner_exec,
      std::filesystem::perms::owner_exec);
}

} // namespace

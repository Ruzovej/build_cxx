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

#include "build_cxx/test_helpers/mng_file.hxx"

#include <doctest/doctest.h>
#include <filesystem>

namespace build_cxx::test_helpers {
namespace {

TEST_CASE("test_helpers::mng_file") {
  mng_file mf;

  auto const &local_tmp_dir{mng_file::tmp_dir(true)};
  auto const &global_tmp_dir{mng_file::tmp_dir(false)};

  SUBCASE("tmp_dir") {
    REQUIRE(!local_tmp_dir.empty());
    REQUIRE(!global_tmp_dir.empty());
    REQUIRE(local_tmp_dir != global_tmp_dir);
  }

  SUBCASE("touch & rm") {
    SUBCASE("relative") {
      static std::string_view constexpr path{"test_tmp_file.txt"};
      auto const abs_path{local_tmp_dir / path};

      REQUIRE(!std::filesystem::exists(abs_path));

      REQUIRE_NOTHROW(mf.touch(path));
      REQUIRE(std::filesystem::exists(abs_path));

      REQUIRE_NOTHROW(mf.rm(abs_path));
      REQUIRE(!std::filesystem::exists(abs_path));
    }
  }
}

} // namespace
} // namespace build_cxx::test_helpers

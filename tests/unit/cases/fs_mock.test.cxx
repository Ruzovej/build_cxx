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

#include "build_cxx/test_helpers/fs_mock.hxx"

#include <doctest/doctest.h>

namespace build_cxx {
namespace {

TEST_CASE("client::debug_helper") {
  test_helpers::fs_mock fake_fs;

  std::filesystem::path tmp_dir;

  REQUIRE_NOTHROW(tmp_dir = fake_fs.tmp_dir());

  REQUIRE(!fake_fs.file_exists(tmp_dir / "foo1.txt"));
  REQUIRE(!fake_fs.file_exists("foo2.txt"));

  REQUIRE_NOTHROW(fake_fs.touch(tmp_dir / "foo1.txt"));
  REQUIRE_NOTHROW(fake_fs.touch("foo2.txt"));

  REQUIRE(fake_fs.file_exists(tmp_dir / "foo1.txt"));
  REQUIRE(fake_fs.file_exists("foo2.txt"));

  fake_fs.clock.freeze_time(true);

  REQUIRE_NOTHROW(fake_fs.touch("/home/bar/foo2.txt"));

  REQUIRE(fake_fs.file_last_mod_time(tmp_dir / "foo1.txt") <
          fake_fs.file_last_mod_time("foo2.txt"));

  REQUIRE(fake_fs.file_last_mod_time("foo2.txt") ==
          fake_fs.file_last_mod_time("/home/bar/foo2.txt"));

  REQUIRE_NOTHROW(fake_fs.rm(tmp_dir / "foo1.txt"));
  REQUIRE_NOTHROW(fake_fs.rm("foo2.txt"));

  REQUIRE(!fake_fs.file_exists(tmp_dir / "foo1.txt"));
  REQUIRE(!fake_fs.file_exists("foo2.txt"));
}

} // namespace
} // namespace build_cxx

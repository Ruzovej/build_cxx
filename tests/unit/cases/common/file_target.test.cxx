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

#include "build_cxx/common/file_target.hxx"

#include <doctest/doctest.h>

#include "build_cxx/client/core.hxx"
#include "build_cxx/test_helpers/mock_file_target.hxx"
#include "build_cxx/test_helpers/mock_fs.hxx"
#include "build_cxx/test_helpers/mock_project.hxx"

namespace build_cxx {
namespace {

TEST_CASE("common::file_target") {
  static std::string_view constexpr fake_filename{
      "/fake/dir/file_target.test.cxx"};

  std::mutex mtx;
  test_helpers::built_targets_t built_targets;
  test_helpers::mock_fs fake_fs;
  test_helpers::mock_project test_project{
      &mtx, &built_targets, &fake_fs, "cfttp", "0.1.0", fake_filename};

  SUBCASE("relative path") {
    auto *const ft{test_project.add_mock_file_target(fake_filename, true, "tft",
                                                     false, {})};

    REQUIRE(ft->resolved_kind.empty());
    REQUIRE(ft->resolved_name.empty());

    REQUIRE_NOTHROW(ft->resolve_own_traits());

    REQUIRE_EQ(ft->resolved_kind, common::file_target::kind);
    REQUIRE_EQ(ft->resolved_name, "/fake/dir/tft");
    REQUIRE_EQ(std::filesystem::path{"/fake/dir/tft"},
               common::file_target::resolve_path(ft->loc->filename, ft->name));

    REQUIRE_NOTHROW(ft->recipe({}));
    REQUIRE_EQ(built_targets.size(), 1);
    REQUIRE_EQ(*built_targets.begin(), ft);
    REQUIRE(fake_fs.file_exists(ft->resolved_name));

    // TODO test `last_modification_time` ...
  }

  SUBCASE("absolute path") {
    auto *const ft{test_project.add_mock_file_target(
        fake_filename, true, "/another/fake/dir/tft", true, {})};

    REQUIRE(ft->resolved_kind.empty());
    REQUIRE(ft->resolved_name.empty());

    REQUIRE_NOTHROW(ft->resolve_own_traits());

    REQUIRE_EQ(ft->resolved_kind, common::file_target::kind);
    REQUIRE_EQ(ft->resolved_name, "/another/fake/dir/tft");
    REQUIRE_EQ(std::filesystem::path{"/another/fake/dir/tft"},
               common::file_target::resolve_path(ft->loc->filename, ft->name));

    ft->built_targets = &built_targets;

    REQUIRE_NOTHROW(ft->recipe({}));
    // expect 0 because it's marked as read-only ...:
    REQUIRE_EQ(built_targets.size(), 0);
    REQUIRE(!fake_fs.file_exists(ft->resolved_name));

    // TODO test `last_modification_time` ...
  }
}

} // namespace
} // namespace build_cxx

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
#include "build_cxx/test_helpers/test_file_target.hxx"

namespace build_cxx::common {
namespace {

TEST_CASE("common::file_target") {
  static std::string_view constexpr fake_filename{
      "/fake/dir/file_target.test.cxx"};

  project test_project{"cfttp", "0.1.0", fake_filename};

  static location constexpr loc{fake_filename, __LINE__, location::no_index};
  // testing it on single "isolated" target should be enough:
  BUILD_CXX_DEFINE_DEPS_ARRAY(deps_arr, deps_n);

  std::unordered_set<abstract_target const *> built_targets;

  SUBCASE("relative path") {
    test_file_target ft{&loc, true, "tft", deps_arr, deps_n};

    test_project.add_target(&ft);

    REQUIRE(ft.resolved_kind.empty());
    REQUIRE(ft.resolved_name.empty());

    REQUIRE_NOTHROW(ft.resolve_own_traits());

    REQUIRE_EQ(ft.resolved_kind, file_target::kind);
    REQUIRE_EQ(ft.resolved_name, "/fake/dir/tft");
    REQUIRE_EQ(std::filesystem::path{"/fake/dir/tft"},
               file_target::resolve_path(ft.loc->filename, ft.name));

    ft.built_targets = &built_targets;

    REQUIRE_NOTHROW(ft.build({}));
    REQUIRE_EQ(built_targets.size(), 1);
    REQUIRE_EQ(*built_targets.begin(), &ft);

    // TODO test `last_modification_time` ...
  }

  SUBCASE("absolute path") {
    test_file_target ft{&loc, true, "/another/fake/dir/tft", deps_arr, deps_n};

    test_project.add_target(&ft);

    REQUIRE(ft.resolved_kind.empty());
    REQUIRE(ft.resolved_name.empty());

    REQUIRE_NOTHROW(ft.resolve_own_traits());

    REQUIRE_EQ(ft.resolved_kind, file_target::kind);
    REQUIRE_EQ(ft.resolved_name, "/another/fake/dir/tft");
    REQUIRE_EQ(std::filesystem::path{"/another/fake/dir/tft"},
               file_target::resolve_path(ft.loc->filename, ft.name));

    ft.built_targets = &built_targets;

    REQUIRE_NOTHROW(ft.build({}));
    REQUIRE_EQ(built_targets.size(), 1);
    REQUIRE_EQ(*built_targets.begin(), &ft);

    // TODO test `last_modification_time` ...
  }
}

} // namespace
} // namespace build_cxx::common

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

#include "build_cxx/driver/processed_targets.hxx"

#include <doctest/doctest.h>

#include "build_cxx/client/core.hxx"
#include "build_cxx/common/location.hxx"
#include "build_cxx/test_helpers/mock_project.hxx"
#include "build_cxx/test_helpers/test_file_target.hxx"
#include "build_cxx/test_helpers/test_phony_target.hxx"

namespace build_cxx {
namespace {

TEST_CASE("driver::processed_targets") {
  static std::string_view constexpr fake_root_file1{
      "/fake/dir/project1.root.cxx"};

  test_helpers::built_targets_t built_targets;
  test_helpers::mock_project test_project1{"dpttp1", "0.1.0", fake_root_file1};
  test_project1.built_targets = &built_targets;

  driver::processed_targets driver_pt{};

  SUBCASE("empty project") {
    REQUIRE_NOTHROW(driver_pt.process_project(&test_project1));

    REQUIRE_EQ(driver_pt.projects_by_name.size(), 1);
    REQUIRE_EQ(driver_pt.projects_by_name.begin()->first, test_project1.name);
    REQUIRE_EQ(driver_pt.projects_by_name.begin()->second, &test_project1);
    REQUIRE_EQ(driver_pt.targets_by_project.size(), 0);
    REQUIRE_EQ(driver_pt.project_of_target.size(), 0);
    REQUIRE_EQ(driver_pt.targets_by_resolved_name.size(), 0);
    REQUIRE_EQ(driver_pt.intermediate_targets.size(), 0);
    // REQUIRE_EQ(pt.built_targets.size(), 0); // private ...
    REQUIRE_EQ(driver_pt.get_target_resolved_deps().size(), 0);
    // REQUIRE_EQ(pt.unresolved, 0); // private ...

    bool all_resolved{false};
    REQUIRE_NOTHROW(all_resolved = driver_pt.resolve_deps());
    REQUIRE(all_resolved);

    REQUIRE_NOTHROW(driver_pt.build_all_targets(false));
    REQUIRE_EQ(built_targets.size(), 0);
  }

  SUBCASE("2 non-empty projects") {
    static std::string_view constexpr fake_root_file2{
        "/fake/dir/project2.root.cxx"};

    test_helpers::mock_project test_project2{"dpttp2", "0.1.0",
                                             fake_root_file2};
    test_project2.built_targets = &built_targets;

    SUBCASE(
        "each with single phony target without cross-project dependencies") {
      auto *const pt_1{test_project1.add_mock_phony_target(fake_root_file1,
                                                           true, "pt_1", {})};
      REQUIRE_EQ(pt_1->resolved_name, "");

      auto *const pt_2{test_project2.add_mock_phony_target(fake_root_file2,
                                                           true, "pt_2", {})};
      REQUIRE_EQ(pt_2->resolved_name, "");

      REQUIRE_NOTHROW(driver_pt.process_project(&test_project1));
      REQUIRE_EQ(pt_1->resolved_name, "dpttp1::pt_1");

      REQUIRE_NOTHROW(driver_pt.process_project(&test_project2));
      REQUIRE_EQ(pt_2->resolved_name, "dpttp2::pt_2");

      REQUIRE_EQ(driver_pt.projects_by_name.size(), 2);
      REQUIRE_EQ(driver_pt.targets_by_project.size(), 2);
      REQUIRE_EQ(driver_pt.targets_by_project.begin()->second.size(), 1);
      REQUIRE_EQ(std::next(driver_pt.targets_by_project.begin())->second.size(),
                 1);
      REQUIRE_EQ(driver_pt.project_of_target.size(), 2);
      REQUIRE_EQ(driver_pt.targets_by_resolved_name.size(), 2);
      REQUIRE_EQ(driver_pt.intermediate_targets.size(), 0);
      // REQUIRE_EQ(pt.built_targets.size(), 0); // private ...
      REQUIRE_EQ(driver_pt.get_target_resolved_deps().size(), 2);
      // REQUIRE_EQ(pt.unresolved, 2); // private ...

      bool all_resolved{false};
      REQUIRE_NOTHROW(all_resolved = driver_pt.resolve_deps());
      REQUIRE(all_resolved);

      REQUIRE(built_targets.empty());
      REQUIRE_NOTHROW(driver_pt.build_target(pt_1, false));
      REQUIRE_EQ(built_targets.size(), 1);
      REQUIRE_EQ(*built_targets.begin(), pt_1);

      built_targets.clear();
      REQUIRE_NOTHROW(driver_pt.build_target(pt_2, false));
      REQUIRE_EQ(built_targets.size(), 1);
      REQUIRE_EQ(*built_targets.begin(), pt_2);
    }

    SUBCASE(
        "each with single phony target without cross-project dependencies") {
      auto *const pt_1{test_project1.add_mock_phony_target(fake_root_file1,
                                                           true, "pt_1", {})};

      auto *const pt_2{test_project1.add_mock_phony_target(
          fake_root_file1, true, "pt_2",
          {"pt_1"})}; // will resovle dependency locally

      auto *const pt_3{test_project2.add_mock_phony_target(
          fake_root_file2, true, "pt_3", {"dpttp1::pt_1"})};

      REQUIRE_NOTHROW(driver_pt.process_project(&test_project1));
      REQUIRE_NOTHROW(driver_pt.process_project(&test_project2));

      bool all_resolved{false};
      REQUIRE_NOTHROW(all_resolved = driver_pt.resolve_deps());
      REQUIRE(all_resolved);

      REQUIRE(built_targets.empty());

      SUBCASE("build them separately") {
        // without deps
        REQUIRE_NOTHROW(driver_pt.build_target(pt_1, false));
        REQUIRE_EQ(built_targets.size(), 1);
        REQUIRE_EQ(*built_targets.begin(), pt_1);

        // internal set will contain both ptrs, but this one is cleared in order
        // to have easier checks below
        built_targets.clear();

        REQUIRE_NOTHROW(driver_pt.build_target(pt_2, false));
        REQUIRE_EQ(built_targets.size(), 1);
        REQUIRE_EQ(*built_targets.begin(), pt_2);

        // ...
        built_targets.clear();

        REQUIRE_NOTHROW(driver_pt.build_target(pt_3, false));
        REQUIRE_EQ(built_targets.size(), 1);
        REQUIRE_EQ(*built_targets.begin(), pt_3);
      }

      SUBCASE("build them together") {
        // builds even its dependency
        REQUIRE_NOTHROW(driver_pt.build_all_targets(false));
        REQUIRE_EQ(built_targets.size(), 3);
      }
    }
  }
  // TODO write cases for various nontrivial "graphs" of dependencies, etc.
}

} // namespace
} // namespace build_cxx

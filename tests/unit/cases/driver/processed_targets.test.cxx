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
#include <memory>

#include "build_cxx/client/core.hxx"
#include "build_cxx/common/location.hxx"
#include "build_cxx/test_helpers/test_file_target.hxx"
#include "build_cxx/test_helpers/test_phony_target.hxx"

namespace build_cxx::driver {
namespace {

TEST_CASE("driver::processed_targets") {
  static std::string_view constexpr fake_root_file1{
      "/fake/dir/project1.root.cxx"};
  common::project test_project1{"dpttp1", "0.1.0", fake_root_file1};

  std::unordered_set<common::abstract_target const *> built_targets;

  auto const make_phony_1 = [&](auto const name) {
    static common::location loc{fake_root_file1, __LINE__,
                                common::location::no_index};
    BUILD_CXX_DEFINE_DEPS_ARRAY(deps_arr, deps_n);

    auto pt{std::make_unique<common::test_phony_target>(&loc, true, name,
                                                        deps_arr, deps_n)};
    pt->built_targets = &built_targets;

    test_project1.add_target(pt.get());

    return pt;
  };

  processed_targets pt{};

  SUBCASE("empty project") {
    REQUIRE_NOTHROW(pt.process_project(&test_project1));

    REQUIRE_EQ(pt.projects_by_name.size(), 1);
    REQUIRE_EQ(pt.projects_by_name.begin()->first, test_project1.name);
    REQUIRE_EQ(pt.projects_by_name.begin()->second, &test_project1);
    REQUIRE_EQ(pt.targets_by_project.size(), 0);
    REQUIRE_EQ(pt.project_of_target.size(), 0);
    REQUIRE_EQ(pt.targets_by_resolved_name.size(), 0);
    REQUIRE_EQ(pt.intermediate_targets.size(), 0);
    // REQUIRE_EQ(pt.built_targets.size(), 0); // private ...
    REQUIRE_EQ(pt.get_target_resolved_deps().size(), 0);
    // REQUIRE_EQ(pt.unresolved, 0); // private ...

    bool all_resolved{false};
    REQUIRE_NOTHROW(all_resolved = pt.resolve_deps());
    REQUIRE(all_resolved);
  }

  SUBCASE("2 non-empty projects") {
    static std::string_view constexpr fake_root_file2{
        "/fake/dir/project2.root.cxx"};
    common::project test_project2{"dpttp2", "0.1.0", fake_root_file2};

    auto const make_phony_2 = [&](auto const name) {
      static common::location loc{fake_root_file2, __LINE__,
                                  common::location::no_index};
      BUILD_CXX_DEFINE_DEPS_ARRAY(deps_arr, deps_n);

      auto pt{std::make_unique<common::test_phony_target>(&loc, true, name,
                                                          deps_arr, deps_n)};
      pt->built_targets = &built_targets;

      test_project2.add_target(pt.get());

      return pt;
    };

    auto pt_1{make_phony_1("pt_1")};
    REQUIRE_EQ(pt_1->resolved_name, "");

    auto pt_2{make_phony_2("pt_2")};
    REQUIRE_EQ(pt_2->resolved_name, "");

    REQUIRE_NOTHROW(pt.process_project(&test_project1));
    REQUIRE_EQ(pt_1->resolved_name, "dpttp1::pt_1");

    REQUIRE_NOTHROW(pt.process_project(&test_project2));
    REQUIRE_EQ(pt_2->resolved_name, "dpttp2::pt_2");

    REQUIRE_EQ(pt.projects_by_name.size(), 2);
    REQUIRE_EQ(pt.targets_by_project.size(), 2);
    REQUIRE_EQ(pt.project_of_target.size(), 2);
    REQUIRE_EQ(pt.targets_by_resolved_name.size(), 2);
    REQUIRE_EQ(pt.intermediate_targets.size(), 0);
    // REQUIRE_EQ(pt.built_targets.size(), 0); // private ...
    REQUIRE_EQ(pt.get_target_resolved_deps().size(), 2);
    // REQUIRE_EQ(pt.unresolved, 2); // private ...

    bool all_resolved{false};
    REQUIRE_NOTHROW(all_resolved = pt.resolve_deps());
    REQUIRE(all_resolved);

    REQUIRE(built_targets.empty());
    REQUIRE_NOTHROW(pt.build_target(pt_1.get()));
    REQUIRE_EQ(built_targets.size(), 1);
    REQUIRE_EQ(*built_targets.begin(), pt_1.get());

    built_targets.clear();
    REQUIRE_NOTHROW(pt.build_target(pt_2.get()));
    REQUIRE_EQ(built_targets.size(), 1);
    REQUIRE_EQ(*built_targets.begin(), pt_2.get());
  }

  SUBCASE("2 non-empty projects with cross dependency") {
    auto pt_1{make_phony_1("pt_1")};
    REQUIRE_EQ(pt_1->resolved_name, "");

    REQUIRE_NOTHROW(pt.process_project(&test_project1));
    REQUIRE_EQ(pt_1->resolved_name, "dpttp1::pt_1");

    static std::string_view constexpr fake_root_file2{
        "/fake/dir/project2.root.cxx"};
    common::project test_project2{"dpttp2", "0.1.0", fake_root_file2};

    auto const make_phony_2 = [&](auto const name) {
      static common::location loc{fake_root_file2, __LINE__,
                                  common::location::no_index};
      BUILD_CXX_DEFINE_DEPS_ARRAY(deps_arr, deps_n, "dpttp1::pt_1");

      auto pt{std::make_unique<common::test_phony_target>(&loc, true, name,
                                                          deps_arr, deps_n)};
      pt->built_targets = &built_targets;

      test_project2.add_target(pt.get());

      return pt;
    };

    auto pt_2{make_phony_2("pt_2")};
    REQUIRE_EQ(pt_2->resolved_name, "");

    REQUIRE_NOTHROW(pt.process_project(&test_project2));
    REQUIRE_EQ(pt_2->resolved_name, "dpttp2::pt_2");

    REQUIRE_EQ(pt.projects_by_name.size(), 2);
    REQUIRE_EQ(pt.targets_by_project.size(), 2);
    REQUIRE_EQ(pt.project_of_target.size(), 2);
    REQUIRE_EQ(pt.targets_by_resolved_name.size(), 2);
    REQUIRE_EQ(pt.intermediate_targets.size(), 0);
    // REQUIRE_EQ(pt.built_targets.size(), 0); // private ...
    REQUIRE_EQ(pt.get_target_resolved_deps().size(), 2);
    // REQUIRE_EQ(pt.unresolved, 2); // private ...

    bool all_resolved{false};
    REQUIRE_NOTHROW(all_resolved = pt.resolve_deps());
    REQUIRE(all_resolved);

    REQUIRE(built_targets.empty());

    SUBCASE("build them separately") {
      // without deps
      REQUIRE_NOTHROW(pt.build_target(pt_1.get()));
      REQUIRE_EQ(built_targets.size(), 1);
      REQUIRE_EQ(*built_targets.begin(), pt_1.get());

      // internal set will contain both ptrs, but this one is cleared in order
      // to have easier checks below
      built_targets.clear();
      // deps on the previous
      REQUIRE_NOTHROW(pt.build_target(pt_2.get()));
      REQUIRE_EQ(built_targets.size(), 1);
      REQUIRE_EQ(*built_targets.begin(), pt_2.get());
    }

    SUBCASE("build them together") {
      // builds even its dependency
      REQUIRE_NOTHROW(pt.build_target(pt_2.get()));
      REQUIRE_EQ(built_targets.size(), 2);
    }
  }

  // TODO write cases for various nontrivial "graphs" of dependencies, etc.
}

} // namespace
} // namespace build_cxx::driver

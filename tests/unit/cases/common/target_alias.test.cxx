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

#include "build_cxx/common/target_alias.hxx"

#include <doctest/doctest.h>

#include "build_cxx/client/core.hxx"
#include "build_cxx/test_helpers/mock_project.hxx"

namespace build_cxx {
namespace {

TEST_CASE("common::target_alias") {
  std::mutex mtx;
  test_helpers::built_targets_t built_targets;
  test_helpers::mock_project test_project{&mtx,    &built_targets, nullptr,
                                          "ctatp", "0.1.0",        __FILE__};

  SUBCASE("basics") {
    auto *const ta{
        test_project.add_target_alias(__FILE__, true, "alias_none", {})};

    REQUIRE(ta->resolved_kind.empty());
    REQUIRE(ta->resolved_name.empty());

    REQUIRE_NOTHROW(ta->resolve_own_traits());

    REQUIRE_EQ(ta->resolved_kind, common::phony_target::kind);
    REQUIRE_EQ(ta->resolved_name,
               std::string{test_project.name} + "::" + std::string{ta->name});
    REQUIRE_EQ(ta->resolved_name,
               common::phony_target::resolve_name(test_project.name, ta->name));

    REQUIRE_THROWS(ta->recipe({})); // not aliasing anything -> throws
    REQUIRE_EQ(built_targets.size(), 0);
  }

  SUBCASE("aliasing other target") {
    auto *const pt{
        test_project.add_mock_phony_target(__FILE__, true, "pt", {})};

    auto *const ta{
        test_project.add_target_alias(__FILE__, true, "alias_pt", {pt->name})};

    REQUIRE_NOTHROW(ta->resolve_own_traits());

    REQUIRE_NOTHROW(ta->recipe({pt})); // aliasing something -> does not throw

    // not enough infrastructure to test the rest here -> see other unit tests
  }
}

} // namespace
} // namespace build_cxx

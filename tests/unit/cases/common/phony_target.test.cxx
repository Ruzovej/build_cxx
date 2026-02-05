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

#include "build_cxx/common/phony_target.hxx"

#include <doctest/doctest.h>

#include "build_cxx/client/core.hxx"
#include "build_cxx/test_helpers/mock_phony_target.hxx"
#include "build_cxx/test_helpers/mock_project.hxx"

namespace build_cxx {
namespace {

TEST_CASE("common::phony_target") {
  test_helpers::built_targets_t built_targets;
  test_helpers::mock_project test_project{&built_targets, nullptr, "cpttp",
                                          "0.1.0", __FILE__};

  auto *const pt{test_project.add_mock_phony_target(__FILE__, true,
                                                    "test_phony_target", {})};

  REQUIRE(pt->resolved_kind.empty());
  REQUIRE(pt->resolved_name.empty());

  REQUIRE_NOTHROW(pt->resolve_own_traits());

  REQUIRE_EQ(pt->resolved_kind, common::phony_target::kind);
  REQUIRE_EQ(pt->resolved_name,
             std::string{test_project.name} + "::" + std::string{pt->name});
  REQUIRE_EQ(pt->resolved_name,
             common::phony_target::resolve_name(test_project.name, pt->name));

  REQUIRE_NOTHROW(pt->recipe({}));
  REQUIRE_EQ(built_targets.size(), 1);
  REQUIRE_EQ(*built_targets.begin(), pt);

  // TODO test that value of `last_modification_time` means "always out of date"
}

} // namespace
} // namespace build_cxx

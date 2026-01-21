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

#include "build_cxx/client/debug_helper.hxx"

#include <doctest/doctest.h>

#include "build_cxx/client/core.hxx"
#include "build_cxx/common/location.hxx"
#include "build_cxx/test_helpers/mock_phony_target.hxx"
#include "build_cxx/test_helpers/mock_project.hxx"

namespace build_cxx {
namespace {

TEST_CASE("client::debug_helper") {
  test_helpers::built_targets_t built_targets;
  test_helpers::mock_project test_project{"cdhtp", "0.1.0", __FILE__};
  test_project.built_targets = &built_targets;

  // testing it on single "isolated" target should be enough:
  auto *const pt{test_project.add_mock_phony_target(__FILE__, true,
                                                    "test_phony_target", {})};

  REQUIRE_NOTHROW(pt->resolve_own_traits());

  std::string res;

  // this is very fragile ... but on the other hand, I don't expect this to be
  // used much, nor shouldn't it change often and/or drastically:

  REQUIRE_NOTHROW(res = client::abstract_target_basic_info(pt, true));
  REQUIRE_EQ(res, pt->name);

  REQUIRE_NOTHROW(res = client::abstract_target_basic_info(pt, false));
  REQUIRE_EQ(res, std::string{pt->name} + " defined in '" +
                      std::string{pt->loc->filename} + ':' +
                      std::to_string(pt->loc->line) + '\'');

  REQUIRE_NOTHROW(res = client::abstract_target_build_info(pt, {}));
  REQUIRE_EQ(res, std::string{pt->name} + " has deps {}");
}

} // namespace
} // namespace build_cxx

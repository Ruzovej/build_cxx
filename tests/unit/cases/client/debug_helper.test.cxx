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

namespace build_cxx {
namespace {

TEST_CASE("client::debug_helper") {
  common::project test_project{"cdhtp", "0.1.0", __FILE__};

  BUILD_CXX_DEFINE_LOCATION(loc, common::location::no_index);
  // testing it on single "isolated" target should be enough:
  BUILD_CXX_DEFINE_DEPS_ARRAY(deps_arr, deps_n);

  // testing it on dummy test_phony_target should be enough
  test_helpers::mock_phony_target pt{&loc, true, "test_phony_target", deps_arr,
                                     deps_n};

  test_project.add_target(&pt);

  REQUIRE_NOTHROW(pt.resolve_own_traits());

  std::string res;

  // this is very fragile ... but on the other hand, I don't expect this to be
  // used much, nor shouldn't it change often and/or drastically:

  REQUIRE_NOTHROW(res = client::abstract_target_basic_info(&pt, true));
  REQUIRE_EQ(res, pt.name);

  REQUIRE_NOTHROW(res = client::abstract_target_basic_info(&pt, false));
  REQUIRE_EQ(res, std::string{pt.name} + " defined in '" +
                      std::string{loc.filename} + ':' +
                      std::to_string(loc.line) + '\'');

  REQUIRE_NOTHROW(res = client::abstract_target_build_info(&pt, {}));
  REQUIRE_EQ(res, std::string{pt.name} + " has deps {}");
}

} // namespace
} // namespace build_cxx
